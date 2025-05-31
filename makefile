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

include_files=$(wildcard ./include/* ./include/*/* ./src/*)
c_files=./src/xdg-shell-protocol.c \
		./include/graphics/text.c \
		./include/graphics/entity.c \
		./include/graphics/camera.c \
		./include/graphics/graphics.c \
		./include/graphics/map.c \
		./include/graphics/eva.c \
		./src/shader.c \
		./src/xdg-shell-client-protocol.h \
		./src/glad.c

build/main: ./src/main.c $(include_files)
	@echo "Making main c file"
	gcc -o build/main -g ./src/main.c $(c_files) $(CFLAGS) -I ./include -lm


# deprecated and old :pensive: 
# test: test.c xdg-shell-client-protocol.h xdg-shell-protocol.c memory.h shader.h shader.c glad.c
# 	gcc -o test test.c xdg-shell-protocol.c shader.c xdg-shell-client-protocol.h $(CFLAGS)
