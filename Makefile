default: build



build: main.c
	gcc main.c bf.c -o bf -ljit -Wall -g

run: build
	./bf hw.bf

clean:
	rm bf


