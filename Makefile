PORT?=8099
main: main.c
	gcc -o m.o main.c -lpthread
	./m.o $(PORT)
