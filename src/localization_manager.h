#pragma once

#include <string>
#include <unordered_map>

#ifndef LOCALIZATIONMANAGER_H
#define LOCALIZATIONMANAGER_H

class LocalizationManager
{
public:
	LocalizationManager() = delete;
	static const std::u16string& GetLocalizedStringFor1C(const std::wstring& resourceId);
	static const std::wstring& GetLocalizedString(const std::wstring& resourceId);
	static const std::u16string& GetLocalizedStringFor1C(const std::wstring_view& resourceId);
	static const std::wstring& GetLocalizedString(const std::wstring_view& resourceId);
	static void SetLanguageCode(const std::u16string& langCode);

private:

	struct langEntry {
		std::u16string u_name;
		std::wstring w_name;
	};

    struct WStringHash {
        using is_transparent = void;
        size_t operator()(std::wstring_view sv) const noexcept {
            return std::hash<std::wstring_view>{}(sv);
        }
        size_t operator()(const std::wstring& s) const noexcept {
            return std::hash<std::wstring_view>{}(s);
        }
    };

    struct WStringEq {
        using is_transparent = void;
        bool operator()(std::wstring_view a, std::wstring_view b) const noexcept { return a == b; }
        bool operator()(const std::wstring& a, const std::wstring& b) const noexcept { return a == b; }
        bool operator()(const std::wstring& a, std::wstring_view b) const noexcept { return a == b; }
        bool operator()(std::wstring_view a, const std::wstring& b) const noexcept { return a == b; }
    };

	static std::u16string lang_code;
	static std::unordered_map<std::wstring, std::unordered_map<std::u16string, langEntry>,
        WStringHash,
        WStringEq
    > localization_map;
};

#endif // LOCALIZATIONMANAGER_H
