#include "pch.h"

#include "interface_connection.h"
#include "string_conversion.h"
#include "logger.h"

bool TcpConnection::connect(const std::string& host, std::optional<uint16_t> port)
{
    try {
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

std::optional<std::vector<uint8_t>> TcpConnection::receive(std::optional<uint32_t> timeoutMs) {
    try {
        std::vector<uint8_t> buffer(1024); // Adjust the buffer size as needed
        boost::system::error_code ec;
        size_t len = socket_.read_some(boost::asio::buffer(buffer), ec);

        if (ec) {
            auto text_error = L"Receive failed: " + convertStringToWString(ec.message());
            notifyError(text_error);
            LOG_ERROR_ADD(L"TcpConnection", text_error);
            return {};
        }

        buffer.resize(len); // Resize buffer to the actual data length
        return buffer;
    }
    catch (const std::exception& e) {
        auto error = std::string(e.what());
        auto text_error = L"Exception: " + convertStringToWString(error);
        notifyError(text_error);
        LOG_ERROR_ADD(L"TcpConnection", text_error);
        return {};
    }
}

bool WebSocketConnection::connect(const std::string& host, std::optional<uint16_t> port)
{
	return false;
}

bool WebSocketConnection::send(const std::span<const uint8_t> data)
{
    return false;
}

std::optional<std::vector<uint8_t>> WebSocketConnection::receive(std::optional<uint32_t> timeoutMs)
{
	return std::vector<uint8_t>();
}

void WebSocketConnection::disconnect()
{
}
