#include "pch.h"
#include "str_utils.h"
#include <vector>
#include <unicode/ucnv.h>

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
		
		std::wstring wstr = {};

		// Determine the size of wchar_t
#if WCHAR_MAX == UINT32_MAX
	// Linux: wchar_t is UTF-32
		int32_t length = unicodeStr.length();
		std::vector<UChar32> utf32Buffer(length);
		unicodeStr.toUTF32(utf32Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-32 string: " << u_errorName(error) << std::endl;
			return std::wstring();
		}
		// Construct std::wstring from UTF-32 buffer
		wstr = std::wstring(utf32Buffer.begin(), utf32Buffer.end());
#elif WCHAR_MAX == UINT16_MAX
	// Windows: wchar_t is UTF-16
		int32_t length = unicodeStr.length();
		std::vector<UChar> utf16Buffer(length);
		unicodeStr.extract(utf16Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-16 string: " << u_errorName(error) << std::endl;
			return std::wstring();
		}
		// Construct std::wstring from UTF-16 buffer
		wstr = std::wstring(utf16Buffer.begin(), utf16Buffer.end());
#else
		// Unsupported platform
		#error "Unsupported platform: wchar_t size is not 16-bit or 32-bit"
#endif
		return wstr;
	}

	std::wstring str_utils::to_wstring(const std::u16string& str)
	{
		// Convert std::u16string to ICU UnicodeString
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str.c_str()), str.length());

		std::wstring wstr = {};
		// Determine the size of wchar_t
#if WCHAR_MAX == INT32_MAX
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
		wstr = std::wstring(utf32Buffer.begin(), utf32Buffer.end());
#elif WCHAR_MAX == UINT16_MAX
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
		wstr = std::wstring(utf16Buffer.begin(), utf16Buffer.end());
#else
		// Unsupported platform
		#error "Unsupported platform: wchar_t size is not 16-bit or 32-bit"
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

		std::wstring wstr = {};
		// Determine the size of wchar_t
#if WCHAR_MAX == UINT32_MAX
	// Linux: wchar_t is UTF-32
		int32_t length = unicodeStr.length();
		std::vector<UChar32> utf32Buffer(length);
		unicodeStr.toUTF32(utf32Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-32 string: " << u_errorName(error) << std::endl;
			return std::wstring();
		}
		// Construct std::wstring from UTF-32 buffer
		wstr = std::wstring(utf32Buffer.begin(), utf32Buffer.end());
#elif WCHAR_MAX == UINT16_MAX
	// Windows: wchar_t is UTF-16
		int32_t length = unicodeStr.length();
		std::vector<UChar> utf16Buffer(length);
		unicodeStr.extract(utf16Buffer.data(), length, error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to extract UTF-16 string: " << u_errorName(error) << std::endl;
			return std::wstring();
		}
		// Construct std::wstring from UTF-16 buffer
		wstr = std::wstring(utf16Buffer.begin(), utf16Buffer.end());
#else
		// Unsupported platform
		#error "Unsupported platform: wchar_t size is not 16-bit or 32-bit"
#endif

		return wstr;
	}

	std::wstring str_utils::to_wstring(const char16_t* str)
	{
		// Convert const char16_t* to ICU UnicodeString
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str));

		std::wstring wstr = {};
		// Determine the size of wchar_t
#if WCHAR_MAX == UINT32_MAX
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
		wstr = std::wstring(utf32Buffer.begin(), utf32Buffer.end());
#elif WCHAR_MAX == UINT16_MAX
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
		wstr = std::wstring(utf16Buffer.begin(), utf16Buffer.end());
#else
		// Unsupported platform
		#error "Unsupported platform: wchar_t size is not 16-bit or 32-bit"
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
#if WCHAR_MAX == UINT32_MAX
		// Linux: wchar_t is UTF-32
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str.c_str()), str.length());
#elif WCHAR_MAX == UINT16_MAX
		// Windows: wchar_t is UTF-16
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str.c_str()), str.length());
#else
		// Unsupported platform
		#error "Unsupported platform: wchar_t size is not 16-bit or 32-bit"
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
#if WCHAR_MAX == UINT32_MAX
		// Linux: wchar_t is UTF-32
		auto length = std::wcslen(str);
		icu::UnicodeString unicodeStr = icu::UnicodeString::fromUTF32(reinterpret_cast<const UChar32*>(str), static_cast<int32_t>(length));
