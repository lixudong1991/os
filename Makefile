cc = i686-elf-gcc
prom = main
deps = $(shell find ./ -maxdepth 1 -name "*.h")
src = $(shell find ./ -maxdepth 1 -name "*.c")
musllibdir = /home/lxd/glibc/musl-1.2.4/musl-1.2.4/lib
asmobj = ./obj/mem.o ./obj/puts.o ./obj/boot.o ./obj/harddisk.o ./obj/callgate.o ./obj/interruptGate.o ./obj/ps2deviceinit.o
D_OBJ = ./obj
incpath = ./include /home/lxd/glibc/musl-1.2.4/musl-1.2.4/include
obj = $(addprefix $(D_OBJ)/,$(src:%.c=%.o))

$(prom): $(obj)
	$(cc) -Ttext 0xc0036000 -o $(prom) $(obj) $(asmobj) -L $(musllibdir) -static -lc -nostdlib -ffreestanding -lgcc
$(D_OBJ)/%.o: %.c $(deps)
	$(cc) -c $< -I $(incpath) -o $@ -ffreestanding
clean:
	rm -rf $(obj) $(prom) $(asmobj)
