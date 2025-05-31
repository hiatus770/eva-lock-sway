# please fix this awful makefile

CC=gcc
CFLAGS=	-lwayland-client \
		-lwayland-egl \
		-lEGL \
		-lGLESv2 \
		-lfreetype \
		-I/usr/include/freetype2 \
		-I/usr/include/libpng16 \
		-I/usr/include/harfbuzz \
		-I/usr/include/glib-2.0 \
		-I/usr/lib/glib-2.0/include \
		-I/usr/include/sysprof-6 \
		-pthread -Wall \
		-pg 

include_files=$(wildcard ../* ../include/* ../include/*/* ./*)
c_files=xdg-shell-protocol.c \
		../include/graphics/text.c \
		../include/graphics/entity.c \
		../include/graphics/camera.c \
		../include/graphics/graphics.c \
		../include/graphics/map.c \
		../include/graphics/eva.c \
		shader.c \
		xdg-shell-client-protocol.h \
		glad.c

build/main: main.c $(include_files)
	@echo "Making main c file"
	gcc -o build/main -g main.c $(c_files) $(CFLAGS) -I /home/hiatus/Documents/waylandplaying/include -lm

test: test.c xdg-shell-client-protocol.h xdg-shell-protocol.c memory.h shader.h shader.c glad.c
	gcc -o test test.c xdg-shell-protocol.c shader.c xdg-shell-client-protocol.h $(CFLAGS)
