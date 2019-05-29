CFLAGS := -g -Wall -Wextra -Wpedantic
LFLAGS := -g -Wall -Wextra -Wpedantic

bfint: main.o
	gcc main.o -o $@
