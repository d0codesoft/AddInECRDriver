#pragma once

#ifndef SYSUTILS_H
#define SYSUTILS_H

#include <string>

static class SysUtils
{
public: 
	SysUtils() = delete;

	static std::string get_full_path(const std::string& path);

};

#endif // SYSUTILS_H

