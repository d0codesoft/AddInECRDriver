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
    virtual void addError(uint32_t wcode, const std::string& source,
        const std::string& descriptor, long code) = 0;
};

#endif // IADDINBASE_H