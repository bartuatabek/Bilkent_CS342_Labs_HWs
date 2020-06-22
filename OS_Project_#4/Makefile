obj-m +=module-1.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -Wall -o app app.c -lpthread
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -fr app app.o *.txt *~
