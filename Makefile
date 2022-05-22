CARAT_FLAGS = "-Xclang -load -Xclang /files10/cs446/rt116146/TexasProtection.so"
export CARAT_FLAGS
LINUX = /files10/cs446/rt116146/mini-linux/linux

obj-m += test.o
ccflags-y = $(CARAT_FLAGS)
ccflags-m := $(CARAT_FLAGS)

all:
	echo $(ccflags-y)
	echo $(ccflags-m)
	make -C $(LINUX)/ M=$(PWD) ccflags-y=$(ccflags-y) modules  
clean:
	-echo "THIS IS NOT WORKING - CLEAN IT YOURSELF"



