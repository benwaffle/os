AS=x86_64-elf-as
CC=x86_64-elf-gcc
LD=x86_64-elf-ld

CFLAGS = -std=gnu11 -ffreestanding -nostdlib -Wall -Wextra -fPIC
LDFLAGS = -lgcc

OBJS = src/kernel.o \
		src/smbios.o \
		src/stdlib.o \
		src/console.o \
		src/lemon.o

os.iso: os.bin
	cp $< isodir
	xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		isodir -o $@

os.bin: $(OBJS) linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -T linker.ld -o $@ $(OBJS)

run: os.iso
	qemu-system-x86_64 \
		-drive if=pflash,format=raw,unit=0,file=OVMF_CODE.fd,readonly=on \
		-drive if=pflash,format=raw,unit=1,file=OVMF_VARS.fd \
		-cdrom $< -serial stdio

.PHONY: clean
clean:
	$(RM) $(OBJS)
	$(RM) os.bin
	$(RM) os.iso
