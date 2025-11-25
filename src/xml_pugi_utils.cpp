#include "pch.h"
#include "xml_pugi_utils.h"
#include "logger.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <combaseapi.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#endif

namespace pugi_utils {

    std::optional<long> get_long_attr(const pugi::xml_node& node, const std::wstring& name) {
        if (!node.attribute(name).empty()) {
            long long v64 = node.attribute(name).as_llong(0);
            if (v64 < static_cast<long long>(std::numeric_limits<long>::min()) ||
                v64 > static_cast<long long>(std::numeric_limits<long>::max())) {
                LOG_ERROR_ADD(L"CommonTypes",
                    L"Attribute \"" + name + L"\" value out of range for long");
                return std::nullopt;
            }
            return static_cast<long>(v64); // explicit, safe narrowing
        }
        return std::nullopt;
    }

    std::optional<double> get_double_attr(const pugi::xml_node& node, const std::wstring& name) {
        if (!node.attribute(name).empty()) {
            return node.attribute(name).as_double(0.0);
        }
        return std::nullopt;
    }

    std::wstring get_wstring_attr(const pugi::xml_node& node, const std::wstring& name) {
        if (!node.attribute(name).empty()) {
            return node.attribute(name).as_string();
        }
        return L"";
    }

}
