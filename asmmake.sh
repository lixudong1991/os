nasm -f elf32 mem.asm -o obj/mem.o
nasm -f elf32 puts.asm -o obj/puts.o
nasm -f elf32 boot.asm -o obj/boot.o
nasm -f elf32 harddisk.asm -o obj/harddisk.o
nasm -f elf32 callgate.asm -o obj/callgate.o
nasm -f elf32 interruptGate.asm -o obj/interruptGate.o
nasm -f elf32 ps2deviceinit.asm -o obj/ps2deviceinit.o
