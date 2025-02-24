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
};

#endif // IADDINBASE_H