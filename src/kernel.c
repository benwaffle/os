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
_Static_assert(sizeof(IdtEnt) == 16); // x86-64

IdtEnt idt[256] = {0};

void myInterrupt() {
    printf("interrupted!!!\n");
    picEoi(2);
    // TODO: do I need iret?
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
