#include "pch.h"
#include "protocol_version.h"

void ProtocolVersion_PB_V1::InitProtocolCommand()
{
	protocolCommands = {
		POSTerminalCommandType::Cmd_ServicePbP,
		POSTerminalCommandType::Cmd_ServiceRefPbP,
		POSTerminalCommandType::Cmd_ServicePbPperiod,
		POSTerminalCommandType::Cmd_ServiceRefPbPperiod,
		POSTerminalCommandType::Cmd_ServicePartlyRefPbPperiod,
		POSTerminalCommandType::Cmd_ServicePartlyRefPbP,
		POSTerminalCommandType::Cmd_ServiceInstantPbI,
		POSTerminalCommandType::Cmd_ServiceRefPbI,
		POSTerminalCommandType::Cmd_ServicePartlyRefPbI,
		POSTerminalCommandType::Cmd_ServicePbIAct,
		POSTerminalCommandType::Cmd_ServiceRefPbIAct,
		POSTerminalCommandType::Cmd_ServicePartlyRefPbIAct,
		POSTerminalCommandType::Cmd_ServiceGeneric,
		POSTerminalCommandType::Cmd_Cashback,
		POSTerminalCommandType::Cmd_Audit,
		POSTerminalCommandType::Cmd_Verify,
		POSTerminalCommandType::Cmd_VerifyCopy,
		POSTerminalCommandType::Cmd_PrintReceiptNum,
		POSTerminalCommandType::Cmd_PrintBatchJournal,
		POSTerminalCommandType::Cmd_PrintBatchJournal,
		POSTerminalCommandType::Cmd_ReadBonusCard,
		POSTerminalCommandType::Cmd_GetPinBonusCard,
		POSTerminalCommandType::Cmd_PinChangeBonusCard,
		POSTerminalCommandType::Cmd_GetPhoneNumber,
		POSTerminalCommandType::Cmd_Preauthorization,
		POSTerminalCommandType::Cmd_SaleCompletion,
		POSTerminalCommandType::Cmd_GetOTPpassword,
		POSTerminalCommandType::Cmd_GetReceiptInfo,
		POSTerminalCommandType::Cmd_ServiceMessage_deviceBusy,
		POSTerminalCommandType::Cmd_ServiceMessage_interrupt,
		POSTerminalCommandType::Cmd_ServiceMessage_interruptTransmitted,
		POSTerminalCommandType::Cmd_ServiceMessage_getMerchantList,
		POSTerminalCommandType::Cmd_ServiceMessage_getMaskList,
		POSTerminalCommandType::Cmd_ServiceMessage_debug,
		POSTerminalCommandType::Cmd_ServiceMessage_getLastResult,
		POSTerminalCommandType::Cmd_ServiceMessage_getLastStatMsgCode,
		POSTerminalCommandType::Cmd_ServiceMessage_getLastStatMsgDescription,
		POSTerminalCommandType::Cmd_ServiceMessage_identify,
		POSTerminalCommandType::Cmd_ServiceMessage_getDiscountName,
		POSTerminalCommandType::Cmd_ServiceMessage_correctTransaction
	};
}
