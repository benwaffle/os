#pragma once

typedef enum {
    serial,
    fb,
} output;

void init_serial();
void puts(output o, char *s);
void fprintf(output o, const char *fmt, ...);
void printf(const char *fmt, ...);
