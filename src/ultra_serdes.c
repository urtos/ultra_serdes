#include "ultra_serdes.h"
#include <string.h>
#include <stdio.h>

uint32_t manual_htonl(uint32_t hostlong) {
    return ((hostlong & 0x000000FF) << 24) |
           ((hostlong & 0x0000FF00) << 8) |
           ((hostlong & 0x00FF0000) >> 8) |
           ((hostlong & 0xFF000000) >> 24);
}

uint32_t manual_ntohl(uint32_t netlong) {
    return ((netlong & 0xFF000000) >> 24) |
           ((netlong & 0x00FF0000) >> 8) |
           ((netlong & 0x0000FF00) << 8) |
           ((netlong & 0x000000FF) << 24);
}

uint16_t manual_htons(uint16_t hostshort) {
    return ((hostshort & 0x00FF) << 8) | ((hostshort & 0xFF00) >> 8);
}

uint16_t manual_ntohs(uint16_t netshort) {
    return ((netshort & 0xFF00) >> 8) | ((netshort & 0x00FF) << 8);
}

ultra_serdes_context *ultra_serdes_init(size_t initial_size) {
    ultra_serdes_context *ctx = (ultra_serdes_context *)malloc(sizeof(ultra_serdes_context));
    if (!ctx) return NULL;

    ctx->buffer = (uint8_t *)calloc(1, initial_size);
    if (!ctx->buffer) {
        free(ctx);
        return NULL;
    }

    ctx->buffer_size = initial_size;
    ctx->offset = 0;
    return ctx;
}

void ultra_serdes_free(ultra_serdes_context *ctx) {
    if (!ctx) return;
    if (ctx->buffer) free(ctx->buffer);
    free(ctx);
}

ultra_serdes_result ultra_ser_field(ultra_serdes_context *ctx, field_descriptor *desc) {
    if (!ctx || !desc || !desc->data) return ULTRA_SERDES_ERR_INVALID;

    if (desc->type == FIELD_VALUE) {
        uint8_t *temp = malloc(desc->size);
        if (!temp) return ULTRA_SERDES_ERR_MEMORY;
        memcpy(temp, desc->data, desc->size);
        if (desc->callback) desc->callback(temp, desc->size, desc->user_data);
        ultra_serdes_result res = ultra_ser_to_buffer(ctx, temp, desc->size);
        free(temp);
        return res;
    }
    if (desc->type == FIELD_STRING) {
        const char *str = *(const char **)desc->data;
        size_t len = strlen(str) + 1;
        uint32_t net_len = manual_htonl((uint32_t)len);
        ultra_serdes_result res = ultra_ser_to_buffer(ctx, &net_len, sizeof(uint32_t));
        if (res != ULTRA_SERDES_OK) return res;
        return ultra_ser_to_buffer(ctx, (void *)str, len);
    }
    if (desc->type == FIELD_BUFFER) {
        uint32_t net_size = manual_htonl((uint32_t)desc->size);
        ultra_serdes_result res = ultra_ser_to_buffer(ctx, &net_size, sizeof(uint32_t));
        if (res != ULTRA_SERDES_OK) return res;
        return ultra_ser_to_buffer(ctx, desc->data, desc->size);
    }
    return ULTRA_SERDES_ERR_INVALID;
}

