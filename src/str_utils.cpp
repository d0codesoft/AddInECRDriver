#include "str_utils.h"
#include <vector>

	std::wstring str_utils::to_wstring(const std::string& str, const std::string& encoding) {
		// Create a Unicode converter for the given encoding
		UErrorCode error = U_ZERO_ERROR;
		UConverter* converter = ucnv_open(encoding.c_str(), &error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to create converter: " << u_errorName(error) << std::endl;
			return std::wstring();
		}

		// Convert the input string to UnicodeString
		icu::UnicodeString unicodeStr(str.c_str(), str.length(), converter, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to convert string: " << u_errorName(error) << std::endl;
			ucnv_close(converter);
			return std::wstring();
		}

		// Close the converter
		ucnv_close(converter);

		// Determine the size of wchar_t
#if defined(__linux__) || defined(__APPLE__)
	// Linux: wchar_t is UTF-32
		int32_t length = unicodeStr.length();
		std::vector<UChar32> utf32Buffer(length);
		unicodeStr.toUTF32(utf32Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-32 string: " << u_errorName(error) << std::endl;
			return std::wstring();
		}
		// Construct std::wstring from UTF-32 buffer
		std::wstring wstr(utf32Buffer.begin(), utf32Buffer.end());
#else
	// Windows: wchar_t is UTF-16
		int32_t length = unicodeStr.length();
		std::vector<UChar> utf16Buffer(length);
		unicodeStr.extract(utf16Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-16 string: " << u_errorName(error) << std::endl;
			return std::wstring();
		}
		// Construct std::wstring from UTF-16 buffer
		std::wstring wstr(utf16Buffer.begin(), utf16Buffer.end());
#endif

		return wstr;
	}

	std::wstring str_utils::to_wstring(const std::u16string& str)
	{
		// Convert std::u16string to ICU UnicodeString
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str.c_str()), str.length());

		// Determine the size of wchar_t
#if defined(__linux__)
	// Linux: wchar_t is UTF-32
		int32_t length = unicodeStr.length();
		std::vector<UChar32> utf32Buffer(length);
		UErrorCode error = U_ZERO_ERROR;
		unicodeStr.toUTF32(utf32Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-32 string: " << u_errorName(error) << std::endl;
			return std::wstring();
		}
		// Construct std::wstring from UTF-32 buffer
		std::wstring wstr(utf32Buffer.begin(), utf32Buffer.end());
#else
	// Windows: wchar_t is UTF-16
		int32_t length = unicodeStr.length();
		std::vector<UChar> utf16Buffer(length);
		UErrorCode error = U_ZERO_ERROR;
		unicodeStr.extract(utf16Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-16 string: " << u_errorName(error) << std::endl;
			return std::wstring();
		}
		// Construct std::wstring from UTF-16 buffer
		std::wstring wstr(utf16Buffer.begin(), utf16Buffer.end());
#endif

		return wstr;
	}

	std::wstring str_utils::to_wstring(const char* str, const std::string& encoding)
	{
		// Create a Unicode converter for the given encoding
		UErrorCode error = U_ZERO_ERROR;
		UConverter* converter = ucnv_open(encoding.c_str(), &error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to create converter: " << u_errorName(error) << std::endl;
			return std::wstring();
		}

		// Convert the input string to UnicodeString
		icu::UnicodeString unicodeStr(str, -1, converter, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to convert string: " << u_errorName(error) << std::endl;
			ucnv_close(converter);
			return std::wstring();
		}

		// Close the converter
		ucnv_close(converter);

		// Determine the size of wchar_t
#if defined(__linux__) || defined(__APPLE__)
	// Linux: wchar_t is UTF-32
		int32_t length = unicodeStr.length();
		std::vector<UChar32> utf32Buffer(length);
		unicodeStr.toUTF32(utf32Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-32 string: " << u_errorName(error) << std::endl;
			return std::wstring();
		}
		// Construct std::wstring from UTF-32 buffer
		std::wstring wstr(utf32Buffer.begin(), utf32Buffer.end());
#else
	// Windows: wchar_t is UTF-16
		int32_t length = unicodeStr.length();
		std::vector<UChar> utf16Buffer(length);
		unicodeStr.extract(utf16Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-16 string: " << u_errorName(error) << std::endl;
			return std::wstring();
		}
		// Construct std::wstring from UTF-16 buffer
		std::wstring wstr(utf16Buffer.begin(), utf16Buffer.end());
#endif

		return wstr;
	}

	std::wstring str_utils::to_wstring(const char16_t* str)
	{
		// Convert const char16_t* to ICU UnicodeString
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str));

		// Determine the size of wchar_t
