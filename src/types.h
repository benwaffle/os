#pragma once

#include <stdint.h>
#include <stdbool.h>

#define NULL ((void*)0)

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

u64 strlen(const char *);
int memcmp(const char *a, const char *b, uint16_t n);
bool streq(char *a, char *b);

void outb(u16 port, char data);
u8 inb(u16 port);
