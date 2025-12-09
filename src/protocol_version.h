#pragma once

#include <vector>
#include <string>
#include <optional>
#include <algorithm>
#include <unordered_map>
#include <variant>
#include "common_types.h"
#include "protocol_operation_types.h"

class IProtocolVersion {
public:
	virtual const wchar_t* getVersion() = 0;

	virtual ~IProtocolVersion() = default;

	virtual bool isCommandSupported(POSTerminalCommandType cmd) = 0;

	virtual bool createOperationRequestV2(POSTerminalOperationRequestV2& operationRequest, POSTerminalCommandType cmd) = 0;
};


class ProtocolVersion_PB_V1 : public IProtocolVersion {
public:

	ProtocolVersion_PB_V1() {
		InitProtocolCommand();
	}

	const wchar_t* getVersion() override {
		return L"1.0";
	}

	bool isCommandSupported(POSTerminalCommandType cmd) override {
		for (const auto& command : protocolCommands) {
			if (command == cmd) {
				return true;
			}
		}
		return false;
	}

	bool createOperationRequestV2(POSTerminalOperationRequestV2& operationRequest, POSTerminalCommandType cmd) override
	{
		if (!isCommandSupported(cmd)) {
			return false;
		}
		operationRequest.OperationType = cmd;
		return true;
	}

protected:
	void InitProtocolCommand();

private:
	std::vector<POSTerminalCommandType> protocolCommands;
};