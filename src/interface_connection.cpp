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

        // --- resolve with explicit error handling ---
        boost::system::error_code ec;
        auto results = resolver_.resolve(host, port ? std::to_string(*port) : "2000", ec);
        if (ec) {
            LOG_ERROR_ADD(L"TcpConnection", L"DNS resolve failed: " + str_utils::to_wstring(ec.message()));
            return false;
        }

        // --- iterative connect with per-endpoint diagnostics ---
        boost::system::error_code ecConnect;
        for (auto const& ep : results) {
            socket_.connect(ep.endpoint(), ecConnect);
            if (!ecConnect) {
                LOG_INFO_ADD(L"TcpConnection", L"Connected to endpoint: " +
                    str_utils::to_wstring(ep.endpoint().address().to_string() + ":" + std::to_string(ep.endpoint().port())));
                break;
            }
            LOG_ERROR_ADD(L"TcpConnection",
                L"Endpoint connection failed: " + str_utils::to_wstring(ecConnect.message()));
        }
        if (ecConnect) {
            LOG_ERROR_ADD(L"TcpConnection", L"All endpoints failed, aborting connection");
            return false;
        }

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
    readingThread_ = false;
	keepAlive_ = false;
	if (listeningThread_.joinable()) {
		listeningThread_.join();
	}

    if (socket_.is_open())
    {
        boost::system::error_code ec;
        socket_.cancel(ec);
        if (ec) {
            notifyError(L"Failed to cancel operations: " + str_utils::to_wstring(ec.message()));
            LOG_ERROR_ADD(L"TcpConnection", L"Failed to cancel operations: " + str_utils::to_wstring(ec.message()));
		}
		ec = {};
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        if (ec) {
            notifyError(L"Failed to shutdown socket: " + str_utils::to_wstring(ec.message()));
            LOG_ERROR_ADD(L"TcpConnection", L"Failed to shutdown socket: " + str_utils::to_wstring(ec.message()));
		}
		ec = {};
        socket_.close(ec);
        if (ec) {
            notifyError(L"Failed to disconnect: " + str_utils::to_wstring(ec.message()));
            LOG_ERROR_ADD(L"TcpConnection", L"Failed to disconnect: " + str_utils::to_wstring(ec.message()));
        }
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
        keepAlive_ = true;
        host_ = host;
        port_ = port;

        LOG_INFO_ADD(L"WebSocketConnection", L"Start connection : " + str_utils::to_wstring(host) + L" port: " + portToWstring(port));

        ioc_.restart();
        startIoThreadIfNeeded_(); // starting the background thread io_context

        // --- resolve with explicit error handling ---
        boost::system::error_code ec;
        auto results = resolver_.resolve(host, port ? std::to_string(*port) : "2000", ec);
        if (ec) {
            LOG_ERROR_ADD(L"WebSocketConnection", L"DNS resolve failed: " + str_utils::to_wstring(ec.message()));
            return false;
        }
        
        // --- iterative connect with per-endpoint diagnostics ---
        boost::asio::ip::tcp::socket& rawSocket = ws_.next_layer();
        boost::system::error_code ecConnect;
        for (auto const& ep : results) {
            rawSocket.connect(ep.endpoint(), ecConnect);
            if (!ecConnect) {
                LOG_INFO_ADD(L"WebSocketConnection", L"Connected to endpoint: " +
                    str_utils::to_wstring(ep.endpoint().address().to_string() + ":" + std::to_string(ep.endpoint().port())));
                break;
            }
            LOG_ERROR_ADD(L"WebSocketConnection",
                L"Endpoint connection failed: " + str_utils::to_wstring(ecConnect.message()));
        }
        if (ecConnect) {
            LOG_ERROR_ADD(L"WebSocketConnection", L"All endpoints failed, aborting connection");
            return false;
        }
        
        // Set a decorator to change the User-Agent of the handshake
        ws_.set_option(boost::beast::websocket::stream_base::decorator(
            [](boost::beast::websocket::request_type& req)
            {
                req.set(boost::beast::http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                    "POSTerminal");
            }));

        std::string host_header = host;
		if (port) {
			host_header.append(":").append(std::to_string(*port));
		}

        LOG_INFO_ADD(L"WebSocketConnection",
            L"Opening WS: " + str_utils::to_wstring(host_header));

        // --- handshake with error handling ---
        boost::system::error_code ecHandshake;
        ws_.handshake(host_header, "/", ecHandshake);
        if (ecHandshake) {
            LOG_ERROR_ADD(L"WebSocketConnection", L"Handshake failed: " + str_utils::to_wstring(ecHandshake.message()));
            return false;
        }

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

    readingThread_ = false;
    keepAlive_ = false;
    if (listeningThread_.joinable()) {
        listeningThread_.join();
    }

    if (connected_) {
        try {
            ws_.close(boost::beast::websocket::close_code::normal);
        }
        catch (...) {
        }
        connected_ = false;
    }

    stopIoThread_();
}

