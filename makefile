all: master slave view

view: view.c
	gcc -o view view.c

slave: slave.c
	gcc -o slave slave.c

master: master.c
	gcc -o master master.c -pthread -lrt -lm -std=c99 -D_XOPEN_SOURCE=500
clean:
	rm -rf master slave view
.PHONY: all clean
