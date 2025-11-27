#include "pch.h"
#include "str_utils.h"
#include <limits>
#include <vector>
#include <algorithm>

#if defined(_WIN32)
#include <windows.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#else
#include <iconv.h>
#include <cstring>
#endif

std::wstring str_utils::to_wstring(const std::string& str, const std::string& encoding) {
#if defined(_WIN32)
	UINT codePage;


	//std::string lowerEncoding = encoding;
	//std::transform(lowerEncoding.cbegin(), lowerEncoding.cend(), lowerEncoding.cbegin(),
	//	[]{ return std::tolower(c); });

	if (encoding == "utf-8" || encoding == "UTF-8") codePage = CP_UTF8;
	else if (encoding == "windows-1251" || encoding == "WINDOWS-1251") codePage = 1251;
	else if (encoding == "windows-1252" || encoding == "WINDOWS-1252") codePage = 1252;
	else if (encoding == "iso-8859-1" || encoding == "ISO-8859-1") codePage = 28591;
	else throw std::invalid_argument("Unsupported encoding: " + encoding);

	int wlen = MultiByteToWideChar(codePage, 0, str.c_str(), -1, nullptr, 0);
	if (wlen == 0)
		throw std::runtime_error("MultiByteToWideChar failed");

	std::wstring wstr(wlen, L'\0');
	if (!MultiByteToWideChar(codePage, 0, str.c_str(), -1, &wstr[0], wlen))
		throw std::runtime_error("MultiByteToWideChar failed during conversion");

	wstr.pop_back();  // remove null terminator
	return wstr;

#else
	// POSIX version using iconv
	iconv_t cd = iconv_open("WCHAR_T", encoding.c_str());  // Convert from encoding to wchar_t
	if (cd == (iconv_t)-1)
		throw std::runtime_error("iconv_open failed for encoding: " + encoding);

	size_t in_size = str.size();
	size_t out_size = in_size * sizeof(wchar_t) * 2;

	std::wstring wstr;
	wstr.resize(out_size / sizeof(wchar_t));

	char* in_buf = const_cast<char*>(str.data());
	char* out_buf = reinterpret_cast<char*>(&wstr[0]);
	size_t in_bytes_left = in_size;
	size_t out_bytes_left = out_size;

	size_t result = iconv(cd, &in_buf, &in_bytes_left, &out_buf, &out_bytes_left);
	iconv_close(cd);

	if (result == (size_t)-1)
		throw std::runtime_error("iconv conversion failed");

	size_t converted_chars = (out_size - out_bytes_left) / sizeof(wchar_t);
	wstr.resize(converted_chars);

	return wstr;
#endif
}

std::wstring str_utils::to_wstring(const std::u16string& str)
{
	std::wstring result;

#if WCHAR_MAX >= 0x10FFFF  // Typically true on Linux/macOS (wchar_t is 4 bytes)
	// Direct 1-to-1 copy for BMP and surrogate decoding for others
	size_t i = 0;
	while (i < str.size()) {
		char16_t ch = str[i++];

		if (ch >= 0xD800 && ch <= 0xDBFF) { // high surrogate
			if (i < str.size()) {
				char16_t ch2 = str[i++];
				if (ch2 >= 0xDC00 && ch2 <= 0xDFFF) {
					uint32_t codepoint = ((ch - 0xD800) << 10) + (ch2 - 0xDC00) + 0x10000;
					result += static_cast<wchar_t>(codepoint);
				}
				else {
					throw std::runtime_error("Invalid UTF-16 sequence: expected low surrogate");
				}
			}
			else {
				throw std::runtime_error("Truncated UTF-16 sequence after high surrogate");
			}
		}
		else if (ch >= 0xDC00 && ch <= 0xDFFF) {
			throw std::runtime_error("Unexpected low surrogate without preceding high surrogate");
		}
		else {
			result += static_cast<wchar_t>(ch);
		}
	}

#else  // WCHAR_MAX <= 0xFFFF (usually Windows)
	// On Windows, wchar_t is 16-bit; we assume u16string matches layout
	result.assign(str.begin(), str.end());

#endif

	return result;
}

