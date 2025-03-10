#pragma once

#ifndef ADDINDEFBASE_H
#define ADDINDEFBASE_H

#include "common_platform.h"
#include "../include/vncomp/AddInDefBase.h"
#include "../include/vncomp/IMemoryManager.h"
#include <iostream>
#include <unordered_map>
#include <unicode/ucnv_err.h>
#include <unicode/unistr.h>
#include <variant>
#include <ComponentBase.h>
#include "str_utils.h"

std::vector<std::u16string> splitString(const std::u16string& input, char16_t delimiter);

class MemoryManagerStub : public IMemoryManager {
public:
    bool ADDIN_API AllocMemory(void** pMemory, unsigned long ulCountByte) override {
        *pMemory = malloc(ulCountByte);
        return *pMemory != nullptr;
    }

    void ADDIN_API FreeMemory(void** pMemory) override {
        if (pMemory && *pMemory) {
            free(*pMemory);
            *pMemory = nullptr;
        }
    }
};

class AddInDefBaseStub : public IAddInDefBase {
public:
    bool ADDIN_API AddError(unsigned short wcode, const WCHAR_T* source,
        const WCHAR_T* descr, long scode) override {

        // Конвертация WCHAR_T* в std::wstring с использованием ICU
        icu::UnicodeString sourceStr(source);
        icu::UnicodeString descrStr(descr);

        std::string _source, _descr;
        sourceStr.toUTF8String(_source);
        descrStr.toUTF8String(_descr);

        // Вывод сообщения об ошибке в консоль
        std::cout << "Error Code: " << wcode << "\n"
            << "Source: " << _source << "\n"
            << "Description: " << _descr << "\n"
            << "code: " << scode << std::endl;

        return true;
    }

    bool ADDIN_API Read(WCHAR_T* wszPropName, tVariant* pVal, long* pErrCode,
        WCHAR_T** errDescriptor) override {
        // Заглушка: возвращает false
        return false;
    }

    bool ADDIN_API Write(WCHAR_T* wszPropName, tVariant* pVar) override {
        // Заглушка: возвращает false
        return false;
    }

    bool ADDIN_API RegisterProfileAs(WCHAR_T* wszProfileName) override {

        // Конвертация WCHAR_T* в std::wstring с использованием ICU
        icu::UnicodeString profileUni(wszProfileName);

        std::string _profile;
        profileUni.toUTF8String(_profile);

        // Вывод сообщения о регистрации профиля в консоль
        std::cout << "Profile registered as: " << _profile << std::endl;

        return true;
    }

    bool ADDIN_API SetEventBufferDepth(long lDepth) override {

        m_eventBufferDepth = lDepth;
        std::cout << "Set event buffer depth: " << m_eventBufferDepth << std::endl;
        return true;
    }

    long ADDIN_API GetEventBufferDepth() override {
        return m_eventBufferDepth;
    }

    bool ADDIN_API ExternalEvent(WCHAR_T* wszSource, WCHAR_T* wszMessage,
        WCHAR_T* wszData) override {

        icu::UnicodeString sourceStr = str_utils::to_UnicodeString(wszSource);
        icu::UnicodeString messageStr = str_utils::to_UnicodeString(wszMessage);

        std::string _source, _message;
        sourceStr.toUTF8String(_source);
        messageStr.toUTF8String(_message);

        std::cout << "External event: " << _source << " Messsage: " << _message << " " << std::endl;
        return true;
    }

    void ADDIN_API CleanEventBuffer() override {
        // Заглушка: ничего не делает
    }

    bool ADDIN_API SetStatusLine(WCHAR_T* wszStatusLine) override {

        icu::UnicodeString statusLine = str_utils::to_UnicodeString(wszStatusLine);
        std::string _statusLine;
        statusLine.toUTF8String(_statusLine);
        std::cout << "Set status line: " << _statusLine << std::endl;
        return true;
    }

    void ADDIN_API ResetStatusLine() override {
        // Заглушка: ничего не делает
    }

private:

    long m_eventBufferDepth = 5;
};

#endif // ADDINDEFBASE_H