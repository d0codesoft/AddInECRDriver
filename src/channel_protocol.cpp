#include "pch.h"
#include "channel_protocol.h"
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

std::unique_ptr<POSTerminalOperationResponse> POSTerminalController::processTransaction(POSTerminalOperationParameters& paramPayement, std::wstring& outError)
{
	if (!isValidPOSTerminalOperationParameters(paramPayement, POSTerminalOperationType::Pay)) {
		outError = L"Invalid parameters for payment";
		LOG_ERROR_ADD(L"POSTerminalController", L"Invalid parameters for payment");
		return nullptr;
	}

	/*Params _paramPayement = {
		{ L"amount" , doubleToAmountString(paramPayement.Amount.value()) },
		{ L"discount" , _discount },
		{ L"merchantId" , paramPayement.MerchantNumber.has_value() ? std::to_wstring(paramPayement.MerchantNumber.value()) : L"0" },
		{ L"facepay" , paramPayement.isFacepay() ? L"true" : L"false"},
		{ L"subMerchant" , paramPayement.SubMerchant.has_value() ? std::to_wstring(paramPayement.SubMerchant.value()) : L""}
	};*/
	//sendData dataPayement = { PROTOCOL_METHOD_PURCHASE, 0, _paramPayement };
	
	_clearAllMessages();

	auto sendData = protocol_->encodeRequest(paramPayement);
	if (!connection_->send(sendData)) {
		outError = L"Failed to send payment data";
		LOG_ERROR_ADD(L"POSTerminalController", L"Failed to send payment data");
		return nullptr;
	}

	_transitionTo(POSTerminalState::WaitingResponse);

	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
	while (!stopListening_) {
		if (std::chrono::steady_clock::now() > deadline) {
			outError = L"Timeout awaiting terminal response";
			_transitionTo(POSTerminalState::Idle);
			return nullptr;
		}

		auto response = protocol_->getResponseData();
		if (!response.has_value()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
			continue;
		}

		const auto& msg = response.value();

		// Unsolicited service messages
		if (_msgIsServiceMessage(msg)) {
			if (_msgIsBussy(msg)) {
				_transitionTo(POSTerminalState::Busy);
				return nullptr;
			}
			_handleServiceMessage(msg.params); // update UI/log/internal flags
		}
		else if (str_utils::iequals(msg.method, PROTOCOL_METHOD_PURCHASE)) {

			std::wstring responseCode = msg.GetParams(PROTOCOL_MESSAGE_PARAM_RESPONSECODE).value_or(L"");

			UINT responseCodeUInt = str_utils::to_UInt(responseCode).value_or(0);

			if (responseCodeUInt != 0) {
				if (responseCodeUInt >= 1000) {
					outError = L"Terminal returned error code: " + responseCode + L" (" + ProtocolResponseDescription(responseCodeUInt) + L")";
				}
				else {
					outError = L"Transaction failed with error code: " + responseCode + L" (" + msg.errorDescription + L")";
				}
				_transitionTo(POSTerminalState::OperationFailed);
				return nullptr;
			}

			auto result = msg.to_OperationResponse();
			result->result = true;
			_transitionTo(POSTerminalState::OperationCompleted);
			return result;
		}
	}
	return nullptr;
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

	LOG_INFO_ADD(L"POSTerminalController", L"Start listening POS Terminal");
	connection_->startListening([this](std::vector<uint8_t> data) {
		this->_processIncomingData(data);
		});

	LOG_INFO_ADD(L"POSTerminalController", L"Start Handshake POS terminal");
	auto resultHandshake = _handshakeTerminal();
	if (!resultHandshake) {
		LOG_ERROR_ADD(L"POSTerminalController", L"Error Handshake POS terminal");
		connection_->disconnect();
		return false;
	}
	LOG_INFO_ADD(L"POSTerminalController", L"Disconnect POS Terminal");
	connection_->disconnect();

	LOG_INFO_ADD(L"POSTerminalController", L"Connect POS Terminal");
	if (!connection_->connect(address, port)) {
		LOG_ERROR_ADD(L"POSTerminalController", L"Error connect POS Terminal");
		return false;
	}
	LOG_INFO_ADD(L"POSTerminalController", L"Start listening POS Terminal");
	connection_->startListening([this](std::vector<uint8_t> data) {
		this->_processIncomingData(data);
		});

	LOG_INFO_ADD(L"POSTerminalController", L"Start Identify POS Terminal");
	auto resultIdentify = _identifyTerminal();
	if (!resultIdentify) {
		LOG_ERROR_ADD(L"POSTerminalController", L"Error Identify POS Terminal");
		connection_->disconnect();
		return false;
	}
	LOG_INFO_ADD(L"POSTerminalController", L"Disconnect POS Terminal");
	connection_->disconnect();

	LOG_INFO_ADD(L"POSTerminalController", L"Connect POS Terminal");
	if (!connection_->connect(address, port)) {
		LOG_ERROR_ADD(L"POSTerminalController", L"Error connect POS Terminal");
		return false;
	}

	LOG_INFO_ADD(L"POSTerminalController", L"Start listening POS Terminal");
	connection_->startListening([this](std::vector<uint8_t> data) {
		this->_processIncomingData(data);
		});
	
	connection_->setReconnectDelay(std::chrono::milliseconds(5000));

	return connection_->isConnected();
}

