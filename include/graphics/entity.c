#include "entity.h"
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

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

static int floats_per_vertex(enum LOAD_TYPE t){
    switch(t){
        case VERTICES_COLOR: return 6; 
        case VERTICES_COLOR_TEXTURE: return 8; 
        case VERTICES: return 3; 
        case VERTICES_TEXTURE: return 5; 
    }
}

void render_entity(struct entity *entity){

    if (entity->type == VERTICES_COLOR){
        entity->shader->use(entity->shader);
        entity->shader->set_mat4(entity->shader, "model", entity->model);
        entity->shader->set_mat4(entity->shader, "projection", *entity->camera->get_projection_matrix(entity->camera));
        entity->shader->set_mat4(entity->shader, "view", *entity->camera->get_view_matrix(entity->camera));
        glBindVertexArray(entity->VAO);
        glDrawArrays(entity->gl_enum, 0, entity->vertices_length);
    } else if (entity->type == VERTICES_COLOR_TEXTURE){
        // we change it up a little here
        entity->shader->use(entity->shader);
        entity->shader->set_mat4(entity->shader, "model", entity->model);
        entity->shader->set_mat4(entity->shader, "projection", *entity->camera->get_projection_matrix(entity->camera));
        entity->shader->set_mat4(entity->shader, "view", *entity->camera->get_view_matrix(entity->camera));
        entity->shader->set_int(entity->shader, "tex", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, entity->texture);
        glBindVertexArray(entity->VAO);
        glDrawArrays(entity->gl_enum, 0, entity->vertices_length);
    } else if (entity->type == VERTICES_COLOR){

    } else if (entity->type == VERTICES){

    }
}

void init_entity(struct entity *entity, struct camera *camera, struct shader *shader, enum LOAD_TYPE new_type, float vert_data[], int data_size, unsigned int gl_draw_enum){
    entity->type = new_type;
    entity->shader = shader;
    entity->camera = camera;
    entity->gl_enum = gl_draw_enum;
    entity->vertices_length = data_size/(floats_per_vertex(entity->type) * sizeof(float)); // Data size should just be the sizeof call while vertices_length is that divided by sizeof(float)
    glm_mat4_identity(entity->model); // make sure to init to the identity matrix

    // maybe i should port this to rust
    glGenVertexArrays(1, &entity->VAO);
    glBindVertexArray(entity->VAO);

    // Now gen the buffers
    glGenBuffers(1, &entity->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, entity->VBO);
    glBufferData(GL_ARRAY_BUFFER, data_size, vert_data, GL_STATIC_DRAW); // Might have to change the hints to opengl to make it run faster
    log_debug("INITIALIZING ARRAY WITH SIZE %d", data_size);

    // Now is where we have to be careful since the way we load depends on what we are doing, for now we will assume a few things about textures, colors etc
    if (entity->type == VERTICES){
        // This is just the plain case with nothing nada zilch, we need a shader just for this since we might screw up the rendering if we try to use colors when we aren't, maybe have a fallback color!
        // TODO., also implement this haha
    }
    if (entity->type == VERTICES_COLOR){
        log_debug("\n\nBEING INITIALIZED WITH VAO AND VBO %d %d", entity->VAO, entity->VBO);
        // This is probably the most common thing we are going to be rendering
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    if (entity->type == VERTICES_COLOR_TEXTURE){
        // this should never be called here
        log_error("WRONG INIT FUNCTION CALLED");
    }
    if (entity->type == VERTICES_TEXTURE){
        // This needs another init func
    }
    
    log_error("[%s] floats=%d fpv=%d verts=%d\n",
           (entity->type==VERTICES_COLOR?"gradient":"panel"),
           data_size / sizeof(float),
           floats_per_vertex(new_type),
           entity->vertices_length
    );

    entity->render = &render_entity;
}

// Handles init with vertices_color_texture and vertices_texture
void init_entity_texture(struct entity *entity, struct camera *camera, struct shader *shader, enum LOAD_TYPE new_type, float vert_data[], int data_size, unsigned int gl_draw_enum, char* texture_path){
    entity->type = new_type;
    entity->shader = shader;
    entity->camera = camera;
    entity->gl_enum = gl_draw_enum;
    entity->vertices_length = data_size/(floats_per_vertex(entity->type)*sizeof(float)); // Data size should just be the sizeof call while vertices_length is that divided by sizeof(float)
    glm_mat4_identity(entity->model); // make sure to init to the identity matrix

    glGenTextures(1, &entity->texture);
    glBindTexture(GL_TEXTURE_2D , entity->texture);

    glGenVertexArrays(1, &entity->VAO);
    glBindVertexArray(entity->VAO);

    // Now gen the buffers
    glGenBuffers(1, &entity->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, entity->VBO);
    glBufferData(GL_ARRAY_BUFFER, data_size, vert_data, GL_STATIC_DRAW); // Might have to change the hints to opengl to make it run faster
    log_debug("INITIALIZING TEXTURE ARRAY WITH SIZE %d", data_size);

    // Now is where we have to be careful since the way we load depends on what we are doing, for now we will assume a few things about textures, colors etc
    if (entity->type == VERTICES){
        // This is just the plain case with nothing nada zilch, we need a shader just for this since we might screw up the rendering if we try to use colors when we aren't, maybe have a fallback color!
        // TODO., also implement this haha
    }
    if (entity->type == VERTICES_COLOR){
        // This is probably the most common thing we are going to be rendering
    }
    if (entity->type == VERTICES_COLOR_TEXTURE){
        log_debug("\n\nTEXTURE BEING INITIALIZED WITH VAO AND VBO %d %d", entity->VAO, entity->VBO);
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load("/home/hiatus/Documents/waylandplaying/src/textures/clock_background.png", &width, &height, &nrChannels, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (!data){
            log_error("Unable to find file for texture");
            return;
        }
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);


        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }
    if (entity->type == VERTICES_TEXTURE){

    }
    log_error("[%s] floats=%d fpv=%d verts=%d\n",
           (entity->type==VERTICES_COLOR?"gradient":"panel"),
           data_size / sizeof(float),
           floats_per_vertex(new_type),
           entity->vertices_length
    );
    
    entity->render = &render_entity;
}
