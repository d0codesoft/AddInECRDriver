#pragma once

#include <span>
#include "common_types.h"

#ifndef SETTINGDRIVERPOS_H
#define SETTINGDRIVERPOS_H

struct SettingItem {
    std::wstring Value;
    std::wstring DisplayName;
};

struct SettingParameter {
    std::wstring Name;
    std::wstring Caption;
    std::wstring Description;
    std::wstring TypeValue;
    std::wstring FieldFormat;
    std::wstring DefaultValue;
    bool ReadOnly = false;
    std::vector<SettingItem> ChoiceList;
};

struct SettingGroup {
    std::wstring Caption;
    std::vector<SettingParameter> Parameters;
};

struct SettingPage {
    std::wstring Caption;
    std::vector<SettingGroup> Groups;
};

struct SettingSettings {
    std::vector<SettingPage> pages;
};

class SettingDriverPos
{
public:
    SettingDriverPos();
	const SettingSettings& getSettings();
	std::u16string getSettingXML();

private:
    const SettingSettings m_settings;
};

std::u16string toXML(const SettingSettings& settings, std::span<const DriverParameter> parameters);

#endif // SETTINGDRIVERPOS_H