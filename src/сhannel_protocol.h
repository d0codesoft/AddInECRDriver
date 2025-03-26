#pragma once

#include "interface_connection.h"
#include <jsoncons/json.hpp>
#include "str_utils.h"
#include "common_types.h"
#include <queue>

#ifndef ICHANNELPROTOCOL_H
#define ICHANNELPROTOCOL_H

/// All non-ASCII characters, including Cyrillic characters, 
/// are transmitted as the value of a JSON field in utf-8 encoding.



using Params = std::unordered_map<std::wstring, std::variant<std::wstring, int, bool>>;

struct jsonResiveData {
	std::wstring method;
	int step;
	Params params;
	bool error;
	std::wstring errorDescription;
};

struct jsonSendData {
	std::wstring method;
	int step;
	Params params;
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
	virtual bool sendJson(const jsonSendData& jsonData) = 0;

	/// @brief Receives JSON data from the connection.
	/// @param timeoutMs Timeout in milliseconds.
	/// @return Parsed JSON object if data is received, otherwise std::nullopt.
	virtual std::optional<jsonResiveData> receiveJson(std::optional<uint32_t> timeoutMs = 15000) = 0;

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

protected:
	std::unique_ptr<IConnection> connection_;
};

class JsonChannel : public IChannelProtocol {
public:
	explicit JsonChannel(std::unique_ptr<IConnection> connection)
		: IChannelProtocol(std::move(connection)) {
	}

	// Implementing connection
	bool connect(const std::string& address, std::optional<uint16_t> port = 2000) override {
		// Step 1: Connect to IP, port
		if (!connection_->connect(address, port)) {
			return false;
		}

		// Step 2: Send a handshake
		jsonSendData handshake = { L"PingDevice", 0, {} };
		if (!sendJson(handshake)) {
			return false;
		}

		auto response = receiveJson();
		if (!response || response->method != L"PingDevice" || response->params[L"code"] != L"00") {
			return false;
		}

		// Step 3: Disconnect
		connection_->disconnect();

		// Step 4: Send identification
		if (!connection_->connect(address, port)) {
			return false;
		}

		jsonSendData identify = { L"ServiceMessage", 0, {{L"msgType", L"identify"}} };
		if (!sendJson(identify)) {
			return false;
		}

		response = receiveJson();
		if (!response || response->method != L"ServiceMessage" || response->params[L"msgType"] != L"identify") {
			return false;
		}

		// Step 5: Disconnect
		connection_->disconnect();

		// Step 6: Open the connection, keep it open (keepalive) and listen to incoming data
		if (!connection_->connect(address, port)) {
			return false;
		}

		connection_->enableKeepAlive(true);
		connection_->startListening([this](std::vector<uint8_t> data) {
			// Handle incoming data
			processIncomingData(data);
			});

		return true;
	}

	void disconnect() override {
		connection_->disconnect();
	}

	bool isConnected() const override {
		return connection_->isConnected();
	}

	bool sendJson(const jsonSendData& jsonData) override {
		
		jsoncons::json json;
		json["method"] = str_utils::to_string(jsonData.method);
		json["step"] = jsonData.step;

		for (const auto& [key, value] : jsonData.params) {
			auto str_key = str_utils::to_string(key);
			if (std::holds_alternative<std::wstring>(value)) {
				json[jsoncons::json::string_view_type(str_key)] = str_utils::to_string(std::get<std::wstring>(value));
			}
			else if (std::holds_alternative<int>(value)) {
				json[jsoncons::json::string_view_type(str_key)] = std::get<int>(value);
			}
			else if (std::holds_alternative<bool>(value)) {
				json[jsoncons::json::string_view_type(str_key)] = std::get<bool>(value);
			}
		}

		std::string jsonString = json.to_string();

		// Convert JSON string to UTF-8
		std::vector<uint8_t> data;
		if (jsonData.method == L"PingDevice") {
			data.push_back(0x00); // Add 0x00 at the beginning for PingDevice
		}
		data.insert(data.end(), jsonString.begin(), jsonString.end());
		data.push_back(0x00); // Add 0x00 at the end

		// Send data over the connection
		return connection_->send(std::span<const uint8_t>(data.data(), data.size()));
	}

