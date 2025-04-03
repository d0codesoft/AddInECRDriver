#pragma once

#include <string>
#include <optional>
#include <span>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>

#ifndef INTERFACECONNECTION_H
#define INTERFACECONNECTION_H

enum class ConnectionType {
    COM,
    TCP,
    WebSocket,
    USB,
	Unknown
};

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

	virtual ConnectionType getType() const {
		return ConnectionType::Unknown;
	}

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
class TcpConnection : public IConnection {
public:
    TcpConnection() : socket_(io_context_) {}

    bool connect(const std::string& host, std::optional<uint16_t> port) override;

    bool send(const std::span<const uint8_t> data) override;

    std::optional<std::vector<uint8_t>> receive() override;

    void disconnect() override;

    bool isConnected() const override;

    ConnectionType getType() const override {
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

	boost::asio::io_context io_context_; ///< Boost.Asio I/O context
	boost::asio::ip::tcp::socket socket_; ///< Boost.Asio TCP socket

	std::atomic<bool> readingThread_{ false }; ///< On thread reading data
	std::atomic<bool> keepAlive_{ true }; ///< Flag to keep the connection alive
	std::atomic<bool> stopListening_{ false }; ///< Flag to stop listening
	std::chrono::milliseconds reconnectDelay_{ 5000 }; ///< Delay before reconnecting
	std::thread listeningThread_; ///< Thread for listening to incoming data

	std::string host_; ///< Host to connect to
	std::optional<uint16_t> port_; ///< Port to connect to

	std::vector<IErrorObserver*> observers_; ///< Observers for error notifications
};

// 🌐 WebSocket
class WebSocketConnection : public IConnection {
public:
    WebSocketConnection()
        : resolver_(ioc_), ws_(ioc_) {
    }

    bool connect(const std::string& host, std::optional<uint16_t> port) override;

    bool send(const std::span<const uint8_t> data) override;

    std::optional<std::vector<uint8_t>> receive() override;

    void disconnect() override;

    bool isConnected() const override;

    virtual ConnectionType getType() const override {
        return ConnectionType::WebSocket;
    }

	void enableKeepAlive(bool enable) override {
		keepAlive_ = enable;
	}

	void setReconnectDelay(std::chrono::milliseconds delay) override {
		reconnectDelay_ = delay;
	}

    void startListening(std::function<void(std::vector<uint8_t>)> callback) override {
		// Not implemented
		throw std::runtime_error("Not implemented");
    }

private:
    boost::asio::io_context ioc_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;
    bool connected_ = false;
	std::atomic<bool> keepAlive_{ true };
	std::chrono::milliseconds reconnectDelay_{ 5000 };
};

class ComConnection : public IConnection {
public:
	ComConnection(const std::string& port, uint32_t baud_rate = 9600);
    bool connect(const std::string& port, std::optional<uint16_t> baudRate) override {
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

	ConnectionType getType() const override {
		return ConnectionType::COM;
	}

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
};

class ConnectionFactory {
public:
    static std::unique_ptr<IConnection> create(ConnectionType type) {
        switch (type) {
        case ConnectionType::TCP:
            return std::make_unique<TcpConnection>();
        case ConnectionType::WebSocket:
            return std::make_unique<WebSocketConnection>();
        default:
            throw std::invalid_argument("Unsupported connection type");
        }
    }
};

// Utility function to convert wstring to ConnectionType
inline std::optional<ConnectionType> wstringToConnectionType(const std::wstring& type) {
    static const std::unordered_map<std::wstring, ConnectionType> typeMap = {
        {L"COM", ConnectionType::COM},
        {L"TCP", ConnectionType::TCP},
        {L"WebSocket", ConnectionType::WebSocket},
		{ L"0", ConnectionType::COM },
		{L"1", ConnectionType::TCP},
		{L"2", ConnectionType::WebSocket}
    };

    auto it = typeMap.find(type);
    if (it != typeMap.end()) {
        return it->second;
    }
    return std::nullopt;
}

// Utility function to convert wstring to ConnectionType
inline std::optional<ConnectionType> u16stringToConnectionType(const std::u16string& type) {
    static const std::unordered_map<std::u16string, ConnectionType> typeMap = {
        {u"COM", ConnectionType::COM},
        {u"TCP", ConnectionType::TCP},
        {u"WebSocket", ConnectionType::WebSocket},
		{u"0", ConnectionType::COM },
		{u"1", ConnectionType::TCP},
		{u"2", ConnectionType::WebSocket}
    };

    auto it = typeMap.find(type);
    if (it != typeMap.end()) {
        return it->second;
    }
    return std::nullopt;
}



#endif // INTERFACECONNECTION_H
