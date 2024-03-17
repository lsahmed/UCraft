#include "encryption.h"
#include "https.h"
#include "log.h"
#include "config.h"
#include "util.h"
#include "socketio.h"
#include "player.h"

#include "wrapper.h"
#include <string.h>
#include <netdb.h>
#include <time.h>

#ifdef ONLINE_MODE
static encryptionData_t encryptionData;
encryptionData_t *encryptionGetData()
{
    return &encryptionData;
}

int encryptionBegin()
{
    const char *pers = "gen_key";
    int ret;
    mbedtls_pk_init(&encryptionData.key);
    mbedtls_ctr_drbg_init(&encryptionData.ctr_drbg);
    mbedtls_entropy_init(&encryptionData.entropy);
    if ((ret = mbedtls_ctr_drbg_seed(&encryptionData.ctr_drbg, mbedtls_entropy_func, &encryptionData.entropy, (const unsigned char *)pers, strlen(pers))) != 0)
    {
        printl(LOG_ERROR, "mbedtls_ctr_drbg_seed returned %d\n", ret);
        return 1;
    }
    if ((ret = mbedtls_pk_setup(&encryptionData.key, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA))) != 0)
    {
        printl(LOG_ERROR, "mbedtls_pk_setup returned %d\n", ret);
        return 1;
    }
    if ((ret = mbedtls_rsa_gen_key(mbedtls_pk_rsa(encryptionData.key), mbedtls_ctr_drbg_random, &encryptionData.ctr_drbg, 1024, 65537)) != 0)
    {
        printl(LOG_ERROR, "mbedtls_rsa_gen_key returned %d\n", ret);
        return 1;
    }

    return 0;
}
static int bytes_to_hexstring(char hexstr[45], unsigned char bytes[20], int len, uint8_t neg)
{
    static const char hexchars[] = "0123456789abcdef";
    int i;
    int shift = 0;
    for (i = 0; i < len; i++)
    {
        // remove the first trailing 0
        if ((bytes[i] & 0xF0) == 0 && i == 0)
        {
            shift = 1;
            if (neg)
            {
                hexstr[i * 2 + shift] = '-';
            }
            hexstr[i * 2 + 1 + shift] = hexchars[bytes[i] & 0xF];
            continue;
        }
        hexstr[i * 2 + shift] = hexchars[(bytes[i] >> 4) & 0xF];
        hexstr[i * 2 + 1 + shift] = hexchars[bytes[i] & 0xF];
    }
    hexstr[i * 2 + shift] = '\0';
    if (shift)
    {
        return 2 - (neg & 1);
    }
    else
    {
        return 0;
    }
}

// generate a minecraft style hexdigest
//  TODO: this could be buggy, add some testing later
void encryptionHexDigest(unsigned char dest[45], unsigned char src[20], size_t len)
{
    uint8_t hash[20];
    char hash_dest[45];
    uint8_t shift_byte = 0;
    memcpy(hash, src, 20);
    memset(hash_dest, 0, 45);
    if (hash[0] & 0x80)
    {
        // perform one's complement
        int i = 0;
        for (; i < 20; i++)
        {
            hash[i] = ~hash[i];
        }
        // add one
        for (i = 19; i >= 0; i--)
        {
            if (hash[i] == 0xFF)
            {
                hash[i] = 0;
                continue;
            }
            hash[i]++;
            break;
        }
        shift_byte = 1;
        hash_dest[0] = '-';
    }
    int skip = bytes_to_hexstring(hash_dest + shift_byte, hash, 20, shift_byte);
    memcpy(dest, &hash_dest[skip], strnlen(&hash_dest[skip], len - skip) + 1);
}
void LoginC2S_encryption_response()
{
    int ret;
    size_t decrypted_size;
    uint8_t decrypted[24];
    uint8_t encrypted[128];
    readPacketVars_t *readPacketValue = readValues();
    player_t *currentPlayer = readPacketValue->player;
    if (readVarInt() != 128)
    {
        printl(LOG_WARN, "shared secret length != 128\n");
        currentPlayer->remove_player = 1;
        return;
    }
    readBuffer((char *)encrypted, sizeof(encrypted));
    ret = mbedtls_rsa_rsaes_pkcs1_v15_decrypt(mbedtls_pk_rsa(encryptionData.key), mbedtls_ctr_drbg_random, &encryptionData.ctr_drbg, MBEDTLS_RSA_PRIVATE, &decrypted_size, encrypted, decrypted, sizeof(decrypted));
    if (ret != 0)
    {
        printl(LOG_WARN, "mbedtls_rsa_rsaes_oaep_decrypt failed! %d\n", ret);
        currentPlayer->remove_player = 1;
        return;
    }
    if (decrypted_size != 16)
    {
        printl(LOG_WARN, "decrypted size != 16 %d\n", decrypted_size);
        currentPlayer->remove_player = 1;
        return;
    }
    memcpy(currentPlayer->iv_encrypt, decrypted, 16);
    memcpy(currentPlayer->iv_decrypt, decrypted, 16);
    if (readVarInt() != 128)
    {
        printl(LOG_WARN, "shared verify token length != 128\n");
        currentPlayer->remove_player = 1;
        return;
    }
    readBuffer((char *)encrypted, sizeof(encrypted));
    ret = mbedtls_rsa_rsaes_pkcs1_v15_decrypt(mbedtls_pk_rsa(encryptionData.key), mbedtls_ctr_drbg_random, &encryptionData.ctr_drbg,MBEDTLS_RSA_PRIVATE, &decrypted_size, encrypted, decrypted, sizeof(decrypted));
    if (ret != 0)
    {
        printl(LOG_WARN, "mbedtls_rsa_rsaes_oaep_decrypt failed! %d\n", ret);
        currentPlayer->remove_player = 1;
        return;
    }
    if (decrypted_size != 4)
    {
        printl(LOG_WARN, "decrypted size != 4\n");
        currentPlayer->remove_player = 1;
        return;
    }
    if (memcmp(currentPlayer->verify_token, decrypted, decrypted_size) != 0)
    {
        printl(LOG_WARN, "verify token mismatch!\n");
        currentPlayer->remove_player = 1;
        return;
    }

    // init AES/CFB8 stream
    // TOOD: use the higher level API functions for the cipher instead of mbedtls_aes_xxx functions
    // see https://github.com/Mbed-TLS/mbedtls/issues/3689
    // We can get away with it because there is only one direction in use at a time
    mbedtls_aes_init(&currentPlayer->aes_ctx);

    ret = mbedtls_aes_setkey_enc(&currentPlayer->aes_ctx, currentPlayer->iv_encrypt, 128);
    if (ret != 0)
    {
        printl(LOG_ERROR, "mbedtls_aes_setkey_enc returned %d\n", ret);
        currentPlayer->remove_player = 1;
        return;
    }
    currentPlayer->encryption_verified = 1;
}
void encryptionCleanup()
{
    mbedtls_pk_free(&encryptionData.key);
    mbedtls_ctr_drbg_free(&encryptionData.ctr_drbg);
    mbedtls_entropy_free(&encryptionData.entropy);
}
#endif /*ONLINE_MODE*/