cc = gcc
prom = main
deps = $(shell find ./ -name "*.h")
src = $(shell find ./ -name "*.c")
obj = $(src:%.c=%.o)

$(prom): $(obj)
	$(cc) -o $(prom) $(obj) mem.o puts.o
%.o: %.c $(deps)
	$(cc) -fno-stack-protector -no-pie -fno-pic -c $< -o $@ 
clean:
	rm -rf $(obj) $(prom)