#elif WCHAR_MAX == UINT16_MAX
	// Windows: wchar_t is UTF-16
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str));
#else
		// Unsupported platform
		#error "Unsupported platform: wchar_t size is not 16-bit or 32-bit"
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
#if WCHAR_MAX == UINT32_MAX
		// Linux: wchar_t is UTF-32
		icu::UnicodeString unicodeStr = icu::UnicodeString::fromUTF32(reinterpret_cast<const UChar32*>(str.c_str()), static_cast<int32_t>(str.length()));
#elif WCHAR_MAX == UINT16_MAX
		// Windows: wchar_t is UTF-16
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str.c_str()), str.length());
#else
		// Unsupported platform
		#error "Unsupported platform: wchar_t size is not 16-bit or 32-bit"
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
#if WCHAR_MAX == UINT32_MAX
		// Linux: wchar_t is UTF-32
		auto length = std::wcslen(str);
		icu::UnicodeString unicodeStr = icu::UnicodeString::fromUTF32(reinterpret_cast<const UChar32*>(str), static_cast<int32_t>(length));
#elif WCHAR_MAX == UINT16_MAX
	// Windows: wchar_t is UTF-16
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str));
#else
		// Unsupported platform
		#error "Unsupported platform: wchar_t size is not 16-bit or 32-bit"
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
#if WCHAR_MAX == UINT32_MAX
		// Linux: wchar_t is UTF-32
		icu::UnicodeString unicodeStr = icu::UnicodeString::fromUTF32(reinterpret_cast<const UChar32*>(str.c_str()), static_cast<int32_t>(str.length()));
#elif WCHAR_MAX == UINT16_MAX
		// Windows: wchar_t is UTF-16
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str.c_str()), str.length());
#else
		// Unsupported platform
		#error "Unsupported platform: wchar_t size is not 16-bit or 32-bit"
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
#if WCHAR_MAX == UINT32_MAX
		// Linux: wchar_t is UTF-32
		auto length = std::wcslen(str);
		icu::UnicodeString unicodeStr = icu::UnicodeString::fromUTF32(reinterpret_cast<const UChar32*>(str), static_cast<int32_t>(length));
#elif WCHAR_MAX == UINT16_MAX
		// Windows: wchar_t is UTF-16
		icu::UnicodeString unicodeStr(reinterpret_cast<const UChar*>(str));
