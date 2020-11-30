all:
	gcc -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L helpers.c files.c sort.c options.c list.c -lm -o list

clean:
	rm list