void POSTerminalController::_processIncomingData(const std::vector<uint8_t>& data)
{
#ifdef _DEBUG
	std::wcout << L"	_processIncomingData size:" << data.size() << std::endl;

	std::wstring out;
	out.reserve(data.size() * 2);
	for (uint8_t b : data) {
		std::format_to(std::back_inserter(out), L"{:02X}", b);
	}

	std::wstring debugData = L"Process incoming data. Size: ";
	debugData += str_utils::to_wstring(data.size());
	debugData += L" bytes .\nRaw: " + out;
	
	LOG_INFO_ADD(L"POSTerminalController", debugData);
#endif

	protocol_->pushResponse(data);
}

void POSTerminalController::_handleParsedJson(const jsoncons::json& json)
{
	// not implemented yet
}

void POSTerminalController::_transitionTo(POSTerminalState newState)
{
	if (m_state != newState) {
		m_state = newState;
	}
}

void POSTerminalController::_handleServiceMessage(const Params& params)
{
	// not implemented yet
}

bool POSTerminalController::_sendInterrupt()
{
	try {
		jsoncons::json json;
		json["method"] = "ServiceMessage";
		json["step"] = 0;

		jsoncons::json params_json;
		params_json["msgType"] = "interrupt";
		json["params"] = params_json;

		const std::string payload = json.to_string();
		std::vector<uint8_t> data(payload.begin(), payload.end());
		data.push_back(0x00);

		LOG_INFO_ADD(L"POSTerminalController", L"Send ServiceMessage interrupt: " + str_utils::to_wstring(payload));
		return connection_ && connection_->send(data);
	}
	catch (const std::exception& e) {
		LOG_ERROR_ADD(L"POSTerminalController", L"Failed to send interrupt: " + str_utils::to_wstring(e.what()));
		return false;
	}
}

int POSTerminalController::_getLastResult()
{
	sendData handshakeRequest;
	handshakeRequest.method = PROTOCOL_METHOD_SERVICEMESSAGE;
	handshakeRequest.step = 0;
	handshakeRequest.params = {
		{ PROTOCOL_MESSAGE_PARAM_MSGTYPE, std::wstring(PROTOCOL_MESSAGE_PARAM_GET_LAST_RESULT) }
	};

	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);

	while (!stopListening_) {
		if (std::chrono::steady_clock::now() > deadline) {
			LOG_ERROR_ADD(L"POSTerminalController", L"Timeout waiting for getLastResult");
			return -1;
		}

		auto response = protocol_->getResponseData();
		if (!response.has_value()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
			continue;
		}

		const auto& msg = response.value();

		if (!str_utils::iequals(msg.method, PROTOCOL_METHOD_SERVICEMESSAGE)) {
			continue;
		}

		if (msg.error) {
			LOG_ERROR_ADD(L"POSTerminalController", L"getLastResult error: " + msg.errorDescription);
			return -1;
		}

		auto codeOpt = msg.GetParams(PROTOCOL_MESSAGE_PARAM_LAST_RESULT);
		if (!codeOpt.has_value()) {
			LOG_ERROR_ADD(L"POSTerminalController", L"getLastResult: missing LastResult param");
			return -1;
		}

		const auto& code = codeOpt.value();

		if (code == L"0") return 0;
		if (code == L"1") return 1;

		LOG_ERROR_ADD(L"POSTerminalController", L"getLastResult: invalid LastResult param: " + code);
		return -1;
	}
	return -1;
}

