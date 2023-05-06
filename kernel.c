#include <stdbool.h>
#include <stdint.h>

#define NULL ((void*)0)

uint8_t streq(char *a, char *b) {
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

char *find_smbios() {
    char *mem = (char*)0xF0000;
    char *end = (char*)0xFFFFFF;

    while (mem <= end) {
        if (memcmp(mem, "_SM_", 4)) {
            uint8_t length = mem[5];
            uint8_t checksum = 0;
            for (int i = 0; i < length; ++i)
                checksum += mem[i];

            if (checksum == 0)
                return mem;
        }

        mem += 16;
    }
    return NULL;
}

typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
} vga_color;

uint8_t make_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

uint16_t color_char(unsigned char c, uint8_t color) {
    return (uint16_t) c | (uint16_t) color << 8;
}

uint16_t *vgatext = (uint16_t*)0xB8000;
uint8_t row = 0;
uint8_t col = 0;
vga_color color;
const int width = 80;
const int height = 25;

void putchar(char c) {
    if (c == '\n') {
        ++row;
        if (row == height)
            row = 0;
        col = 0;
        return;
    }

    vgatext[row * width + col] = color_char(c, color);
    ++col;
    if (col == width) {
        col = 0;
        ++row;
        if (row == height)
            row = 0;
    }
}

void puts(char *s) {
    while (*s) {
        putchar(*s);
        ++s;
    }
}

const char *hex = "0123456789abcdef";

void printf(char *s, uint32_t i) {
    if (s[1] == 'd') {
        int len = 0;
        int j = i;
        while (j) {
            len++;
            j /= 10;
        }
        char out[len + 1];
        for (int k = 0; k < len; ++k) {
            out[len - k - 1] = (i % 10) + '0';
            i /= 10;
        }
        out[len] = 0;
        puts(out);
    } else if (s[1] == 'x') {
        puts("0x");
        int len = 0;
        int j = i;
        while (j) {
            len++;
            j /= 16;
        }
        char out[len + 1];
        for (int k = 0; k < len; ++k) {
            out[len - k - 1] = hex[i % 16];
            i /= 16;
        }
        out[len] = 0;
        puts(out);
    }
}

void kernel_main() {
    color = make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    for (int i = 0; i < width * height; ++i)
        putchar(' ');

    char *smbios = find_smbios();

    if (smbios) {
        puts("smbios found at ");
        printf("%x", (uint32_t)smbios);
        putchar('\n');
    } else {
        puts("smbios not found");
    }

    puts("now what\n");
}
