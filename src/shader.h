#pragma once
#include <EGL/egl.h>
#include <GL/gl.h>
#include <stdbool.h>

struct shader {
    unsigned int ID;
    char *vertex_path;
    char *fragment_path;

    void (*use)(struct shader* shader); // Opengl use function make it usable

    void (*set_bool)(struct shader* shader, const char* name, bool value);
    void (*set_int)(struct shader* shader, const char* name, int value);
    void (*set_float)(struct shader* shader, const char* name, float value);
};

char* get_file_name_extension(const char* path); 

void init_shader(struct shader* shader, const char* vertex_path, const char* fragment_path);

void uninit_shader(struct shader* shader); // TODO: do this later pls :pray: 

unsigned int compile(const char* path);

char* get_source(const char* file_path); // Gets the source for the shader

void use(struct shader* shader);
