#include <stdarg.h>
#include "console.h"
#include "types.h"
#include "limine.h"
#include "font.h"

extern const struct bitmap_font font;

static volatile struct limine_framebuffer_request fb_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};

const u16 COM1 = 0x3f8;
u8 row = 0;
u8 col = 0;

void init_serial() {
    outb(COM1 + 1, 0x00);    // Disable all interrupts
    outb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1 + 1, 0x00);    //                  (hi byte)
    outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    //outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    //outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    // set it in normal operation mode
   // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(COM1 + 4, 0x0F);
}

bool serial_tx_empty() {
    return inb(COM1 + 5) & 0b0100000;
}

struct limine_framebuffer *getFb() {
    return fb_request.response->framebuffers[0];
}

int fb_chars_height() {
  // at 2x scale
  int px = getFb()->height;
  return px / font.Height / 2;
}

int fb_chars_width() {
  // at 2x scale
  int px = getFb()->width;
  return px / font.Width; // TODO check widths array
}

void draw_real_pixel(struct limine_framebuffer *fb, uint16_t x, uint16_t y, uint32_t color) {
    uint16_t bytes_per_pixel = fb->bpp / 8;
    uint64_t index = y * fb->pitch + x * bytes_per_pixel;

    ((uint32_t*)fb->address)[index / 4] = color;
}

void draw_pixel(struct limine_framebuffer *fb, uint16_t x, uint16_t y, uint32_t color) {
    uint8_t red = color >> 16;
    uint8_t green = color >> 8;
    uint8_t blue = color;

    uint32_t screen_color = (red << fb->red_mask_shift) | (green << fb->green_mask_shift) | (blue << fb->blue_mask_shift);

    // render at 2x
    draw_real_pixel(fb, x * 2, y * 2, screen_color);
    draw_real_pixel(fb, x * 2, y * 2 + 1, screen_color);
    draw_real_pixel(fb, x * 2 + 1, y * 2, screen_color);
    draw_real_pixel(fb, x * 2 + 1, y * 2 + 1, screen_color);
}

void draw_char(char c, u8 row, u8 col, u32 color) {
    int index = 0;
    for (int j = 0; j < font.Chars; ++j) {
        if (font.Index[j] == (unsigned)c) {
            index = j;
            break;
        }
    }

    int x = col * font.Widths[index];
    int y = row * font.Height;

    index *= font.Height * 2;
    for (int j = 0; j < font.Height * 2; j += 2) {
        char glyph = font.Bitmap[index + j];
        for (s8 bit = 7; bit >= 0; --bit) {
            u32 pixel_color = ((glyph >> bit) & 1) ? color : 0;
            draw_pixel(getFb(), x + 8-bit-1, y + j/2, pixel_color);
        }
    }
}

void putchar(output o, char c) {
    if (o == serial) {
        while (!serial_tx_empty())
            ;

        outb(COM1, c);
    } else if (o == fb) {
        if (c == '\n') {
            ++row;
            if (row == fb_chars_height())
                row = 0;
            col = 0;
            return;
        }

        if (c == '\t') {
            col += 4;
            if (col > fb_chars_width()) {
                col = 0;
                row++;
                if (row == fb_chars_height())
                    row = 0;
            }
            return;
        }

        if (c == '\b') {
            if (col == 0) {
                col = fb_chars_width() - 1;
                if (row != 0)
                    --row;
            } else {
                --col;
            }

            draw_char(' ', row, col, 0xffffff);
            return;
        }

        draw_char(c, row, col, 0xffffff);
        ++col;
        if (col == fb_chars_width()) {
            col = 0;
            ++row;
            if (row == fb_chars_height())
                row = 0;
        }
    }
}

void puts(output o, char *s) {
    while (*s) {
        putchar(o, *s);
        ++s;
    }
}

const char *hex = "0123456789abcdef";

int numeric_width(int i, int base) {
    int len = 0;

    while (i) {
        len++;
        i /= base;
    }

    if (len == 0)
        len = 1;
    return len;
}

void print_number(output out, int i, int base) {
    int len = numeric_width(i, base);
    char result[len + 1];
    for (int k = 0; k < len; ++k) {
        result[len - k - 1] = hex[i % base];
        i /= base;
    }
    result[len] = 0;
    puts(out, result);
}

void vfprintf(const output out, const char *fmt, va_list args) {
    while (*fmt) {
        if (fmt[0] == '%') {
            if (fmt[1] == 'd') {
                print_number(out, va_arg(args, int), 10);
            } else if (fmt[1] == 'x') {
                puts(out, "0x");
                print_number(out, va_arg(args, uint64_t), 16);
            } else if (fmt[1] == 's') {
                puts(out, va_arg(args, char*));
            } else if (fmt[1] == 'c') {
                putchar(out, va_arg(args, int));
            } else {
                puts(out, "%?");
            }
            ++fmt;
        } else {
            putchar(out, *fmt);
        }

        ++fmt;
    }
}

void fprintf(const output out, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vfprintf(out, fmt, args);

    va_end(args);
}

void printf(const char *fmt, ...) {
    va_list args1, args2;
    va_start(args1, fmt);
    va_copy(args2, args1);

    vfprintf(fb, fmt, args1);
    vfprintf(serial, fmt, args2);

    va_end(args1);
    va_end(args2);
}