int POSTerminalController::_getLastStatMsgCode()
{
	sendData handshakeRequest;
	handshakeRequest.method = PROTOCOL_METHOD_SERVICEMESSAGE;
	handshakeRequest.step = 0;
	handshakeRequest.params = {
		{ PROTOCOL_MESSAGE_PARAM_MSGTYPE, std::wstring(PROTOCOL_MESSAGE_TYPE_GET_LAST_MSG_CODE) }
	};

	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);

	while (!stopListening_) {
		if (std::chrono::steady_clock::now() > deadline) {
			LOG_ERROR_ADD(L"POSTerminalController", L"Timeout waiting for getLastStatMsgCode");
			return -1;
		}

		auto response = protocol_->getResponseData();
		if (!response.has_value()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
			continue;
		}

		const auto& msg = response.value();

		if (!str_utils::iequals(msg.method, PROTOCOL_METHOD_SERVICEMESSAGE)) {
			continue;
		}

		if (msg.error) {
			LOG_ERROR_ADD(L"POSTerminalController", L"getLastStatMsgCode error: " + msg.errorDescription);
			return -1;
		}

		auto codeOpt = msg.GetParams(PROTOCOL_MESSAGE_PARAM_LAST_STAT_MSG_CODE);
		if (!codeOpt.has_value()) {
			LOG_ERROR_ADD(L"POSTerminalController", L"getLastStatMsgCode: missing LastResult param");
			return -1;
		}

		const auto& code = codeOpt.value();

		if (code == L"0") return 0;
		if (code == L"1") return 1;

		LOG_ERROR_ADD(L"POSTerminalController", L"getLastStatMsgCode: invalid LastResult param: " + code);
		return -1;
	}
	return -1;
}

bool POSTerminalController::_handshakeTerminal()
{
	// Send handshake request
	sendData handshakeRequest;
	handshakeRequest.method = PROTOCOL_METHOD_PING_DEVICE;
	handshakeRequest.step = 0;
	handshakeRequest.params = {};

	_clearAllMessages();

	auto sendData = protocol_->encodeRequest(handshakeRequest);
	if (!connection_->send(sendData)) {
		LOG_ERROR_ADD(L"POSTerminalController", L"Failed to send data on handshake POS terminal");
		return false;
	}

	// Wait for handshake response
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
	while (!stopListening_) {
		if (std::chrono::steady_clock::now() > deadline) {
			LOG_ERROR_ADD(L"POSTerminalController", L"Timeout waiting for handshake response");
			return false;
		}

		auto response = protocol_->getResponseData();
		if (!response.has_value()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
			continue;
		}

		const auto& msg = response.value();
		if (str_utils::iequals(msg.method, PROTOCOL_METHOD_PING_DEVICE)) {
			if (msg.error) {
				LOG_ERROR_ADD(L"POSTerminalController", L"Handshake error: " + msg.errorDescription);
				return false;
			}
			auto code = msg.GetParams(PROTOCOL_MESSAGE_PARAM_CODE);
			auto response_code = msg.GetParams(PROTOCOL_MESSAGE_PARAM_RESPONSE_CODE);

			LOG_INFO_ADD(L"POSTerminalController", L"Handshake successful, code: " +
				(code.has_value() ? *code : L"") +
				L", responseCode: " +
				(response_code.has_value() ? *response_code : L""));
			return true;
		}
	}

	return false;
}

