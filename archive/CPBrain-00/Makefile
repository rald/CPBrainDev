pbrain: pbrain.c
	gcc pbrain.c -o pbrain -Wall -Wextra -pedantic

.PHONY: style install clean

style:
	astyle -A2 -t2 -xg -U pbrain.c

install:
	cp pbrain /usr/local/bin/

clean:
	rm pbrain
