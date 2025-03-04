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
    // ���������� ������������ ���� (������������� � ��������� ���)
    bool SetLicense(const std::wstring& licenseKey);

    // �������� ������� ������ ��������
    inline bool IsLicenseValid() const;

    // �������� ���������� ������������ ����
    std::optional<std::wstring> GetLicense() const;

    // �������� ���� ��������� ��������
    std::optional<std::chrono::system_clock::time_point> GetExpirationDate() const;

    // ������� �������� �������� (�������� ������� RSA)
    bool VerifyLicenseData(const std::wstring& signedData, const std::wstring& originalData) const;

private:
    std::wstring storedLicenseKey;  // �������� ��������
    std::chrono::system_clock::time_point expirationDate; // ���� ��������� ��������

	std::wstring getOpenSSLErrorString() const;

    // ������� ������������ (RSA)
    bool DecryptLicenseData(const std::wstring& encryptedData, std::wstring& decryptedData) const;

    // ��������� ��������� � ��������� �����
    static const std::string GetPublicKey();

    EVP_PKEY* LoadPublicKey() const;
};

#endif // LICENSE_MANAGER_H