bool POSTerminalController::_identifyTerminal()
{
	// Send handshake request
	sendData request;
	request.method = PROTOCOL_METHOD_SERVICEMESSAGE;
	request.step = 0;
	request.params = {
		{ PROTOCOL_MESSAGE_PARAM_MSGTYPE , std::wstring(PROTOCOL_MESSAGE_TYPE_IDENTIFY) }
	};


	_clearAllMessages();

	auto sendData = protocol_->encodeRequest(request);
	if (!connection_->send(sendData)) {
		LOG_ERROR_ADD(L"POSTerminalController", L"Failed to send data on identify POS terminal");
		return false;
	}

	// Wait for handshake response
	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
	while (!stopListening_) {
		if (std::chrono::steady_clock::now() > deadline) {
			LOG_ERROR_ADD(L"POSTerminalController", L"Timeout waiting for identify terminal response");
			return false;
		}

		auto response = protocol_->getResponseData();
		if (!response.has_value()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
			continue;
		}

		const auto& msg = response.value();
		if (str_utils::iequals(msg.method, PROTOCOL_METHOD_SERVICEMESSAGE)) {
			if (msg.error) {
				LOG_ERROR_ADD(L"POSTerminalController", L"Identify terminal error: " + msg.errorDescription);
				return false;
			}
			auto msgType = msg.GetParams(PROTOCOL_MESSAGE_PARAM_MSGTYPE);
			auto result = msg.GetParams(PROTOCOL_MESSAGE_PARAM_RESULT);

			if (!str_utils::iequals(msgType.value_or(L""), PROTOCOL_MESSAGE_TYPE_IDENTIFY))
			{
				LOG_ERROR_ADD(L"POSTerminalController", L"Identify terminal: unexpected msgType: " + msgType.value_or(L""));
				return false;
			}

			if (str_utils::iequals(result.value_or(L""), L"true")) {
				LOG_INFO_ADD(L"POSTerminalController", L"Identify terminal successful");
			}
			else {
				LOG_ERROR_ADD(L"POSTerminalController", L"Identify terminal failed, result: " + result.value_or(L""));
				return false;
			}

			terminalModel_ = msg.GetParams(PROTOCOL_MESSAGE_PARAM_VENDOR).value_or(L"");
			terminalVendor_ = msg.GetParams(PROTOCOL_MESSAGE_PARAM_MODEL).value_or(L"");

			return true;
		}
	}
	return false;
}

//void POSTerminalController::_parceAllMessages()
//{
//	auto response = protocol_->getResponseData();
//	while (response.has_value()) {
//		const auto& msg = response.value();
//		// Unsolicited service messages
//		if (msg.method == L"ServiceMessage") {
//			_handleServiceMessage(msg.params); // update UI/log/internal flags
//		}
//		response = protocol_->getResponseData();
//	}
//}
//
//void POSTerminalController::_handleServiceMessage(const Params& params)
//{
//	auto getStr = [&](const wchar_t* key) -> std::optional<std::wstring> {
//		auto it = params.find(key);
//		if (it == params.end()) return std::nullopt;
//		if (std::holds_alternative<std::wstring>(it->second)) {
//			return std::get<std::wstring>(it->second);
//		}
//		if (std::holds_alternative<int>(it->second)) {
//			return std::to_wstring(std::get<int>(it->second));
//		}
//		if (std::holds_alternative<bool>(it->second)) {
//			return std::get<bool>(it->second) ? L"true" : L"false";
//		}
//		return std::nullopt;
//		};
//
//	auto getInt = [&](const wchar_t* key) -> std::optional<int> {
//		auto it = params.find(key);
//		if (it == params.end()) return std::nullopt;
//		if (std::holds_alternative<int>(it->second)) {
//			return std::get<int>(it->second);
//		}
//		if (std::holds_alternative<std::wstring>(it->second)) {
//			try {
//				return std::stoi(std::get<std::wstring>(it->second));
//			}
//			catch (...) {
//				return std::nullopt;
//			}
//		}
//		return std::nullopt;
//		};
//
//	auto getBool = [&](const wchar_t* key) -> std::optional<bool> {
//		auto it = params.find(key);
//		if (it == params.end()) return std::nullopt;
//		if (std::holds_alternative<bool>(it->second)) {
//			return std::get<bool>(it->second);
//		}
//		if (std::holds_alternative<int>(it->second)) {
//			return std::get<int>(it->second) != 0;
//		}
//		if (std::holds_alternative<std::wstring>(it->second)) {
//			const auto& s = std::get<std::wstring>(it->second);
//			if (s == L"true" || s == L"1") return true;
//			if (s == L"false" || s == L"0") return false;
//		}
//		return std::nullopt;
//		};
//
//	const std::wstring msgType = getStr(L"msgType").value_or(L"");
//
//	if (msgType == L"deviceBusy") {
//		LOG_INFO_ADD(L"POSTerminalController", L"ServiceMessage: deviceBusy");
//		_transitionTo(POSTerminalState::Busy);
//		return;
//	}
//
//	if (msgType == L"interrupt") {
//		LOG_INFO_ADD(L"POSTerminalController", L"ServiceMessage: interrupt -> sending interrupt request");
//		// Immediately request interrupt on terminal. Terminal will later
//		// send ServiceMessage 'interruptTransmitted' and return active op with responseCode 1001.
//		(void)_sendInterrupt();
//		_transitionTo(POSTerminalState::WaitingResponse);
//		return;
//	}
//
//	if (msgType == L"interruptTransmitted") {
//		LOG_INFO_ADD(L"POSTerminalController", L"ServiceMessage: interruptTransmitted (acknowledged by terminal)");
//		// Await the active transaction error with responseCode 1001
//		_transitionTo(POSTerminalState::WaitingResponse);
//		return;
//	}
//
//	if (msgType == L"methodNotImplemented") {
//		LOG_ERROR_ADD(L"POSTerminalController", L"ServiceMessage: methodNotImplemented");
//		_transitionTo(POSTerminalState::Error);
//		return;
//	}
//
//	// Ignore other service messages here (they are request/response driven)
//	LOG_INFO_ADD(L"POSTerminalController", L"ServiceMessage ignored (not unsolicited)");
//}

