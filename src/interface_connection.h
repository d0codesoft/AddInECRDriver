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
    virtual std::optional<std::vector<uint8_t>> receive(std::optional<uint32_t> timeoutMs = 15000) = 0;

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

    std::optional<std::vector<uint8_t>> receive(std::optional<uint32_t> timeoutMs) override;

    void disconnect() override;

    bool isConnected() const override;

    virtual ConnectionType getType() const override {
        return ConnectionType::TCP;
    }

private:
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::socket socket_;
};

// 🌐 WebSocket
class WebSocketConnection : public IConnection {
public:
    WebSocketConnection()
        : resolver_(ioc_), ws_(ioc_) {
    }

    bool connect(const std::string& host, std::optional<uint16_t> port) override;

    bool send(const std::span<const uint8_t> data) override;

    std::optional<std::vector<uint8_t>> receive(std::optional<uint32_t> timeoutMs) override;

    void disconnect() override;

    bool isConnected() const override;

    virtual ConnectionType getType() const override {
        return ConnectionType::WebSocket;
    }


private:
    boost::asio::io_context ioc_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;
    bool connected_ = false;
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
        {L"WebSocket", ConnectionType::WebSocket}
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
        {u"WebSocket", ConnectionType::WebSocket}
    };

    auto it = typeMap.find(type);
    if (it != typeMap.end()) {
        return it->second;
    }
    return std::nullopt;
}



#endif // INTERFACECONNECTION_H