ultra_serdes_result ultra_des_field(ultra_serdes_context *ctx, field_descriptor *desc) {
    if (!ctx || !desc || !desc->data) return ULTRA_SERDES_ERR_INVALID;

    if (desc->type == FIELD_VALUE) {
        uint8_t *temp = malloc(desc->size);
        if (!temp) return ULTRA_SERDES_ERR_MEMORY;
        ultra_serdes_result res = ultra_des_from_buffer(ctx, temp, desc->size);
        if (res == ULTRA_SERDES_OK && desc->callback) desc->callback(temp, desc->size, desc->user_data);
        if (res == ULTRA_SERDES_OK) memcpy(desc->data, temp, desc->size);
        free(temp);
        return res;
    }
    if (desc->type == FIELD_STRING) {
        uint32_t len;
        ultra_serdes_result res = ultra_des_from_buffer(ctx, &len, sizeof(uint32_t));
        if (res != ULTRA_SERDES_OK) return res;
        len = manual_ntohl(len);
        char *str = malloc(len);
        if (!str) return ULTRA_SERDES_ERR_MEMORY;
        res = ultra_des_from_buffer(ctx, str, len);
        if (res != ULTRA_SERDES_OK) {
            free(str);
            return res;
        }
        *(char **)desc->data = str;
        return ULTRA_SERDES_OK;
    }
    if (desc->type == FIELD_BUFFER) {
        uint32_t len;
        ultra_serdes_result res = ultra_des_from_buffer(ctx, &len, sizeof(uint32_t));
        if (res != ULTRA_SERDES_OK) return res;
        len = manual_ntohl(len);
        void *buf = malloc(len);
        if (!buf) return ULTRA_SERDES_ERR_MEMORY;
        res = ultra_des_from_buffer(ctx, buf, len);
        if (res != ULTRA_SERDES_OK) {
            free(buf);
            return res;
        }
        *(void **)desc->data = buf;
        desc->size = len;
        return ULTRA_SERDES_OK;
    }
    return ULTRA_SERDES_ERR_INVALID;
}

ultra_serdes_result ultra_ser_to_buffer(ultra_serdes_context *ctx, void *data, size_t size) {
    if (ctx->offset + size > ctx->buffer_size) {
        size_t new_size = ctx->buffer_size * 2 + size;
        uint8_t *new_buffer = realloc(ctx->buffer, new_size);
        if (!new_buffer) return ULTRA_SERDES_ERR_MEMORY;
        memset(new_buffer + ctx->buffer_size, 0, new_size - ctx->buffer_size);
        ctx->buffer = new_buffer;
        ctx->buffer_size = new_size;
    }

    memcpy(ctx->buffer + ctx->offset, data, size);
    ctx->offset += size;
    return ULTRA_SERDES_OK;
}

ultra_serdes_result ultra_des_from_buffer(ultra_serdes_context *ctx, void *data, size_t size) {
    if (ctx->offset + size > ctx->buffer_size) return ULTRA_SERDES_ERR_INVALID;

    memcpy(data, ctx->buffer + ctx->offset, size);
    ctx->offset += size;
    return ULTRA_SERDES_OK;
}

void ultra_ser_hton(void *data, size_t size, void *user_data) {
    (void)user_data;
    if (size == 4) {
        *(uint32_t *)data = manual_htonl(*(uint32_t *)data);
    } else if (size == 2) {
        *(uint16_t *)data = manual_htons(*(uint16_t *)data);
    }
}

void ultra_des_ntoh(void *data, size_t size, void *user_data) {
    (void)user_data;
    if (size == 4) {
        *(uint32_t *)data = manual_ntohl(*(uint32_t *)data);
    } else if (size == 2) {
        *(uint16_t *)data = manual_ntohs(*(uint16_t *)data);
    }
}

char *ultra_serdes_buffer_to_hex(ultra_serdes_context *ctx) {
    if (!ctx || !ctx->buffer || ctx->offset == 0) return NULL;

    char *hex_str = malloc(ctx->offset * 2 + 1);
    if (!hex_str) return NULL;

    for (size_t i = 0; i < ctx->offset; i++) {
        sprintf(hex_str + i * 2, "%02x", ctx->buffer[i]);
    }
    hex_str[ctx->offset * 2] = '\0';
    return hex_str;
}

ultra_serdes_result ultra_serdes_hex_to_buffer(ultra_serdes_context *ctx, const char *hex_str) {
    if (!ctx || !hex_str) return ULTRA_SERDES_ERR_INVALID;

    size_t len = strlen(hex_str);
    if (len % 2 != 0) return ULTRA_SERDES_ERR_INVALID;

    size_t buffer_size = len / 2;
    if (ctx->buffer_size < buffer_size) {
        uint8_t *new_buffer = realloc(ctx->buffer, buffer_size);
        if (!new_buffer) return ULTRA_SERDES_ERR_MEMORY;
        ctx->buffer = new_buffer;
        ctx->buffer_size = buffer_size;
    }

    ctx->offset = 0;
    for (size_t i = 0; i < len; i += 2) {
        char byte_str[3] = {hex_str[i], hex_str[i + 1], '\0'};
        ctx->buffer[ctx->offset++] = (uint8_t)strtol(byte_str, NULL, 16);
    }

    return ULTRA_SERDES_OK;
}
