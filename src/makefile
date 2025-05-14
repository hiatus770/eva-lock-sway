CC=gcc
CFLAGS=-lwayland-client -lwayland-egl -lEGL -lGLESv2

build/main: main.c xdg-shell-client-protocol.h xdg-shell-protocol.c memory.h shader.h shader.c glad.c
	echo "Making main c file"
	gcc -o build/main main.c xdg-shell-protocol.c shader.c xdg-shell-client-protocol.h glad.c $(CFLAGS) -I /home/hiatus/Documents/waylandplaying/include

test: test.c xdg-shell-client-protocol.h xdg-shell-protocol.c memory.h shader.h shader.c glad.c
	gcc -o test test.c xdg-shell-protocol.c shader.c xdg-shell-client-protocol.h $(CFLAGS)
