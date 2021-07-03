#include <stdint.h>

void kernel_main() {
    uint16_t *buf = 0xB8000;
    char msg[] = "Welcome to BenOS";
    for (int i=0; i<sizeof(msg)-1; ++i) {
        (buf+32 + 80*11)[i] = msg[i] | ((4 | 1 << 4) << 8);
    }
}
