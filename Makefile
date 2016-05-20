CC=clang
CFLAGS=-pthread -Wall

SRC= $(wildcard *.c)
OBJ= $(patsubst %.c, obj/%.o, $(SRC))

all: $(OBJ)
	$(CC) $(CFLAGS) -o server $^

clean:
	rm -f *~ obj/*.o

nodir:
	CFLAGS += -DNO_DIRECTORY

obj/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)
