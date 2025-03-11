#include "parser_call.h"

std::optional<CallType> getCallTypeFromString(const std::string& str)
{
	if (str == "CallAsProc") {
		return CallType::CallAsProc;
	}
	else if (str == "CallAsFunc") {
		return CallType::CallAsFunc;
	}
	else {
		return std::nullopt;
	}
}

std::optional<ParsedCall> parseCallString(const std::string& input)
{
	std::istringstream stream(input);
	std::string token;
	std::optional<ParsedCall> result = std::nullopt;
	ParsedCall resultCall;

	// Read call type
	if (!std::getline(stream, token, '=')) {
		return result;
	}

	auto callType = getCallTypeFromString(token);
	if (!callType.has_value()) {
		return result;
	}
	resultCall.callType = callType.value();

	// Read method name and parameters
	std::getline(stream, token);
	std::istringstream paramStream(token);
	std::vector<std::string> tokens;
	while (std::getline(paramStream, token, '|')) {
		tokens.push_back(token);
	}

	// Trim spaces
	for (auto& t : tokens) {
		size_t start = t.find_first_not_of(" ");
		size_t end = t.find_last_not_of(" ");
		t = (start != std::string::npos) ? t.substr(start, end - start + 1) : "";
	}

	if (tokens.size() < 2) {
		return result;
	}

	resultCall.methodName = tokens[0];
	resultCall.paramCount = std::stoi(tokens[1]);

	for (size_t i = 2; i < tokens.size(); ++i) {
		if (resultCall.callType == CallType::CallAsFunc && i == tokens.size() - 1) {
			resultCall.resultVar = tokens[i];
		}
		else {
			resultCall.params.push_back(tokens[i]);
		}
	}

	result = resultCall;
	return result;
}