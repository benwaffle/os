.set ALIGN, 1<<0
.set MEMINFO, 1<<1
.set FLAGS, MEMINFO | ALIGN
.set MAGIC, 0x1BADB002
.set CHECKSUM, -(FLAGS + MAGIC)

# multiboot (grub) header
.section multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# set up stack
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KB
stack_top:

# entry point
.section .text
.global _start
.type _start, @function
_start:
    mov $stack_top, %esp
    call kernel_main
    cli
1:  hlt
    jmp 1b

# size of _start, useful for debugging or call tracing
.size _start, . - _start
