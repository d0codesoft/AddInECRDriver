#include "pch.h"
#include "connection_types.h"

std::wstring getConnectionTypeIndex(ConnectionType type)
{
    return std::to_wstring(static_cast<int>(type));
}

std::wstring getConnectionTypeName(ConnectionType type)
{
    auto it = ConnectionTypeNames.find(type);
    if (it != ConnectionTypeNames.end()) {
        return it->second;
    }
    return L"";
}
