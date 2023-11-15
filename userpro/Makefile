cc = i686-elf-gcc
prom = a.out
deps = $(shell find ./ -maxdepth 1 -name "*.h")
src = $(shell find ./ -maxdepth 1 -name "*.c")
#mobj = $(shell find ./lib/ -name "*.o")
musllibdir = /home/lxd/glibc/musl-1.2.4/musl-1.2.4/lib
asmobj = $(shell find ./asmlib/ -name "*.o")
D_OBJ = ./
incpath =/home/lxd/glibc/musl-1.2.4/musl-1.2.4/include
obj = $(addprefix $(D_OBJ)/,$(src:%.c=%.o))

$(prom): $(obj)
	$(cc) -o $(prom) $(obj) $(asmobj) -L $(musllibdir) -static -lc -nostdlib -ffreestanding -lgcc
$(D_OBJ)/%.o: %.c $(deps)
	$(cc) -c $< -I $(incpath) -o $@ -ffreestanding
clean:
	rm -rf $(obj) $(prom) $(asmobj)
