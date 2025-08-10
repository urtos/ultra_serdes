#include "ultra_serdes.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    uint32_t id;
    char *name;
    float score;
} Person;

int main() {
    Person p = {1, strdup("Alice"), 95.5f};
    if (!p.name) {
        printf("Memory allocation failed\n");
        return 1;
    }

    ultra_serdes_context *mem_ctx = ultra_serdes_init(1024);
    if (!mem_ctx) {
        printf("Failed to initialize memory context\n");
        free(p.name);
        return 1;
    }

    field_descriptor fields[] = {
        {FIELD_VALUE, &p.id, sizeof(p.id), ultra_ser_hton, NULL},
        {FIELD_STRING, &p.name, 0, NULL, NULL},
        {FIELD_VALUE, &p.score, sizeof(p.score), NULL, NULL}
    };
    for (int i = 0; i < 3; i++) {
        if (ultra_ser_field(mem_ctx, &fields[i]) != ULTRA_SERDES_OK) {
            printf("Serialization failed\n");
            free(p.name);
            ultra_serdes_free(mem_ctx);
            return 1;
        }
    }

    char *hex_str = ultra_serdes_buffer_to_hex(mem_ctx);
    if (hex_str) {
        printf("Serialized u8 string: %s\n", hex_str);
    }

    ultra_serdes_context *ctx2 = ultra_serdes_init(1024);
    if (ultra_serdes_hex_to_buffer(ctx2, hex_str) != ULTRA_SERDES_OK) {
        printf("Failed to convert hex string to buffer\n");
        free(hex_str);
        free(p.name);
        ultra_serdes_free(mem_ctx);
        return 1;
    }
    free(hex_str);

    Person p2 = {0, NULL, 0.0f};
    field_descriptor fields2[] = {
        {FIELD_VALUE, &p2.id, sizeof(p2.id), ultra_des_ntoh, NULL},
        {FIELD_STRING, &p2.name, 0, NULL, NULL},
        {FIELD_VALUE, &p2.score, sizeof(p2.score), NULL, NULL}
    };
    ctx2->offset = 0;
    for (int i = 0; i < 3; i++) {
        if (ultra_des_field(ctx2, &fields2[i]) != ULTRA_SERDES_OK) {
            printf("Deserialization failed\n");
            free(p.name);
            free(p2.name);
            ultra_serdes_free(mem_ctx);
            ultra_serdes_free(ctx2);
            return 1;
        }
    }

    printf("Deserialized: id=%u, name=%s, score=%.1f\n", p2.id, p2.name, p2.score);

    free(p.name);
    free(p2.name);
    ultra_serdes_free(mem_ctx);
    ultra_serdes_free(ctx2);
    
    return 0;
}
