# Main makefile 
CC=gcc
# These are the dependencies for the project 
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
		-pthread \
		-pg \
		-I ./include \
		-lm

# Finds all include files to be use din the project 
include_files=$(wildcard ./include/* ./include/*/* ./src/*)
# Finds all files that the project is dependent on 
dep_files=$(wildcard makefile ./include/* ./include/*/* ./src/* ./src/*/*)
# Finds all the source files in the src folder 
src_files=$(wildcard ./src/*.c) 

build/main: $(dep_files)
	@echo "Making main c file"
	gcc -o build/main -g $(src_files) $(CFLAGS) 

# deprecated and old :pensive: 
# test: test.c xdg-shell-client-protocol.h xdg-shell-protocol.c memory.h shader.h shader.c glad.c
# 	gcc -o test test.c xdg-shell-protocol.c shader.c xdg-shell-client-protocol.h $(CFLAGS)
