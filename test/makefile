CC=gcc
CFLAGS=-lwayland-client -lwayland-egl -lEGL -lGLESv2

main: main.c xdg-shell-client-protocol.h xdg-shell-protocol.c memory.h 
	gcc -o main main.c xdg-shell-protocol.c xdg-shell-client-protocol.h $(CFLAGS)