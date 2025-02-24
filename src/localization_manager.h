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
	static void SetLanguageCode(const std::u16string& langCode);

private:
	static std::u16string lang_code;
	static std::unordered_map<std::wstring, std::unordered_map<std::u16string, std::u16string>> localization_map;
	static std::unordered_map<std::wstring, std::unordered_map<std::u16string, std::wstring>> localization_map_ws;
};

#endif // LOCALIZATIONMANAGER_H
