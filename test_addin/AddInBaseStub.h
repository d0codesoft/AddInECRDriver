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
        std::string sourceStr = str_utils::to_string(source);
        std::string descrStr = str_utils::to_string(descr);

        // Вывод сообщения об ошибке в консоль
        std::cout << "Error Code: " << wcode << "\n"
            << "Source: " << sourceStr << "\n"
            << "Description: " << descrStr << "\n"
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
        std::string profileUni = str_utils::to_string(wszProfileName);

        // Вывод сообщения о регистрации профиля в консоль
        std::cout << "Profile registered as: " << profileUni << std::endl;

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

        std::string sourceStr = str_utils::to_string(wszSource);
        std::string messageStr = str_utils::to_string(wszMessage);

        std::cout << "External event: " << sourceStr << " Messsage: " << messageStr << " " << std::endl;
        return true;
    }

    void ADDIN_API CleanEventBuffer() override {
        // Заглушка: ничего не делает
    }

    bool ADDIN_API SetStatusLine(WCHAR_T* wszStatusLine) override {

        std::string statusLine = str_utils::to_string(wszStatusLine);
        std::cout << "Set status line: " << statusLine << std::endl;
        return true;
    }

    void ADDIN_API ResetStatusLine() override {
        // Заглушка: ничего не делает
    }

private:

    long m_eventBufferDepth = 5;
};

#endif // ADDINDEFBASE_H