#include "pch.h"
#include "interface_connection.h"
#include "string_conversion.h"
#include "logger.h"
#include "str_utils.h"

bool TcpConnection::connect(const std::string& host, std::optional<uint16_t> port)
{
    try {
		keepAlive_ = true;
		host_ = host;
		port_ = port;

		LOG_INFO_ADD(L"TcpConnection", L"Start connection : " + str_utils::to_wstring(host) + L" port: " + portToWstring(port));

        io_context_.restart();
        startIoThreadIfNeeded_(); // запуск фонового потока io_context

        boost::asio::ip::tcp::resolver resolver(io_context_);
		auto port_ = port.value_or(2000);
        boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port_));

        boost::asio::connect(socket_, endpoints);

        return socket_.is_open();
    }
    catch (const std::exception& e) {
        auto error = std::string(e.what());
        auto text_error = L"Exception: " + str_utils::to_wstring(error);
        //notifyError(text_error);
		LOG_ERROR_ADD(L"TcpConnection", text_error);
        return false;
    }
}

bool TcpConnection::send(const std::span<const uint8_t> data)
{
	try {
		boost::asio::write(socket_, boost::asio::buffer(data));
	}
	catch (const std::exception& e) {
		auto error = std::string(e.what());
		auto text_error = L"Exception: " + str_utils::to_wstring(error);
		notifyError(text_error);
		LOG_ERROR_ADD(L"TcpConnection", text_error);
		return false;
	}
	return true;
}

void TcpConnection::disconnect()
{
	// Canceled all operations in threading context
    // Stop the listening thread
    readingThread_ = false;
	keepAlive_ = false;
	if (listeningThread_.joinable()) {
		listeningThread_.join();
	}

    socket_.cancel();
    
    boost::system::error_code ec;
	socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	socket_.close(ec);
	if (ec) {
		notifyError(L"Failed to disconnect: " + str_utils::to_wstring(ec.message()));
		LOG_ERROR_ADD(L"TcpConnection", L"Failed to disconnect: " + str_utils::to_wstring(ec.message()));
	}

    stopIoThread_();
}

bool TcpConnection::isConnected() const
{
    return socket_.is_open();
}

std::optional<std::vector<uint8_t>> TcpConnection::receive()
{
    std::vector<uint8_t> buf(1024);
    size_t len = socket_.read_some(boost::asio::buffer(buf));

    return std::optional<std::vector<uint8_t>>(buf);
}

void TcpConnection::startListening(std::function<void(std::vector<uint8_t>)> callback)
{
    auto self = shared_from_this();
    socket_.async_read_some(
        boost::asio::buffer(read_buf_),
        [self, callback](const boost::system::error_code& ec, std::size_t n) {
            if (!ec) {
                if (n > 0) {
                    callback(std::vector<uint8_t>(self->read_buf_.data(), self->read_buf_.data() + n));
                }
                LOG_INFO_ADD(L"TcpConnection", L"Received bytes: " + str_utils::to_wstring(n));
                // Continue reading
                self->startListening(callback);
            }
            else if (ec == boost::asio::error::eof) {
                // peer closed connection — can notify/log if needed
                LOG_INFO_ADD(L"TcpConnection", L"Connection closed by peer");
            }
            else if (ec == boost::asio::error::operation_aborted) {
                // cancelled by stop()/close()/cancel()
                LOG_INFO_ADD(L"TcpConnection", L"Async read aborted");
            }
            else {
                // other errors
                auto text_error = L"read error: " + str_utils::to_wstring(ec.message());
                LOG_ERROR_ADD(L"TcpConnection", text_error);
            }
        }
    );
}

void TcpConnection::enableKeepAlive(bool enable)
{
	keepAlive_ = enable;
}

void TcpConnection::setReconnectDelay(std::chrono::milliseconds delay)
{
	reconnectDelay_ = delay;
}

void TcpConnection::startIoThreadIfNeeded_()
{
    if (!ioThread_.joinable()) {
        workGuard_.emplace(io_context_.get_executor());     // держит io_context живым
        ioThread_ = std::thread([this] {
            try {
                io_context_.run();
            }
            catch (...) {}
            });
    }
}

void TcpConnection::stopIoThread_()
{
    workGuard_.reset();             // разрешаем run() завершиться
    io_context_.stop();
    if (ioThread_.joinable()) {
        ioThread_.join();
    }
    io_context_.restart();
}

/// -----------------------------
/// WebSocketConnection implementation
/// -----------------------------

bool WebSocketConnection::connect(const std::string& host, std::optional<uint16_t> port) {
    try {
        auto const results = resolver_.resolve(host, port ? std::to_string(*port) : "2000");
        boost::asio::connect(ws_.next_layer().next_layer(), results);

        ws_.handshake(host, "/");
        connected_ = true;
        return true;
    }
    catch (const std::exception& e) {
		LOG_ERROR_ADD(L"WebSocketConnection", L"Failed to connect: " + str_utils::to_wstring(e.what()));
        return false;
    }
}

bool WebSocketConnection::send(const std::span<const uint8_t> data) {
    if (!connected_) return false;
    try {
        ws_.binary(true);
        ws_.write(boost::asio::buffer(data.data(), data.size()));
        return true;
    }
    catch (const std::exception& e) {
		LOG_ERROR_ADD(L"WebSocketConnection", L"Failed to send: " + str_utils::to_wstring(e.what()));
        return false;
    }
}

std::optional<std::vector<uint8_t>> WebSocketConnection::receive() {
    if (!connected_) return std::nullopt;
    try {
        std::string data;
        auto buf = boost::asio::dynamic_buffer(data);
        ws_.read(buf);
		std::vector<uint8_t> result(data.begin(), data.end());
        return result;
    }
    catch (const std::exception& e) {
		LOG_ERROR_ADD(L"WebSocketConnection", L"Failed to receive: " + str_utils::to_wstring(e.what()));
        return std::nullopt;
    }
}

void WebSocketConnection::disconnect() {
    if (connected_) {
        try {
            ws_.close(boost::beast::websocket::close_code::normal);
        }
        catch (...) {
        }
        connected_ = false;
    }
}

bool WebSocketConnection::isConnected() const {
    return connected_;
}

ComConnection::ComConnection()
    : serial_(io_context_)
{
    this->port = "COM1";
	this->baud_rate_ = 9600;
}

void ComConnection::startListening(std::function<void(std::vector<uint8_t>)> callback)
{
    std::thread([this, callback]() {
		while (true) {
			auto data = this->receive();
			if (data) {
				callback(data.value());
			}
		}
	}).detach();
}
