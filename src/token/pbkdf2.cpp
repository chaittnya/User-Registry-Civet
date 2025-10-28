#include "pbkdf2.hpp"
#include <openssl/evp.h>
#include <openssl/sha.h>

std::string derive_token(const std::string& mobile, int iterations) {
    if (iterations <= 0) iterations = 700000;

    // Deterministic salt from mobile (helps testing)
    unsigned char salt[32];
    std::string pref = "fixed-salt-prefix::" + mobile;
    SHA256(reinterpret_cast<const unsigned char*>(pref.c_str()), pref.size(), salt);

    unsigned char key[32];
    PKCS5_PBKDF2_HMAC(
        mobile.c_str(), static_cast<int>(mobile.size()),
        salt, sizeof(salt),
        iterations, EVP_sha256(),
        sizeof(key), key
    );

    static const char* hexd = "0123456789abcdef";
    std::string out; out.resize(sizeof(key)*2);
    for (size_t i=0;i<sizeof(key);++i) {
        out[2*i  ] = hexd[(key[i]>>4)&0xF];
        out[2*i+1] = hexd[(key[i])   &0xF];
    }
    return out;
}
