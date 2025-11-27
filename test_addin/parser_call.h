#pragma once

#ifndef PARSER_CALL_H
#define PARSER_CALL_H

#include <string>
#include <vector>
#include <sstream>
#include <optional>
#include <iostream>

enum class CallType {
	Undefined = -1,
	CallAsProc,
	CallAsFunc,
};

struct ParsedCall {
	CallType callType = CallType::Undefined;		// "CallAsProc" or "CallAsFunc"
	std::string methodName{};						// "Init" or "Compute"
	int paramCount = 0;								// Number of parameters
	std::vector<std::string> params{};				// Parameters as strings
	std::optional<std::string> resultVar;			// Only for CallAsFunc
};

std::optional<CallType> getCallTypeFromString(const std::string& str);

std::optional<ParsedCall> parseCallString(const std::string& input);

#endif // PARSER_CALL_H

