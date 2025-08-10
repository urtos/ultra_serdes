#ifndef ULTRA_SERDES_H
#define ULTRA_SERDES_H

#include <stdint.h>
#include <stdlib.h>

typedef enum {
    ULTRA_SERDES_OK = 0,
    ULTRA_SERDES_ERR_MEMORY = -1,
    ULTRA_SERDES_ERR_INVALID = -2,
} ultra_serdes_result;

typedef enum {
    FIELD_VALUE,
    FIELD_STRING,
    FIELD_BUFFER
} field_type;

typedef void (*ultra_serdes_callback)(void *data, size_t size, void *user_data);

typedef struct {
    field_type type;
    void *data;
    size_t size;
    ultra_serdes_callback callback;
    void *user_data;
} field_descriptor;

typedef struct {
    uint8_t *buffer;
    size_t buffer_size;
    size_t offset;
} ultra_serdes_context;

uint32_t manual_htonl(uint32_t hostlong);
uint32_t manual_ntohl(uint32_t netlong);
uint16_t manual_htons(uint16_t hostshort);
uint16_t manual_ntohs(uint16_t netshort);

ultra_serdes_context *ultra_serdes_init(size_t initial_size);
void ultra_serdes_free(ultra_serdes_context *ctx);
ultra_serdes_result ultra_ser_field(ultra_serdes_context *ctx, field_descriptor *desc);
ultra_serdes_result ultra_des_field(ultra_serdes_context *ctx, field_descriptor *desc);
ultra_serdes_result ultra_ser_to_buffer(ultra_serdes_context *ctx, void *data, size_t size);
ultra_serdes_result ultra_des_from_buffer(ultra_serdes_context *ctx, void *data, size_t size);

void ultra_ser_hton(void *data, size_t size, void *user_data);
void ultra_des_ntoh(void *data, size_t size, void *user_data);

char *ultra_serdes_buffer_to_hex(ultra_serdes_context *ctx);

ultra_serdes_result ultra_serdes_hex_to_buffer(ultra_serdes_context *ctx, const char *hex_str);

#endif
