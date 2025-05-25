#include "graphics.h"
#include "../glad/glad.h"
#include "../../src/globals.h"
#include <GL/gl.h>
#include <assert.h>
#include <stdlib.h>
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
#include "eva.h"
#include "shader.h"
#include "camera.h"
#include "entity.h"
#include "text.h"

#include <ft2build.h>
#include FT_FREETYPE_H

// This is the main graphics file that will have all the opengl functionality and whatnot
struct shader global_shader;
struct shader text_shader;
struct camera global_camera;
struct entity test_entity;
float global_tick = 0.0f;

// Bloom related code
unsigned int fbo;
unsigned int pingpongFBO[2];
unsigned int pingpongBuffer[2]; // This is the texture passed into each other
unsigned int colorBuffers[2];
unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

font test_font;
struct shader b_shader;
struct shader global_shader_bloom;
struct shader gaussian;  /// Eventually adding more post processing effects!
struct shader final; // combines several or more textures from the framebuffers into one final output

// Main fonts used throughout
font matisse;
font timer;
font helvetica;
font clock_bloom; 

float eva_gradient[] = {
    // lowkey probably should figure out how to do this in blender and then import it :)
    0.5f, 0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
    0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   // bottom left
    -0.5f, 0.5f, 0.0f,   0.0f, 0.0f, 1.0f,    // top
    -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   // bottom right
   -0.5f, 0.5f, 0.0f,    0.0f, 1.0f, 0.0f,   // bottom left
    0.5f,  -0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top
};

float eva_gradient2[] = {
    // lowkey probably should figure out how to do this in blender and then import it :)
    1.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,   // bottom right
    1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   // bottom left
    -1.0f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,    // top
    -1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   // bottom right
   -1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f,   // bottom left
    1.0f,  -1.0f, 0.0f,  0.0f, 0.0f, 1.0f    // top
};

float eva_gradienttest[] = {
    -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 
    -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
}; 

// Main render function of the program
void render(struct client_state *state){
    // glViewport(0,0, state->width, state->height);
    glViewport(0,0, 1920, 1080);
    glClearColor(0.0, 0.0, 0.0, 0.5);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vec3 look_at_goal = {-0.5, 0.0, -1.0f};
    glm_vec3_copy(look_at_goal, global_camera.direction);
    
    // TODO make this call all the entity's render function in the program!
    char *goal = "h活動限界まで内部主エネルギー供給システムやめるスロー正常レース";

    float color[] = {1.0, 0.5, 0.0};

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    test_entity.render(&test_entity); // maps to texture 0  -- no bloom
    render_font(&test_font, goal, -1.0, -1.0, 0.005/3, CLOCK_TEXT_COLOR, global_camera); // maps to texture 1 -- will get bloomed on
    render_clock(&clock_bloom, global_camera);

    glActiveTexture(GL_TEXTURE0); // this line is needed for it to work, probably because without it the ping pong buffer has no clue what its doing the texturing on
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    bool horizontal = true, first_iteration = true;
    unsigned int amount = 0;
    gaussian.use(&gaussian);
    gaussian.set_int(&gaussian, "image", 0);
    for (unsigned int i= 0; i < amount; i++){
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]); // me when boolean as index
        gaussian.set_int(&gaussian, "horizontal", horizontal);
        glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongBuffer[!horizontal]);
        render_quad();
        horizontal = !horizontal;
        if (first_iteration == true){
            first_iteration = false;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Now render the final outptut
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    final.use(&final);
    final.set_int(&final, "scene", 0);
    final.set_int(&final, "bloom", 1);
    glActiveTexture(GL_TEXTURE0); // Prepare to bind color buffer from our fbo to texture
    glBindTexture(GL_TEXTURE_2D, colorBuffers[0]); // take our original source
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
    render_quad();



    // global_shader.use(&global_shader);

    // vec3 model_translate = {-1.0f, -1.0f, 0.0f};
    // mat4 model; glm_mat4_identity(model); glm_translate(model, model_translate);
    // mat4 projection; glm_perspective(glm_rad(45.0f), 1920.0f/1080.0f, 0.1f, 100.0f, projection);

    // global_shader.set_mat4(&global_shader, "model", model);
    // global_shader.set_mat4(&global_shader, "view", *global_camera.get_view_matrix(&global_camera));
    // global_shader.set_mat4(&global_shader, "projection", *global_camera.get_projection_matrix(&global_camera));


    // glBindVertexArray(VAO);
    // glDrawArrays(GL_TRIANGLES, 0, 6);
    
    
    
    // glm_mat4_identity(test_entity.model);
    // glm_translate(test_entity.model, model_translate);
    // test_entity.render(&test_entity);
    
    
    
    
    eglSwapBuffers(state->egl_display, state->egl_surface);
    
    

}
/**
* This function is responsible for compiling and setting up shaders for their use in the render() function of the program, this is basically run for initializing opengl and all its quirks
* - make sure to have the egl context ready before calling this
* - make sure that all the file paths are correct when running this command as well
*/


