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
#include <climits> // For ULONG_MAX
#include <iostream> // Add this line to include the iostream header

#pragma comment(lib, "bcrypt.lib")

#ifndef STATUS_AUTH_TAG_MISMATCH
#define STATUS_AUTH_TAG_MISMATCH ((NTSTATUS)0xC000A002L) // Define the constant manually
#endif


#define KEY_LENGTH 32 // AES-256
#define IV_LENGTH 12  // Recommended for GCM
#define SALT_LENGTH 16
#define ITERATION_COUNT 100000

std::vector<BYTE> generate_random_bytes(ULONG length) {
    if (length > ULONG_MAX) {
        throw CryptoException("Requested length exceeds ULONG_MAX.", STATUS_INVALID_PARAMETER);
    }
    std::vector<BYTE> buffer(length);
    NTSTATUS status = BCryptGenRandom(
        NULL,
        buffer.data(),
        static_cast<ULONG>(buffer.size()),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG
    );
    if (!BCRYPT_SUCCESS(status)) {
        throw CryptoException("Failed to generate random bytes.", status);
    }
    return buffer;
}

NTSTATUS derive_key_pbkdf2(
    const std::wstring& password,
    const std::vector<BYTE>& salt,
    ULONG iteration_count,
    std::vector<BYTE>& key_output,
    ULONG key_length_bytes
) {
    if (salt.size() > ULONG_MAX) {
        throw CryptoException("Salt size exceeds ULONG_MAX.", STATUS_INVALID_PARAMETER);
    }
    BCRYPT_ALG_HANDLE hAlg = NULL;
    NTSTATUS status = BCryptOpenAlgorithmProvider(
        &hAlg,
        BCRYPT_SHA256_ALGORITHM,
        NULL,
        0
    );
    if (!BCRYPT_SUCCESS(status)) {
        if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("BCryptOpenAlgorithmProvider failed for PBKDF2.", status);
    }

    std::string narrow_password;
    int len = WideCharToMultiByte(CP_UTF8, 0, password.c_str(), (int)password.length(), NULL, 0, NULL, NULL);
    if (len > 0) {
        narrow_password.resize(len);
        WideCharToMultiByte(CP_UTF8, 0, password.c_str(), (int)password.length(), &narrow_password[0], len, NULL, NULL);
    }
    else if (password.length() > 0) {
        if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("Failed to convert password to UTF-8.", GetLastError());
    }

    std::vector<BYTE> password_bytes(narrow_password.begin(), narrow_password.end());
    if (password_bytes.size() > ULONG_MAX) {
        if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("Password size exceeds ULONG_MAX.", STATUS_INVALID_PARAMETER);
    }
    key_output.resize(key_length_bytes);

    status = BCryptDeriveKeyPBKDF2(
        hAlg,
        (PUCHAR)password_bytes.data(),
        static_cast<ULONG>(password_bytes.size()),
        (PUCHAR)salt.data(),
        static_cast<ULONG>(salt.size()),
        iteration_count,
        key_output.data(),
        key_length_bytes,
        0
    );

    if (!BCRYPT_SUCCESS(status)) {
        if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("BCryptDeriveKeyPBKDF2 failed.", status);
    }

    if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
    return status;
}

NTSTATUS encrypt_aes_gcm(
    const std::vector<BYTE>& key,
    const std::vector<BYTE>& plaintext,
    const std::vector<BYTE>& iv,
    const std::vector<BYTE>& aad,
    std::vector<BYTE>& ciphertext,
    std::vector<BYTE>& auth_tag
) {
    if (key.size() > ULONG_MAX || plaintext.size() > ULONG_MAX || iv.size() > ULONG_MAX || aad.size() > ULONG_MAX) {
        throw CryptoException("Input size exceeds ULONG_MAX.", STATUS_INVALID_PARAMETER);
    }
    NTSTATUS status;
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;
    ULONG cbCiphertext = 0;
    ULONG cbResult = 0;

    status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0);
    if (!BCRYPT_SUCCESS(status)) {
        throw CryptoException("BCryptOpenAlgorithmProvider failed for AES.", status);
    }

    status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("BCryptSetProperty for BCRYPT_CHAIN_MODE_GCM failed.", status);
    }

    auth_tag.resize(16);

    status = BCryptGenerateSymmetricKey(
        hAlg,
        &hKey,
        NULL, 0,
        (PBYTE)key.data(),
        static_cast<ULONG>(key.size()),
        0
    );
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("BCryptGenerateSymmetricKey failed.", status);
    }

    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = (PUCHAR)iv.data();
    authInfo.cbNonce = static_cast<ULONG>(iv.size());
    authInfo.pbAuthData = (PUCHAR)aad.data();
    authInfo.cbAuthData = static_cast<ULONG>(aad.size());
    authInfo.pbTag = auth_tag.data();
    authInfo.cbTag = static_cast<ULONG>(auth_tag.size());

    status = BCryptEncrypt(
        hKey,
        (PUCHAR)plaintext.data(),
        static_cast<ULONG>(plaintext.size()),
        &authInfo,
        NULL, 0,
        NULL,
        0,
        &cbCiphertext,
        0
    );
    if (!BCRYPT_SUCCESS(status)) {
        BCryptDestroyKey(hKey);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("BCryptEncrypt (sizing) failed.", status);
    }

    ciphertext.resize(cbCiphertext);

    status = BCryptEncrypt(
        hKey,
        (PUCHAR)plaintext.data(),
        static_cast<ULONG>(plaintext.size()),
        &authInfo,
        NULL, 0,
        ciphertext.data(),
        static_cast<ULONG>(ciphertext.size()),
        &cbResult,
        0
    );
    if (!BCRYPT_SUCCESS(status)) {
        BCryptDestroyKey(hKey);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("BCryptEncrypt (encryption) failed.", status);
    }

    if (hKey) BCryptDestroyKey(hKey);
    if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
    return status;
}

