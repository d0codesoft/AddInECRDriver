#include "pch.h"

#include "interface_connection.h"
#include "string_conversion.h"
#include "logger.h"
#include "str_utils.h"

bool TcpConnection::connect(const std::string& host, std::optional<uint16_t> port)
{
    try {
		this->host_ = host;
		this->port_ = port;

        boost::asio::ip::tcp::resolver resolver(io_context_);
		auto port_ = port.value_or(2000);
        boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port_));

        std::promise<void> connect_promise;
        auto connect_future = connect_promise.get_future();

        boost::asio::async_connect(socket_, endpoints,
            [this, &connect_promise](boost::system::error_code ec, boost::asio::ip::tcp::endpoint) {
                if (ec) {
                    auto text_error = L"Failed to connect: " + convertStringToWString(ec.message());
                    notifyError(text_error);
                    LOG_ERROR_ADD(L"TcpConnection", text_error);
                }
                connect_promise.set_value();
            });

        io_context_.run();
        connect_future.wait();
        return socket_.is_open();
    }
    catch (const std::exception& e) {
        auto error = std::string(e.what());
        auto text_error = L"Exception: " + convertStringToWString(error);
        notifyError(text_error);
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
		auto text_error = L"Exception: " + convertStringToWString(error);
		notifyError(text_error);
		LOG_ERROR_ADD(L"TcpConnection", text_error);
		return false;
	}
	return true;
}

std::optional<std::vector<uint8_t>> TcpConnection::receive() {
    try {
        std::vector<uint8_t> buffer(1024); // Adjust the buffer size as needed
        boost::system::error_code ec;
        size_t len = socket_.read_some(boost::asio::buffer(buffer), ec);

        if (ec) {
            auto text_error = L"Receive failed: " + str_utils::to_wstring(ec.message());
            notifyError(text_error);
            LOG_ERROR_ADD(L"TcpConnection", text_error);
            return std::nullopt;
        }

        buffer.resize(len); // Resize buffer to the actual data length
        return buffer;
    }
    catch (const std::exception& e) {
        auto error = std::string(e.what());
        auto text_error = L"Exception: " + str_utils::to_wstring(error);
        notifyError(text_error);
        LOG_ERROR_ADD(L"TcpConnection", text_error);
        return std::nullopt;
    }
}

void TcpConnection::disconnect()
{
	// Stop the listening thread
	keepAlive_ = false;
	if (listeningThread_.joinable()) {
		listeningThread_.join();
	}

	boost::system::error_code ec;
	socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	socket_.close(ec);
	if (ec) {
		notifyError(L"Failed to disconnect: " + std::wstring(ec.message().begin(), ec.message().end()));
	}
}

bool TcpConnection::isConnected() const
{
    return socket_.is_open();
}

void TcpConnection::startListening(std::function<void(std::vector<uint8_t>)> callback)
{
	listeningThread_ = std::thread([this, callback]() {
		while (keepAlive_) {
			if (!isConnected()) {
				std::this_thread::sleep_for(reconnectDelay_);
				connect(host_, port_);
			}
			else {
				auto data = receive();
				if (data) {
					callback(data.value());
				}
			}
		}
		});
	listeningThread_.detach();
}

bool WebSocketConnection::connect(const std::string& host, std::optional<uint16_t> port) {
    try {
        auto resolved = resolver_.resolve(host, port ? std::to_string(*port) : "80");
        boost::asio::connect(ws_.next_layer(), resolved);

        ws_.handshake(host, "/");
        connected_ = true;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "WebSocket connection error: " << e.what() << std::endl;
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
        std::cerr << "WebSocket send error: " << e.what() << std::endl;
        return false;
    }
}

std::optional<std::vector<uint8_t>> WebSocketConnection::receive(std::optional<uint32_t> timeoutMs) {
    if (!connected_) return std::nullopt;
    try {
        std::string data;
        auto buf = boost::asio::dynamic_buffer(data);
        ws_.read(buf);
		std::vector<uint8_t> result(data.begin(), data.end());
        return result;
    }
    catch (const std::exception& e) {
        std::cerr << "WebSocket receive error: " << e.what() << std::endl;
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