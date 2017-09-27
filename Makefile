obj-m += mailslot_module.o
mailslot_module-objs := main_module.o mail.o mailslot.o mailslot_vector.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

install:
	sudo insmod mailslot_module.ko
#	sudo mknod mail c 245 0
#	sudo chmod 666 mail

redo:
	rm -Rf mail
	sudo rmmod mailslot_module

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
