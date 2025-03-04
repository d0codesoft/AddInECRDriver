#include "pch.h"
#include "license_manager.h"

#include <iostream>
#include <stdexcept>
#include <jsoncons/json.hpp>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <vector>
#include <pugixml.hpp>
#include "string_conversion.h"
#include "logger.h"

// Helper function to convert a byte vector to a base64 string
std::string Base64Encode(const std::vector<unsigned char>& data) {
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // Do not add newlines
    BIO_write(bio, data.data(), data.size());
    BIO_flush(bio);

    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string base64Str(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);
    return base64Str;
}

// Helper function to decode a base64 string to a regular string
std::string Base64Decode(const std::string& base64Str) {
    BIO* bio = BIO_new_mem_buf(base64Str.data(), base64Str.size());
    BIO* b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // Do not add newlines

    std::vector<unsigned char> buffer(base64Str.size());
    int decodedLength = BIO_read(bio, buffer.data(), buffer.size());
    BIO_free_all(bio);

    if (decodedLength < 0) {
        throw std::runtime_error("Base64 decoding failed");
    }

    return std::string(buffer.begin(), buffer.begin() + decodedLength);
}

// 📌 Public Key for Verification
const std::string LicenseManager::GetPublicKey() {
    return R"(-----BEGIN PUBLIC KEY-----
MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAuLHVj4uz1oF2RHaNbOsw
D1cDr3gtEr6+UCFzbeC/OyMgburw5WBzz/1eZOeEa8VSyCOYndmZgSHPVUWVgC4l
5HvsEZFDtWUxb8vgKjHsuHHL32nf0xKNbr0h+OxFT1h9ifJiCVGKJk3hXqqpflJu
dRDwfGwEjm42UD694nbF7pgnnlM3t6Xaj9PQvs2Z5svVhoomHFRwh5Rnayp7iafA
GZCVUxTsFQBQ4MpXnTl3bEM+WxiJyT7iYI8DCbhVaK34sQ0z0Fcs6qxdqSzw0oNi
pWpTa2FVWa/en90KQ2NSIdKzLLpJjWROgA+ASnrhGoS6sRbi0+mtNpMVZqwIeoIw
2NYfysPSNejO1Z/YYKzPnlhE+jHnSH3P1kR9S44ONTabV3tuWFXYQO4oQFHMcegq
ihhlmor/zx77dGeIcrQZwQfPDQEGLO6VBzeYIf2nR4oc93WtVv/FO1VeJwvp/mUr
sOxk7EY+bBs4g0DUigPVnKg1Okp6LPOS/4NkbH3Ikx/vwCDSjl9vnGWjHyjPxPhN
aExWo84tfTZWwZM9RdNfA+n7CMW5zm+mDTaPgzk6kaTG7VCwswxQu/9rB8zfYDkv
ZVRegWxnqZQMWozqfwYPpGdH6xVuGCxnUUKwQdvlgcZQiys5ptFqO2SSB8uTy8bp
65ZwnkcaKqXbi6qy+/zZlNECAwEAAQ==
-----END PUBLIC KEY-----)";
}

std::wstring LicenseManager::getOpenSSLErrorString() const {
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
        LOG_ERROR_ADD(L"LicenseManager", L"Failed to create BIO");
        throw std::runtime_error("Failed to create BIO");
    }

    ERR_print_errors(bio);

    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string errorString(bufferPtr->data, bufferPtr->length);
    BIO_free(bio);

    return convertStringToWString(errorString);
}

// 📌 Loading the public key
EVP_PKEY* LicenseManager::LoadPublicKey() const {
    auto pemKey = GetPublicKey();
    BIO* bio = BIO_new_mem_buf(pemKey.c_str(), pemKey.size());
    if (!bio) return nullptr;

    EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!pkey) {
		LOG_ERROR_ADD(L"LicenseManager", getOpenSSLErrorString());
        return nullptr;
    }

    return pkey;
}

// Checking the validity of the license
inline bool LicenseManager::IsLicenseValid() const {
    return !this->storedLicenseKey.empty() && std::chrono::system_clock::now() < this->expirationDate;
}

// Obtaining a current license key
std::optional<std::wstring> LicenseManager::GetLicense() const {
    if (storedLicenseKey.empty()) {
        return std::nullopt;
    }
    return storedLicenseKey;
}

// Obtaining a license expiration date
std::optional<std::chrono::system_clock::time_point> LicenseManager::GetExpirationDate() const {
    if (storedLicenseKey.empty()) {
        return std::nullopt;
    }
    return expirationDate;
}

// 📌 Verify license data signature
bool LicenseManager::VerifyLicenseData(const std::wstring& signedData, const std::wstring& originalData) const {
    EVP_PKEY* pkey = LoadPublicKey();
    if (!pkey) return false;

    auto data_sign = Base64Decode(convertWStringToString(signedData));
    auto origin_data = convertWStringToString(originalData);

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        EVP_PKEY_free(pkey);
        return false;
    }

    if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pkey) <= 0) {
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return false;
    }

    int result = EVP_DigestVerify(ctx,
        (const unsigned char*)signedData.c_str(), signedData.size(),
        (const unsigned char*)originalData.c_str(), originalData.size());

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);

    return result == 1;
}

// 📌 Set and Validate License
bool LicenseManager::SetLicense(const std::wstring& licenseKey) {
    std::wstring decryptedData;
    if (!VerifyLicenseData(licenseKey, decryptedData)) return false;

    try {
        // Convert std::wstring to std::string (jsoncons expects UTF-8)
        std::string utf8Data = convertWStringToString(decryptedData);

        // Parse JSON using jsoncons
        jsoncons::wjson json = jsoncons::wjson::parse(decryptedData);

        if (json.contains(L"expiration_date")) {
            std::wstring expirationDateStr = json[L"expiration_date"].as<std::wstring>();

            std::tm tm = {};
            std::wistringstream ss(expirationDateStr);
            std::chrono::sys_time<std::chrono::seconds> tp;
            ss >> std::chrono::parse(L"%Y-%m-%dT%H:%M:%SZ", tp);

            if (ss.fail() || ss.bad()) return false;  // More robust failure check

            if (std::chrono::system_clock::now() > tp) return false;

            storedLicenseKey = licenseKey;
            return true;
        }
    }
    catch (const std::exception& e) {
        LOG_ERROR_ADD(L"LicenseManager", L"Error set license");
    }

    return false;
}