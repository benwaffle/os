#include <stdbool.h>
#include <stdint.h>

#define NULL ((void*)0)

typedef uint8_t u8;
typedef uint16_t u16;

typedef struct {
    u8 type;
    u8 length;
    u16 handle;
} smbios_header;

typedef struct {
    char magic[4]; // _SM_
    u8 checksum;
    u8 length;
    u8 major;
    u8 minor;
    u16 max_struct_size;
    u8 entry_point_revision;
    u8 formatted_area[5];
    u8 ieps_magic[5]; // _DMI_
    u8 ieps_checksum;
    u16 table_length;
    smbios_header *table;
    u16 num_structs;
    u8 bcd_revision;
} smbios_entry;

typedef enum {
    BIOS_INFO = 0,
    SYS_INFO = 1,
    ENCLOSURE = 3,
    PROCESSOR = 4,
} smbios_type;

typedef struct {
    smbios_header header;
    u8 vendor;
    u8 version;
} bios_info;

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

smbios_header *find_next_header(smbios_header *h) {
    u8 *mem = (u8*)h + h->length;
    while (!(mem[0] == 0 && mem[1] == 0))
        ++mem;
    mem += 2;
    return (smbios_header*)mem;
}

char *get_string(smbios_header *h, u8 index) {
    char *strings = (char*)h + h->length;
    for (int i = 0; i < index - 1; ++i) {
        while (*strings)
            strings++;
        strings++;
    }
    return strings;
}

smbios_entry *find_smbios() {
    char *mem = (char*)0xF0000;
    char *end = (char*)0xFFFFFF;

    while (mem <= end) {
        if (memcmp(mem, "_SM_", 4) == 0) {
            uint8_t length = mem[5];
            uint8_t checksum = 0;
            for (int i = 0; i < length; ++i)
                checksum += mem[i];

            if (checksum == 0)
                return (smbios_entry*)mem;
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
        if (len == 0)
            len = 1;
        char out[len + 1];
        for (int k = 0; k < len; ++k) {
            out[len - k - 1] = (i % 10) + '0';
            i /= 10;
        }
        out[len] = 0;

        color = make_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        puts(out);
        color = make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    } else if (s[1] == 'x') {
        int len = 0;
        int j = i;
        while (j) {
            len++;
            j /= 16;
        }
        if (len == 0)
            len = 1;
        char out[len + 1];
        for (int k = 0; k < len; ++k) {
            out[len - k - 1] = hex[i % 16];
            i /= 16;
        }
        out[len] = 0;

        color = make_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        puts("0x");
        puts(out);
        color = make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    }
}

void kernel_main() {
    for (int i = 0; i < width * height; ++i)
        putchar(' ');

    color = make_color(VGA_COLOR_BLUE, VGA_COLOR_LIGHT_RED);
    puts("\
                          \n\
 ~~~ Welcome to BenOS ~~~ \n\
                          \n\
");
    color = make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    smbios_entry *smbios = find_smbios();

    if (smbios) {
        puts("smbios found at ");
        printf("%x", (uint32_t)smbios);
        putchar('\n');

        puts("table at ");
        printf("%x", smbios->table);
        putchar('\n');

        puts("version ");
        printf("%d", smbios->major);
        puts(".");
        printf("%d", smbios->minor);
        puts("\n");

        puts("num structs ");
        printf("%d", smbios->num_structs);
        puts("\n");

        smbios_header *header = smbios->table;
        for (int i = 0; i < smbios->num_structs; ++i) {
            puts("header type ");
            printf("%d", header->type);
            puts("\n");

            if (header->type == BIOS_INFO) {
                bios_info *info = (bios_info*)header;
                puts("Vendor: ");
                puts(get_string(header, info->vendor));
                puts(" version: ");
                puts(get_string(header, info->version));
                puts("\n");
            }
            header = find_next_header(header);
        }
    } else {
        puts("smbios not found");
    }
}
