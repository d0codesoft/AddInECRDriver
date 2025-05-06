#pragma once

#include "interface_connection.h"
#include <jsoncons/json.hpp>
#include "str_utils.h"
#include "common_types.h"
#include "connection_types.h"
#include "logger.h"
#include <queue>

#ifndef ICHANNELPROTOCOL_H
#define ICHANNELPROTOCOL_H

#define WAIT_MILISECONDS_MESSAGE 5000

/// All non-ASCII characters, including Cyrillic characters, 
/// are transmitted as the value of a JSON field in utf-8 encoding.

using Params = std::unordered_map<std::wstring, std::variant<std::wstring, long, double, bool>>;

using DataResult = std::unordered_map<std::wstring, std::variant<std::wstring, long, double, bool>>;

struct DataPayResult {
	std::wstring cardNumber;
	std::wstring receiptNumber;
	std::wstring rrnCode;
	std::wstring authorizationCode;
	std::wstring slip;
	std::wstring merchantId;
};

struct receiveData {
	std::wstring method;
	int step = 0;
	Params params;
	bool error = false;
	std::wstring errorDescription;

	std::wstring toString() const {
		std::wstringstream ss;
		ss << this;
		return ss.str();
	}

	// Overload the << operator for receiveData
	friend std::wostream& operator<<(std::wostream& os, const receiveData& data) {
		os << L"Method: " << data.method << L" Step: " << data.step << L" Error: " << (data.error ? "true" : "false");
		os << L" Error Description: " << data.errorDescription;
		if (data.params.empty()) {
			os << L"Params: none\n";
			return os;
		}
		else {
			os << L"Params: \n";
			for (const auto& [key, value] : data.params) {
				os << L"  " << key << L": ";
				if (std::holds_alternative<std::wstring>(value)) {
					os << std::get<std::wstring>(value);
				}
				else if (std::holds_alternative<long>(value)) {
					os << std::get<long>(value);
				}
				else if (std::holds_alternative<double>(value)) {
					os << std::get<double>(value);
				}
				else if (std::holds_alternative<bool>(value)) {
					os << (std::get<bool>(value) ? L"true" : L"false");
				}
				os << L"\n";
			}
		}
		return os;
	}
};

struct sendData {
	std::wstring method;
	int step = 0;
	Params params;

	// Overload the << operator for sendData
	friend std::wostream& operator<<(std::wostream& os, const sendData& data) {
		os << L"Method: " << data.method;
		os << L" Step: " << data.step;
		if (data.params.empty()) {
			os << L" Params: none\n";
			return os;
		}
		else {
			os << L" Params: \n";
			for (const auto& [key, value] : data.params) {
				os << L"  " << key << L": ";
				if (std::holds_alternative<std::wstring>(value)) {
					os << std::get<std::wstring>(value);
				}
				else if (std::holds_alternative<long>(value)) {
					os << std::get<long>(value);
				}
				else if (std::holds_alternative<double>(value)) {
					os << std::get<double>(value);
				}
				else if (std::holds_alternative<bool>(value)) {
					os << (std::get<bool>(value) ? L"true" : L"false");
				}
				os << L"\n";
			}
		}
		return os;
	}
};

enum class POSTerminalState {
	Idle,
	WaitingResponse,
	Busy,
	Error,
	OperationCompleted,
	OperationFailed,
	MethodNotImplemented
};

// Observer interface for error notifications
class INotifyObserver {
public:
	virtual ~INotifyObserver() = default;
	virtual void onNotify(const std::wstring& errorMessage) = 0;
};

class IChannelProtocol {
public:
	virtual std::vector<uint8_t> encodeRequest(POSTerminalOperationParameters& op) = 0;
	virtual void pushResponse(const std::vector<uint8_t>& rawData) = 0;
	virtual std::optional<receiveData> getResponseData() = 0;
	virtual ~IChannelProtocol() = default;
};

class JsonChannelProtocol : public IChannelProtocol {
public:
	JsonChannelProtocol() {
		dataBuffer_.reserve(1024);
	}
	std::vector<uint8_t> encodeRequest(POSTerminalOperationParameters& op) override;

	void pushResponse(const std::vector<uint8_t>& rawData) override;

	std::optional<receiveData> getResponseData() override;

private:

