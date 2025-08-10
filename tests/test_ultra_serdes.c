#include "ultra_serdes.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    uint32_t id;
    char *name;
    float score;
} Person;

void test_byte_order_preservation() {
    uint32_t original32 = 1;
    uint16_t original16 = 1;
    uint32_t data32 = original32;
    uint16_t data16 = original16;

    uint32_t converted32 = manual_htonl(data32);
    assert(converted32 == 0x01000000);
    assert(data32 == original32);
    uint32_t back32 = manual_ntohl(converted32);
    assert(back32 == original32);

    uint16_t converted16 = manual_htons(data16);
    assert(converted16 == 0x0100);
    assert(data16 == original16);
    uint16_t back16 = manual_ntohs(converted16);
    assert(back16 == original16);

    data32 = original32;
    ultra_ser_hton(&data32, sizeof(data32), NULL);
    assert(data32 == 0x01000000);
    ultra_des_ntoh(&data32, sizeof(data32), NULL);
    assert(data32 == original32);

    uint32_t len = 11;
    uint32_t net_len = manual_htonl(len);
    char len_hex[9];
    sprintf(len_hex, "%08x", net_len);
    printf("Length field hex: %s\n", len_hex);
    assert(strcmp(len_hex, "0b000000") == 0);

    printf("Byte order preservation test passed\n");
}

void test_string_serialization() {
    char *original = strdup("TestString");
    assert(original != NULL);
    char *str = original;

    ultra_serdes_context *ctx = ultra_serdes_init(1024);
    assert(ctx != NULL);

    field_descriptor desc = {FIELD_STRING, &str, 0, NULL, NULL};
    assert(ultra_ser_field(ctx, &desc) == ULTRA_SERDES_OK);
    assert(strcmp(original, "TestString") == 0);

    char *hex_str = ultra_serdes_buffer_to_hex(ctx);
    assert(hex_str != NULL);
    printf("String serialized hex: %s\n", hex_str);
    const char *expected_hex = "0000000b54657374537472696e6700";
    assert(strcmp(hex_str, expected_hex) == 0);

    ultra_serdes_context *ctx2 = ultra_serdes_init(1024);
    assert(ultra_serdes_hex_to_buffer(ctx2, hex_str) == ULTRA_SERDES_OK);
    free(hex_str);

    char *deserialized = NULL;
    field_descriptor desc2 = {FIELD_STRING, &deserialized, 0, NULL, NULL};
    ctx2->offset = 0;
    assert(ultra_des_field(ctx2, &desc2) == ULTRA_SERDES_OK);
    printf("String deserialized: %s\n", deserialized);
    assert(strcmp(deserialized, "TestString") == 0);

    free(original);
    free(deserialized);
    ultra_serdes_free(ctx);
    ultra_serdes_free(ctx2);
    printf("String serialization test passed\n");
}

void test_memory_serialization() {
    Person p = {1, strdup("Alice"), 95.5f};
    assert(p.name != NULL);
    printf("Original: id=%u, name=%s, score=%.1f\n", p.id, p.name, p.score);

    ultra_serdes_context *ctx = ultra_serdes_init(1024);
    assert(ctx != NULL);

    field_descriptor fields[] = {
        {FIELD_VALUE, &p.id, sizeof(p.id), ultra_ser_hton, NULL},
        {FIELD_STRING, &p.name, 0, NULL, NULL},
        {FIELD_VALUE, &p.score, sizeof(p.score), NULL, NULL}
    };
    for (int i = 0; i < 3; i++) {
        assert(ultra_ser_field(ctx, &fields[i]) == ULTRA_SERDES_OK);
    }
    printf("After serialization: id=%u, name=%s, score=%.1f\n", p.id, p.name, p.score);

    char *hex_str = ultra_serdes_buffer_to_hex(ctx);
    assert(hex_str != NULL);
    printf("Serialized hex string: %s\n", hex_str);
    const char *expected_hex = "0000000100000006416c696365000000bf42";
    assert(strcmp(hex_str, expected_hex) == 0);

    ultra_serdes_context *ctx2 = ultra_serdes_init(1024);
    assert(ultra_serdes_hex_to_buffer(ctx2, hex_str) == ULTRA_SERDES_OK);
    free(hex_str);

    Person p2 = {0, NULL, 0.0f};
    field_descriptor fields2[] = {
        {FIELD_VALUE, &p2.id, sizeof(p2.id), ultra_des_ntoh, NULL},
        {FIELD_STRING, &p2.name, 0, NULL, NULL},
        {FIELD_VALUE, &p2.score, sizeof(p2.score), NULL, NULL}
    };
    ctx2->offset = 0;
    for (int i = 0; i < 3; i++) {
        assert(ultra_des_field(ctx2, &fields2[i]) == ULTRA_SERDES_OK);
    }

    printf("Deserialized: id=%u, name=%s, score=%.1f\n", p2.id, p2.name, p2.score);
    assert(p.id == p2.id);
    assert(strcmp(p.name, p2.name) == 0);
    assert(p.score == p2.score);

    free(p.name);
    free(p2.name);
    ultra_serdes_free(ctx);
    ultra_serdes_free(ctx2);
    printf("Memory serialization test passed\n");
}

int main() {
    test_byte_order_preservation();
    test_string_serialization();
    test_memory_serialization();
    printf("All tests passed!\n");
    return 0;
}
