#include "../include/fmt.h"

extern void fmt_conn_base64url(enum exch_type type, uint8_t conn_data[MAX_EXCH_DATA_LENGTH], char encoded[MAX_EXCH_ENCODED_LENGTH])
{
    static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    uint8_t *p = conn_data;
    int n = (int)type;
    int i = 0; // conn data index
    int o = 0; // base64 offset

    while (i < n)
    {
        int32_t chunk = 0;

        int rem = n - i;
        chunk |= (int32_t)(p[i] << 16);
        if (rem > 1)
        {
            chunk |= (int32_t)(p[i + 1] << 8);
        }
        if (rem > 2)
        {
            chunk |= (int32_t)(p[i + 2]);
        }

        encoded[o++] = b64[(chunk >> 18) & 0x3F];
        encoded[o++] = b64[(chunk >> 12) & 0x3F];
        if (rem > 1)
        {
            encoded[o++] = b64[(chunk >> 6) & 0x3F];
        }
        if (rem > 2)
        {
            encoded[o++] = b64[chunk & 0x3F];
        }

        i += 3;
    }

    encoded[o] = '\0';
}

extern void fmt_conn_plaintext(enum exch_type type, uint8_t conn_data[MAX_EXCH_DATA_LENGTH], char plaintext[MAX_EXCH_PLAINTEXT_LENGTH])
{
    uint8_t *p = conn_data;

    if (type == IPV4_LOCAL_AREA)
    {
        snprintf(plaintext, MAX_EXCH_PLAINTEXT_LENGTH, "%u.%u.%u.%u", p[0], p[1], p[2], p[3]);
        return;
    }

    if (type == IPV4_REFLEXIVE)
    {
        uint16_t port = ((uint16_t)p[4] << 8) | p[5];
        snprintf(plaintext, MAX_EXCH_PLAINTEXT_LENGTH, "%u.%u.%u.%u:%u", p[0], p[1], p[2], p[3], port);
        return;
    }

    // unknown type
    plaintext[0] = '\0';
}