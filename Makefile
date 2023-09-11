cc = i686-elf-gcc
prom = main
deps = $(shell find ./ -maxdepth 1 -name "*.h")
src = $(shell find ./ -maxdepth 1 -name "*.c")
mobj = $(shell find ./lib/ -name "*.o")
D_OBJ = ./obj
obj = $(addprefix $(D_OBJ)/,$(src:%.c=%.o)) ./obj/mem.o ./obj/puts.o ./obj/boot.o ./obj/harddisk.o ./obj/syscall.o ./obj/interruptGate.o

$(prom): $(obj)
	$(cc) -Ttext 0xc0036000 -o $(prom) $(obj) $(mobj) -nostdlib -ffreestanding -lgcc
$(D_OBJ)/%.o: %.c $(deps)
	$(cc) -c $< -o $@ -ffreestanding
clean:
	rm -rf $(obj) $(prom)