	std::optional<jsonResiveData> receiveJson(std::optional<uint32_t> timeoutMs = 15000) override {
		std::vector<uint8_t> buffer;
		while (true) {
			auto responseData = connection_->receive(timeoutMs);
			if (!responseData) {
				return std::nullopt;
			}

			buffer.insert(buffer.end(), responseData->begin(), responseData->end());

			// Check if the last byte is 0x00, indicating the end of the packet
			if (!buffer.empty() && buffer.back() == 0x00) {
				buffer.pop_back(); // Remove the terminating 0x00
				break;
			}
		}

		std::string jsonString(buffer.begin(), buffer.end());
		try {
			auto json = jsoncons::json::parse(jsonString);

			jsonResiveData result;
			result.method = str_utils::to_wstring(json["method"].as<std::string>());
			result.step = json["step"].as<int>();
			result.error = json["error"].as<bool>();
			result.errorDescription = str_utils::to_wstring(json["errorDescription"].as<std::string>());

			for (const auto& item : json["params"].object_range()) {
				std::wstring key = str_utils::to_wstring(item.key());
				if (item.value().is_string()) {
					result.params[key] = str_utils::to_wstring(item.value().as<std::string>());
				}
				else if (item.value().is_int64()) {
					result.params[key] = item.value().as<int>();
				}
				else if (item.value().is_bool()) {
					result.params[key] = item.value().as<bool>();
				}
			}

			return result;
		}
		catch (const std::exception& e) {
			return std::nullopt;
		}
	}

private:
	
	class ThreadSafeQueue {
	public:
		void push(const jsonResiveData& data) {
			std::lock_guard<std::mutex> lock(mutex_);
			queue_.push(data);
			cond_var_.notify_one();
		}

		std::optional<jsonResiveData> pop() {
			std::unique_lock<std::mutex> lock(mutex_);
			cond_var_.wait(lock, [this] { return !queue_.empty(); });
			if (queue_.empty()) {
				return std::nullopt;
			}
			jsonResiveData data = queue_.front();
			queue_.pop();
			return data;
		}

	private:
		std::queue<jsonResiveData> queue_;
		std::mutex mutex_;
		std::condition_variable cond_var_;
	};

	void processIncomingData(const std::vector<uint8_t>& data) {
		static std::vector<uint8_t> buffer;
		for (auto byte : data) {
			if (byte == 0x00) {
				// Process the complete datagram
				std::string jsonString(buffer.begin(), buffer.end());
				try {
					auto json = jsoncons::json::parse(jsonString);
					// Handle the parsed JSON data
					handleParsedJson(json);
				}
				catch (const std::exception& e) {
					// Handle parsing error
					handleError(e.what());
				}
				buffer.clear();
			}
			else {
				buffer.push_back(byte);
			}
		}
	}

	void handleParsedJson(const jsoncons::json& json) {
		jsonResiveData data;
		data.method = str_utils::to_wstring(json["method"].as<std::string>());
		data.step = json["step"].as<int>();
		data.error = json["error"].as<bool>();
		data.errorDescription = str_utils::to_wstring(json["errorDescription"].as<std::string>());
		for (const auto& item : json["params"].object_range()) {
			std::wstring key = str_utils::to_wstring(item.key());
			if (item.value().is_string()) {
				data.params[key] = str_utils::to_wstring(item.value().as<std::string>());
			}
			else if (item.value().is_int64()) {
				data.params[key] = item.value().as<int>();
			}
			else if (item.value().is_bool()) {
				data.params[key] = item.value().as<bool>();
			}
		}
		jsonQueue_.push(data);
	}

	void handleError(const std::string& error) {
		
	}

	ThreadSafeQueue jsonQueue_;
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

	bool sendJson(const jsonSendData& jsonData) override {
		// Implement the BaseECR specific send logic here
		return false;
	}

	std::optional<jsonResiveData> receiveJson(std::optional<uint32_t> timeoutMs = 15000) override {
		// Implement the BaseECR specific receive logic here
		return std::nullopt;
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

