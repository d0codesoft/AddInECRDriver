#pragma once

#ifndef IADDINBASE_H
#define IADDINBASE_H

#include "AddInDefBase.h"
#include "IMemoryManager.h"
#include <string>

class IAddInBase {
public:
    virtual ~IAddInBase() = default;

    // Function to get IAddInDefBase pointer
    virtual IAddInDefBase* getAddInDefBase() const = 0;

    // Function to get IMemoryManager pointer
    virtual IMemoryManager* getMemoryManager() const = 0;

    // Function to add an error
    virtual void addError(uint32_t wcode, const std::u16string& source,
        const std::u16string& descriptor, long code) = 0;

    virtual bool getString1C(const std::u16string& source, WCHAR_T* value, uint32_t& length) = 0;
    virtual bool getString1C(const std::u16string& source, WCHAR_T** value, uint32_t& length) = 0;
	virtual bool getStringFromWchart(const WCHAR_T* source, std::u16string& desct) = 0;
    virtual bool setStringValue(tVariant* pvarParamDefValue, const std::u16string& source) = 0;
    virtual bool setBoolValue(tVariant* pvarParamDefValue, const bool flag) = 0;

	// Function to save and load value to storage 1C
	virtual bool saveValue(const std::u16string& key, const std::u16string& value) = 0;
    virtual bool saveValue(const std::u16string& key, const int value) = 0;
    virtual bool saveValue(const std::u16string& key, const bool value) = 0;
    virtual bool loadValue(const std::u16string& key, std::u16string& value) = 0;
	virtual bool loadValue(const std::u16string& key, int& value) = 0;
	virtual bool loadValue(const std::u16string& key, bool& value) = 0;
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