bool WebSocketConnection::isConnected() const {
    return ws_.is_open();
}

void WebSocketConnection::startListening(std::function<void(std::vector<uint8_t>)> callback)
{
    auto self = shared_from_this();
    ws_.async_read_some(
        boost::asio::buffer(read_buf_),
        [self, callback](const boost::system::error_code& ec, std::size_t n) {
            if (!ec) {
                if (n > 0) {
                    callback(std::vector<uint8_t>(self->read_buf_.data(), self->read_buf_.data() + n));
                }
                LOG_INFO_ADD(L"WebSocketConnection", L"Received bytes: " + str_utils::to_wstring(n));
                // Continue reading
                self->startListening(callback);
            }
            else if (ec == boost::asio::error::eof) {
                // peer closed connection — can notify/log if needed
                LOG_INFO_ADD(L"WebSocketConnection", L"Connection closed by peer");
            }
            else if (ec == boost::asio::error::operation_aborted) {
                // cancelled by stop()/close()/cancel()
                LOG_INFO_ADD(L"WebSocketConnection", L"Async read aborted");
            }
            else {
                // other errors
                auto text_error = L"read error: " + str_utils::to_wstring(ec.message());
                LOG_ERROR_ADD(L"WebSocketConnection", text_error);
            }
        }
    );
}

void WebSocketConnection::startIoThreadIfNeeded_()
{
    if (!ioThread_.joinable()) {
        workGuard_.emplace(ioc_.get_executor());
        ioThread_ = std::thread([this] {
            try {
                ioc_.run();
            }
            catch (...) {
                // swallow exceptions to avoid terminating the process
            }
            });
    }
}

void WebSocketConnection::stopIoThread_()
{
    workGuard_.reset();
    ioc_.stop();

    if (ioThread_.joinable()) {
        try {
            ioThread_.join();
        }
        catch (...) {
            // Suppress any exceptions on join
        }
    }
    ioc_.restart();
}

ComConnection::ComConnection()
    : serial_(io_context_)
{
    port_ = "COM1";
    baud_rate_ = 9600;
}

bool ComConnection::connect(const std::string& host, std::optional<uint16_t> port)
{
    try {
        keepAlive_ = true;
        port_ = host; // for serial, 'host' is a device path (e.g., "COM3" or "/dev/ttyUSB0")
        if (port) {
            baud_rate_ = *port; // treat optional 'port' as baud rate if provided
        }

        // Start IO thread for async operations (listening)
        io_context_.restart();
        startIoThreadIfNeeded_();

        boost::system::error_code ec;
        serial_.open(port_, ec);
        if (ec) {
            notifyError(L"COM open failed: " + str_utils::to_wstring(ec.message()));
            LOG_ERROR_ADD(L"ComConnection", L"Open failed: " + str_utils::to_wstring(ec.message()));
            return false;
        }

        // Configure serial options
        serial_.set_option(boost::asio::serial_port_base::baud_rate(baud_rate_));
        serial_.set_option(boost::asio::serial_port_base::character_size(8));
        serial_.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        serial_.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
        serial_.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));

        LOG_INFO_ADD(L"ComConnection", L"Opened " + str_utils::to_wstring(port_) + L" @ " + str_utils::to_wstring(std::to_string(baud_rate_)));
        return serial_.is_open();
    }
    catch (const std::exception& e) {
        notifyError(L"COM connect exception: " + str_utils::to_wstring(e.what()));
        LOG_ERROR_ADD(L"ComConnection", L"Exception: " + str_utils::to_wstring(e.what()));
        return false;
    }
}

