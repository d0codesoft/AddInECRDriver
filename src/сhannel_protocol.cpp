#include "pch.h"
#include "сhannel_protocol.h"
#include "str_utils.h"

enum class JsonErrorCode {
	GeneralError = 1000,
	TransactionCanceledByUser = 1001,
	EMVDecline = 1002,
	TransactionLogFull = 1003,
	NoConnectionWithHost = 1004,
	NoPaperInPrinter = 1005,
	ErrorCryptoKeys = 1006,
	CardReaderNotConnected = 1007,
	TransactionAlreadyComplete = 1008
};

bool JsonChannel::connect(const std::string& address, std::optional<uint16_t> port /*= 2000*/)
{
#ifdef _DEBUG
	std::wcout << L"Start connection on " << str_utils::to_wstring(address) << std::endl;
#endif
	// Step 1: Connect to IP, port
	if (!_connect(address, port)) {
		LOG_ERROR_ADD(L"JsonChannel", L"Error connected, address: " + str_utils::to_wstring(address) + L" port: " + portToWstring(port));
		return false;
	}

#ifdef _DEBUG
	std::wcout << L"	Step 2: Send a handshake " << std::endl;
#endif
	// Step 2: Send a handshake
	sendData handshake = { L"PingDevice", 0, {} };
	if (!sendJson(handshake)) {
		LOG_ERROR_ADD(L"JsonChannel", L"Error Send a handshake");
		return false;
	}

	auto response = receiveJson();
	if (response.has_value() && (response->method != L"PingDevice" || response->error == true)) {
		std::wstring errorResponseCode = {}, errorCode = {};
		auto fp = response->params.find(L"responseCode");
		if (fp != response->params.end()) {
			errorResponseCode = std::get<std::wstring>(fp->second);
		}
		fp = response->params.find(L"code");
		if (fp != response->params.end()) {
			errorCode = std::get<std::wstring>(fp->second);
		}
		LOG_ERROR_ADD(L"JsonChannel", L"Failed to connect: " + response->errorDescription + L" Code: " + errorResponseCode);
#ifdef _DEBUG
		std::wcout << L"	RESULT: " << response.value() << std::endl;
#endif
	}

#ifdef _DEBUG
	std::wcout << L"	Disconnect " << std::endl;
#endif
	// Step 3: Disconnect
	disconnect();

#ifdef _DEBUG
	std::wcout << L"	Reconnect " << std::endl;
#endif
	// Step 4: Send identification
	if (!_connect(address, port)) {
		LOG_ERROR_ADD(L"JsonChannel", L"Error reconnect address: " + str_utils::to_wstring(address) + L" port: " + portToWstring(port));
		return false;
	}

#ifdef _DEBUG
	std::wcout << L"	Step 4: Send identification " << std::endl;
#endif
	sendData identify = { L"ServiceMessage", 0, { {L"msgType", L"identify"} } };
	if (!sendJson(identify)) {
		LOG_ERROR_ADD(L"JsonChannel", L"Failed to send service message");
		return false;
	}


	response = receiveJson();
	if (!response || response->method != L"ServiceMessage") {
		LOG_ERROR_ADD(L"JsonChannel", L"Failed to receive service message");
		return false;
	}
#ifdef _DEBUG
	std::wcout << L"	RESULT: " << response.value() << std::endl;
#endif

	// Step 5: Disconnect
	disconnect();

	// Step 6: Open the connection, keep it open (keepalive) and listen to incoming data
	if (!_connect(address, port)) {
		LOG_ERROR_ADD(L"JsonChannel", L"Failed to reconnect pos terminal");
		return false;
	}

	LOG_INFO_ADD(L"JsonChannel", L"Star listening data from device");

	return true;
}

void JsonChannel::disconnect()
{
	stopListening_ = true;

	LOG_INFO_ADD(L"JsonChannel", L"Disconnect");
	connection_->disconnect();
}

bool JsonChannel::isConnected() const
{
	return connection_->isConnected();
}

bool JsonChannel::sendJson(const sendData& jsonData)
{
	jsoncons::json json;
	json["method"] = str_utils::to_string(jsonData.method);
	json["step"] = jsonData.step;

	// Вложенный объект "params"
	jsoncons::json params_json;

	for (const auto& [key, value] : jsonData.params) {
		auto str_key = str_utils::to_string(key);
		if (std::holds_alternative<std::wstring>(value)) {
			params_json[str_key] = str_utils::to_string(std::get<std::wstring>(value));
		}
		else if (std::holds_alternative<long>(value)) {
			params_json[str_key] = std::get<long>(value);
		}
		else if (std::holds_alternative<bool>(value)) {
			params_json[str_key] = std::get<bool>(value);
		}
	}

	// Вставляем "params" как объект в основной JSON
	json["params"] = params_json;

	std::string jsonString = json.to_string();
	LOG_INFO_ADD(L"JsonChannel", L"Send json data: " + str_utils::to_wstring(jsonString));

	// Convert JSON string to UTF-8
	std::vector<uint8_t> data;
	if (jsonData.method == L"PingDevice") {
		//data.push_back(0x00); // Add 0x00 at the beginning for PingDevice
	}
	data.insert(data.end(), jsonString.begin(), jsonString.end());
	data.push_back(0x00); // Add 0x00 at the end

	// Send data over the connection
	return connection_->send(std::span<const uint8_t>(data.data(), data.size()));
}