std::vector<uint8_t> JsonChannelProtocol::encodeRequest(POSTerminalOperationParameters& op)
{
	jsoncons::json json;
	// Вложенный объект "params"
	jsoncons::json params_json;

	switch  (op.OperationType) {
		case POSTerminalOperationType::Pay:
			json["method"] = PROTOCOL_METHOD_PURCHASE;
			params_json["amount"] = op.Amount.value_or(0);
			params_json["discount"] = op.Discount.value_or(0);
			params_json["merchantId"] = str_utils::to_wstring(op.MerchantNumber.value_or(0));
			params_json["facepay"] = op.isFacepay();
			params_json["subMerchant"] = op.SubMerchant.has_value() ? str_utils::to_wstring(op.SubMerchant.value()) : L"";
			break;
		case POSTerminalOperationType::ReturnPayment:
			json["method"] = PROTOCOL_METHOD_RETURN_PAYMENT;
			params_json["amount"] = op.Amount.value_or(0);
			params_json["discount"] = op.Discount.value_or(0);
			params_json["merchantId"] = str_utils::to_wstring(op.MerchantNumber.value_or(0));
			params_json["rrn"] = op.RRNCode;
			params_json["subMerchant"] = op.SubMerchant.has_value() ? str_utils::to_wstring(op.SubMerchant.value()) : L"";
			break;
		case POSTerminalOperationType::CancelPayment:
			json["method"] = PROTOCOL_METHOD_CANCEL_PAYMENT;
			params_json["nvoiceNumber"] = op.ReceiptNumber;
			break;
		case POSTerminalOperationType::Authorisation:
			json["method"] = PROTOCOL_METHOD_AUTHORIZATION;
			break;
		case POSTerminalOperationType::AuthConfirmation:
			json["method"] = PROTOCOL_METHOD_AUTH_CONFIRMATION;
			break;
		case POSTerminalOperationType::CancelAuthorisation:
			json["method"] = PROTOCOL_METHOD_CANCEL_AUTHORIZATION;
			break;
		case POSTerminalOperationType::PayWithCashWithdrawal:
			json["method"] = PROTOCOL_METHOD_PAY_WITH_CASH_WITHDRAWAL;
			break;
		default:
			LOG_ERROR_ADD(L"POSTerminalController", L"Invalid OperationType in POSTerminalOperationParameters");
			throw std::invalid_argument("Invalid OperationType in POSTerminalOperationParameters");
	}
	json["step"] = "0";
	// Insert "params" the object on main JSON
	json["params"] = params_json;

	std::string jsonString = json.to_string();
	LOG_INFO_ADD(L"POSTerminalController", L"Send json data: " + str_utils::to_wstring(jsonString));

	// Convert JSON string to UTF-8
	std::vector<uint8_t> data;

	data.insert(data.end(), jsonString.begin(), jsonString.end());
	data.push_back(0x00); // Add 0x00 at the end

	// Send data over the connection
	return data;
}

