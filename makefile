CC=gcc
CFLAGS=-g -Wall -o
SOURCES=main.c password.c mainmenu.c window_switcher.c newtrans.c RNG.c history.c settlement.c option.c registration.c config.c
EXECUTABLE=nproto

all:
	$(CC) $(CFLAGS) $(EXECUTABLE) $(SOURCES) -export-dynamic `pkg-config gtk+-3.0 libglade-2.0 --cflags --libs` `pkg-config --cflags --libs libconfig`

clean:
	rm -rf *o  $(EXECUTABLE)
