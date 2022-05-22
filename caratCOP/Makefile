CARAT_FLAGS = "-Xclang -load -Xclang /nfs-scratch/ncw9966/TexasProtection.so -Xclang -TexasProtection"
export CARAT_FLAGS
LINUX = /files10/cs446/rt116146/mini-linux/linux

obj-m += carat-cop.o

all:
	make -C $(LINUX)/ M=$(PWD) modules CFLAGS=$(CARAT_FLAGS)
	clang user-space-app.c -o user-space-app
user-space-app.o: user-space-app.c
	make -C $(LINUX)/ M=$(PWD) -o
clean:
	-echo "THIS IS NOT WORKING - CLEAN IT YOURSELF"



