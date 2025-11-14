#pragma once

#include <optional>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <variant>
#include <type_traits>


#ifndef CONNECTION_TYPES_H
#define CONNECTION_TYPES_H

enum class ConnectionType {
    TCP = 0,
    COM,
    WebSocket,
    USB
};

const std::unordered_map<ConnectionType, std::wstring> ConnectionTypeNames = {
        { ConnectionType::TCP, L"TCP" },
        { ConnectionType::COM, L"COM" },
        { ConnectionType::WebSocket, L"WebSocket" },
        { ConnectionType::USB, L"USB" }
};

std::wstring getConnectionTypeIndex(ConnectionType type);
std::wstring getConnectionTypeName(ConnectionType type);

template <typename T>
int toConnectionTypeValue(const T& value) {

    if constexpr (std::is_same<T, std::wstring>::value) {
        bool success = false;
        try {
            int intValue = std::stoi(value);
            success = true;
            switch (intValue) {
            case 0: return static_cast<int>(ConnectionType::TCP);
            case 1: return static_cast<int>(ConnectionType::COM);
            case 2: return static_cast<int>(ConnectionType::WebSocket);
            case 3: return static_cast<int>(ConnectionType::USB);
            default: return static_cast<int>(ConnectionType::TCP);
            }
        }
        catch (...) {
            success = false;
        }
        if (!success) {
            auto it = std::find_if(ConnectionTypeNames.begin(), ConnectionTypeNames.end(),
                [&value](const auto& pair) {
                    return pair.second == value;
                });
            if (it != ConnectionTypeNames.end()) {
                return static_cast<int>(it->first);
            }
        }
    }
    else if constexpr (std::is_same<T, int>::value) {
        switch (value) {
        case 0: return static_cast<int>(ConnectionType::TCP);
        case 1: return static_cast<int>(ConnectionType::COM);
        case 2: return static_cast<int>(ConnectionType::WebSocket);
        case 3: return static_cast<int>(ConnectionType::USB);
        default: return static_cast<int>(ConnectionType::TCP);
        }
    }

    return static_cast<int>(ConnectionType::TCP);
};

template <typename T>
std::optional<ConnectionType> toConnectionType(const T& value) {

    if constexpr (std::is_same<T, std::wstring>::value) {
        bool success = false;
        try {
            int intValue = std::stoi(value);
            success = true;
            switch (intValue) {
            case 0: return ConnectionType::TCP;
            case 1: return ConnectionType::COM;
            case 2: return ConnectionType::WebSocket;
            case 3: return ConnectionType::USB;
            default: return std::nullopt;
            }
        }
        catch (...) {
            success = false;
        }
        if (!success) {
            auto it = std::find_if(ConnectionTypeNames.begin(), ConnectionTypeNames.end(),
                [&value](const auto& pair) {
                    return pair.second == value;
                });
            if (it != ConnectionTypeNames.end()) {
                return it->first;
            }
        }
    }
    else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, long>)  {
        switch (value) {
        case 0: return ConnectionType::TCP;
        case 1: return ConnectionType::COM;
        case 2: return ConnectionType::WebSocket;
        case 3: return ConnectionType::USB;
        default: return std::nullopt;
        }
    }

    return std::nullopt;
};

#endif // CONNECTION_TYPES_H
