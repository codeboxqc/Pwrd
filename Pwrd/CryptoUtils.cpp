#define WIN32_LEAN_AND_MEAN
#include "pch.h"
#include "CryptoUtils.h"
#include <windows.h>
#include <bcrypt.h>
#include <stdexcept>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <climits>
#include <iostream>
#include <random>
#include <algorithm> // Ensure <algorithm> is included for std::min



// Add this typedef to avoid conflicts with the BYTE macro
typedef unsigned char BYTE;

// Custom PBKDF2 implementation using HMAC-SHA256
class PBKDF2_HMAC_SHA256 {
private:
    static std::vector<BYTE> HMAC_SHA256(const std::vector<BYTE>& key, const std::vector<BYTE>& data) {
        BCRYPT_ALG_HANDLE hAlg = nullptr;
        BCRYPT_HASH_HANDLE hHash = nullptr;
        NTSTATUS status;
        std::vector<BYTE> result(32); // SHA256 output size

        status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG);
        if (!NT_SUCCESS(status)) {
            throw std::runtime_error("Failed to open SHA256 HMAC algorithm");
        }

        status = BCryptCreateHash(hAlg, &hHash, nullptr, 0, (PUCHAR)key.data(), (ULONG)key.size(), 0);
        if (!NT_SUCCESS(status)) {
            BCryptCloseAlgorithmProvider(hAlg, 0);
            throw std::runtime_error("Failed to create HMAC hash");
        }

        status = BCryptHashData(hHash, (PUCHAR)data.data(), (ULONG)data.size(), 0);
        if (NT_SUCCESS(status)) {
            status = BCryptFinishHash(hHash, result.data(), (ULONG)result.size(), 0);
        }

        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);

        if (!NT_SUCCESS(status)) {
            throw std::runtime_error("Failed to compute HMAC");
        }

        return result;
    }

    static void XOR(std::vector<BYTE>& dest, const std::vector<BYTE>& src) {
        for (size_t i = 0; i < dest.size() && i < src.size(); ++i) {
            dest[i] ^= src[i];
        }
    }

public:
    static std::vector<BYTE> derive(const std::vector<BYTE>& password, const std::vector<BYTE>& salt,
        ULONG iterations, ULONG keyLength) {
        std::vector<BYTE> result;
        result.reserve(keyLength);

        ULONG blocks = (keyLength + 31) / 32; // 32 = SHA256 output size

        for (ULONG block = 1; block <= blocks; ++block) {
            // Create salt + block number
            std::vector<BYTE> saltBlock = salt;
            saltBlock.push_back((block >> 24) & 0xFF);
            saltBlock.push_back((block >> 16) & 0xFF);
            saltBlock.push_back((block >> 8) & 0xFF);
            saltBlock.push_back(block & 0xFF);

            // First iteration
            std::vector<BYTE> u = HMAC_SHA256(password, saltBlock);
            std::vector<BYTE> t = u;

            // Remaining iterations
            for (ULONG i = 1; i < iterations; ++i) {
                u = HMAC_SHA256(password, u);
                XOR(t, u);
            }

            // Append to result
            size_t bytesToCopy = std::min((size_t)32, keyLength - result.size());
            result.insert(result.end(), t.begin(), t.begin() + bytesToCopy);
        }

        result.resize(keyLength);
        return result;
    }
};

// Forward declarations
std::string WstringToUtf8(const std::wstring& wstr);
NTSTATUS encrypt_aes_gcm(const std::vector<BYTE>& key, const std::vector<BYTE>& plaintext,
    const std::vector<BYTE>& iv, const std::vector<BYTE>& aad,
    std::vector<BYTE>& ciphertext, std::vector<BYTE>& auth_tag);

NTSTATUS decrypt_aes_gcm(const std::vector<BYTE>& key, const std::vector<BYTE>& ciphertext,
    const std::vector<BYTE>& iv, const std::vector<BYTE>& aad,
    const std::vector<BYTE>& auth_tag, std::vector<BYTE>& plaintext);

