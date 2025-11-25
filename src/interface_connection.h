#pragma once

#include <string>
#include <optional>
#include <span>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/beast/ssl.hpp>
#include "connection_types.h"

#ifndef INTERFACECONNECTION_H
#define INTERFACECONNECTION_H

// Observer interface for error notifications
class IErrorObserver {
public:
    virtual ~IErrorObserver() = default;
    virtual void onError(const std::wstring& errorMessage) = 0;
};

class IConnection {
public:
    virtual ~IConnection() = default;

    /// @brief Connecting to a resource.
    /// @param address Connection address (COM port, IP address or URL for WebSocket).
    /// @param port Port for TCP/IP connection (optional, default 2000 ).
    /// @return true, if the connection is successful, otherwise false.
    virtual bool connect(const std::string& address, std::optional<uint16_t> port = 2000) = 0;

    /// @brief Disconnecting from the resource.
    virtual void disconnect() = 0;

    /// @brief Checking the connection status.
    /// @return true, if the connection is active.
    virtual bool isConnected() const = 0;

    /// @brief Sending binary data.
    /// @param data Binary data to send.
    /// @return true, if the shipment is successful.
    virtual bool send(const std::span<const uint8_t> data) = 0;

    /// @brief Obtaining a binary answer.
    /// @param timeoutMs Timeout in milliseconds.
    /// @return Data vector if a response is received, otherwise std::nullopt.
    virtual std::optional<std::vector<uint8_t>> receive() = 0;

    /// @brief Adding an observer to receive error notifications.
    /// @param observer Pointer to the observer.
    void addObserver(IErrorObserver* observer) {
        observers_.push_back(observer);
    }

    /// @brief Observer Removal.
    /// @param observer Pointer to the observer.
    void removeObserver(IErrorObserver* observer) {
        observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
    }

	virtual ConnectionType getType() = 0;

	/// <summary>
	/// включает/выключает режим поддержания соединения.
	/// </summary>
	/// <param name="enable"></param>
	virtual void enableKeepAlive(bool enable) = 0;

	/// <summary>
	/// задаёт задержку в милисекундах перед попыткой переподключения.
	/// </summary>
	/// <param name="delay"></param>
	virtual void setReconnectDelay(std::chrono::milliseconds delay) = 0;

	/// <summary>
	/// запускает поток, который читает данные и передает их в обратном вызове.
	/// </summary>
	/// <param name="callback"></param>
	virtual void startListening(std::function<void(std::vector<uint8_t>)> callback) = 0;

protected:
    /// @brief Notifying all observers of the error.
    /// @param errorMessage Error message.
    void notifyError(const std::wstring& errorMessage) {
        for (auto observer : observers_) {
            observer->onError(errorMessage);
        }
    }

private:
    std::vector<IErrorObserver*> observers_;
};

// 🔄 COM connection
class TcpConnection : 
    public IConnection, 
    public std::enable_shared_from_this<TcpConnection> 
{
public:
    TcpConnection() : 
        io_context_(),
        socket_(io_context_)
    {
    }

    bool connect(const std::string& host, std::optional<uint16_t> port) override;

    bool send(const std::span<const uint8_t> data) override;

    void disconnect() override;

    bool isConnected() const override;

	std::optional<std::vector<uint8_t>> receive() override;

    ConnectionType getType() override {
        return ConnectionType::TCP;
    }

    void startListening(std::function<void(std::vector<uint8_t>)> callback) override;

	/// <summary>
	/// Enable or disable the keep-alive feature
	/// </summary>
	/// <param name="enable"></param>
	void enableKeepAlive(bool enable);

	/// <summary>
	/// Set the delay before reconnecting
	/// </summary>
	/// <param name="delay"></param>
	void setReconnectDelay(std::chrono::milliseconds delay);

private:
    using WorkGuard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    using Strand = boost::asio::strand<boost::asio::io_context::executor_type>;

    void startIoThreadIfNeeded_();
    void stopIoThread_();

    boost::asio::io_context io_context_;
    Strand strand_{ boost::asio::make_strand(io_context_) };
	boost::asio::ip::tcp::socket socket_; ///< Boost.Asio TCP socket

	std::array<char, 4096> read_buf_{}; ///< Buffer for reading data

	std::atomic<bool> readingThread_{ false }; ///< On thread reading data
	std::atomic<bool> keepAlive_{ true }; ///< Flag to keep the connection alive
	std::atomic<bool> stopListening_{ false }; ///< Flag to stop listening
	std::chrono::milliseconds reconnectDelay_{ 5000 }; ///< Delay before reconnecting
    std::thread ioThread_;
    std::optional<WorkGuard> workGuard_;
	std::thread listeningThread_; ///< Thread for listening to incoming data

	std::string host_; ///< Host to connect to
	std::optional<uint16_t> port_; ///< Port to connect to

	std::vector<IErrorObserver*> observers_; ///< Observers for error notifications
};

