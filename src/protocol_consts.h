#pragma once
#include <string>

#if !defined(_WIN32) && !defined(_WIN64)
using UINT = unsigned int;
#endif

const std::wstring PROTOCOL_VERSION = L"1.0";

// Methods
const std::wstring PROTOCOL_METHOD_SERVICEMESSAGE = L"ServiceMessage";
const std::wstring PROTOCOL_METHOD_PAY = L"Pay";
const std::wstring PROTOCOL_METHOD_RETURN_PAYMENT = L"Refund";
const std::wstring PROTOCOL_METHOD_CANCEL_PAYMENT = L"Withdrawal";
const std::wstring PROTOCOL_METHOD_AUTHORIZATION = L"Preauthorization";
const std::wstring PROTOCOL_METHOD_AUTH_CONFIRMATION = L"SaleCompletion";
const std::wstring PROTOCOL_METHOD_CANCEL_AUTHORIZATION = L"";
const std::wstring PROTOCOL_METHOD_PAY_WITH_CASH_WITHDRAWAL = L"Cashback";
const std::wstring PROTOCOL_METHOD_PAY_ELECTRONIC_CERTIFICATE = L"";
const std::wstring PROTOCOL_METHOD_RETURN_ELECTRONIC_CERTIFICATE = L"";


const std::wstring PROTOCOL_METHOD_PURCHASE = L"Purchase";
const std::wstring PROTOCOL_METHOD_PING_DEVICE = L"PingDevice";

// Message types
const std::wstring PROTOCOL_MESSAGE_TYPE = L"msgType";
const std::wstring PROTOCOL_MESSAGE_TYPE_DEVICEBUSY = L"deviceBusy";
const std::wstring PROTOCOL_MESSAGE_TYPE_INTERRUPT = L"interrupt";
const std::wstring PROTOCOL_MESSAGE_TYPE_GET_LAST_MSG_CODE = L"getLastStatMsgCode";
const std::wstring PROTOCOL_MESSAGE_PARAM_RESPONSECODE = L"responseCode";
const std::wstring PROTOCOL_MESSAGE_ERROR_DESCRIPTION = L"errorDescritption";
const std::wstring PROTOCOL_MESSAGE_PARAM_CODE = L"code";
const std::wstring PROTOCOL_MESSAGE_PARAM_RESPONSE_CODE = L"responseCode";
const std::wstring PROTOCOL_MESSAGE_PARAM_MSGTYPE = L"msgType";
const std::wstring PROTOCOL_MESSAGE_TYPE_IDENTIFY = L"identify";
const std::wstring PROTOCOL_MESSAGE_PARAM_RESULT = L"result";
const std::wstring PROTOCOL_MESSAGE_PARAM_VENDOR = L"vendor";
const std::wstring PROTOCOL_MESSAGE_PARAM_MODEL = L"model";
const std::wstring PROTOCOL_MESSAGE_PARAM_GET_LAST_RESULT = L"getLastResult";
const std::wstring PROTOCOL_MESSAGE_PARAM_LAST_RESULT = L"LastResult";
const std::wstring PROTOCOL_MESSAGE_PARAM_LAST_STAT_MSG_CODE = L"LastStatMsgCode";

const std::wstring PROTOCOL_MESSAGE_PARAM_DEVICE_TYPE = L"deviceType";
const std::wstring PROTOCOL_MESSAGE_PARAM_DEVICE_TYPE_TERMINAL = L"Terminal";
const std::wstring PROTOCOL_MESSAGE_PARAM_DEVICE_TYPE_PIN_PAD = L"PinPad";

// Response codes (as UINT)
const UINT PROTOCOL_RESPONSE_CODE_GENERAL_ERROR = 1000; // General error (exceptional case)
const UINT PROTOCOL_RESPONSE_CODE_CANCELED_BY_USER = 1001; // Transaction canceled by user
const UINT PROTOCOL_RESPONSE_CODE_EMV_DECLINE = 1002; // EMV Decline
const UINT PROTOCOL_RESPONSE_CODE_TRANSACTION_LOG_FULL = 1003; // Transaction log is full. Need close batch
const UINT PROTOCOL_RESPONSE_CODE_NO_HOST_CONNECTION = 1004; // No connection with host
const UINT PROTOCOL_RESPONSE_CODE_NO_PAPER = 1005; // No paper in printer
const UINT PROTOCOL_RESPONSE_CODE_CRYPTO_KEYS_ERROR = 1006; // Error Crypto keys
const UINT PROTOCOL_RESPONSE_CODE_CARD_READER_NOT_CONNECTED = 1007; // Card reader is not connected
const UINT PROTOCOL_RESPONSE_CODE_ALREADY_COMPLETE = 1008; // Transaction is already complete


const std::wstring ProtocolResponseDescription(UINT code) noexcept;