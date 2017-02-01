#include <stdint.h>

void kernel_main() {
    uint16_t *buf = 0xB8000;
    char msg[] = "it works!";
    for (int i=0; i<sizeof(msg)-1; ++i) {
        buf[i] = msg[i] | ((4 | 2 << 4) << 8);
    }
}
