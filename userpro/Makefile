cc = i686-elf-gcc
prom = a.out
deps = $(shell find ./ -maxdepth 1 -name "*.h")
src = $(shell find ./ -maxdepth 1 -name "*.c")
mobj = $(shell find ../lib/ -name "*.o")
asmobj = $(shell find ./lib/ -name "*.o")
D_OBJ = ./
incpath = ../include
obj = $(addprefix $(D_OBJ)/,$(src:%.c=%.o))

$(prom): $(obj)
	$(cc) -o $(prom) $(obj) $(asmobj) $(mobj) -nostdlib -ffreestanding -lgcc
$(D_OBJ)/%.o: %.c $(deps)
	$(cc) -c $< -I $(incpath) -o $@ -ffreestanding
clean:
	rm -rf $(obj) $(prom) $(asmobj)