std::wstring str_utils::to_wstring(const char* str, const std::string& encoding)
{
	if (!str)
		return {};

#if defined(_WIN32)
	// Windows: use MultiByteToWideChar with code page
	UINT codePage;

	if (encoding == "utf-8" || encoding == "UTF-8") codePage = CP_UTF8;
	else if (encoding == "windows-1251" || encoding == "WINDOWS-1251") codePage = 1251;
	else if (encoding == "windows-1252" || encoding == "WINDOWS-1252") codePage = 1252;
	else if (encoding == "iso-8859-1" || encoding == "ISO-8859-1") codePage = 28591;
	else throw std::invalid_argument("Unsupported encoding: " + encoding);

	int wlen = MultiByteToWideChar(codePage, 0, str, -1, nullptr, 0);
	if (wlen == 0)
		throw std::runtime_error("MultiByteToWideChar failed");

	std::wstring wstr(wlen, L'\0');
	if (!MultiByteToWideChar(codePage, 0, str, -1, &wstr[0], wlen))
		throw std::runtime_error("MultiByteToWideChar conversion failed");

	wstr.pop_back(); // remove null terminator
	return wstr;

#else
	// POSIX: use iconv to convert from encoding to wchar_t
	iconv_t cd = iconv_open("WCHAR_T", encoding.c_str()); // to wchar_t from encoding
	if (cd == (iconv_t)-1)
		throw std::runtime_error("iconv_open failed for encoding: " + encoding);

	size_t in_bytes = strlen(str);
	size_t out_bytes = in_bytes * sizeof(wchar_t) * 2;

	std::wstring wstr;
	wstr.resize(out_bytes / sizeof(wchar_t));

	char* in_buf = const_cast<char*>(str);
	char* out_buf = reinterpret_cast<char*>(&wstr[0]);
	size_t in_left = in_bytes;
	size_t out_left = out_bytes;

	size_t result = iconv(cd, &in_buf, &in_left, &out_buf, &out_left);
	iconv_close(cd);

	if (result == (size_t)-1)
		throw std::runtime_error("iconv conversion failed");

	size_t converted = (out_bytes - out_left) / sizeof(wchar_t);
	wstr.resize(converted);

	return wstr;
#endif
}

std::wstring str_utils::to_wstring(const char16_t* str)
{
	if (!str) return {};

	std::wstring result;

#if WCHAR_MAX >= 0x10FFFF  // wchar_t is 4 bytes (Linux/macOS)
	// Decode UTF-16 surrogate pairs into 32-bit wchar_t
	while (*str) {
		char16_t ch = *str++;

		if (ch >= 0xD800 && ch <= 0xDBFF) { // high surrogate
			char16_t ch2 = *str;
			if (ch2 >= 0xDC00 && ch2 <= 0xDFFF) {
				++str;
				uint32_t codepoint = ((ch - 0xD800) << 10) + (ch2 - 0xDC00) + 0x10000;
				result += static_cast<wchar_t>(codepoint);
			}
			else {
				throw std::runtime_error("Invalid UTF-16: expected low surrogate after high surrogate");
			}
		}
		else if (ch >= 0xDC00 && ch <= 0xDFFF) {
			throw std::runtime_error("Invalid UTF-16: unexpected low surrogate");
		}
		else {
			result += static_cast<wchar_t>(ch);
		}
	}

#else  // wchar_t is 2 bytes (Windows)
	// Direct copy from UTF-16 to wchar_t (same layout)
	while (*str) {
		result += static_cast<wchar_t>(*str++);
	}
#endif

	return result;
}

std::wstring str_utils::to_wstring(const int value)
{
	return std::to_wstring(value);
}

std::wstring str_utils::to_wstring(const size_t value)
{
	return std::to_wstring(value);
}

std::wstring str_utils::to_wstring(const long value)
{
	return std::to_wstring(value);
}

