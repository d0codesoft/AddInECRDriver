#pragma once

#ifndef IADDINBASE_H
#define IADDINBASE_H

#include "AddInDefBase.h"
#include "IMemoryManager.h"
#include "common_types.h"
#include <string>
#include <optional>

class IAddInBase {
public:
    virtual ~IAddInBase() = default;

    // Function to get IAddInDefBase pointer
    virtual IAddInDefBase* getAddInDefBase() const = 0;

    // Function to get IMemoryManager pointer
    virtual IMemoryManager* getMemoryManager() const = 0;

    // Function to add an error
    // If scode has a non-zero value, an exception will be generated that can be intercepted and handled by the built-in language of 1C:Enterprise.
    virtual void sendError(const std::u16string& source, const std::u16string& descriptor,
        UiAddinError codeMsg = UiAddinError::Info, FacilityCode code = FacilityCode::None) = 0;

    virtual void sendMsg(const std::u16string& source, const std::u16string& descriptor,
        UiAddinError codeMsg = UiAddinError::Ordinary) = 0;

    virtual bool ExternalEvent(const std::u16string& message,
		                       const std::u16string& data) = 0;
    virtual bool ExternalEvent(const std::wstring& message,
        const std::wstring& data) = 0;

	virtual long GetEventBufferDepth() = 0;
    virtual void SetEventBufferDepth(long depth) = 0;
	virtual void CleanEventBuffer() = 0;

    virtual bool getString1C(const std::u16string& source, WCHAR_T** value, uint32_t& length) = 0;
	virtual bool getString(const WCHAR_T* source, std::u16string& dest) = 0;
    virtual bool setStringValue(tVariant* pvarParamDefValue, const std::u16string& source) = 0;
    virtual bool setDoubleValue(tVariant* pvarParamDefValue, const double source) = 0;
    virtual bool setBoolValue(tVariant* pvarParamDefValue, const bool flag) = 0;
    virtual bool setNullValue(tVariant* pvarParamDefValue) = 0;
    virtual bool setIntValue(tVariant* pvarParamDefValue, const int value) = 0;

	// Function to save and load value to storage 1C
	virtual bool saveValue(const std::u16string& key, const std::u16string& value) = 0;
    virtual bool saveValue(const std::u16string& key, const int value) = 0;
    virtual bool saveValue(const std::u16string& key, const bool value) = 0;
    virtual bool loadValue(const std::u16string& key, std::u16string& value) = 0;
	virtual bool loadValue(const std::u16string& key, int& value) = 0;
	virtual bool loadValue(const std::u16string& key, bool& value) = 0;

	virtual LanguageCode getLanguageCode() = 0;
    virtual void setDefaultLanguage(LanguageCode lang) = 0;
};

// Function to check if the variant is a string
#define IS_STRING(var) ( \
    (TV_VT(var) == VTYPE_PSTR)  || \
    (TV_VT(var) == VTYPE_PWSTR)   \
)

#define IS_LONG(var) ( \
    (TV_VT(var) == VTYPE_I4)   || \
    (TV_VT(var) == VTYPE_INT)  || \
    (TV_VT(var) == VTYPE_I8)   || \
    (TV_VT(var) == VTYPE_UI4)  || \
    (TV_VT(var) == VTYPE_UINT) || \
    (TV_VT(var) == VTYPE_UI8)    \
)

#define IS_BOOL(var) (TV_VT(var) == VTYPE_BOOL)

#endif // IADDINBASE_H