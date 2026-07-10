exec = test.out
sources = $(wildcard src/*.c)
objects = $(sources:.c=.o)
flags = -g -std=c23 -Wall -Wextra -Wvla -g3 -fsigned-char -fsanitize=address,undefined

$(exec): $(objects)
	gcc $(objects) $(flags) -o $(exec)

%.o: %.c include/%.h
	gcc -c $(flags) $< -o $@

clean:
	-rm *.out
	-rm src/*.o

install:
	cp ./test.out ~/bin/tied