NTSTATUS decrypt_aes_gcm(
    const std::vector<BYTE>& key,
    const std::vector<BYTE>& ciphertext,
    const std::vector<BYTE>& iv,
    const std::vector<BYTE>& aad,
    const std::vector<BYTE>& auth_tag,
    std::vector<BYTE>& plaintext
) {
    if (key.size() > ULONG_MAX || ciphertext.size() > ULONG_MAX || iv.size() > ULONG_MAX || aad.size() > ULONG_MAX || auth_tag.size() > ULONG_MAX) {
        throw CryptoException("Input size exceeds ULONG_MAX.", STATUS_INVALID_PARAMETER);
    }
    NTSTATUS status;
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_KEY_HANDLE hKey = NULL;
    ULONG cbPlaintext = 0;

    status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0);
    if (!BCRYPT_SUCCESS(status)) {
        throw CryptoException("BCryptOpenAlgorithmProvider failed for AES.", status);
    }

    status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("BCryptSetProperty for BCRYPT_CHAIN_MODE_GCM failed.", status);
    }

    status = BCryptGenerateSymmetricKey(
        hAlg,
        &hKey,
        NULL, 0,
        (PBYTE)key.data(),
        static_cast<ULONG>(key.size()),
        0
    );
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("BCryptGenerateSymmetricKey failed.", status);
    }

    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
    BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
    authInfo.pbNonce = (PUCHAR)iv.data();
    authInfo.cbNonce = static_cast<ULONG>(iv.size());
    authInfo.pbAuthData = (PUCHAR)aad.data();
    authInfo.cbAuthData = static_cast<ULONG>(aad.size());
    authInfo.pbTag = (PUCHAR)auth_tag.data();
    authInfo.cbTag = static_cast<ULONG>(auth_tag.size());

    plaintext.resize(ciphertext.size());

    status = BCryptDecrypt(
        hKey,
        (PUCHAR)ciphertext.data(),
        static_cast<ULONG>(ciphertext.size()),
        &authInfo,
        NULL, 0,
        plaintext.data(),
        static_cast<ULONG>(plaintext.size()),
        &cbPlaintext,
        0
    );

    if (!BCRYPT_SUCCESS(status)) {
        BCryptDestroyKey(hKey);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        if (status == STATUS_AUTH_TAG_MISMATCH) {
            throw CryptoException("Decryption failed: Authentication tag mismatch.", status);
        }
        throw CryptoException("BCryptDecrypt failed.", status);
    }

    plaintext.resize(cbPlaintext);

    if (hKey) BCryptDestroyKey(hKey);
    if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
    return status;
}

NTSTATUS hash_sha256(
    const std::vector<BYTE>& data,
    std::vector<BYTE>& hash_output
) {
    if (data.size() > ULONG_MAX) {
        throw CryptoException("Data size exceeds ULONG_MAX.", STATUS_INVALID_PARAMETER);
    }
    NTSTATUS status;
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_HASH_HANDLE hHash = NULL;
    ULONG cbHashObject = 0;
    ULONG cbData = 0;
    ULONG cbHash = 0;
    std::vector<BYTE> hashObject;

    status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0);
    if (!BCRYPT_SUCCESS(status)) {
        throw CryptoException("BCryptOpenAlgorithmProvider failed for SHA256.", status);
    }

    status = BCryptGetProperty(
        hAlg,
        BCRYPT_OBJECT_LENGTH,
        (PBYTE)&cbHashObject,
        sizeof(ULONG),
        &cbData,
        0
    );
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("BCryptGetProperty(BCRYPT_OBJECT_LENGTH) failed for SHA256.", status);
    }
    hashObject.resize(cbHashObject);

    status = BCryptGetProperty(
        hAlg,
        BCRYPT_HASH_LENGTH,
        (PBYTE)&cbHash,
        sizeof(ULONG),
        &cbData,
        0
    );
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("BCryptGetProperty(BCRYPT_HASH_LENGTH) failed for SHA256.", status);
    }
    hash_output.resize(cbHash);

    status = BCryptCreateHash(
        hAlg,
        &hHash,
        hashObject.data(),
        static_cast<ULONG>(hashObject.size()),
        NULL,
        0,
        0
    );
    if (!BCRYPT_SUCCESS(status)) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("BCryptCreateHash failed for SHA256.", status);
    }

    status = BCryptHashData(
        hHash,
        (PBYTE)data.data(),
        static_cast<ULONG>(data.size()),
        0
    );
    if (!BCRYPT_SUCCESS(status)) {
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("BCryptHashData failed for SHA256.", status);
    }

    status = BCryptFinishHash(
        hHash,
        hash_output.data(),
        static_cast<ULONG>(hash_output.size()),
        0
    );
    if (!BCRYPT_SUCCESS(status)) {
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw CryptoException("BCryptFinishHash failed for SHA256.", status);
    }

    if (hHash) BCryptDestroyHash(hHash);
    if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
    return status;
}