void TestBCrypt() {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    NTSTATUS status;

    // Test SHA256 with HMAC
    status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG);
    if (NT_SUCCESS(status) && hAlg) {
        MessageBoxW(nullptr, L"SHA256 HMAC supported", L"Info", MB_OK);
        BCryptCloseAlgorithmProvider(hAlg, 0);
    }
    else {
        MessageBoxW(nullptr, L"SHA256 HMAC failed", L"Info", MB_OK | MB_ICONERROR);
    }

    // Test custom PBKDF2
    try {
        std::vector<BYTE> password = { 116, 101, 115, 116 }; // "test"
        std::vector<BYTE> salt = { 115, 97, 108, 116 }; // "salt"
        std::vector<BYTE> key = PBKDF2_HMAC_SHA256::derive(password, salt, 1000, 32);
        MessageBoxW(nullptr, L"Custom PBKDF2 succeeded", L"Info", MB_OK);
    }
    catch (...) {
        MessageBoxW(nullptr, L"Custom PBKDF2 failed", L"Info", MB_OK | MB_ICONERROR);
    }
}

std::vector<BYTE> generate_random_bytes(size_t length) {
    std::vector<BYTE> bytes(length);
    NTSTATUS status = BCryptGenRandom(nullptr, bytes.data(), (ULONG)length, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (!NT_SUCCESS(status)) {
        // Fallback to C++ random
        std::random_device rd;
        std::uniform_int_distribution<int> dist(0, 255);
        for (size_t i = 0; i < length; ++i) {
            bytes[i] = static_cast<BYTE>(dist(rd));
        }
    }
    return bytes;
}

NTSTATUS derive_key_pbkdf2_custom(const std::wstring& password, const std::vector<BYTE>& salt,
    ULONG iterations, std::vector<BYTE>& key, ULONG keyLength) {
    try {
        // Convert password to UTF-8
        std::string passwordUtf8;
        int utf8Length = WideCharToMultiByte(CP_UTF8, 0, password.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8Length <= 0) {
            return STATUS_INVALID_PARAMETER;
        }
        passwordUtf8.resize(utf8Length - 1); // Exclude null terminator
        WideCharToMultiByte(CP_UTF8, 0, password.c_str(), -1, &passwordUtf8[0], utf8Length, nullptr, nullptr);

        std::vector<BYTE> passwordBytes(passwordUtf8.begin(), passwordUtf8.end());
        key = PBKDF2_HMAC_SHA256::derive(passwordBytes, salt, iterations, keyLength);
        return STATUS_SUCCESS;
    }
    catch (...) {
        return STATUS_UNSUCCESSFUL;
    }
}

NTSTATUS derive_key_pbkdf2(const std::wstring& password, const std::vector<BYTE>& salt,
    ULONG iterations, std::vector<BYTE>& key, ULONG keyLength) {
    WCHAR debugMsg[512];
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_HASH_HANDLE hHash = nullptr;
    NTSTATUS status;

    // First try native PBKDF2
    status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG);
    if (!NT_SUCCESS(status)) {
        // Fall back to custom implementation immediately
        return derive_key_pbkdf2_custom(password, salt, iterations, key, keyLength);
    }

    status = BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0);
    if (!NT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return derive_key_pbkdf2_custom(password, salt, iterations, key, keyLength);
    }

    // Convert password to UTF-8
    std::string passwordUtf8;
    int utf8Length = WideCharToMultiByte(CP_UTF8, 0, password.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8Length <= 0) {
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return STATUS_INVALID_PARAMETER;
    }
    passwordUtf8.resize(utf8Length - 1); // Exclude null terminator
    WideCharToMultiByte(CP_UTF8, 0, password.c_str(), -1, &passwordUtf8[0], utf8Length, nullptr, nullptr);

    // Try native PBKDF2
    key.resize(keyLength);
    status = BCryptDeriveKeyPBKDF2(hHash, (PUCHAR)passwordUtf8.data(), utf8Length - 1,
        (PUCHAR)salt.data(), (ULONG)salt.size(), iterations,
        key.data(), keyLength, 0);

    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);

    // If native PBKDF2 fails with STATUS_NOT_SUPPORTED, use custom implementation
    if (status == STATUS_NOT_SUPPORTED || status == ((NTSTATUS)0xC00000BBL)) {
        return derive_key_pbkdf2_custom(password, salt, iterations, key, keyLength);
    }

    if (!NT_SUCCESS(status)) {
        swprintf_s(debugMsg, L"BCryptDeriveKeyPBKDF2 failed. Status: 0x%08X", status);
        MessageBoxW(nullptr, debugMsg, L"Encryption Error", MB_OK | MB_ICONERROR);
    }

    return status;
}

