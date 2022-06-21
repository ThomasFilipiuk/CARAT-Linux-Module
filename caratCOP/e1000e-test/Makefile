CARAT_FLAGS = "-Xclang -load -Xclang /files10/cs446/rt116146/mini-linux/module/build/TexasProtection.so"

LINUX = /files10/cs446/rt116146/mini-linux/linux

obj-m += carat-cop.o
obj-m += e1000e.o

e1000e-objs := 82571.o ich8lan.o 80003es2lan.o \
	       mac.o manage.o nvm.o phy.o \
	       param.o ethtool.o netdev.o ptp.o

all:
	make -C $(LINUX) M=$(PWD) modules ccflags-y=$(CARAT_FLAGS)
