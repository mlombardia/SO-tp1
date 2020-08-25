all:
	master slave view

master: master.c
	gcc -o master master.c -lm

slave: slave.c
	gcc -o slave slave.c

view: view.c
	gcc -o view view.c

clean:
	rm -rf master slave view

