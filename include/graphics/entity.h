#pragma once
#include "../glad/glad.h"
#include "../../src/globals.h"
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <wayland-client-core.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include "../../src/xdg-shell-client-protocol.h" // TODO FIX THESE IMPORTS SO THEY AREN"T THIS UGLY!
#include "../client_state.h"
#include "camera.h"
#include "../logger.h"
#include "shader.h"

enum LOAD_TYPE {
    VERTICES,
    VERTICES_COLOR,
    VERTICES_TEXTURE,
    VERTICES_COLOR_TEXTURE
};



struct entity {
    // This is the main entity struct that will be used to represent something that needs a render function and has certain data associated to it (if i had more time i would make a mesh system but i do not have that  time)
    float *vertices;  // This is the main thing we use for the data, we need to specify what kind of loading we are using
    int vertices_length;
    struct shader *shader; // This is a pointer to the shader that we will call use program on!
    struct camera *camera; // This is the main camera that we will be using to render, gives us its projection information etc etc

    unsigned int VBO, VAO;
    enum LOAD_TYPE type;  // This is the type of loading that we are going to use for the shader!
    mat4 model; // This stores the position and rotation of the object!
    unsigned int gl_enum;
    
    unsigned int texture;  // if we render w/ texture we change the way we render 


    void (*render)(struct entity *entity);  // This should be in charge of setting all the proper model transforms etc etc
};

void render_entity(struct entity *entity); 

void init_entity(struct entity *entity, struct camera *camera, struct shader *shader, enum LOAD_TYPE new_type, float vert_data[], int data_size, unsigned int gl_draw_enum); 

void init_entity_texture(struct entity *entity, struct camera *camera, struct shader *shader, enum LOAD_TYPE new_type, float vert_data[], int data_size, unsigned int gl_draw_enum, char* texture_path); 