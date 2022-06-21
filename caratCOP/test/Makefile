CARAT_FLAGS = "-Xclang -load -Xclang /files10/cs446/rt116146/mini-linux/module/build/TexasProtection.so"

LINUX = /files10/cs446/rt116146/mini-linux/linux

obj-m += test.o
obj-m += carat-cop.o

all:
	make -C $(LINUX)/ M=$(PWD) modules ccflags-y=$(CARAT_FLAGS)
clean:
	-echo "THIS IS NOT WORKING - CLEAN IT YOURSELF"