void ComConnection::disconnect()
{
    try {
        readingThread_ = false;
        keepAlive_ = false;

        boost::system::error_code ec;
        if (serial_.is_open()) {
            serial_.cancel(ec);
            serial_.close(ec);
            if (ec) {
                notifyError(L"Failed to close COM: " + str_utils::to_wstring(ec.message()));
                LOG_ERROR_ADD(L"ComConnection", L"Close failed: " + str_utils::to_wstring(ec.message()));
            }
        }

        stopIoThread_();
    }
    catch (...) {
        // suppress
    }
}

bool ComConnection::isConnected() const
{
    return serial_.is_open();
}

bool ComConnection::send(const std::span<const uint8_t> data)
{
    try {
        if (!serial_.is_open()) {
            notifyError(L"COM not open");
            return false;
        }
        boost::asio::write(serial_, boost::asio::buffer(data.data(), data.size()));
        return true;
    }
    catch (const std::exception& e) {
        notifyError(L"COM write exception: " + str_utils::to_wstring(e.what()));
        LOG_ERROR_ADD(L"ComConnection", L"Write exception: " + str_utils::to_wstring(e.what()));
        return false;
    }
}

std::optional<std::vector<uint8_t>> ComConnection::receive()
{
    try {
        if (!serial_.is_open()) {
            return std::nullopt;
        }
        std::array<uint8_t, 1024> buf{};
        boost::system::error_code ec;
        std::size_t n = serial_.read_some(boost::asio::buffer(buf), ec);
        if (ec) {
            if (ec != boost::asio::error::operation_aborted) {
                notifyError(L"COM read failed: " + str_utils::to_wstring(ec.message()));
                LOG_ERROR_ADD(L"ComConnection", L"Read failed: " + str_utils::to_wstring(ec.message()));
            }
            return std::nullopt;
        }
        return std::vector<uint8_t>(buf.data(), buf.data() + n);
    }
    catch (const std::exception& e) {
        notifyError(L"COM receive exception: " + str_utils::to_wstring(e.what()));
        LOG_ERROR_ADD(L"ComConnection", L"Receive exception: " + str_utils::to_wstring(e.what()));
        return std::nullopt;
    }
}

void ComConnection::startListening(std::function<void(std::vector<uint8_t>)> callback)
{
    auto self = shared_from_this();
    serial_.async_read_some(
        boost::asio::buffer(read_buf_),
        [self, callback](const boost::system::error_code& ec, std::size_t n) {
            if (!ec) {
                if (n > 0) {
                    callback(std::vector<uint8_t>(
                        reinterpret_cast<uint8_t*>(self->read_buf_.data()),
                        reinterpret_cast<uint8_t*>(self->read_buf_.data()) + n));
                }
                LOG_INFO_ADD(L"ComConnection", L"Received bytes: " + str_utils::to_wstring(n));
                self->startListening(callback); // continue
            }
            else if (ec == boost::asio::error::operation_aborted) {
                LOG_INFO_ADD(L"ComConnection", L"Async read aborted");
            }
            else {
                self->notifyError(L"COM async read error: " + str_utils::to_wstring(ec.message()));
                LOG_ERROR_ADD(L"ComConnection", L"Async read error: " + str_utils::to_wstring(ec.message()));
            }
        }
    );
}

void ComConnection::startIoThreadIfNeeded_()
{
    if (!ioThread_.joinable()) {
        workGuard_.emplace(io_context_.get_executor());
        ioThread_ = std::thread([this] {
            try {
                io_context_.run();
            }
            catch (...) {}
            });
    }
}

void ComConnection::stopIoThread_()
{
    workGuard_.reset();
    io_context_.stop();
    if (ioThread_.joinable()) {
        try { ioThread_.join(); }
        catch (...) {}
    }
    io_context_.restart();
}