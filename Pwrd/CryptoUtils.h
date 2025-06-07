#pragma once

#include <windows.h>
#include <bcrypt.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>

struct PinFileData {
    std::vector<BYTE> salt;
    std::vector<BYTE> hash;
};

std::wstring GetPinFilePath();
std::string WstringToUtf8(const std::wstring& wstr);

class CryptoException : public std::runtime_error {
public:
    CryptoException(const std::string& message, NTSTATUS status)
        : std::runtime_error(message + " NTSTATUS: 0x" + to_hex(status)), nt_status(status) {}

    NTSTATUS get_status() const {
        return nt_status;
    }

private:
    NTSTATUS nt_status;
    static std::string to_hex(NTSTATUS status) {
        std::stringstream ss;
        ss << std::hex << std::setw(8) << std::setfill('0') << status;
        return ss.str();
    }
};

std::vector<BYTE> generate_random_bytes(ULONG length);

NTSTATUS derive_key_pbkdf2(
    const std::wstring& password,
    const std::vector<BYTE>& salt,
    ULONG iteration_count,
    std::vector<BYTE>& key_output,
    ULONG key_length_bytes
);

NTSTATUS encrypt_aes_gcm(
    const std::vector<BYTE>& key,
    const std::vector<BYTE>& plaintext,
    const std::vector<BYTE>& iv,
    const std::vector<BYTE>& aad,
    std::vector<BYTE>& ciphertext,
    std::vector<BYTE>& auth_tag
);

NTSTATUS decrypt_aes_gcm(
    const std::vector<BYTE>& key,
    const std::vector<BYTE>& ciphertext,
    const std::vector<BYTE>& iv,
    const std::vector<BYTE>& aad,
    const std::vector<BYTE>& auth_tag,
    std::vector<BYTE>& plaintext
);

NTSTATUS hash_sha256(
    const std::vector<BYTE>& data,
    std::vector<BYTE>& hash_output
);


std::vector<BYTE> GenerateKeyFromPassword(const std::wstring& password);
bool EncryptFile(const std::wstring& source, const std::wstring& destination, const std::wstring& password);
bool DecryptFile(const std::wstring& source, const std::wstring& destination, const std::wstring& password);
void BufferExample(const std::wstring& password);