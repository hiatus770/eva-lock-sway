#include "../include/graphics/shader.h"
#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EGL/egl.h>
#include <GL/gl.h>
#include <assert.h>
#include "globals.h"
#include "../include/logger.h"

/**
* Gets the file name extension without the . included, used for determing if its a vertex or fragment shader
*/
char* get_file_name_extension(const char* path){
    // Returns the file name extension WITHOUT the .
    // Iterate through string until we find the .
    char ind = path[0];
    int pos = 0;

    while (ind != '\0'){
        ind = path[pos];
        if (ind == '.'){
            break;
        }
        pos+=1;
    }

    pos+=1; //  We dont want to include the dot in the file name extension

    // Now that we find the position of the dot, we can find all the characters after it
    int end = pos;
    while (ind != '\0'){
        ind = path[end];
        if (ind == '\0')
            break;

        end++;
    }
    // Now we allocate the correct amount of space
    char* result = (char*)malloc((end - pos + 1) * sizeof(char));
    for (int i = pos; i < end; i++){
        result[i - pos] = path[i];
    }
    result[end - pos] = '\0';

    return result;
}

void set_bool(struct shader *shader, const char* name, bool value){
    glUniform1i(glGetUniformLocation(shader->ID, name), value);
}

void set_int(struct shader* shader, const char* name, int value){
    glUniform1i(glGetUniformLocation(shader->ID, name), value);
}

void set_float(struct shader* shader, const char* name, float value){
    glUniform1f(glGetUniformLocation(shader->ID, name), value);
}

void set_mat4(struct shader* shader, const char* name, mat4 value){
    glUniformMatrix4fv(glGetUniformLocation(shader->ID, name), 1, GL_FALSE, (float *)value);
}

void init_shader(struct shader* shader, const char* vertex_path, const char* fragment_path){
    // psuedo constructor for the shader
    char vertex_path_full[200] = PROJECT_DIRECTORY;
    char fragment_path_full[200] = PROJECT_DIRECTORY;

    strcat(vertex_path_full, vertex_path);
    strcat(fragment_path_full, fragment_path);

    ifd
    log_debug("FILE PATHS IN FULL\n %s \n %s\n", vertex_path_full, fragment_path_full);


    unsigned int vertex, fragment;
    vertex = compile(vertex_path_full);
    fragment = compile(fragment_path_full);

    shader->ID = glCreateProgram();
    glAttachShader(shader->ID, vertex);
    glAttachShader(shader->ID, fragment);
    glLinkProgram(shader->ID);

    int success;
    glGetProgramiv(shader->ID, GL_LINK_STATUS, &success);


    if (!success){
        char infoLog[1000];
        glGetProgramInfoLog(shader->ID, 512, NULL, infoLog);
        log_error("Error: Linking program %s and %s, error: %s", vertex_path, fragment_path, infoLog);
    }

    // Remove shaders now that we are done compiling and linking them!
    glDeleteShader(vertex); glDeleteShader(fragment);

    // Setup functions for the struct to use
    shader->use = &use;
    shader->set_bool = &set_bool;
    shader->set_int = &set_int;
    shader->set_float = &set_float;
    shader->set_mat4 = &set_mat4;
}

unsigned int compile(const char* path){
    char* source = get_source(path);
    int shader_type = GL_FRAGMENT_SHADER;

    if (strcmp(get_file_name_extension(path), "vs") == 0){
        shader_type = GL_VERTEX_SHADER;
    }

    unsigned int shader_id;
    int success;

    shader_id = glCreateShader(shader_type);
    // assert(shader_id != 0);

    glShaderSource(shader_id, 1, (const char **)&source, NULL);
    glCompileShader(shader_id);

    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success){
        char log[10000];
		GLsizei len;
		glGetShaderInfoLog(shader_id, 10000, &len, log);
		log_error("Error: compiling %s: %*s\n",
			shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment",
			len, log);
		exit(1);
    }

    ifd {
        log_debug("SUCCESSFULLY COMPILED %s NOW HAS ID: %d\n", path, shader_id);
    }

    return shader_id;
}

char* get_source(const char* file_path){
    long length;
    FILE *fptr;
    fptr = fopen(file_path, "r"); // We want to read from the path

    if (!fptr){
        fprintf(stderr, "NO FILE FOUND %s\n", file_path);
        return "";
    }

    char* return_string;

    if (fptr){
        fseek(fptr, 0, SEEK_END);
        length = ftell (fptr);
        fseek(fptr, 0, SEEK_SET);
        return_string = (char*) malloc((length + 1)*sizeof(char));
        if (return_string){
            fread(return_string, sizeof(char), length, fptr);
        }
        fclose(fptr);
    }
    return_string[length] = '\0';

    return return_string;
}

void use(struct shader* shader){
    glUseProgram(shader->ID);
}
