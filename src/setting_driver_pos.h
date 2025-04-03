#pragma once

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
	static const SettingSettings& getSettings();
	static std::u16string getSettingXML();

private:
    static const SettingSettings m_settings;

};

std::u16string toXML(const SettingSettings& settings, std::span<const DriverParameter> parameters);

#endif // SETTINGDRIVERPOS_H