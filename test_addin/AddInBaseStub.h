#pragma once
#include "common_platform.h"
#include "../include/vncomp/AddInDefBase.h"
#include "../include/vncomp/IMemoryManager.h"
#include <iostream>
#include <unordered_map>
#include <unicode/ucnv_err.h>
#include <unicode/unistr.h>
#include <variant>
#include <ComponentBase.h>

//using GetClassObjectFunc = intptr_t(*)(const WCHAR_T*, IComponentBase**);
//using GetClassNamesFunc = const WCHAR_T* (*)();

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

		icu::UnicodeString sourceStr(wszSource);
		icu::UnicodeString messageStr(wszMessage);

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
        
		icu::UnicodeString statusLine(wszStatusLine);
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

//class AddInBaseStub : public IAddInBase 
//{
//public:
//
//	AddInBaseStub() {
//	}
//
//    IAddInDefBase* getAddInDefBase() const override {
//        return const_cast<AddInDefBaseStub*>(&addInDefBaseStub_);
//    }
//
//    IMemoryManager* getMemoryManager() const override {
//        return const_cast<MemoryManagerStub*>(&memoryManagerStub_);
//    }
//
//    void addError(uint32_t wcode, const std::u16string& source,
//        const std::u16string& descriptor, long code) override {
//		addInDefBaseStub_.AddError(wcode, source.c_str(), descriptor.c_str(), code);
//    }
//
//    bool setStringValue(tVariant* pvarParamDefValue, const std::u16string& source) override {
//		pvarParamDefValue->vt = VTYPE_PWSTR;
//		pvarParamDefValue->pwstrVal = const_cast<WCHAR_T*>(source.c_str());
//        return true;
//    }
//
//    bool setBoolValue(tVariant* pvarParamDefValue, const bool flag) override {
//		pvarParamDefValue->vt = VTYPE_BOOL;
//		pvarParamDefValue->bVal = flag;
//        return false;
//    }
//
//    bool saveValue(const std::u16string& key, const std::u16string& value) override {
//        values_[key] = value;
//        return true;
//    }
//
//    bool saveValue(const std::u16string& key, const int value) override {
//        values_[key] = value;
//        return true;
//    }
//
//    bool saveValue(const std::u16string& key, const bool value) override {
//        values_[key] = value;
//        return true;
//    }
//
//    bool loadValue(const std::u16string& key, std::u16string& value) override {
//        if (auto it = values_.find(key); it != values_.end()) {
//            if (std::holds_alternative<std::u16string>(it->second)) {
//                value = std::get<std::u16string>(it->second);
//                return true;
//            }
//        }
//        return false;
//    }
//
//    bool loadValue(const std::u16string& key, int& value) override {
//        if (auto it = values_.find(key); it != values_.end()) {
//            if (std::holds_alternative<int>(it->second)) {
//                value = std::get<int>(it->second);
//                return true;
//            }
//        }
//        return false;
//    }
//
//    bool loadValue(const std::u16string& key, bool& value) override {
//        if (auto it = values_.find(key); it != values_.end()) {
//            if (std::holds_alternative<bool>(it->second)) {
//                value = std::get<bool>(it->second);
//                return true;
//            }
//        }
//        return false;
//    }
//
//	bool getString1C(const std::u16string& source, WCHAR_T* value, uint32_t& length) override {
//        value = const_cast<WCHAR_T*>(source.c_str());
//		length = static_cast<uint32_t>(source.size());
//		return true;
//	}
//
//	bool getString1C(const std::u16string& source, WCHAR_T** value, uint32_t& length) override {
//		*value = const_cast<WCHAR_T*>(source.c_str());
//		length = static_cast<uint32_t>(source.size());
//		return true;
//	}
//
//	bool getStringFromWchart(const WCHAR_T* source, std::u16string& desct) override {
//		if (!source) {
//			return false;
//		}
//		size_t length = 0;
//		while (source[length] != 0) {
//			++length;
//		}
//		desct.assign(source, length);
//		return true;
//	}
//
//private:
//    MemoryManagerStub memoryManagerStub_;
//    AddInDefBaseStub addInDefBaseStub_;
//    
//    std::unordered_map<std::u16string, std::variant<std::u16string, int, bool>> values_;
//};