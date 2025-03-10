#pragma once

#ifndef WIDE_CONSOLE_H
#define WIDE_CONSOLE_H

#include <iostream>
#include <string>
#include <locale>
#include <fstream>
#include <chrono>
#include <iomanip>
#include "str_utils.h"

#ifdef _WIN32
#include <windows.h>
#endif

class Console {
public:
    Console(const std::wstring& logFileName = L"test.log") {
#ifdef _WIN32
        // ���������� ��������� ������� �� UTF-8
        SetConsoleOutputCP(CP_UTF8);

        // ���������� ����� ������� �� �����, �������������� ��������� (��������, Consolas)
        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof(cfi);
        cfi.nFont = 0;
        cfi.dwFontSize.X = 0;                   // Width of each character in the font
        cfi.dwFontSize.Y = 16;                  // Height
        cfi.FontFamily = FF_DONTCARE;
        cfi.FontWeight = FW_NORMAL;
        wcscpy_s(cfi.FaceName, L"Consolas");    // Choose your font
        SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
#endif

        std::ifstream check_file(logFileName, std::ios::binary | std::ios::ate);
        bool is_empty = check_file.tellg() == 0;
        check_file.close();

        if (is_empty) {
            std::ofstream logFile(logFileName, std::ios::binary);
            if (logFile.is_open()) {
                const char bom[] = "\xFF\xFE";
                logFile.write(bom, sizeof(bom) - 1);
                logFile.close();
            }
        }

        logFile_.open(logFileName, std::ios::binary | std::ios::app);
		if (logFile_.is_open()) {
            logFile_.imbue(std::locale(".UTF-8"));
		}
    }

    // �������� ������ ��� std::wstring
    Console& operator<<(const std::wstring& text) {
#ifdef _WIN32
        DWORD written;
        WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), text.c_str(), static_cast<DWORD>(text.size()), &written, NULL);
#else
        std::wcout << text;
#endif
        if (logFile_.is_open()) {
            if (addTimestamp) { logFile_ << getCurrentDateTime(); addTimestamp = false; }
            logFile_ << text;
        }
        return *this;
    }

    // �������� ������ ��� std::string (������������� ����������� � UTF-16 �� Windows)
    Console& operator<<(const std::string& text) {
#ifdef _WIN32
        std::wstring wtext = str_utils::to_wstring(text);
        return *this << wtext;
#else
        std::cout << text;
        if (logFile_.is_open()) {
            if (addTimestamp) { logFile_ << getCurrentDateTime(); }
            logFile_ << text;
        }
#endif
        return *this;
    }

    // �������� ������ ��� ����������� ����� (int, double � �. �.)
    template <typename T>
    Console& operator<<(const T& value) {
#ifdef _WIN32
        std::wcout << value;
#else
        std::wcout << value;
#endif
        if (logFile_.is_open()) {
            if (addTimestamp) { logFile_ << getCurrentDateTime(); addTimestamp = false; }
            logFile_ << value;
        }
        return *this;
    }

    // �������� ������ ��� std::endl
    Console& operator<<(std::wostream& (*manip)(std::wostream&)) {
#ifdef _WIN32
        std::wcout << manip;
#else
        std::wcout << manip;
#endif
        if (logFile_.is_open()) {
            logFile_ << manip;
        }
        addTimestamp = true;
        return *this;
    }

private:

    static std::wstring getCurrentDateTime() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm buf;
#ifdef _WIN32
        localtime_s(&buf, &in_time_t);
#else
        localtime_r(&in_time_t, &buf);
#endif
        std::wstringstream ss;
        ss << std::put_time(&buf, L"%Y-%m-%d %X") << L" : ";
        return ss.str();
    }

    std::wofstream logFile_;
    bool addTimestamp = true;
};

extern Console wconsole;

#endif // WIDE_CONSOLE_H