AS=i686-elf-as
CC=i686-elf-gcc
LD=i686-elf-ld

CFLAGS = -std=c11 -ffreestanding -nostdlib -Wall -Wextra
LDFLAGS = -lgcc -Wl,--build-id=none

os.iso: os.bin
	cp $< isodir/boot
	grub-mkrescue -o os.iso isodir

os.bin: boot.o kernel.o linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -T linker.ld -o $@ boot.o kernel.o

boot.o: boot.s
kernel.o: kernel.c

run: os.iso
	qemu-system-x86_64 -cdrom $<

.PHONY: clean
clean:
	$(RM) boot.o
	$(RM) kernel.o
	$(RM) os.bin
	$(RM) os.iso