std::u16string str_utils::to_u16string(const std::string& str, const std::string& encoding)
{
	if (str.empty()) return {};

#if defined(_WIN32)
	// Step 1: convert to UTF-16 using MultiByteToWideChar
	UINT codePage;

	if (encoding == "utf-8" || encoding == "UTF-8") codePage = CP_UTF8;
	else if (encoding == "windows-1251" || encoding == "WINDOWS-1251") codePage = 1251;
	else if (encoding == "windows-1252" || encoding == "WINDOWS-1252") codePage = 1252;
	else if (encoding == "iso-8859-1" || encoding == "ISO-8859-1") codePage = 28591;
	else throw std::invalid_argument("Unsupported encoding: " + encoding);

	int len = MultiByteToWideChar(codePage, 0, str.c_str(), -1, nullptr, 0);
	if (len <= 0) throw std::runtime_error("MultiByteToWideChar failed");

	std::wstring wstr(len, L'\0');
	MultiByteToWideChar(codePage, 0, str.c_str(), -1, &wstr[0], len);
	wstr.pop_back(); // remove null terminator

	// Convert wstring to u16string
	return std::u16string(wstr.begin(), wstr.end());

#else
	// POSIX systems: use iconv to convert to UTF-16LE
	iconv_t cd = iconv_open("UTF-16LE", encoding.c_str());
	if (cd == (iconv_t)-1) {
		throw std::runtime_error("iconv_open failed for encoding: " + encoding);
	}

	size_t in_bytes = str.size();
	size_t out_bytes = in_bytes * 4; // max possible size in UTF-16
	std::vector<char> outbuf(out_bytes);

	char* in_ptr = const_cast<char*>(str.data());
	char* out_ptr = outbuf.data();
	size_t in_left = in_bytes;
	size_t out_left = out_bytes;

	size_t result = iconv(cd, &in_ptr, &in_left, &out_ptr, &out_left);
	iconv_close(cd);

	if (result == (size_t)-1)
		throw std::runtime_error("iconv conversion failed");

	size_t converted_bytes = out_bytes - out_left;
	std::u16string out_str(reinterpret_cast<char16_t*>(outbuf.data()), converted_bytes / 2);
	return out_str;
#endif
}

std::u16string str_utils::to_u16string(const std::wstring& str)
{
	std::u16string result;

	for (wchar_t wc : str) {
		uint32_t code = static_cast<uint32_t>(wc);

		if (code <= 0xFFFF) {
			result.push_back(static_cast<char16_t>(code));
		}
		else if (code <= 0x10FFFF) {
			code -= 0x10000;
			char16_t high_surrogate = 0xD800 + (code >> 10);
			char16_t low_surrogate = 0xDC00 + (code & 0x3FF);
			result.push_back(high_surrogate);
			result.push_back(low_surrogate);
		}
		else {
			throw std::runtime_error("Invalid Unicode code point in std::wstring");
		}
	}

	return result;
}

std::u16string str_utils::to_u16string(const char* str, const std::string& encoding)
{
	if (!str || !*str) return {};

#if defined(_WIN32)
	// Convert multibyte input to UTF-16 using Windows API
	UINT codePage;

	if (encoding == "utf-8" || encoding == "UTF-8") codePage = CP_UTF8;
	else if (encoding == "windows-1251" || encoding == "WINDOWS-1251") codePage = 1251;
	else if (encoding == "windows-1252" || encoding == "WINDOWS-1252") codePage = 1252;
	else if (encoding == "iso-8859-1" || encoding == "ISO-8859-1") codePage = 28591;
	else throw std::invalid_argument("Unsupported encoding: " + encoding);

	int len = MultiByteToWideChar(codePage, 0, str, -1, nullptr, 0);
	if (len <= 0)
		throw std::runtime_error("MultiByteToWideChar failed");

	std::wstring wstr(len, L'\0');
	if (!MultiByteToWideChar(codePage, 0, str, -1, &wstr[0], len))
		throw std::runtime_error("MultiByteToWideChar conversion failed");

	wstr.pop_back(); // Remove null terminator
	return std::u16string(wstr.begin(), wstr.end());

#else
	// Convert using iconv on POSIX systems (macOS, Linux)
	iconv_t cd = iconv_open("UTF-16LE", encoding.c_str());
	if (cd == (iconv_t)-1) {
		throw std::runtime_error("iconv_open failed for encoding: " + encoding);
	}

	size_t in_bytes = strlen(str);
	size_t out_bytes = in_bytes * 4; // Overestimate UTF-16 size
	std::vector<char> outbuf(out_bytes);

	char* in_ptr = const_cast<char*>(str);
	char* out_ptr = outbuf.data();
	size_t in_left = in_bytes;
	size_t out_left = out_bytes;

	size_t result = iconv(cd, &in_ptr, &in_left, &out_ptr, &out_left);
	iconv_close(cd);

	if (result == (size_t)-1)
		throw std::runtime_error("iconv conversion failed");

	size_t converted_bytes = out_bytes - out_left;
	return std::u16string(reinterpret_cast<char16_t*>(outbuf.data()), converted_bytes / 2);
#endif
}