NTSTATUS derive_key_fallback(const std::wstring& password, const std::vector<BYTE>& salt,
    std::vector<BYTE>& key, ULONG keyLength) {
    WCHAR debugMsg[512];
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_HASH_HANDLE hHash = nullptr;
    NTSTATUS status;

    status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    if (!NT_SUCCESS(status)) {
        swprintf_s(debugMsg, L"Fallback: BCryptOpenAlgorithmProvider (SHA256) failed. Status: 0x%08X", status);
        MessageBoxW(nullptr, debugMsg, L"Encryption Error", MB_OK | MB_ICONERROR);
        return status;
    }

    status = BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0);
    if (!NT_SUCCESS(status)) {
        swprintf_s(debugMsg, L"Fallback: BCryptCreateHash failed. Status: 0x%08X", status);
        MessageBoxW(nullptr, debugMsg, L"Encryption Error", MB_OK | MB_ICONERROR);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return status;
    }

    // Convert password to UTF-8
    std::string passwordUtf8;
    int utf8Length = WideCharToMultiByte(CP_UTF8, 0, password.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8Length <= 0) {
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return STATUS_INVALID_PARAMETER;
    }
    passwordUtf8.resize(utf8Length - 1);
    WideCharToMultiByte(CP_UTF8, 0, password.c_str(), -1, &passwordUtf8[0], utf8Length, nullptr, nullptr);

    // Hash password and salt multiple times for key stretching
    std::vector<BYTE> currentHash;
    for (int i = 0; i < 1000; ++i) {  // Simple key stretching
        BCryptHashData(hHash, (PUCHAR)passwordUtf8.data(), utf8Length - 1, 0);
        BCryptHashData(hHash, (PUCHAR)salt.data(), (ULONG)salt.size(), 0);
        if (i > 0) {
            BCryptHashData(hHash, currentHash.data(), (ULONG)currentHash.size(), 0);
        }

        currentHash.resize(32); // SHA256 size
        NTSTATUS hashStatus = BCryptFinishHash(hHash, currentHash.data(), 32, 0);
        if (!NT_SUCCESS(hashStatus)) break;

        // Reset hash for next iteration
        BCryptDestroyHash(hHash);
        BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0);
    }

    key.resize(keyLength);
    if (currentHash.size() >= keyLength) {
        memcpy(key.data(), currentHash.data(), keyLength);
        status = STATUS_SUCCESS;
    }
    else {
        // Extend key if needed
        size_t copied = 0;
        while (copied < keyLength) {
            size_t toCopy = std::min(currentHash.size(), keyLength - copied);
            memcpy(key.data() + copied, currentHash.data(), toCopy);
            copied += toCopy;
        }
        status = STATUS_SUCCESS;
    }

    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    return status;
}

