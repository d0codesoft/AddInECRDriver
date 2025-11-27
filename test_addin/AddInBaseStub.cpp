#include "AddInBaseStub.h"

std::vector<std::u16string> splitString(const std::u16string& input, char16_t delimiter)
{
    std::vector<std::u16string> result;
    std::u16string token;

    for (char16_t ch : input) {
        if (ch == delimiter) {
            if (!token.empty()) {
                result.push_back(token);
                token.clear();
            }
        }
        else {
            token += ch;
        }
    }

    if (!token.empty()) {
        result.push_back(token);
    }

    return result;
}
