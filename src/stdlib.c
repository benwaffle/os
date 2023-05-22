#include "types.h"

bool streq(char *a, char *b) {
    while (*a && *b && *a == *b) {
        ++a;
        ++b;
    }
    return *a == *b;
}

int memcmp(const char *a, const char *b, uint16_t n) {
    for (uint16_t i = 0; i < n; ++i)
        if (a[i] != b[i])
            return 1;
    return 0;
}

u64 strlen(const char *s) {
    u64 len = 0;
    while (*s) {
        ++len;
        ++s;
    }
    return len;
}

void outb(u16 port, char data) {
    asm volatile("outb %1, %0" : : "dN" (port), "a" (data));
}

u8 inb(u16 port) {
    u8 rv;
    asm volatile("inb %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}
