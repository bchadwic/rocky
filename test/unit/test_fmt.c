#include "../../include/fmt.h"
#include "../../include/exch.h"
#include <string.h>
#include <assert.h>


int main(void)
{
  uint8_t conn_data[MAX_EXCH_DATA_LENGTH] = {192, 168, 12, 2};

  char encoded[MAX_EXCH_ENCODED_LENGTH];
  char plaintext[MAX_EXCH_PLAINTEXT_LENGTH];

  fmt_conn_base64url(IPV4_LOCAL_AREA, conn_data, encoded);
  fmt_conn_plaintext(IPV4_LOCAL_AREA, conn_data, plaintext);

  assert(strcmp(encoded, "wKgMAg") == 0);
  assert(strcmp(plaintext, "192.168.12.2") == 0);
  return 0;
}