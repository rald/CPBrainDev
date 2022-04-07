pbrain: pbrain.c
	gcc pbrain.c -o pbrain -Wall -Wextra -pedantic

.PHONY: style install uninstall clean

style:
	astyle -A2s2xgU pbrain.c

install:
	mkdir -p /usr/local/bin
	cp pbrain /usr/local/bin/pbrain

uninstall:
	rm /usr/local/bin/pbrain

clean:
	rm pbrain