std::u16string str_utils::to_u16string(const wchar_t* str)
{
	std::u16string result;

	if (!str) return result;

	while (*str) {
		uint32_t code = static_cast<uint32_t>(*str);

		if (code <= 0xFFFF) {
			result.push_back(static_cast<char16_t>(code));
		}
		else if (code <= 0x10FFFF) {
			code -= 0x10000;
			char16_t high_surrogate = 0xD800 + (code >> 10);
			char16_t low_surrogate = 0xDC00 + (code & 0x3FF);
			result.push_back(high_surrogate);
			result.push_back(low_surrogate);
		}
		else {
			throw std::runtime_error("Invalid Unicode code point in wchar_t string");
		}

		++str;
	}

	return result;
}

std::string str_utils::to_string(const std::wstring& str)
{
	std::string result;
	result.reserve(str.size());  // rough estimate

	for (wchar_t wc : str) {
		uint32_t codepoint = static_cast<uint32_t>(wc);

#if WCHAR_MAX > 0xFFFF
		// On Linux/macOS where wchar_t is 32-bit, may include codepoints > 0xFFFF
		if (codepoint > 0x10FFFF)
			throw std::runtime_error("Invalid Unicode code point in wchar_t");
#else
		// On Windows, wchar_t is 16-bit, and surrogate pairs may be pre-split
		if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
			throw std::runtime_error("Invalid surrogate in wchar_t stream");
#endif

		// Encode as UTF-8
		if (codepoint <= 0x7F) {
			result += static_cast<char>(codepoint);
		}
		else if (codepoint <= 0x7FF) {
			result += static_cast<char>(0xC0 | (codepoint >> 6));
			result += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
		else if (codepoint <= 0xFFFF) {
			result += static_cast<char>(0xE0 | (codepoint >> 12));
			result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
			result += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
		else {
			result += static_cast<char>(0xF0 | (codepoint >> 18));
			result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
			result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
			result += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
	}

	return result;
}

std::string str_utils::to_string(const std::u16string& str)
{
	std::string result;
	result.reserve(str.size());  // a rough reserve

	for (size_t i = 0; i < str.size(); ++i) {
		uint32_t codepoint;
		char16_t ch = str[i];

		// High-surrogate?
		if (ch >= 0xD800 && ch <= 0xDBFF) {
			if (i + 1 >= str.size())
				throw std::runtime_error("Invalid UTF-16: high surrogate without low surrogate");

			char16_t ch2 = str[i + 1];
			if (ch2 < 0xDC00 || ch2 > 0xDFFF)
				throw std::runtime_error("Invalid UTF-16: expected low surrogate after high surrogate");

			// combine into codepoint
			codepoint = (((uint32_t)(ch - 0xD800) << 10)
				| (uint32_t)(ch2 - 0xDC00))
				+ 0x10000;
			++i;  // consume low surrogate
		}
		// Unexpected low-surrogate?
		else if (ch >= 0xDC00 && ch <= 0xDFFF) {
			throw std::runtime_error("Invalid UTF-16: unexpected low surrogate");
		}
		// BMP codepoint
		else {
			codepoint = ch;
		}

		// now encode codepoint into UTF-8
		if (codepoint <= 0x7F) {
			result += static_cast<char>(codepoint);
		}
		else if (codepoint <= 0x7FF) {
			result += static_cast<char>(0xC0 | (codepoint >> 6));
			result += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
		else if (codepoint <= 0xFFFF) {
			result += static_cast<char>(0xE0 | (codepoint >> 12));
			result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
			result += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
		else {
			result += static_cast<char>(0xF0 | (codepoint >> 18));
			result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
			result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
			result += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
	}

	return result;
}

std::string str_utils::to_string(const wchar_t* str)
{
	if (!str) return {};

#if defined(_WIN32)
	// Convert wide string (UTF-16) to UTF-8
	int len = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
	if (len <= 0) {
		throw std::runtime_error("WideCharToMultiByte failed");
	}

	std::string result(len - 1, '\0'); // exclude null terminator
	WideCharToMultiByte(CP_UTF8, 0, str, -1, result.data(), len, nullptr, nullptr);
	return result;

#else
	// On Linux/macOS, wchar_t is typically UTF-32
	std::u32string u32str(reinterpret_cast<const char32_t*>(str));

	// Convert UTF-32 to UTF-8 using iconv
	iconv_t cd = iconv_open("UTF-8", "UTF-32LE"); // or "UTF-32" depending on platform endianness
	if (cd == (iconv_t)-1) {
		throw std::runtime_error("iconv_open failed");
	}

	size_t in_bytes = u32str.size() * sizeof(char32_t);
	size_t out_bytes = in_bytes * 4; // UTF-8 max expansion
	std::vector<char> outbuf(out_bytes);

	char* in_ptr = reinterpret_cast<char*>(u32str.data());
	char* out_ptr = outbuf.data();
	size_t in_left = in_bytes;
	size_t out_left = out_bytes;

	size_t res = iconv(cd, &in_ptr, &in_left, &out_ptr, &out_left);
	iconv_close(cd);

	if (res == (size_t)-1) {
		throw std::runtime_error("iconv conversion failed");
	}

	return std::string(outbuf.data(), out_bytes - out_left);
#endif
}

std::string str_utils::to_string(const char16_t* str)
{
	std::string result;

	while (*str) {
		uint32_t codepoint;

		char16_t ch = *str++;

		// Check for surrogate pair
		if (ch >= 0xD800 && ch <= 0xDBFF) {
			char16_t ch2 = *str;
			if (ch2 >= 0xDC00 && ch2 <= 0xDFFF) {
				++str;
				codepoint = (((ch - 0xD800) << 10) | (ch2 - 0xDC00)) + 0x10000;
			}
			else {
				throw std::runtime_error("Invalid UTF-16: expected low surrogate after high surrogate");
			}
		}
		else if (ch >= 0xDC00 && ch <= 0xDFFF) {
			throw std::runtime_error("Invalid UTF-16: unexpected low surrogate");
		}
		else {
			codepoint = ch;
		}

		// Encode UTF-8
		if (codepoint <= 0x7F) {
			result += static_cast<char>(codepoint);
		}
		else if (codepoint <= 0x7FF) {
			result += static_cast<char>(0xC0 | (codepoint >> 6));
			result += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
		else if (codepoint <= 0xFFFF) {
			result += static_cast<char>(0xE0 | (codepoint >> 12));
			result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
			result += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
		else {
			result += static_cast<char>(0xF0 | (codepoint >> 18));
			result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
			result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
			result += static_cast<char>(0x80 | (codepoint & 0x3F));
		}
	}

	return result;
}

bool str_utils::iequals(const std::wstring& a, const std::wstring& b) noexcept
{
#ifdef _WIN32
	// Unicode-aware, ordinal, case-insensitive
	return CompareStringOrdinal(
		a.c_str(), static_cast<int>(a.size()),
		b.c_str(), static_cast<int>(b.size()),
		TRUE) == CSTR_EQUAL;
#else
	if (a.size() != b.size()) return false;
	const auto& facet = std::use_facet<std::ctype<wchar_t>>(std::locale());
	for (size_t i = 0; i < a.size(); ++i) {
		if (facet.tolower(a[i]) != facet.tolower(b[i])) return false;
	}
	return true;
#endif
}

bool str_utils::iequals(const std::string& a, const std::string& b) noexcept
{
	if (a.size() != b.size())
		return false;

	return std::equal(a.begin(), a.end(), b.begin(),
		[](unsigned char c1, unsigned char c2) {
			return std::tolower(c1) == std::tolower(c2);
		});
}

std::optional<uint32_t> str_utils::to_UInt(const std::wstring& value)
{
	// Trim whitespace
	const auto first = value.find_first_not_of(L" \t\r\n");
	if (first == std::wstring::npos) return std::nullopt;
	const auto last = value.find_last_not_of(L" \t\r\n");

	uint32_t result = 0;
	for (size_t i = first; i <= last; ++i) {
		const wchar_t ch = value[i];
		if (ch < L'0' || ch > L'9') {
			return std::nullopt; // non-digit found
		}
		const uint32_t digit = static_cast<uint32_t>(ch - L'0');
		if (result > (std::numeric_limits<uint32_t>::max() - digit) / 10u) {
			return std::nullopt; // overflow
		}
		result = result * 10u + digit;
	}
	return result;
}

std::optional<double> str_utils::to_Double(const std::wstring& value)
{
	try {
		// Normalize decimal separator: accept comma by converting it to dot
		std::wstring normalized = value;
		for (auto& ch : normalized) {
			if (ch == L',') ch = L'.';
		}

		size_t idx = 0;
		double result = std::stod(normalized, &idx);
		// Ensure the whole (normalized) string was consumed
		if (idx != normalized.size()) {
			return std::nullopt;
		}
		return result;
	}
	catch (const std::invalid_argument&) {
		return std::nullopt;
	}
	catch (const std::out_of_range&) {
		return std::nullopt;
	}
}
