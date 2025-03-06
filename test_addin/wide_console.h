#pragma once

#ifndef WIDE_CONSOLE_H
#define WIDE_CONSOLE_H

#include <iostream>
#include <string>
#include <locale>

#ifdef _WIN32
#include <windows.h>
#endif

class Console {
public:
    Console() {
#ifdef _WIN32
        // Установить кодировку консоли на UTF-8
        SetConsoleOutputCP(CP_UTF8);

        // Установить шрифт консоли на шрифт, поддерживающий кириллицу (например, Consolas)
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
    }

    // Оператор вывода для std::wstring
    Console& operator<<(const std::wstring& text) {
#ifdef _WIN32
        DWORD written;
        WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), text.c_str(), static_cast<DWORD>(text.size()), &written, NULL);
#else
        std::wcout << text;
#endif
        return *this;
    }

    // Оператор вывода для std::string (автоматически преобразует в UTF-16 на Windows)
    Console& operator<<(const std::string& text) {
#ifdef _WIN32
        std::wstring wtext = utf8ToWstring(text);
        return *this << wtext;
#else
        std::cout << text;
#endif
        return *this;
    }

    // Оператор вывода для стандартных типов (int, double и т. д.)
    template <typename T>
    Console& operator<<(const T& value) {
#ifdef _WIN32
        std::wcout << value;
#else
        std::wcout << value;
#endif
        return *this;
    }

    // Оператор вывода для std::endl
    Console& operator<<(std::wostream& (*manip)(std::wostream&)) {
#ifdef _WIN32
        std::wcout << manip;
#else
        std::wcout << manip;
#endif
        return *this;
    }

private:
#ifdef _WIN32
    // Преобразование UTF-8 в UTF-16
    static std::wstring utf8ToWstring(const std::string& str) {
        int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
        std::wstring wstr(size - 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
        return wstr;
    }
#endif
};

extern Console wconsole;

#endif // WIDE_CONSOLE_H