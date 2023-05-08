AS=x86_64-elf-as
CC=x86_64-elf-gcc
LD=x86_64-elf-ld

CFLAGS = -std=gnu11 -ffreestanding -nostdlib -Wall -Wextra -fPIC
LDFLAGS = -lgcc

os.iso: os.bin
	cp $< isodir
	xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		isodir -o $@

os.bin: kernel.o linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -T linker.ld -o $@ kernel.o

kernel.o: kernel.c

run: os.iso
	qemu-system-x86_64 -cdrom $< -serial stdio

.PHONY: clean
clean:
	$(RM) kernel.o
	$(RM) os.bin
	$(RM) os.iso