/**
* This function handles getting the codepoints given a utf-8 encoded string for functionality later in the program
*/
uint32_t* utf8_to_codepoints(const char *s, size_t *out_len) {
    size_t cap = 16;
    size_t len = 0;
    uint32_t *codepoints = malloc(cap * sizeof *codepoints);
    if (!codepoints) return NULL;

    const unsigned char *p = (const unsigned char*)s;
    while (*p) {
        uint32_t cp;
        size_t nbytes;

        if (*p < 0x80) {
            // 1‑byte sequence: 0xxxxxxx
            cp = *p;
            nbytes = 1;
        }
        else if ((p[0] & 0xE0) == 0xC0) {
            // 2‑byte sequence: 110xxxxx 10xxxxxx
            cp = p[0] & 0x1F;
            nbytes = 2;
        }
        else if ((p[0] & 0xF0) == 0xE0) {
            // 3‑byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
            cp = p[0] & 0x0F;
            nbytes = 3;
        }
        else if ((p[0] & 0xF8) == 0xF0) {
            // 4‑byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            cp = p[0] & 0x07;
            nbytes = 4;
        }
        else {
            // invalid leading byte
            free(codepoints);
            *out_len = 0;
            return NULL;
        }

        // Consume continuation bytes
        for (size_t i = 1; i < nbytes; ++i) {
            if ((p[i] & 0xC0) != 0x80) {
                // invalid continuation byte
                free(codepoints);
                *out_len = 0;
                return NULL;
            }
            cp = (cp << 6) | (p[i] & 0x3F);
        }

        // advance pointer
        p += nbytes;

        // append cp to array
        if (len + 1 > cap) {
            cap *= 2;
            uint32_t *tmp = realloc(codepoints, cap * sizeof *tmp);
            if (!tmp) {
                free(codepoints);
                *out_len = 0;
                return NULL;
            }
            codepoints = tmp;
        }
        codepoints[len++] = cp;
    }

    *out_len = len;
    return codepoints;
}

void initgl(struct client_state *state){

    // REGULAR RENDERING CODE

    char *goal = "活動限界まで内部主エネルギー供給システムやめるスロー正常レース";
    init_font(&matisse, &text_shader, "/home/hiatus/Documents/waylandplaying/include/graphics/matias.otf", goal, 48*2, 1.0f, 1.0f);

    init_font(&timer, &text_shader, "/home/hiatus/Documents/waylandplaying/include/graphics/Digital-Display.ttf", goal, 300, 1.0f, 2.0f);

    init_font(&helvetica, &text_shader, "/home/hiatus/Documents/waylandplaying/include/graphics/Helvetica.ttf", goal, 48*1.5, 1.0f, 1.0f);

    float colors[][3] = {{0.745, 0.341, 0.254}, {0.67f, 0.792f, 0.301f}, {0.227, 0.5686, 0.2901}};

    int length;
    float* main_eva_gradient = generate_gradient(3, colors, &length);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    init_shader(&global_shader, "/shaders/vertex.vs", "/shaders/fragment.fs");
    init_shader(&text_shader, "/shaders/text_vertex.vs", "/shaders/text_fragment.fs");

    vec3 temp_position = {0.0f, 0.0f, 10.0f};
    init_camera(&global_camera, temp_position);
    global_camera.set_position(&global_camera, temp_position);

    // BLOOOM RELATED CODE
    init_shader(&global_shader_bloom, "/shaders/vertex.vs", "/shaders/bloom_fragment.fs"); // takes in two textures and combines them into one output
    init_shader(&b_shader, "/shaders/text_bloom.vs", "/shaders/text_bloom.fs"); // renders a text field only to be blurred
    init_shader(&gaussian, "/shaders/gaussian.vs", "/shaders/gaussian.fs"); // does ping pong gaussian blurring on the texture several times
    init_shader(&final, "/shaders/final.vs", "/shaders/final.fs"); // takes in two textures and combines them into one output

    init_font(&test_font, &b_shader, "/home/hiatus/Documents/waylandplaying/include/graphics/matias.otf", goal, 48*2, 1.0f, 1.0f);
    init_font(&clock_bloom, &b_shader, "/home/hiatus/Documents/waylandplaying/include/graphics/Digital-Display.ttf", goal, 48*2, 1.0f, 1.0f);

    vec3 forward = {0.1, 0.5, 0.0};
    // init_entity(&test_entity, &global_camera, &global_shader, VERTICES_COLOR, eva_gradienttest, sizeof(eva_gradienttest), GL_TRIANGLES);
    // init_entity(&test_entity, &global_camera, &global_shader_bloom, VERTICES_COLOR, main_eva_gradient, length * sizeof(float), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    init_entity(&test_entity, &global_camera, &global_shader_bloom, VERTICES_COLOR, eva_gradienttest, sizeof(eva_gradienttest), GL_TRIANGLE_STRIP); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    // glm_translate(test_entity.model, forward);
    // glm_scale(test_entity.model, (vec3){0.1, 0.1, 0.1});

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(2, colorBuffers);

    for (unsigned int i = 0; i < 2; i++){
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    glDrawBuffers(2, attachments);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // The original global_shader is what will be used to merge into the final.fs, and that is what will get displayed

    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffer);
    for (unsigned int i = 0; i < 2; i++){
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0
        );
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            log_error("FRAMEBUFFER NOT COMPLETE");
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void create_window_egl(struct client_state *state, int32_t width, int32_t height){
    EGLint attributes[] = {
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE
    };

    EGLConfig config;
    EGLint num_config, major, minor;
    EGLBoolean ret;

    state->egl_display = eglGetDisplay(state->wl_display);
    ret = eglInitialize(state->egl_display, &major, &minor);
    assert(ret == EGL_TRUE);
    ret = eglBindAPI(EGL_OPENGL_API);
    assert(ret == EGL_TRUE);

    eglChooseConfig(state->egl_display, attributes, &config, 1, &num_config);
    state->egl_context = eglCreateContext(state->egl_display, config, EGL_NO_CONTEXT, NULL);

    state->egl_window = wl_egl_window_create(state->wl_surface, width, height);
    state->egl_surface = eglCreateWindowSurface(state->egl_display, config, state->egl_window, NULL);
    eglMakeCurrent(state->egl_display, state->egl_surface, state->egl_surface, state->egl_context);
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void render_quad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
