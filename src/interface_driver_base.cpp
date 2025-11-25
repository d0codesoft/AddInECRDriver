#include "pch.h"
#include "interface_driver_base.h"

MethodName IDriver1CUniBase::createMethod(
	const uint32_t& methodId,
	const std::u16string& name_en,
	const std::u16string& name_ru,
	const std::u16string& descr,
	const bool& hasRetVal,
	const uint32_t& paramCount,
	CallAsFunc1C ptr_method
)
{
	MethodName method;
	method.methodId = methodId;
	method.name_en = name_en;
	method.name_ru = name_ru;
	method.descr = descr;
	method.hasRetVal = hasRetVal;
	method.paramCount = paramCount;
	method.ptr_method = ptr_method;
	return method;
}