std::vector<uint8_t> JsonChannelProtocol::encodeRequest(sendData& op)
{
	jsoncons::json json;
	json["method"] = op.method;
	json["step"] = op.step;

	// Вложенный объект "params"
	jsoncons::json params_json;

	for (const auto& [key, value] : op.params) {
		if (std::holds_alternative<std::wstring>(value)) {
			params_json[str_utils::to_string(key)] = str_utils::to_string(std::get<std::wstring>(value));
		}
		else if (std::holds_alternative<long>(value)) {
			params_json[str_utils::to_string(key)] = std::get<long>(value);
		}
		else if (std::holds_alternative<double>(value)) {
			params_json[str_utils::to_string(key)] = std::get<double>(value);
		}
		else if (std::holds_alternative<bool>(value)) {
			params_json[str_utils::to_string(key)] = std::get<bool>(value);
		}
	}

	// Вставляем "params" как объект в основной JSON
	json["params"] = params_json;

	std::string jsonString = json.to_string();
	LOG_INFO_ADD(L"POSTerminalController", L"Send json data: " + str_utils::to_wstring(jsonString));

	// Convert JSON string to UTF-8
	std::vector<uint8_t> data;

	data.insert(data.end(), jsonString.begin(), jsonString.end());
	data.push_back(0x00); // Add 0x00 at the end

	// Send data over the connection
	return data;
}

void JsonChannelProtocol::pushResponse(const std::vector<uint8_t>& rawData)
{
#ifdef _DEBUG
	std::wcout << L"	_handleParsedJson lock data for parce bytes" << std::endl;
#endif
	std::lock_guard<std::mutex> lock(queueMutex_);
	// Append the new data to the buffer
	dataBuffer_.insert(dataBuffer_.end(), rawData.begin(), rawData.end());
	_analyzeData();
	// unlock happens automatically when lock goes out of scope
}

std::optional<receiveData> JsonChannelProtocol::getResponseData()
{
	// Wait for the data to arrive
	std::unique_lock<std::mutex> lock(queueMutex_);
	if (dataQueue_.empty()) {
		return std::nullopt;
	}
	auto value = std::move(dataQueue_.front());
	dataQueue_.pop_front();
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
		LOG_INFO_ADD(L"JsonChannelProtocol", L"Received json data: " + str_utils::to_wstring(json.to_string()));
		dataQueue_.push_back(data);
	}
	catch (const std::exception& e) {
		LOG_ERROR_ADD(L"POSTerminalController", L"Failed to parse JSON: " + str_utils::to_wstring(e.what()));
		return;
	}
}

std::unique_ptr<POSTerminalOperationResponse> receiveData::to_OperationResponse() const
{
	auto getParam = [](const receiveData* m, const wchar_t* k) -> std::wstring {
		return m->GetParams(k).value_or(L"");
		};

	auto r = std::make_unique<POSTerminalOperationResponse>();
	r->amount				= getParam(this, L"amount");
	r->approvalCode			= getParam(this, L"approvalCode");
	r->captureReference		= getParam(this, L"captureReference");
	r->cardExpiryDate		= getParam(this, L"cardExpiryDate");
	r->cardHolderName		= getParam(this, L"cardHolderName");
	r->date					= getParam(this, L"date");
	r->discount				= getParam(this, L"discount");
	r->hstFld63Sf89			= getParam(this, L"hstFld63Sf89");
	r->invoiceNumber		= getParam(this, L"invoiceNumber");
	r->issuerName			= getParam(this, L"issuerName");
	r->merchant				= getParam(this, L"merchant");
	r->pan					= getParam(this, L"pan");
	r->posConditionCode		= getParam(this, L"posConditionCode");
	r->posEntryMode			= getParam(this, L"posEntryMode");
	r->processingCode		= getParam(this, L"processingCode");
	r->receipt				= getParam(this, L"receipt");
	r->responseCode			= getParam(this, PROTOCOL_MESSAGE_PARAM_RESPONSECODE.c_str());
	r->rrn					= getParam(this, L"rrn");
	r->rrnExt				= getParam(this, L"rrnExt");
	r->terminalId			= getParam(this, L"terminalId");
	r->time					= getParam(this, L"time");
	r->track1				= getParam(this, L"track1");
	r->signVerif			= getParam(this, L"signVerif");
	r->txnType				= getParam(this, L"txnType");
	r->trnStatus			= getParam(this, L"trnStatus");
	r->adv					= getParam(this, L"adv");
	r->adv2p				= getParam(this, L"adv2p");
	r->bankAcquirer			= getParam(this, L"bankAcquirer");
	r->paymentSystem		= getParam(this, L"paymentSystem");
	r->subMerchant			= getParam(this, L"subMerchant");
	return r;
}
