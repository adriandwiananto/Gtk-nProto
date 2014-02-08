CC=gcc
CFLAGS=-g -Wall -o
SOURCES=main.c password.c mainmenu.c window_switcher.c newtrans.c RNG.c
EXECUTABLE=nproto

all:
	$(CC) $(CFLAGS) $(EXECUTABLE) $(SOURCES) -export-dynamic `pkg-config gtk+-3.0 libglade-2.0 --cflags --libs`

clean:
	rm -rf *o  $(EXECUTABLE)
