#include <stdarg.h>

#include "types.h"
#include "uefi.h"
#include "limine.h"
#include "console.h"
#include "8259.h"

static volatile struct limine_efi_system_table_request efi_request = {
    .id = LIMINE_EFI_SYSTEM_TABLE_REQUEST,
    .revision = 0,
};

extern void print_smbios();

typedef struct {
    u16 offset1;
    u16 segSel;
    u8 ist;
    u8 attrs;
    u16 offset2;
    u32 offset3;
    u32 zero;
} IdtEnt;
_Static_assert(sizeof(IdtEnt) == 16, "IDT entry must be 16 bytes on x86-64");

IdtEnt idt[256] = {0};

u8 scanCodes[256] = {
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0a] = '9',
    [0x0b] = '0',
    [0x0c] = '-',
    [0x0d] = '=',
    [0x0e] = '\b',
    [0x0f] = '\t',
    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1a] = '[',
    [0x1b] = ']',
    [0x1c] = '\n',
    //[0x1d] = 'left ctrl',
    [0x1e] = 'a',
    [0x1f] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',
    //[0x2a] = 'left shift',
    [0x2b] = '\\',
    [0x2c] = 'z',
    [0x2d] = 'x',
    [0x2e] = 'c',
    [0x2f] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',
};

__attribute__((interrupt)) void myInterrupt(void*) {
    u8 c = scanCodes[inb(0x60)];
    if (c)
        putchar(c);
    picEoi(1);
}

static inline void lidt(void* base, uint16_t size) {
    struct {
        uint16_t length;
        void*    base;
    } __attribute__((packed)) IDTR = { size, base };

    asm ( "lidt %0" : : "m"(IDTR) );  // let the compiler choose an addressing mode
}

void initIdt() {
    idt[33] = (IdtEnt){
        .offset1 = (u64)(&myInterrupt) & 0xFFFF,
        .segSel = 5 * 8, // limine 64-bit code segment
        .ist = 0,
        .attrs = 0x8E,
        .offset2 = ((u64)(&myInterrupt) >> 16) & 0xFFFF,
        .offset3 = ((u64)(&myInterrupt) >> 32) & 0xFFFFFFFF,
        .zero = 0,
    };

    lidt(idt, sizeof(idt));
}

void _start() {

    asm("cli");
    initIdt();
    asm("sti");

    // 0-31 for x86 exceptions
    // 32-48 for PIC interrupts
    // 32 - PIT
    // 33 - PS/2 keyboard
    picRemap(32);
    picUnmask(1);

    init_serial();

    printf("\
                          \n\
 ~~~ Welcome to BenOS ~~~ \n\
                          \n\
");

    print_smbios();

    efi_system_table_t *ST = (efi_system_table_t*)(efi_request.response->address);
    for (uint16_t *c = ST->FirmwareVendor; *c; ++c)
        putchar((uint8_t)*c);
    putchar('\n');
    printf("st %x\n", ST);
    printf("rs %x\n", ST->RuntimeServices);

    while (true)
        asm("hlt");
}
