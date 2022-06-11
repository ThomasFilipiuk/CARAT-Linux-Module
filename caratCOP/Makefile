CARAT_FLAGS = "-Xclang -load -Xclang /files10/cs446/rt116146/mini-linux/module/build/TexasProtection.so"

export CARAT_FLAGS
LINUX = /files10/cs446/rt116146/mini-linux/linux

obj-m += test.o
obj-m += carat-cop.o
#obj-m += caratCOP.o
#carat-cop-objs := ./src/region.o ./src/policy.o

#ccflags-y = $(CARAT_FLAGS)

all:
	make -C $(LINUX)/ M=$(PWD) modules ccflags-y=$(CARAT_FLAGS)
no-guards:
	make -C $(LINUX)/ M=$(PWD) modules
clean:
	-echo "THIS IS NOT WORKING - CLEAN IT YOURSELF"



