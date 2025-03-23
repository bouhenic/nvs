#ifndef SECURE_STORAGE_H
#define SECURE_STORAGE_H

#include <WiFi.h>
#include <Preferences.h>
#include <mbedtls/gcm.h>
#include <mbedtls/sha256.h>

#define AES_KEY_SIZE 32
#define IV_SIZE 12
#define TAG_SIZE 16

class SecureStorage {
public:
    SecureStorage(const char* namespace_nvs = "secure_storage")
        : _namespace(namespace_nvs) {}

    bool storeSecret(const char* key_name, const char* plaintext) {
        uint8_t mac[6];
        WiFi.macAddress(mac);
        unsigned char key[AES_KEY_SIZE];
        deriveKey(mac, key);

        unsigned char iv[IV_SIZE];
        esp_fill_random(iv, IV_SIZE);

        size_t plaintext_len = strlen(plaintext);
        unsigned char ciphertext[plaintext_len];
        unsigned char tag[TAG_SIZE];

        if (!aesGcmEncrypt(key, iv, (unsigned char*)plaintext, plaintext_len, ciphertext, tag))
            return false;

        Preferences prefs;
        prefs.begin(_namespace, false);
        prefs.putBytes((String(key_name) + "_iv").c_str(), iv, IV_SIZE);
        prefs.putBytes((String(key_name) + "_tag").c_str(), tag, TAG_SIZE);
        prefs.putBytes((String(key_name) + "_data").c_str(), ciphertext, plaintext_len);
        prefs.putInt((String(key_name) + "_len").c_str(), plaintext_len);
        prefs.end();

        secureErase(key, AES_KEY_SIZE);
        return true;
    }

    bool retrieveSecret(const char* key_name, char* plaintext, size_t max_plaintext_len) {
        uint8_t mac[6];
        WiFi.macAddress(mac);
        unsigned char key[AES_KEY_SIZE];
        deriveKey(mac, key);

        Preferences prefs;
        prefs.begin(_namespace, true);

        if (!prefs.isKey((String(key_name) + "_data").c_str())) {
            prefs.end();
            return false;
        }

        unsigned char iv[IV_SIZE];
        unsigned char tag[TAG_SIZE];

        size_t iv_len = prefs.getBytes((String(key_name) + "_iv").c_str(), iv, IV_SIZE);
        size_t tag_len = prefs.getBytes((String(key_name) + "_tag").c_str(), tag, TAG_SIZE);
        int ciphertext_len = prefs.getInt((String(key_name) + "_len").c_str(), 0);

        if (iv_len != IV_SIZE || tag_len != TAG_SIZE || ciphertext_len <= 0 || ciphertext_len >= max_plaintext_len) {
            prefs.end();
            return false;
        }

        unsigned char ciphertext[ciphertext_len];
        prefs.getBytes((String(key_name) + "_data").c_str(), ciphertext, ciphertext_len);
        prefs.end();

        bool success = aesGcmDecrypt(key, iv, ciphertext, ciphertext_len, tag, (unsigned char*)plaintext);
        secureErase(key, AES_KEY_SIZE);

        return success;
    }

private:
    const char* _namespace;

    void deriveKey(uint8_t* mac, unsigned char* key) {
        mbedtls_sha256(mac, 6, key, 0);
    }

    bool aesGcmEncrypt(const unsigned char* key, const unsigned char* iv, const unsigned char* input, size_t len,
                       unsigned char* output, unsigned char* tag) {
        mbedtls_gcm_context gcm;
        mbedtls_gcm_init(&gcm);
        if (mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 256)) return false;

        int result = mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT, len, iv, IV_SIZE,
                                               nullptr, 0, input, output, TAG_SIZE, tag);
        mbedtls_gcm_free(&gcm);
        return (result == 0);
    }

    bool aesGcmDecrypt(const unsigned char* key, const unsigned char* iv, const unsigned char* input, size_t len,
                       const unsigned char* tag, unsigned char* output) {
        mbedtls_gcm_context gcm;
        mbedtls_gcm_init(&gcm);
        if (mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, key, 256)) return false;

        int result = mbedtls_gcm_auth_decrypt(&gcm, len, iv, IV_SIZE, nullptr, 0, tag, TAG_SIZE, input, output);
        mbedtls_gcm_free(&gcm);
        output[len] = '\0';
        return (result == 0);
    }

    void secureErase(void* v, size_t n) {
        volatile unsigned char* p = (volatile unsigned char*)v;
        while (n--) *p++ = 0;
    }
};

#endif // SECURE_STORAGE_H
