#ifndef FMT_H
#define FMT_H

#include <inttypes.h>
#include "exch.h"

#define MAX_EXCH_ENCODED_LENGTH 9
#define MAX_EXCH_PLAINTEXT_LENGTH 22

extern void fmt_conn_base64url(enum exch_type type, uint8_t conn_data[MAX_EXCH_DATA_LENGTH], char encoded[MAX_EXCH_ENCODED_LENGTH]);
extern void fmt_conn_plaintext(enum exch_type type, uint8_t conn_data[MAX_EXCH_DATA_LENGTH], char plaintext[MAX_EXCH_PLAINTEXT_LENGTH]);

#endif