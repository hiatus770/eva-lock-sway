#include "../include/files.h"
#include "../include/globals.h"
#include "globals.h"
#include <string.h>
#include <sys/stat.h>
#include "../include/logger.h"
#include <stdlib.h>

char* get_path(char* dir){
    char* path = malloc(300 * sizeof(char));
    sprintf(path, PROJECT_DIRECTORY);
    strcat(path, "/");
    strcat(path, dir);
    struct stat buffer;
    if (stat (path, &buffer) != 0){
        log_error("UNABLE TO FIND PATH %s", path);
        return ""; // This is potentially a bad idea
    }
    return path;
}

char* get_shader_path(char* dir){
    // For this one we just concatenate with the other string but inside of src/shaders/
    char* path = malloc(300 * sizeof(char));
    path[0] = '\0';
    strcat(path, "src/shaders/");
    strcat(path, dir);
    return get_path(path);
}

char* get_texture_path(char* dir){
    char* path = malloc(300 * sizeof(char));
    path[0] = '\0';
    strcat(path, "assets/textures/");
    strcat(path, dir);
    return get_path(path);
}

char* get_font_path(char* dir){
    char* path = malloc(300 * sizeof(char));
    path[0] = '\0';
    strcat(path, "assets/fonts/");
    strcat(path, dir);
    return get_path(path);
}
