all: count main run

count: count.c
	gcc count.c -o count

main: main.c
	gcc main.c -o main

run: main
	./main

clean:
	rm -f count main

.PHONY: all run clean
