#include "types.h"
#include "limine.h"

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
    u32 table;
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

static volatile struct limine_smbios_request smbios_request = {
    .id = LIMINE_SMBIOS_REQUEST,
    .revision = 0,
};

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

extern void printf(const char *, ...);

void print_smbios() {
    smbios_entry *smbios = smbios_request.response->entry_32;

    if (smbios) {
        printf("smbios found at %x\n", smbios);
        printf("table at %x\n", smbios->table);
        printf("version %d.%d\n", smbios->major, smbios->minor);
        printf("%d structs\n", smbios->num_structs);

        printf("\n");

        smbios_header *header = (void*)(u64)smbios->table;
        for (int i = 0; i < smbios->num_structs; ++i) {
            printf("header type %d\n", header->type);

            if (header->type == BIOS_INFO) {
                bios_info *info = (bios_info*)header;
                printf("Vendor [%s], version [%s]\n", get_string(header, info->vendor), get_string(header, info->version));
            }
            header = find_next_header(header);
        }
    } else {
        printf("smbios not found\n");
    }

}