std::optional<receiveData> JsonChannel::receiveJson(std::optional<uint32_t> timeoutMs /*= 15000*/)
{
	// Wait for the data to arrive
	std::unique_lock<std::mutex> lock(queueMutex);

	// First we check if there is already data in the queue
	if (!dataQueue.empty()) {
		auto value = dataQueue.front();
		dataQueue.pop();
		lock.unlock();  // Unlock mutex before processing
		return value;
	}

	if (cv.wait_for(lock, std::chrono::milliseconds(timeoutMs.value_or(15000)), [this] { return !this->dataQueue.empty() || this->stopListening_; })) {
		if (stopListening_ && dataQueue.empty()) {
			return std::nullopt;
		}

		auto value = dataQueue.front();
		dataQueue.pop();
		lock.unlock();
		return value;
	}
	else {
		LOG_INFO_ADD(L"JsonChannel", L"Cancel receive json data by timeout");
		return std::nullopt;
	}
}

bool JsonChannel::Pay(const PaymentParameters paramPayement, DataPayResult& outDataResult, std::wstring& outError)
{
	if (!paramPayement.isValid()) {
		outError = L"Invalid parameters for payment";
		LOG_ERROR_ADD(L"JsonChannel", L"Invalid parameters for payment");
		return false;
	}
	std::wstring _discount = L"";
	if (paramPayement.discount > 0) {
		_discount = doubleToAmountString(paramPayement.discount);
	}

	Params _paramPayement = {
		{ L"amount" , doubleToAmountString(paramPayement.amount) },
		{ L"discount" , _discount },
		{ L"merchantId" , L"0" },
		{ L"facepay" , paramPayement.facepay ? L"true" : L"false" },
		{ L"subMerchant" , paramPayement.subMerchant ? paramPayement.subMerchant.value() : L"" }
	};
	sendData dataPayement = { L"Purchase", 0, _paramPayement };
	
	_clearAllMessages();

	if (!sendJson(dataPayement)) {
		outError = L"Failed to send payment data";
		LOG_ERROR_ADD(L"JsonChannel", L"Failed to send payment data");
		return false;
	}

	auto response = receiveJson();
	if (!response || response->method != L"Purchase") {
		outError = L"Failed to send payment data";
		LOG_ERROR_ADD(L"JsonChannel", L"Failed to receive payment response");
		return false;
	}
	
	for (auto& item : response->params) {
		if (item.first == L"cardNumber") {
			auto _ws = std::get_if<std::wstring>(&item.second);
			if (_ws) {
				outDataResult.cardNumber = *_ws;
			}
		}
		else if (item.first == L"receiptNumber") {
			auto _ws = std::get_if<std::wstring>(&item.second);
			if (_ws) {
				outDataResult.receiptNumber = *_ws;
			}
		}
		else if (item.first == L"rrnCode") {
			auto _ws = std::get_if<std::wstring>(&item.second);
			if (_ws) {
				outDataResult.rrnCode = *_ws;
			}
		}
		else if (item.first == L"authorizationCode") {
			auto _ws = std::get_if<std::wstring>(&item.second);
			if (_ws) {
				outDataResult.authorizationCode = *_ws;
			}
		}
		else if (item.first == L"slip") {
			auto _ws = std::get_if<std::wstring>(&item.second);
			if (_ws) {
				outDataResult.slip = *_ws;
			}
		}
		else if (item.first == L"merchantId") {
			auto _ws = std::get_if<std::wstring>(&item.second);
			if (_ws) {
				outDataResult.merchantId = *_ws;
			}
		}
	}

	if (response->error) {
		outError = response->errorDescription;
		LOG_ERROR_ADD(L"JsonChannel", L"Error in payment response: " + response->errorDescription);
		return false;
	}
	
	return true;
}

bool JsonChannel::_connect(const std::string& address, std::optional<uint16_t> port)
{
	if (connection_->isConnected()) {
		return false;
	}

	connection_->enableKeepAlive(true);

	stopListening_ = false;
	if (!connection_->connect(address, port)) {
		return false;
	}

	connection_->startListening([this](std::vector<uint8_t> data) {
		this->_processIncomingData(data);
		});

	return connection_->isConnected();
}

void JsonChannel::_processIncomingData(const std::vector<uint8_t>& data)
{
	static std::vector<uint8_t> buffer;
	for (auto byte : data) {
		if (byte == 0x00) {
			// Process the complete datagram
			std::string jsonString(buffer.begin(), buffer.end());
			try {
				auto json = jsoncons::json::parse(jsonString);
				// Handle the parsed JSON data
				_handleParsedJson(json);
			}
			catch (const std::exception& e) {
				// Handle parsing error
				LOG_ERROR_ADD(L"JsonChannel", L"Failed to parse JSON: " + str_utils::to_wstring(e.what()));
				_handleError(e.what());
			}
			buffer.clear();
		}
		else {
			buffer.push_back(byte);
		}
	}
}

void JsonChannel::_handleParsedJson(const jsoncons::json& json)
{
	receiveData data;
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
	{
		std::lock_guard<std::mutex> lock(this->queueMutex);
		this->dataQueue.push(data);
	}
	LOG_INFO_ADD(L"JsonChannel", L"Retrieve packet from terminal " + data.toString());
#ifdef _DEBUG
	std::wcout << L"	Retrieve packet from terminal " << data << std::endl;
#endif
	cv.notify_all();
}

void BaseECRChannel::disconnect()
{
	connection_->disconnect();
}

bool BaseECRChannel::isConnected() const
{
	return connection_->isConnected();
}