	void _analyzeData();
	void _handleParsedJson(const jsoncons::json& json);

	std::vector<uint8_t> dataBuffer_;
	std::queue<receiveData> dataQueue_;
	std::mutex queueMutex_;
}

class POSTerminalController {
public:
	explicit POSTerminalController(std::unique_ptr<IConnection> connection, std::unique_ptr<IChannelProtocol> protocol)
		: connection_(std::move(connection)), protocol_(std::move(protocol)) {
	}
	
	~POSTerminalController() override {
		if (connection_ && connection_->isConnected()) {
			disconnect();
		}
	}

	// Implementing connection
	bool connect(const std::string& address, std::optional<uint16_t> port = 2000) override;

	void disconnect() override;

	bool isConnected() const override;

	bool processTransaction(POSTerminalOperationParameters& paramPayement, std::wstring& outError) override;

	std::wstring getLastError() const {
		return lastError_;
	}

	POSTerminalState getState() const {
		return state_;
	}

	bool isBusy() const {
		return state_ == POSTerminalState::Busy || state_ == POSTerminalState::WaitingResponse;
	}

private:
	
	bool _connect(const std::string& address, std::optional<uint16_t> port = 2000);

	void _processIncomingData(const std::vector<uint8_t>& data);

	void _handleParsedJson(const jsoncons::json& json);

	void _clearAllMessages() {
		std::unique_lock<std::mutex> lock(queueMutex);
		while (!dataQueue.empty()) {
			dataQueue.pop();
		}
		lock.unlock();
		cv.notify_all();
	}

	void _handleError(const std::wstring& error) {
		lastError_ = error;
	}

	bool _isValidResponse(const receiveData& recvData) {
		
		if (recvData.error) {
			return false;
		}

		// Check if the method is valid
		// Message methodNotImplemented - a message generated by the terminal in response to a request for an unsupported method.
		auto it = recvData.params.find(L"msgType");
		if (it != recvData.params.end() && std::holds_alternative<std::wstring>(it->second)) {
			const auto& value = std::get<std::wstring>(it->second);
			if (value == L"methodNotImplemented") {
				return false;
			}
		}

		return true;
	}

	void _transitionTo(POSTerminalState newState);
	void _tick(); // обновляет состояние FSM

	std::queue<receiveData> dataQueue = {};
	std::mutex queueMutex = {};
	std::condition_variable cv = {};

	std::atomic<bool> stopListening_{ false };

	std::unique_ptr<IConnection> connection_;
	std::unique_ptr<IChannelProtocol> protocol_;

	std::wstring lastError_ = L"";

	POSTerminalState state_ = POSTerminalState::Idle;
}

class POSTerminalControllerFactory {
public:
	static std::unique_ptr<POSTerminalController> create(POSTerminalProtocol protocol, ConnectionType connectType) {

		std::unique_ptr<IConnection> connection;
		std::unique_ptr<IChannelProtocol> channelProtocol;
		switch (connectType)
		{
		case ConnectionType::TCP:
			connection = std::make_unique<TcpConnection>();
			break;
		case ConnectionType::COM:
			connection = std::make_unique<ComConnection>();
			break;
		case ConnectionType::WebSocket:
			connection = std::make_unique<WebSocketConnection>();
			break;
		case ConnectionType::USB:
			//connection = std::make_unique<UsbConnection>();
			connection = std::make_unique<ComConnection>();
			break;
		default:
			throw std::invalid_argument("Unsupported connection type");
		}

		switch (protocol) {
		case POSTerminalProtocol::JSON:
			channelProtocol = std::make_unique<JsonChannelProtocol>();
			break;
		case POSTerminalProtocol::BaseECR:
			throw std::invalid_argument("Unsupported protocol type");
			//channelProtocol = std::make_unique<JsonChannelProtocol>();
			break;
		default:
			throw std::invalid_argument("Unsupported protocol type");
		}

		if (!connection) {
			throw std::runtime_error("Failed to create connection");
		}
		if (!channelProtocol) {
			throw std::runtime_error("Failed to create channel protocol");
		}
		return std::make_unique<POSTerminalController>(std::move(connection), std::move(channelProtocol));
	}
};

#endif // ICHANNELPROTOCOL_H

