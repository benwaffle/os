#include "8259.h"
#include "types.h"

#define MASTER_CMD  0x20
#define MASTER_DATA 0x21
#define SLAVE_CMD   0xA0
#define SLAVE_DATA  0xA1

#define EOI 0x20
#define ICW1_INIT	0x10		/* Initialization - required! */
#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */

void picEoi(u8 irq) {
    if (irq >= 8)
        outb(SLAVE_CMD, EOI);

    outb(MASTER_CMD, EOI);
}

void picRemap(u8 offset) {
    u8 masterMask = inb(MASTER_DATA);
    u8 slaveMask = inb(SLAVE_DATA);

    outb(MASTER_CMD, ICW1_INIT | ICW1_ICW4);
    outb(SLAVE_CMD,  ICW1_INIT | ICW1_ICW4);

    outb(MASTER_DATA, offset);
    outb(SLAVE_DATA,  offset + 8);

    outb(MASTER_DATA, 1 << 2); // slave at IRQ2
    outb(SLAVE_DATA,  0b10); // arbitrary? slave id number

    outb(MASTER_DATA, ICW4_8086);
    outb(SLAVE_DATA, ICW4_8086);

    outb(MASTER_DATA, masterMask);
    outb(SLAVE_DATA, slaveMask);
}

void picUnmask(u8 irq) {
    if (irq < 8) {
        u8 mask = inb(MASTER_DATA);
        mask &= ~(1 << irq);
        outb(MASTER_DATA, mask);
    } else {
        u8 mask = inb(SLAVE_DATA);
        mask &= ~(1 << (irq - 8));
        outb(SLAVE_DATA, mask);
    }
}