#if defined(__linux__) || defined(__APPLE__)
	// Linux: wchar_t is UTF-32
		int32_t length = unicodeStr.length();
		std::vector<UChar32> utf32Buffer(length);
		UErrorCode error = U_ZERO_ERROR;
		unicodeStr.toUTF32(utf32Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-32 string: " << u_errorName(error) << std::endl;
			return std::wstring();
		}
		// Construct std::wstring from UTF-32 buffer
		std::wstring wstr(utf32Buffer.begin(), utf32Buffer.end());
#else
	// Windows: wchar_t is UTF-16
		int32_t length = unicodeStr.length();
		std::vector<UChar> utf16Buffer(length);
		UErrorCode error = U_ZERO_ERROR;
		unicodeStr.extract(utf16Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-16 string: " << u_errorName(error) << std::endl;
			return std::wstring();
		}
		// Construct std::wstring from UTF-16 buffer
		std::wstring wstr(utf16Buffer.begin(), utf16Buffer.end());
#endif

		return wstr;
	}

	std::u16string str_utils::to_u16string(const std::string& str, const std::string& encoding)
	{
		// Create a Unicode converter for the given encoding
		UErrorCode error = U_ZERO_ERROR;
		UConverter* converter = ucnv_open(encoding.c_str(), &error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to create converter: " << u_errorName(error) << std::endl;
			return std::u16string();
		}

		// Convert the input string to UnicodeString
		icu::UnicodeString unicodeStr(str.c_str(), str.length(), converter, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to convert string: " << u_errorName(error) << std::endl;
			ucnv_close(converter);
			return std::u16string();
		}

		// Close the converter
		ucnv_close(converter);

		// Convert UnicodeString to UTF-16
		int32_t length = unicodeStr.length();
		std::vector<UChar> utf16Buffer(length);
		unicodeStr.extract(utf16Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-16 string: " << u_errorName(error) << std::endl;
			return std::u16string();
		}

		// Construct std::u16string from UTF-16 buffer
		std::u16string u16str(reinterpret_cast<const char16_t*>(utf16Buffer.data()), utf16Buffer.size());

		return u16str;
	}

	std::u16string str_utils::to_u16string(const std::wstring& str)
	{
		// Convert std::wstring to ICU UnicodeString
#if defined(__linux__) || defined(__APPLE__)
	// Linux: wchar_t is UTF-32
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar32*>(str.c_str()), str.length());
#else
	// Windows: wchar_t is UTF-16
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str.c_str()), str.length());
#endif

		// Convert UnicodeString to UTF-16
		int32_t length = unicodeStr.length();
		std::vector<UChar> utf16Buffer(length);
		UErrorCode error = U_ZERO_ERROR;
		unicodeStr.extract(utf16Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-16 string: " << u_errorName(error) << std::endl;
			return std::u16string();
		}

		// Construct std::u16string from UTF-16 buffer
		std::u16string u16str(reinterpret_cast<const char16_t*>(utf16Buffer.data()), utf16Buffer.size());

		return u16str;
	}

	std::u16string str_utils::to_u16string(const char* str, const std::string& encoding)
	{
		// Create a Unicode converter for the given encoding
		UErrorCode error = U_ZERO_ERROR;
		UConverter* converter = ucnv_open(encoding.c_str(), &error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to create converter: " << u_errorName(error) << std::endl;
			return std::u16string();
		}

		// Convert the input string to UnicodeString
		icu::UnicodeString unicodeStr(str, -1, converter, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to convert string: " << u_errorName(error) << std::endl;
			ucnv_close(converter);
			return std::u16string();
		}

		// Close the converter
		ucnv_close(converter);

		// Convert UnicodeString to UTF-16
		int32_t length = unicodeStr.length();
		std::vector<UChar> utf16Buffer(length);
		unicodeStr.extract(utf16Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-16 string: " << u_errorName(error) << std::endl;
			return std::u16string();
		}

		// Construct std::u16string from UTF-16 buffer
		std::u16string u16str(reinterpret_cast<const char16_t*>(utf16Buffer.data()), utf16Buffer.size());

		return u16str;
	}

	std::u16string str_utils::to_u16string(const wchar_t* str)
	{
		// Determine the size of wchar_t
#if defined(__linux__) || defined(__APPLE__)
	// Linux: wchar_t is UTF-32
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar32*>(str));
#else
	// Windows: wchar_t is UTF-16
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str));
#endif

		// Convert UnicodeString to UTF-16
		int32_t length = unicodeStr.length();
		std::vector<UChar> utf16Buffer(length);
		UErrorCode error = U_ZERO_ERROR;
		unicodeStr.extract(utf16Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-16 string: " << u_errorName(error) << std::endl;
			return std::u16string();
		}

		// Construct std::u16string from UTF-16 buffer
		std::u16string u16str(reinterpret_cast<const char16_t*>(utf16Buffer.data()), utf16Buffer.size());

		return u16str;
	}

	std::string str_utils::to_string(const std::wstring& str)
	{
#if defined(__linux__) || defined(__APPLE__)
		// Linux: wchar_t is UTF-32
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar32*>(str.c_str()), str.length());
#else
		// Windows: wchar_t is UTF-16
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str.c_str()), str.length());
#endif

		// Convert UnicodeString to UTF-8
		std::string utf8Str;
		unicodeStr.toUTF8String(utf8Str);

		return utf8Str;
	}

	std::string str_utils::to_string(const std::u16string& str)
	{
		// Convert std::u16string to ICU UnicodeString
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str.c_str()), str.length());

		// Convert UnicodeString to UTF-8
		std::string utf8Str;
		unicodeStr.toUTF8String(utf8Str);

		return utf8Str;
	}

	std::string str_utils::to_string(const wchar_t* str)
	{
		// Determine the size of wchar_t
#if defined(__linux__)
	// Linux: wchar_t is UTF-32
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar32*>(str));
#else
	// Windows: wchar_t is UTF-16
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str));
#endif

		// Convert UnicodeString to UTF-8
		std::string utf8Str;
		unicodeStr.toUTF8String(utf8Str);

		return utf8Str;
	}

	std::string str_utils::to_string(const char16_t* str)
	{
		// Convert const char16_t* to ICU UnicodeString
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str));

		// Convert UnicodeString to UTF-8
		std::string utf8Str;
		unicodeStr.toUTF8String(utf8Str);

		return utf8Str;
	}

	icu::UnicodeString str_utils::to_UnicodeString(const std::string& str, const std::string& encoding)
	{
		// Create a Unicode converter for the given encoding
		UErrorCode error = U_ZERO_ERROR;
		UConverter* converter = ucnv_open(encoding.c_str(), &error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to create converter: " << u_errorName(error) << std::endl;
			return icu::UnicodeString();
		}

		// Convert the input string to UnicodeString
		icu::UnicodeString unicodeStr(str.c_str(), str.length(), converter, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to convert string: " << u_errorName(error) << std::endl;
			ucnv_close(converter);
			return icu::UnicodeString();
		}

		// Close the converter
		ucnv_close(converter);

		return unicodeStr;
	}

	icu::UnicodeString str_utils::to_UnicodeString(const std::wstring& str)
	{
		// Determine the size of wchar_t
#if defined(__linux__) || defined(__APPLE__)
	// Linux: wchar_t is UTF-32
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar32*>(str.c_str()), str.length());
#else
	// Windows: wchar_t is UTF-16
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str.c_str()), str.length());
#endif

		return unicodeStr;
	}

	icu::UnicodeString str_utils::to_UnicodeString(const std::u16string& str)
	{
		// Convert std::u16string to ICU UnicodeString
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str.c_str()), str.length());
		return unicodeStr;
	}

	icu::UnicodeString str_utils::to_UnicodeString(const char* str, const std::string& encoding)
	{
		// Create a Unicode converter for the given encoding
		UErrorCode error = U_ZERO_ERROR;
		UConverter* converter = ucnv_open(encoding.c_str(), &error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to create converter: " << u_errorName(error) << std::endl;
			return icu::UnicodeString();
		}

		// Convert the input string to UnicodeString
		icu::UnicodeString unicodeStr(str, -1, converter, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to convert string: " << u_errorName(error) << std::endl;
			ucnv_close(converter);
			return icu::UnicodeString();
		}

		// Close the converter
		ucnv_close(converter);

		return unicodeStr;
	}

	icu::UnicodeString str_utils::to_UnicodeString(const wchar_t* str)
	{
		// Determine the size of wchar_t
#if defined(__linux__) || defined(__APPLE__)
	// Linux and macOS: wchar_t is UTF-32
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar32*>(str), -1);
#else
	// Windows: wchar_t is UTF-16
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str), -1);
#endif

		return unicodeStr;
	}

	icu::UnicodeString str_utils::to_UnicodeString(const char16_t* str)
	{
		// Convert const char16_t* to ICU UnicodeString
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str), -1);
		return unicodeStr;
	}

	std::wstring str_utils::to_wstring(icu::UnicodeString& unicodeStr)
	{
		UErrorCode error = U_ZERO_ERROR;
		// Determine the size of wchar_t
#if defined(__linux__) || defined(__APPLE__)
	// Linux: wchar_t is UTF-32
		int32_t length = unicodeStr.length();
		std::vector<UChar32> utf32Buffer(length);
		unicodeStr.toUTF32(utf32Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-32 string: " << u_errorName(error) << std::endl;
			return std::wstring();
		}
		// Construct std::wstring from UTF-32 buffer
		std::wstring wstr(utf32Buffer.begin(), utf32Buffer.end());
#else
	// Windows: wchar_t is UTF-16
		int32_t length = unicodeStr.length();
		std::vector<UChar> utf16Buffer(length);
		unicodeStr.extract(utf16Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-16 string: " << u_errorName(error) << std::endl;
			return std::wstring();
		}
		// Construct std::wstring from UTF-16 buffer
		std::wstring wstr(utf16Buffer.begin(), utf16Buffer.end());
#endif

		return wstr;

	}

	std::u16string str_utils::to_u16string(icu::UnicodeString& unicodeStr)
	{
		UErrorCode error = U_ZERO_ERROR;
		// Convert UnicodeString to UTF-16
		int32_t length = unicodeStr.length();
		std::vector<UChar> utf16Buffer(length);
		unicodeStr.extract(utf16Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-16 string: " << u_errorName(error) << std::endl;
			return std::u16string();
		}

		// Construct std::u16string from UTF-16 buffer
		std::u16string u16str(reinterpret_cast<const char16_t*>(utf16Buffer.data()), utf16Buffer.size());

		return u16str;
	}

	std::string str_utils::to_string(icu::UnicodeString& unicodeStr)
	{
		// Convert UnicodeString to UTF-8
		std::string utf8Str;
		unicodeStr.toUTF8String(utf8Str);

		return utf8Str;
	}