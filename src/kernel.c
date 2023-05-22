#include <stdarg.h>

#include "types.h"
#include "uefi.h"
#include "limine.h"
#include "console.h"

static volatile struct limine_efi_system_table_request efi_request = {
    .id = LIMINE_EFI_SYSTEM_TABLE_REQUEST,
    .revision = 0,
};

extern void print_smbios();

void _start() {

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

    asm("cli");
    while (true)
        asm("hlt");
}
