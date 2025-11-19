#pragma once

#ifndef DRIVER1CBASE_H
#define DRIVER1CBASE_H

#include "AddInDefBase.h"
#include <map>
#include "common_types.h"
#include "define_driver.h"

class IDriver1CUniBase
{
public:
    virtual ~IDriver1CUniBase() = default;

	virtual void InitDriver() = 0;

    // Methods implement the functionality of the add-in Driver
    virtual bool GetInterfaceRevision(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool GetDescription(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool GetLastError(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

    //methods version standard > 3.x
    virtual bool GetParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool SetParameter(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool Open(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool Close(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool DeviceTest(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool GetAdditionalActions(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool DoAdditionalAction(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

	//methods version standard 4.x
    virtual bool EquipmentParameters(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool ConnectEquipment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool DisconnectEquipment(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool EquipmentTest(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool EquipmentAutoSetup(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool SetApplicationInformation(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool GetLocalizationPattern(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;
    virtual bool SetLocalization(tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray) = 0;

	virtual const std::vector<MethodName>& GetMethods() = 0;
	virtual const std::vector<PropName>& GetProperties() = 0;
	virtual const DriverDescription& getDescriptionDriver() = 0;
	virtual EquipmentType getEquipmentType() const = 0;

	virtual void AddActionDriver(const std::wstring& name_en, const std::wstring& name_ru,
        const std::wstring& caption_en, const std::wstring& caption_ru, CallAsFunc1C ptr_method) = 0;

	virtual std::span<const ActionDriver> getActions() = 0;
    
    static MethodName createMethod(
        const uint32_t& methodId,
        const std::u16string& name_en,
        const std::u16string& name_ru,
        const std::u16string& descr,
        const bool& hasRetVal,
        const uint32_t& paramCount,
        CallAsFunc1C ptr_method
    );
};

#endif // DRIVER1CBASE_H

