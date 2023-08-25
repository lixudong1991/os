nasm -f elf32 mem.asm -o obj/mem.o
nasm -f elf32 puts.asm -o obj/puts.o
nasm -f elf32 boot.asm -o obj/boot.o
nasm -f elf32 harddisk.asm -o obj/harddisk.o
nasm -f elf32 syscall.asm -o obj/syscall.o
nasm -f elf32 interruptGate.asm -o obj/interruptGate.o
i686-elf-gcc  -c main.c -o obj/main.o -ffreestanding
i686-elf-gcc  -c disk.c -o obj/disk.o -ffreestanding
i686-elf-gcc  -c string.c -o obj/string.o -ffreestanding
i686-elf-gcc  -c table.c -o obj/table.o -ffreestanding
i686-elf-gcc  -c memory.c -o obj/memory.o -ffreestanding
i686-elf-gcc  -c elf.c -o obj/elf.o -ffreestanding
i686-elf-gcc  -c printf.c -o obj/printf.o -ffreestanding

#i686-elf-gcc -T link.ld -Ttext 0x9000 -o main obj/main.o obj/puts.o obj/mem.o obj/boot.o obj/disk.o obj/harddisk.o obj/table.o obj/string.o obj/syscall.o -nostdlib -ffreestanding -lgcc

i686-elf-gcc -Ttext 0xc0036000 -o main obj/main.o obj/puts.o obj/mem.o obj/memory.o obj/elf.o obj/boot.o obj/disk.o obj/harddisk.o obj/table.o obj/string.o obj/syscall.o obj/interruptGate.o obj/printf.o -nostdlib -ffreestanding -lgcc
cat diskdata.txt >> main
# i686-elf-as boot.s -o boot.o
# i686-elf-gcc kernel.c -o kernel.o -ffreestanding
# i686-elf-gcc -T link.ld boot.o kernel.o -o kernel.bin -nostdlib -ffreestanding -lgcc
i686-elf-readelf -l main

nasm -f bin osstart.asm -o osstart.bin

nasm -f elf32 syslib.asm -o syslib.o
i686-elf-gcc  -c main.c -o main.o -ffreestanding
i686-elf-gcc main.o syslib.o -o a.out -nostdlib -ffreestanding -lgcc
cat diskdata.txt >> a.out
