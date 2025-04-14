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

struct PaymentParameters {
	double amount;                              // The payment amount
	std::wstring merchantId;                   // Merchant ID
	std::optional<std::wstring> cardNumber;    // Optional card number
	std::optional<std::wstring> receiptNumber; // Optional receipt number
	double discount = 0;                       // Discount amount (default: 0)
	std::optional<std::wstring> subMerchant;   // Sub-merchant ID (default: empty string)
	bool facepay = false;                      // Facepay flag (default: false)

	// Constructor for easy initialization
	PaymentParameters(
		double amount,
		const std::wstring& merchantId,
		const std::wstring* cardNumber = nullptr,
		const std::wstring* receiptNumber = nullptr,
		double discount = 0,
		const std::wstring* subMerchant = nullptr,
		bool facepay = false
	)
		: amount(amount),
		merchantId(merchantId),
		cardNumber(cardNumber ? std::optional<std::wstring>(*cardNumber) : std::nullopt),
		receiptNumber(receiptNumber ? std::optional<std::wstring>(*receiptNumber) : std::nullopt),
		discount(discount),
		subMerchant(subMerchant ? std::optional<std::wstring>(*subMerchant) : std::nullopt),
		facepay(facepay) {
	}

	bool isValid() const {
		// Validate amount (must be greater than 0)
		if (amount <= 0) {
			return false;
		}

		// Validate merchantId (must not be empty)
		if (merchantId.empty()) {
			return false;
		}

		// Validate discount (must not be negative)
		if (discount < 0) {
			return false;
		}
		// All validations passed
		return true;
	}
};

struct receiveData {
	std::wstring method;
	int step;
	Params params;
	bool error;
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
	int step;
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

/// @brief Interface for sending and receiving JSON data over a connection.
class IChannelProtocol {
public:
	explicit IChannelProtocol(std::unique_ptr<IConnection> connection)
		: connection_(std::move(connection)) {
	}
	virtual ~IChannelProtocol() = default;

	virtual const IConnection* getConnection() const {
		return connection_.get();
	}

	/// @brief Sends JSON data over the connection.
	/// All non-ASCII characters, including Cyrillic characters, 
	/// are transmitted as the value of a JSON field in utf-8 encoding.
	/// @param jsonData JSON object to send.
	/// @return True if the data was sent successfully.
	virtual bool sendJson(const sendData& jsonData) = 0;

	/// @brief Receives JSON data from the connection.
	/// @param timeoutMs Timeout in milliseconds.
	/// @return Parsed JSON object if data is received, otherwise std::nullopt.
	virtual std::optional<receiveData> receiveJson(std::optional<uint32_t> timeoutMs = 15000) = 0;

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


	virtual bool Pay(const PaymentParameters paramPayement, DataPayResult& outDataResult, std::wstring& outError) = 0;

protected:
	std::unique_ptr<IConnection> connection_;
};

class JsonChannel : public IChannelProtocol {
public:
	explicit JsonChannel(std::unique_ptr<IConnection> connection)
		: IChannelProtocol(std::move(connection)) {
	}

	// Implementing connection
	bool connect(const std::string& address, std::optional<uint16_t> port = 2000) override;

	void disconnect() override;

	bool isConnected() const override;

	bool sendJson(const sendData& jsonData) override;

	std::optional<receiveData> receiveJson(std::optional<uint32_t> timeoutMs = 15000) override;

	bool Pay(const PaymentParameters paramPayement, DataPayResult& outDataResult, std::wstring& outError) override;

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

	void _handleError(const std::string& error) {
		
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

	std::queue<receiveData> dataQueue = {};
	std::mutex queueMutex = {};
	std::condition_variable cv = {};

	std::atomic<bool> stopListening_{ false };
};

class BaseECRChannel : public IChannelProtocol {
public:
	explicit BaseECRChannel(std::unique_ptr<IConnection> connection)
		: IChannelProtocol(std::move(connection)) {
	}

	bool connect(const std::string& address, std::optional<uint16_t> port = 2000) override {
		return connection_->connect(address, port);
	}

	void disconnect() override {
		connection_->disconnect();
	}

	bool isConnected() const override {
		return connection_->isConnected();
	}

	bool sendJson(const sendData& jsonData) override {
		// Implement the BaseECR specific send logic here
		return false;
	}

	std::optional<receiveData> receiveJson(std::optional<uint32_t> timeoutMs = 15000) override {
		// Implement the BaseECR specific receive logic here
		return std::nullopt;
	}

	bool Pay(const PaymentParameters paramPayement, DataPayResult& outDataResult, std::wstring& outError) override {
		// Implement the BaseECR specific payment logic here
		return false;
	}
};

class ChannelProtocolFactory {
public:
	static std::unique_ptr<IChannelProtocol> create(ProtocolTerminal protocol, std::unique_ptr<IConnection> connection) {
		switch (protocol) {
		case ProtocolTerminal::JSON:
			return std::make_unique<JsonChannel>(std::move(connection));
		case ProtocolTerminal::BaseECR:
			return std::make_unique<BaseECRChannel>(std::move(connection));
		default:
			throw std::invalid_argument("Unsupported protocol type");
		}
	}
};

#endif // ICHANNELPROTOCOL_H

