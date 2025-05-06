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

bool POSTerminalController::connect(const std::string& address, std::optional<uint16_t> port /*= 2000*/)
{
	bool result = _connect(address, port);

	return result;
}

void POSTerminalController::disconnect()
{
	stopListening_ = true;

	LOG_INFO_ADD(L"POSTerminalController", L"Disconnect");
	connection_->disconnect();
}

bool POSTerminalController::isConnected() const
{
	return connection_->isConnected();
}

bool POSTerminalController::processTransaction(POSTerminalOperationParameters& paramPayement, std::wstring& outError)
{
	if (isValidPOSTerminalOperationParameters(paramPayement, POSTerminalOperationType::Pay)) {
		outError = L"Invalid parameters for payment";
		LOG_ERROR_ADD(L"POSTerminalController", L"Invalid parameters for payment");
		return false;
	}
	std::wstring _discount = L"";
	if (paramPayement.Discount.has_value()) {
		_discount = doubleToAmountString(paramPayement.Discount.value());
	}

	Params _paramPayement = {
		{ L"amount" , doubleToAmountString(paramPayement.Amount.value()) },
		{ L"discount" , _discount },
		{ L"merchantId" , L"0" },
		{ L"facepay" , paramPayement.isFacepay() ? L"true" : L"false"},
		{ L"subMerchant" , paramPayement.SubMerchant.has_value() ? std::to_wstring(paramPayement.SubMerchant.value()) : L""}
	};
	sendData dataPayement = { L"Purchase", 0, _paramPayement };
	
	//_clearAllMessages();

	auto sendData = protocol_->encodeRequest(paramPayement);
	if (!connection_->send(sendData)) {
		outError = L"Failed to send payment data";
		LOG_ERROR_ADD(L"POSTerminalController", L"Failed to send payment data");
		return false;
	}

	while (!stopListening_) {
		auto response = protocol_->getResponseData();
		if (response.has_value()) {
			auto recvData = response.value();
			if (_isValidResponse(recvData)) {
				// Process the response
				if (recvData.method == L"ServiceMessage") {
					handleServiceMessage(recvData.params);
				}
				else if (recvData.method == L"PayResult") {
					handlePayResult(recvData.params);
				}
				else if (recvData.method == L"Error") {
					outError = recvData.errorDescription;
					LOG_ERROR_ADD(L"POSTerminalController", L"Error: " + outError);
					return false;
				}
			}
			else {
				outError = L"Invalid response";
				LOG_ERROR_ADD(L"POSTerminalController", L"Invalid response");
				return false;
			}
		}
	}

	return true;
}

bool POSTerminalController::_connect(const std::string& address, std::optional<uint16_t> port)
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

void POSTerminalController::_processIncomingData(const std::vector<uint8_t>& data)
{
#ifdef _DEBUG
	std::wcout << L"	_processIncomingData size:" << data.size() << std::endl;
#endif

	// Append the new data to the buffer
	protocol_->pushResponse(data);
}

void POSTerminalController::_transitionTo(POSTerminalState newState)
{
	if (state_ != newState) {
		state_ = newState;
	}
}

void POSTerminalController::_tick()
{
	using namespace std::chrono;
	auto now = steady_clock::now();
	if (duration_cast<milliseconds>(now - m_lastTick).count() < 200) return;

	m_lastTick = now;

	switch (m_state) {
	case POSTerminalState::SendingCommand:
		transitionTo(POSTerminalState::WaitingForResponse);
		break;

	case POSTerminalState::WaitingForResponse: {
		auto response = m_channel->receive(); // возврат JSON или структуры
		if (!response.has_value()) break;

		const auto& msg = response.value();

		if (msg.method == L"ServiceMessage") {
			handleServiceMessage(msg.paramsJson);
		}
		else if (msg.error) {
			transitionTo(POSTerminalState::Error);
		}
		else {
			transitionTo(POSTerminalState::Done);
		}
		break;
	}

	case POSTerminalState::Busy:
	case POSTerminalState::Processing:
		updateStatus();
		break;

	default:
		break;
	}
}

std::vector<uint8_t> JsonChannelProtocol::encodeRequest(POSTerminalOperationParameters& op)
{
	jsoncons::json json;
	json["method"] = ""; // str_utils::to_string(op.OperationType);
	json["step"] = "0";

	// Вложенный объект "params"
	jsoncons::json params_json;

	/*for (const auto& [key, value] : jsonData.params) {
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
	}*/

	// Вставляем "params" как объект в основной JSON
	json["params"] = params_json;

	std::string jsonString = json.to_string();
	LOG_INFO_ADD(L"POSTerminalController", L"Send json data: " + str_utils::to_wstring(jsonString));

	// Convert JSON string to UTF-8
	std::vector<uint8_t> data;
	//if (jsonData.method == L"PingDevice") {
		//data.push_back(0x00); // Add 0x00 at the beginning for PingDevice
	//}
	data.insert(data.end(), jsonString.begin(), jsonString.end());
	data.push_back(0x00); // Add 0x00 at the end

	// Send data over the connection
	return data;
}

void JsonChannelProtocol::pushResponse(const std::vector<uint8_t>& rawData)
{
	// Append the new data to the buffer
	dataBuffer_.insert(dataBuffer_.end(), rawData.begin(), rawData.end());
	_analyzeData();
}

std::optional<receiveData> JsonChannelProtocol::getResponseData()
{
	// Wait for the data to arrive
	std::unique_lock<std::mutex> lock(queueMutex_);
	if (dataQueue_.empty()) {
		return std::nullopt;
	}
	auto value = dataQueue_.front();
	dataQueue_.pop();
	return value;
}

void JsonChannelProtocol::_analyzeData()
{
	auto pos = std::find(dataBuffer_.begin(), dataBuffer_.end(), 0x00);
	while (pos != dataBuffer_.end()) {
		std::vector<uint8_t> datagram(dataBuffer_.begin(), pos);
		dataBuffer_.erase(dataBuffer_.begin(), pos + 1); // Remove the processed datagram

		// Process the datagram
		std::string jsonString(datagram.begin(), datagram.end());
		try {
			auto json = jsoncons::json::parse(jsonString);
			// Handle the parsed JSON data
			_handleParsedJson(json);
		}
		catch (const std::exception& e) {
			// Handle parsing error
			LOG_ERROR_ADD(L"JsonChannelProtocol", L"Failed to parse JSON: " + str_utils::to_wstring(e.what()));
		}

		pos = std::find(dataBuffer_.begin(), dataBuffer_.end(), 0x00);
	}
}

void JsonChannelProtocol::_handleParsedJson(const jsoncons::json& json)
{
	receiveData data;

	try {
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
	}
	catch (const std::exception& e) {
#ifdef _DEBUG
		std::wcout << L"	_handleParsedJson Failed to parse JSON " << str_utils::to_wstring(e.what()) << std::endl;
#endif
		LOG_ERROR_ADD(L"POSTerminalController", L"Failed to parse JSON: " + str_utils::to_wstring(e.what()));
		return;
	}

#ifdef _DEBUG
	std::wcout << L"	_handleParsedJson lock data Queue for addin element" << std::endl;
#endif
	{
		std::lock_guard<std::mutex> lock(queueMutex_);
		dataQueue_.push(data);
	}
	LOG_INFO_ADD(L"POSTerminalController", L"Retrieve packet from terminal " + data.toString());
#ifdef _DEBUG
	std::wcout << L"	Retrieve packet from terminal " << data << std::endl;
#endif
	//cv.notify_all();
}
