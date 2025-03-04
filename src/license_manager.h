#pragma once

#ifndef LICENSE_MANAGER_H
#define LICENSE_MANAGER_H

#include <string>
#include <chrono>
#include <optional>
#include <vector>
#include <openssl/types.h>

class LicenseManager {
public:
    // Установить лицензионный ключ (устанавливает и проверяет его)
    bool SetLicense(const std::wstring& licenseKey);

    // Получить текущий статус лицензии
    inline bool IsLicenseValid() const;

    // Получить сохранённый лицензионный ключ
    std::optional<std::wstring> GetLicense() const;

    // Получить дату окончания лицензии
    std::optional<std::chrono::system_clock::time_point> GetExpirationDate() const;

    // Функция проверки лицензии (проверка подписи RSA)
    bool VerifyLicenseData(const std::wstring& signedData, const std::wstring& originalData) const;

private:
    std::wstring storedLicenseKey;  // Хранение лицензии
    std::chrono::system_clock::time_point expirationDate; // Дата окончания лицензии

	std::wstring getOpenSSLErrorString() const;

    // Функция дешифрования (RSA)
    bool DecryptLicenseData(const std::wstring& encryptedData, std::wstring& decryptedData) const;

    // Симуляция закрытого и открытого ключа
    static const std::string GetPublicKey();

    EVP_PKEY* LoadPublicKey() const;
};

#endif // LICENSE_MANAGER_H

