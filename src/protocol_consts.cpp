#include "pch.h"
#include "protocol_consts.h"

// Ukrainian descriptions for response codes
const std::wstring ProtocolResponseDescription(UINT code) noexcept {
	switch (code) {
	case PROTOCOL_RESPONSE_CODE_GENERAL_ERROR:
		return L"Загальна помилка";
	case PROTOCOL_RESPONSE_CODE_CANCELED_BY_USER:
		return L"Операцію скасовано користувачем";
	case PROTOCOL_RESPONSE_CODE_EMV_DECLINE:
		return L"Відхилено EMV";
	case PROTOCOL_RESPONSE_CODE_TRANSACTION_LOG_FULL:
		return L"Журнал транзакцій заповнений. Необхідно закрити журнал";
	case PROTOCOL_RESPONSE_CODE_NO_HOST_CONNECTION:
		return L"Немає з’єднання з хостом";
	case PROTOCOL_RESPONSE_CODE_NO_PAPER:
		return L"Немає паперу в принтері";
	case PROTOCOL_RESPONSE_CODE_CRYPTO_KEYS_ERROR:
		return L"Помилка криптографічних ключів";
	case PROTOCOL_RESPONSE_CODE_CARD_READER_NOT_CONNECTED:
		return L"Зчитувач картки не підключено";
	case PROTOCOL_RESPONSE_CODE_ALREADY_COMPLETE:
		return L"Транзакцію вже завершено";
	default:
		return L"Невідомий код відповіді";
	}
}