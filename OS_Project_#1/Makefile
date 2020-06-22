all: bilshell producer consumer

bilshell: bilshell.c
	gcc -o bilshell bilshell.c

producer: producer.c
	gcc -o producer producer.c

consumer: consumer.c
	gcc -o consumer consumer.c

clean:
	rm -fr bilshell bilshell.o *~
	rm -fr producer producer.o *~
	rm -fr consumer consumer.o *~