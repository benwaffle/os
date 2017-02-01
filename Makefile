CFLAGS = -m32 -std=c11 -ffreestanding -nostdlib -Wl,--build-id=none -Wall
LDFLAGS = -Wl,--build-id=none -lgcc
ASFLAGS = --32 -march=i686

os.iso: os.bin
	cp $< isodir/boot
	grub-mkrescue -o os.iso isodir

os.bin: boot.o kernel.o linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -T linker.ld -o $@ boot.o kernel.o

boot.o: boot.s
kernel.o: kernel.c

run: os.iso
	qemu-system-i386 -cdrom $<

.PHONY: clean
clean:
	$(RM) boot.o
	$(RM) kernel.o
	$(RM) os.bin
	$(RM) os.iso