NTSTATUS encrypt_aes_gcm(const std::vector<BYTE>& key, const std::vector<BYTE>& plaintext,
    const std::vector<BYTE>& iv, const std::vector<BYTE>& aad,
    std::vector<BYTE>& ciphertext, std::vector<BYTE>& auth_tag) {
    WCHAR debugMsg[512];
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_KEY_HANDLE hKey = nullptr;
    NTSTATUS status;
    DWORD cbData = 0, cbCipherText = 0;

    status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
    if (!NT_SUCCESS(status)) {
        swprintf_s(debugMsg, L"BCryptOpenAlgorithmProvider failed in encrypt_aes_gcm. Status: 0x%08X", status);
        MessageBoxW(nullptr, debugMsg, L"Encryption Error", MB_OK | MB_ICONERROR);
        return status;
    }

    status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
    if (!NT_SUCCESS(status)) {
        swprintf_s(debugMsg, L"BCryptSetProperty failed. Status: 0x%08X", status);
        MessageBoxW(nullptr, debugMsg, L"Encryption Error", MB_OK | MB_ICONERROR);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return status;
    }

    status = BCryptGenerateSymmetricKey(hAlg, &hKey, nullptr, 0, (PUCHAR)key.data(), (ULONG)key.size(), 0);
    if (!NT_SUCCESS(status)) {
        swprintf_s(debugMsg, L"BCryptGenerateSymmetricKey failed. Status: 0x%08X", status);
        MessageBoxW(nullptr, debugMsg, L"Encryption Error", MB_OK | MB_ICONERROR);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return status;
    }

    auth_tag.resize(TAG_LENGTH);
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = (PUCHAR)iv.data();
    authInfo.cbNonce = (ULONG)iv.size();
    authInfo.pbTag = auth_tag.data();
    authInfo.cbTag = TAG_LENGTH;

    status = BCryptEncrypt(hKey, (PUCHAR)plaintext.data(), (ULONG)plaintext.size(), &authInfo, nullptr, 0, nullptr, 0, &cbCipherText, 0);
    if (!NT_SUCCESS(status)) {
        swprintf_s(debugMsg, L"BCryptEncrypt (size calc) failed. Status: 0x%08X", status);
        MessageBoxW(nullptr, debugMsg, L"Encryption Error", MB_OK | MB_ICONERROR);
        BCryptDestroyKey(hKey);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return status;
    }

    ciphertext.resize(cbCipherText);
    status = BCryptEncrypt(hKey, (PUCHAR)plaintext.data(), (ULONG)plaintext.size(), &authInfo, nullptr, 0, ciphertext.data(), cbCipherText, &cbData, 0);
    if (!NT_SUCCESS(status)) {
        swprintf_s(debugMsg, L"BCryptEncrypt failed. Status: 0x%08X", status);
        MessageBoxW(nullptr, debugMsg, L"Encryption Error", MB_OK | MB_ICONERROR);
    }

    BCryptDestroyKey(hKey);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    return status;
}

NTSTATUS decrypt_aes_gcm(const std::vector<BYTE>& key, const std::vector<BYTE>& ciphertext,
    const std::vector<BYTE>& iv, const std::vector<BYTE>& aad,
    const std::vector<BYTE>& auth_tag, std::vector<BYTE>& plaintext) {
    if (key.size() > ULONG_MAX || ciphertext.size() > ULONG_MAX ||
        iv.size() > ULONG_MAX || aad.size() > ULONG_MAX || auth_tag.size() > ULONG_MAX) {
        return STATUS_INVALID_PARAMETER;
    }

    NTSTATUS status;
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_KEY_HANDLE hKey = nullptr;
    ULONG cbPlaintext = 0;

    status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
    if (!NT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return status;
    }

    status = BCryptGenerateSymmetricKey(hAlg, &hKey, nullptr, 0, (PBYTE)key.data(), (ULONG)key.size(), 0);
    if (!NT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return status;
    }

    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = (PUCHAR)iv.data();
    authInfo.cbNonce = (ULONG)iv.size();
    authInfo.pbAuthData = (PUCHAR)aad.data();
    authInfo.cbAuthData = (ULONG)aad.size();
    authInfo.pbTag = (PUCHAR)auth_tag.data();
    authInfo.cbTag = (ULONG)auth_tag.size();

    plaintext.resize(ciphertext.size());
    status = BCryptDecrypt(hKey, (PUCHAR)ciphertext.data(), (ULONG)ciphertext.size(), &authInfo,
        nullptr, 0, plaintext.data(), (ULONG)plaintext.size(), &cbPlaintext, 0);

    if (NT_SUCCESS(status)) {
        plaintext.resize(cbPlaintext);
    }

    BCryptDestroyKey(hKey);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    return status;
}

