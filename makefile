all: master slave view

view: view.c
	gcc -Wall -o view view.c -pthread -lrt -lm -std=c99 -D_XOPEN_SOURCE=500

slave: slave.c
	gcc -Wall -o slave slave.c

master: master.c
	gcc -Wall -o master master.c -pthread -lrt -lm -std=c99 -D_XOPEN_SOURCE=500
clean:
	rm -rf master slave view
	rm -rf results.txt
.PHONY: all clean