// 🌐 WebSocket
class WebSocketConnection : public IConnection, public std::enable_shared_from_this<WebSocketConnection> {
public:
    WebSocketConnection()
        : ioc_()
        , strand_(boost::asio::make_strand(ioc_))
        , ssl_ctx_(boost::asio::ssl::context::tlsv13_client)
        , resolver_(ioc_)
        , ws_(strand_, ssl_ctx_)       
    {
        ssl_ctx_.set_default_verify_paths();
        ssl_ctx_.set_verify_mode(boost::asio::ssl::verify_peer);
    }


    bool connect(const std::string& host, std::optional<uint16_t> port) override;

    bool send(const std::span<const uint8_t> data) override;

    std::optional<std::vector<uint8_t>> receive() override;

    void disconnect() override;

    bool isConnected() const override;

    ConnectionType getType() override {
        return ConnectionType::WebSocket;
    }

	void enableKeepAlive(bool enable) override {
		keepAlive_ = enable;
	}

	void setReconnectDelay(std::chrono::milliseconds delay) override {
		reconnectDelay_ = delay;
	}

    void startListening(std::function<void(std::vector<uint8_t>)> callback) override {
        (void)callback; // silence C4100 until implemented
		throw std::runtime_error("Not implemented");
    }

private:
    boost::asio::io_context ioc_;
    boost::asio::ssl::context ssl_ctx_{ boost::asio::ssl::context::tlsv13_client };
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    boost::asio::ip::tcp::resolver resolver_; // For DNS resolution
    boost::beast::websocket::stream<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> ws_;

    bool connected_ = false;
	std::atomic<bool> keepAlive_{ true };
	std::chrono::milliseconds reconnectDelay_{ 5000 };
};

class ComConnection : public IConnection, public std::enable_shared_from_this<ComConnection> {
public:
    ComConnection();
    bool connect(const std::string& host, std::optional<uint16_t> port) override {
		return false;
    }
    void disconnect() override {
		// Not implemented
		throw std::runtime_error("Not implemented");
    }
    bool isConnected() const override {
		return false;
    }
    bool send(const std::span<const uint8_t> data) override {
		throw std::runtime_error("Not implemented");
    }
	std::optional<std::vector<uint8_t>> receive() override {
		throw std::runtime_error("Not implemented");
	}

	ConnectionType getType() override {
		return ConnectionType::COM;
	}

    void startListening(std::function<void(std::vector<uint8_t>)> callback) override;

	void enableKeepAlive(bool enable) override {
		keepAlive_ = enable;
	}

	void setReconnectDelay(std::chrono::milliseconds delay) override {
		reconnectDelay_ = delay;
	}

private:
	boost::asio::io_context io_context_;
	boost::asio::serial_port serial_;
	std::atomic<bool> keepAlive_{ true };
	std::chrono::milliseconds reconnectDelay_{ 5000 };
    std::string port;
	uint32_t baud_rate_;
};

class ConnectionFactory {
public:
    static std::shared_ptr<IConnection> create(ConnectionType type) {
        switch (type) {
        case ConnectionType::TCP:
            return std::make_shared<TcpConnection>();
        case ConnectionType::WebSocket:
            return std::make_shared<WebSocketConnection>();
        default:
            throw std::invalid_argument("Unsupported connection type");
        }
    }
};

#endif // INTERFACECONNECTION_H