bool EncryptFile(const std::wstring& source, const std::wstring& destination, const std::wstring& password) {
    try {
        std::ifstream inFile(source, std::ios::binary);
        if (!inFile.is_open()) return false;

        std::vector<BYTE> plaintext((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        inFile.close();

        std::vector<BYTE> salt = generate_random_bytes(static_cast<size_t>(SALT_LENGTH));
        std::vector<BYTE> iv = generate_random_bytes(static_cast<size_t>(IV_LENGTH));

        std::vector<BYTE> key;
        NTSTATUS status = derive_key_pbkdf2(password, salt, ITERATION_COUNT, key, KEY_LENGTH);
        if (!NT_SUCCESS(status)) return false;

        std::vector<BYTE> ciphertext, auth_tag(TAG_LENGTH);
        status = encrypt_aes_gcm(key, plaintext, iv, {}, ciphertext, auth_tag);
        if (!NT_SUCCESS(status)) return false;

        std::ofstream outFile(destination, std::ios::binary);
        if (!outFile.is_open()) return false;

        outFile.write(reinterpret_cast<const char*>(salt.data()), salt.size());
        outFile.write(reinterpret_cast<const char*>(iv.data()), iv.size());
        outFile.write(reinterpret_cast<const char*>(auth_tag.data()), auth_tag.size());
        outFile.write(reinterpret_cast<const char*>(ciphertext.data()), ciphertext.size());
        outFile.close();

        return true;
    }
    catch (...) {
        return false;
    }
}

bool DecryptFile(const std::wstring& source, const std::wstring& destination, const std::wstring& password) {
    try {
        std::ifstream inFile(source, std::ios::binary);
        if (!inFile.is_open()) return false;

        std::vector<BYTE> fileData((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        inFile.close();

        if (fileData.size() < SALT_LENGTH + IV_LENGTH + TAG_LENGTH) return false;

        std::vector<BYTE> salt(fileData.begin(), fileData.begin() + SALT_LENGTH);
        std::vector<BYTE> iv(fileData.begin() + SALT_LENGTH, fileData.begin() + SALT_LENGTH + IV_LENGTH);
        std::vector<BYTE> auth_tag(fileData.begin() + SALT_LENGTH + IV_LENGTH,
            fileData.begin() + SALT_LENGTH + IV_LENGTH + TAG_LENGTH);
        std::vector<BYTE> ciphertext(fileData.begin() + SALT_LENGTH + IV_LENGTH + TAG_LENGTH, fileData.end());

        std::vector<BYTE> key;
        NTSTATUS status = derive_key_pbkdf2(password, salt, ITERATION_COUNT, key, KEY_LENGTH);
        if (!NT_SUCCESS(status)) return false;

        std::vector<BYTE> plaintext;
        status = decrypt_aes_gcm(key, ciphertext, iv, {}, auth_tag, plaintext);
        if (!NT_SUCCESS(status)) return false;

        std::ofstream outFile(destination, std::ios::binary);
        if (!outFile.is_open()) return false;

        outFile.write(reinterpret_cast<const char*>(plaintext.data()), plaintext.size());
        outFile.close();

        return true;
    }
    catch (...) {
        return false;
    }
}

std::string WstringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), nullptr, 0, nullptr, nullptr);
    if (len <= 0) throw std::runtime_error("Failed to convert wstring to UTF-8");
    std::string result(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &result[0], len, nullptr, nullptr);
    return result;
}

// Generate key from password using provided salt
std::vector<BYTE> GenerateKeyFromPassword(const std::wstring& password, const std::vector<BYTE>& salt) {
    if (password.empty()) {
        throw std::invalid_argument("Password cannot be empty");
    }
    if (salt.size() != SALT_LENGTH) {
        throw std::invalid_argument("Invalid salt length");
    }

    std::vector<BYTE> key(KEY_LENGTH);
    NTSTATUS status = derive_key_pbkdf2(password, salt, ITERATION_COUNT, key, KEY_LENGTH);

    if (!NT_SUCCESS(status)) {
        throw std::runtime_error("Failed to derive key from password");
    }

    return key;
}

// Generate key from password with new random salt (returns both key and salt)
std::pair<std::vector<BYTE>, std::vector<BYTE>> GenerateKeyAndSaltFromPassword(const std::wstring& password) {
    if (password.empty()) {
        throw std::invalid_argument("Password cannot be empty");
    }

   // std::vector<BYTE> salt = generate_random_bytes(SALT_LENGTH);
    std::vector<BYTE> salt = generate_random_bytes(static_cast<size_t>(SALT_LENGTH));
    std::vector<BYTE> key(KEY_LENGTH);
    NTSTATUS status = derive_key_pbkdf2(password, salt, ITERATION_COUNT, key, KEY_LENGTH);

    if (!NT_SUCCESS(status)) {
        throw std::runtime_error("Failed to derive key from password");
    }

    return std::make_pair(key, salt);
}