std::string WstringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), nullptr, 0, nullptr, nullptr);
    if (len <= 0) throw CryptoException("Failed to convert wstring to UTF-8", GetLastError());
    std::string result(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &result[0], len, nullptr, nullptr);
    return result;
}

bool EncryptFile(const std::wstring& source, const std::wstring& destination, const std::wstring& password) {
    try {
        std::ifstream inFile(source, std::ios::binary);
        if (!inFile) {
            std::cerr << "Error opening source file: " << WstringToUtf8(source) << "\n";
            return false;
        }
        std::vector<BYTE> plaintext((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        inFile.close();

        std::vector<BYTE> salt = generate_random_bytes(SALT_LENGTH);
        std::vector<BYTE> iv = generate_random_bytes(IV_LENGTH);
        std::vector<BYTE> key;
        std::vector<BYTE> ciphertext, auth_tag;

        derive_key_pbkdf2(password, salt, ITERATION_COUNT, key, KEY_LENGTH);
        encrypt_aes_gcm(key, plaintext, iv, {}, ciphertext, auth_tag);

        std::ofstream outFile(destination, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error opening destination file: " << WstringToUtf8(destination) << "\n";
            return false;
        }
        outFile.write((char*)salt.data(), salt.size());
        outFile.write((char*)iv.data(), iv.size());
        outFile.write((char*)auth_tag.data(), auth_tag.size());
        outFile.write((char*)ciphertext.data(), ciphertext.size());
        outFile.close();

        return true;
    }
    catch (const CryptoException& e) {
        std::cerr << "Encryption failed: " << e.what() << "\n";
        return false;
    }
}

bool DecryptFile(const std::wstring& source, const std::wstring& destination, const std::wstring& password) {
    try {
        std::ifstream inFile(source, std::ios::binary);
        if (!inFile) {
            std::cerr << "Error opening source file: " << WstringToUtf8(source) << "\n";
            return false;
        }
        std::vector<BYTE> fileData((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
        inFile.close();

        if (fileData.size() < SALT_LENGTH + IV_LENGTH + 16) {
            std::cerr << "Invalid encrypted file format\n";
            return false;
        }

        std::vector<BYTE> salt(fileData.begin(), fileData.begin() + SALT_LENGTH);
        std::vector<BYTE> iv(fileData.begin() + SALT_LENGTH, fileData.begin() + SALT_LENGTH + IV_LENGTH);
        std::vector<BYTE> auth_tag(fileData.begin() + SALT_LENGTH + IV_LENGTH, fileData.begin() + SALT_LENGTH + IV_LENGTH + 16);
        std::vector<BYTE> ciphertext(fileData.begin() + SALT_LENGTH + IV_LENGTH + 16, fileData.end());

        std::vector<BYTE> key;
        derive_key_pbkdf2(password, salt, ITERATION_COUNT, key, KEY_LENGTH);

        std::vector<BYTE> plaintext;
        decrypt_aes_gcm(key, ciphertext, iv, {}, auth_tag, plaintext);

        std::ofstream outFile(destination, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error opening destination file: " << WstringToUtf8(destination) << "\n";
            return false;
        }
        outFile.write((char*)plaintext.data(), plaintext.size());
        outFile.close();

        return true;
    }
    catch (const CryptoException& e) {
        std::cerr << "Decryption failed: " << e.what() << "\n";
        return false;
    }
}

void BufferExample(const std::wstring& password) {
    try {
        std::string plaintext = "Hello, this is a test buffer!";
        std::vector<BYTE> plaintext_bytes(plaintext.begin(), plaintext.end());
        std::vector<BYTE> salt = generate_random_bytes(SALT_LENGTH);
        std::vector<BYTE> iv = generate_random_bytes(IV_LENGTH);
        std::vector<BYTE> key, ciphertext, auth_tag;

        derive_key_pbkdf2(password, salt, ITERATION_COUNT, key, KEY_LENGTH);
        encrypt_aes_gcm(key, plaintext_bytes, iv, {}, ciphertext, auth_tag);
        std::vector<BYTE> decrypted;
        decrypt_aes_gcm(key, ciphertext, iv, {}, auth_tag, decrypted);

        std::cout << "Original: " << plaintext << "\n";
        std::cout << "Decrypted: " << std::string(decrypted.begin(), decrypted.end()) << "\n";
    }
    catch (const CryptoException& e) {
        std::cerr << "Buffer example failed: " << e.what() << "\n";
    }
}