#else
		// Unsupported platform
		#error "Unsupported platform: wchar_t size is not 16-bit or 32-bit"
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
#if WCHAR_MAX == UINT32_MAX
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
#elif WCHAR_MAX == UINT16_MAX
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
#else
		// Unsupported platform
		#error "Unsupported platform: wchar_t size is not 16-bit or 32-bit"
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

	std::unordered_map<std::string, UConverter*> str_utils::strConverter::list_converter;

	std::wstring str_utils::strConverter::to_wstring(const std::string& str, const std::string& encoding)
	{
#if WCHAR_MAX == UINT32_MAX
		auto convTo = get_converter("UTF-32");
#elif WCHAR_MAX == UINT16_MAX
		auto convTo = get_converter("UTF-16"); 
#else
		// Unsupported platform
#error "Unsupported platform: wchar_t size is not 16-bit or 32-bit"
#endif
		if (!convTo) {
			return std::wstring();
		}

		auto convFrom = get_converter(encoding);
		if (!convFrom) {
			return std::wstring();
		}

		UErrorCode error = U_ZERO_ERROR;
		// Calculate the length of the UTF-16 buffer
		int32_t utf16Length = ucnv_toUChars(convFrom, nullptr, 0, str.c_str(), str.length(), &error);
		if (error != U_BUFFER_OVERFLOW_ERROR) {
			std::cerr << "Failed to calculate UTF-16 length: " << u_errorName(error) << std::endl;
			ucnv_close(convFrom);
			return std::wstring();
		}

		error = U_ZERO_ERROR;
		std::vector<UChar> utf16Buffer(utf16Length + 1);
		ucnv_toUChars(convFrom, utf16Buffer.data(), utf16Length + 1, str.c_str(), str.length(), &error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to convert to UTF-16: " << u_errorName(error) << std::endl;
			ucnv_close(convFrom);
			return std::wstring();
		}

		std::wstring destBuffer = {};
#if WCHAR_MAX == UINT32_MAX
		// Convert UTF-16 buffer to UTF-32
		int32_t utf32Length = ucnv_fromUChars(convTo, nullptr, 0, utf16Buffer.data(), utf16Length, &error);
		if (error != U_BUFFER_OVERFLOW_ERROR) {
			std::cerr << "Failed to calculate UTF-32 length: " << u_errorName(error) << std::endl;
			ucnv_close(convTo);
			return std::wstring();
		}

		error = U_ZERO_ERROR;
		std::vector<char> utf32Buffer(utf32Length + 1);
		ucnv_fromUChars(convTo, utf32Buffer.data(), utf32Length + 1, utf16Buffer.data(), utf16Length, &error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to convert to UTF-32: " << u_errorName(error) << std::endl;
			ucnv_close(convTo);
			return std::wstring();
		}

		// Construct std::wstring from UTF-32 buffer
		destBuffer = std::wstring(reinterpret_cast<const wchar_t*>(utf32Buffer.data()), utf32Length / sizeof(wchar_t));
#elif WCHAR_MAX == UINT16_MAX
		// Construct std::wstring from UTF-16 buffer
		destBuffer = std::wstring(reinterpret_cast<const wchar_t*>(utf16Buffer.data()), utf16Length);
#else
		// Unsupported platform
		#error "Unsupported platform: wchar_t size is not 16-bit or 32-bit"
#endif
		return destBuffer;
	}

	std::string str_utils::strConverter::to_string(const std::wstring& str, const std::string& toCoding)
	{
		UErrorCode error = U_ZERO_ERROR;

#if WCHAR_MAX == UINT32_MAX
		auto convFrom = get_converter("UTF-32");
#elif WCHAR_MAX == UINT16_MAX
		auto convFrom = get_converter("UTF-16");
#else
		// Unsupported platform
#error "Unsupported platform: wchar_t size is not 16-bit or 32-bit"
#endif
		if (!convFrom) {
			return std::string();
		}

		auto convTo = get_converter(toCoding);
		if (!convTo) {
			return std::string();
		}

		// Calculate the length of the UTF-16 buffer
		int32_t utf16Length = ucnv_toUChars(convFrom, nullptr, 0, reinterpret_cast<const char*>(str.c_str()), str.length() * sizeof(wchar_t), &error);
		if (error != U_BUFFER_OVERFLOW_ERROR) {
			std::cerr << "Failed to calculate UTF-16 length: " << u_errorName(error) << std::endl;
			ucnv_close(convFrom);
			return std::string();
		}

		error = U_ZERO_ERROR;
		std::vector<UChar> utf16Buffer(utf16Length + 1);
		ucnv_toUChars(convFrom, utf16Buffer.data(), utf16Length + 1, reinterpret_cast<const char*>(str.c_str()), str.length() * sizeof(wchar_t), &error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to convert to UTF-16: " << u_errorName(error) << std::endl;
			ucnv_close(convFrom);
			return std::string();
		}

		ucnv_close(convFrom);

		// Calculate the length of the target encoding buffer
		int32_t targetLength = ucnv_fromUChars(convTo, nullptr, 0, utf16Buffer.data(), utf16Length, &error);
		if (error != U_BUFFER_OVERFLOW_ERROR) {
			std::cerr << "Failed to calculate target encoding length: " << u_errorName(error) << std::endl;
			ucnv_close(convTo);
			return std::string();
		}

		error = U_ZERO_ERROR;
		std::vector<char> targetBuffer(targetLength + 1);
		ucnv_fromUChars(convTo, targetBuffer.data(), targetLength + 1, utf16Buffer.data(), utf16Length, &error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to convert to target encoding: " << u_errorName(error) << std::endl;
			ucnv_close(convTo);
			return std::string();
		}

		ucnv_close(convTo);

		// Construct std::string from target encoding buffer
		return std::string(targetBuffer.data(), targetLength);
	}

	void str_utils::strConverter::destroy()
	{
		for (auto& it : list_converter)
		{
			ucnv_close(it.second);
		}
		list_converter.clear();
	}

	UConverter* str_utils::strConverter::get_converter(const std::string& encoding)
	{
		auto it = list_converter.find(encoding);
		if (it != list_converter.end()) {
			return it->second;
		}

		UErrorCode error = U_ZERO_ERROR;
		auto converter = ucnv_open(encoding.c_str(), &error);
		if (U_FAILURE(error)) {
			std::cerr << "Failed to create converter: " << u_errorName(error) << std::endl;
			return nullptr;
		}

		list_converter[encoding] = converter;
		return list_converter[encoding];
	}
