#pragma once

#include "interface_connection.h"
#include <jsoncons/json.hpp>
#include "str_utils.h"
#include "common_types.h"
#include "connection_types.h"
#include "logger.h"
#include <queue>
#include "protocol_consts.h"

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

	// Get a parameter as string regardless of underlying stored type.
	// Returns std::nullopt if the key is not present or type is unsupported.
	// Inside struct receiveData
	std::optional<std::wstring> GetParams(const std::wstring& name) const {
		// Helper to turn variant into string
		auto toStr = [](const std::variant<std::wstring, long, double, bool>& v) -> std::optional<std::wstring> {
			if (std::holds_alternative<std::wstring>(v)) return std::get<std::wstring>(v);
			if (std::holds_alternative<long>(v))       return std::to_wstring(std::get<long>(v));
			if (std::holds_alternative<double>(v))     return std::to_wstring(std::get<double>(v));
			if (std::holds_alternative<bool>(v))       return std::get<bool>(v) ? L"true" : L"false";
			return std::nullopt;
			};

		// 1) Fast exact match (case-sensitive)
		if (auto it = params.find(name); it != params.end()) {
			return toStr(it->second);
		}

		// 2) Fallback: case-insensitive search
		for (const auto& [k, v] : params) {
			if (str_utils::iequals(k, name)) {
				return toStr(v);
			}
		}

		return std::nullopt;
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

	std::unique_ptr<POSTerminalOperationResponse> to_OperationResponse() const;

	//// Move constructor
	//receiveData(receiveData&& other) noexcept
	//	: method(std::move(other.method)),
	//	step(other.step),
	//	params(std::move(other.params)),
	//	error(other.error),
	//	errorDescription(std::move(other.errorDescription)) {
	//}

	//// Move assignment operator
	//receiveData& operator=(receiveData&& other) noexcept {
	//	if (this != &other) {
	//		method = std::move(other.method);
	//		step = other.step;
	//		params = std::move(other.params);
	//		error = other.error;
	//		errorDescription = std::move(other.errorDescription);
	//	}
	//	return *this;
	//}
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
	virtual std::vector<uint8_t> encodeRequest(sendData& op) = 0;
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
	std::vector<uint8_t> encodeRequest(sendData& op) override;

	void pushResponse(const std::vector<uint8_t>& rawData) override;

	std::optional<receiveData> getResponseData() override;

private:

	void _analyzeData();
	void _handleParsedJson(const jsoncons::json& json);

	std::vector<uint8_t> dataBuffer_;
	std::deque<receiveData> dataQueue_;
	std::mutex queueMutex_;
};

class POSTerminalController {
public:
	explicit POSTerminalController(std::shared_ptr<IConnection> connection, std::shared_ptr<IChannelProtocol> protocol)
		: connection_(std::move(connection)), protocol_(std::move(protocol)) {
	}

	~POSTerminalController() {
		if (connection_ && connection_->isConnected()) {
			disconnect();
		}
	}

	// Implementing connection
	bool connect(const std::string& address, std::optional<uint16_t> port = 2000);

	void disconnect();

	bool isConnected() const;

	std::unique_ptr<POSTerminalOperationResponse> processTransaction(POSTerminalOperationParameters& paramPayement, std::wstring& outError);

	std::wstring getLastError() const {
		return lastError_;
	}

	POSTerminalState getState() const {
		return m_state;
	}

	bool isBusy() const {
		return m_state == POSTerminalState::Busy || m_state == POSTerminalState::WaitingResponse;
	}

	std::wstring getTerminalId() const {
		return terminalId_;
	}
	std::wstring getTerminalModel() const {
		return terminalModel_;
	}
	std::wstring getTerminalVendor() const {
		return terminalVendor_;
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
	void _handleServiceMessage(const Params& params); // update state FSM
	bool _sendInterrupt();

	int _getLastResult();
	int _getLastStatMsgCode();

	bool _msgIsBussy(const receiveData& recvData) {
		auto it = recvData.params.find(PROTOCOL_MESSAGE_TYPE);
		if (it != recvData.params.end() && std::holds_alternative<std::wstring>(it->second)) {
			const auto& value = std::get<std::wstring>(it->second);
			if (str_utils::iequals(value, PROTOCOL_MESSAGE_TYPE_DEVICEBUSY)) {
				return true;
			}
		}
		return false;
	}

	bool _msgIsServiceMessage(const receiveData& recvData) {
		bool result = str_utils::iequals(recvData.method, PROTOCOL_METHOD_SERVICEMESSAGE);
		return result;
	}

	bool _handshakeTerminal();
	bool _identifyTerminal();

	std::queue<receiveData> dataQueue = {};
	std::mutex queueMutex = {};
	std::condition_variable cv = {};

	std::atomic<bool> stopListening_{ false };

	std::shared_ptr<IConnection> connection_;
	std::shared_ptr<IChannelProtocol> protocol_;

	std::wstring lastError_ = L"";
	std::wstring terminalId_ = L"";
	std::wstring terminalModel_ = L"";
	std::wstring terminalVendor_ = L"";

	std::chrono::steady_clock::time_point m_lastTick = std::chrono::steady_clock::now();

	POSTerminalState m_state = POSTerminalState::Idle;
};

// Factory reset data and move to Idle state
class POSTerminalControllerFactory {
public:
	static std::unique_ptr<POSTerminalController> create(POSTerminalProtocol protocol, ConnectionType connectType) {

		std::shared_ptr<IConnection> connection;
		switch (connectType)
		{
		case ConnectionType::TCP:
			connection = std::make_shared<TcpConnection>();
			break;
		case ConnectionType::COM:
			connection = std::make_shared<ComConnection>();
			break;
		case ConnectionType::WebSocket:
			connection = std::make_shared<WebSocketConnection>();
			break;
		case ConnectionType::USB:
			//connection = std::make_unique<UsbConnection>();
			connection = std::make_shared<ComConnection>();
			break;
		default:
			throw std::invalid_argument("Unsupported connection type");
		}

		std::shared_ptr<IChannelProtocol> channelProtocol;
		switch (protocol) {
		case POSTerminalProtocol::JSON:
			channelProtocol = std::make_shared<JsonChannelProtocol>();
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