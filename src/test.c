// This is the testing file for all the main functions of the program
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GL/gl.h>
#include "xdg-shell-client-protocol.h"

#include "shader.h"
#include "memory.h"


int main(){
    printf("Running Test Suite!\n\n");

    // TESTING FUNCTIONALITY OF THE SHADER CLASS INITIALIZATION
    // struct shader test = {};
    // init_shader(&test, "/shaders/vertex.vs", "/shaders/fragment.fs");

    // TESTING FUNCTIONALITY OF GETTING THE SOURCE OF THE FILE
    struct shader testShader = {};
    const char* file_path = "/home/hiatus/Documents/waylandplaying/src/test.c";
    printf("%s", get_source(file_path));


    // TESTING FUNCTIONALIRY OF FILE_NAME_EXTENSION
    file_path = "test.c";
    fprintf(stderr, "PRINTING STRING: %s\n", get_file_name_extension(file_path));
    file_path = "test.";
    fprintf(stderr, "PRINTING STRING: %s\n", get_file_name_extension(file_path));

    file_path = "test.fs";
    fprintf(stderr, "PRINTING STRING: %s\n", get_file_name_extension(file_path));
    file_path = "test.vs";
    fprintf(stderr, "PRINTING STRING: %s\n", get_file_name_extension(file_path));
}
