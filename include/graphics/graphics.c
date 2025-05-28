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
#include "map.h"
#include "shader.h"
#include "camera.h"
#include "entity.h"
#include "text.h"

#include <ft2build.h>
#include FT_FREETYPE_H

int SRC_HEIGHT = 1080;
int SRC_WIDTH = 1920;

// This is the main graphics file that will have all the opengl functionality and whatnot
struct shader global_shader;
struct shader text_shader;

struct camera global_camera;
struct entity test_entity;
struct entity test_entity_2;
struct entity main_gradient;
struct entity main_panel;
struct entity main_border;
struct entity slant; 
struct entity border_1; // rendered once
struct entity border_2; // rendered once
struct entity border_bottom;  // rendered several times


// Bloom related code
unsigned int fbo;
unsigned int pingpongFBO[2];
unsigned int pingpongBuffer[2]; // This is the texture passed into each other
unsigned int colorBuffers[2];
unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

font matisse_bloom;
struct shader b_shader;
struct shader global_shader_bloom; // same as global shader but incorporates bloom effect into the object
struct shader gaussian;  /// Eventually adding more post processing effects!
struct shader final; // combines several or more textures from the framebuffers into one final output
struct shader texture_shader;

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



float quad[] = {
    // positions          // colors           // texture coords
     1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
     1.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -1.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,   // bottom left
    -1.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,   // bottom left
    -1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,    // top left
    1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f  // top right
};

float quad_color[] = {
    0.5f,  0.5f, 0.0f, CLOCK_COLOR_RAW   // top right
    0.5f, -0.5f, 0.0f, CLOCK_COLOR_RAW   // bottom right
   -0.5f, -0.5f, 0.0f, CLOCK_COLOR_RAW
   -0.5f, -0.5f, 0.0f, CLOCK_COLOR_RAW
   -0.5f,  0.5f, 0.0f, CLOCK_COLOR_RAW
    0.5f,  0.5f, 0.0f, CLOCK_COLOR_RAW
};



// Main render function of the program
void render(struct client_state *state){
    // glViewport(0,0, state->width, state->height);

    glViewport(0,0, SRC_WIDTH, SRC_HEIGHT);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vec3 look_at_goal = {-0.5, 0.0, -1.0f};
    // vec3 camera_position = {3.0f, 0.0, 4.0f};
    // glm_vec3_copy(look_at_goal, global_camera.direction);
    // glm_vec3_copy(camera_position, global_camera.position);

    // TODO make this call all the entity's render function in the program!
    char *goal = "活動限界まで内部主エネルギー供給システムやめるスロー正常レース";
    char *top_left = "活動限界まで";
    char *top_left_secondary = "あと";

    float color[] = {1.0, 0.5, 0.0};

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0,0, SRC_WIDTH, SRC_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    main_gradient.render(&main_gradient); // maps to texture 0  -- no bloom
    main_panel.render(&main_panel);


    // Code for rendering the clock
    float x_top_left = -0.75, y_top_left = 0.49;
    render_clock(&clock_bloom, global_camera);

    // // Code for text top left of clock
    render_font(&matisse_bloom, top_left, x_top_left, y_top_left, (0.005/4) *1.08, CLOCK_TEXT_COLOR, global_camera); // maps to texture 1 -- will get bloomed on
    render_font(&matisse_bloom, top_left_secondary, x_top_left, y_top_left - 0.16, (0.005/4)*1.08, CLOCK_TEXT_COLOR, global_camera); // maps to texture 1 -- will get bloomed on
    // test_entity_2.render(&test_entity_2); // maps to texture 0  -- no bloom

    // Code for the main border
    float bl_x = -0.56f, bl_y = -0.7f;
    float line_w = 0.02f;
    draw_line(&main_border, bl_x, bl_y, 2.52f, line_w);
    float l_h = 0.9f;
    draw_line(&main_border, bl_x, bl_y, line_w, l_h);
    float l_w = 0.22f;
    draw_line(&main_border, bl_x - l_w, bl_y + l_h, l_w + line_w, line_w);

    // reposition ourselves
    float cur_x = bl_x - l_w;
    float cur_y = bl_y + l_h;

    float temp_h = 0.47;
    draw_line(&main_border, cur_x, cur_y, line_w, temp_h);
    cur_y += temp_h;

    temp_h = 0.77;
    draw_line(&main_border, cur_x, cur_y, temp_h, line_w);
    cur_x += temp_h; 

    draw_slant(&slant, cur_x, cur_y, line_w, cur_x+0.06f*1.2, cur_y-0.11f*1.2); 
    cur_x += 0.06f*1.2; 
    cur_y -= 0.11f*1.2;
    
    temp_h = 2.0; 
    draw_line(&main_border, cur_x, cur_y, 1.96 - cur_x, line_w);
    



    for(float i = -2.0f; i < 2.0f; i += 0.3333f){
        for(float j = -2.0f; j < 2.0f; j += 0.33333f){
            // char* measure = malloc(50 * sizeof(char));
            // sprintf(measure, "x:%.6f", i);
            // render_font(&matisse_bloom, measure, i, j, 0.0004, (vec3){1.0, 1.0, 0.0}, global_camera);
            // sprintf(measure, "y:%.6f", j);
            // render_font(&matisse_bloom, measure, i, j + 0.1, 0.0004, CLOCK_TEXT_COLOR, global_camera);
            // free(measure);
        }
    }

    glActiveTexture(GL_TEXTURE0); // this line is needed for it to work, probably because without it the ping pong buffer has no clue what its doing the texturing on
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    bool horizontal = true, first_iteration = true;
    unsigned int amount = 20;
    gaussian.use(&gaussian);
    gaussian.set_int(&gaussian, "image", 0);
    for (unsigned int i = 0; i < amount; i++){
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]); // me when boolean as index
        glViewport(0,0, SRC_WIDTH, SRC_HEIGHT);
        gaussian.set_int(&gaussian, "horizontal", horizontal);
        glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongBuffer[!horizontal]);
        render_quad();
        horizontal = !horizontal;
        if (first_iteration == true){
            first_iteration = false;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0, state->width, state->height);

    // Now render the final outptut
    final.use(&final);
    final.set_int(&final, "scene", 0);
    final.set_int(&final, "bloom", 1);
    glActiveTexture(GL_TEXTURE0); // Prepare to bind color buffer from our fbo to texture
    glBindTexture(GL_TEXTURE_2D, colorBuffers[0]); // take our original source
    // glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);


    final.use(&final);
    render_quad();

    eglSwapBuffers(state->egl_display, state->egl_surface);
}

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



// The callback signature
void APIENTRY openglDebugCallback(GLenum source,
                                  GLenum type,
                                  GLuint id,
                                  GLenum severity,
                                  GLsizei length,
                                  const GLchar *message,
                                  const void *userParam)
{
    // Filter out non-significant messages if desired:
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

    // Human-readable source
    const char* srcStrings[] = {
        "API", "Window System", "Shader Compiler",
        "Third Party", "Application", "Other"
    };
    const char* src = "Unknown";
    switch (source) {
        case GL_DEBUG_SOURCE_API:             src = srcStrings[0]; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   src = srcStrings[1]; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: src = srcStrings[2]; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     src = srcStrings[3]; break;
        case GL_DEBUG_SOURCE_APPLICATION:     src = srcStrings[4]; break;
        case GL_DEBUG_SOURCE_OTHER:           src = srcStrings[5]; break;
    }

    // Human-readable type
    const char* typeStrings[] = {
        "Error", "Deprecated Behavior", "Undefined Behavior",
        "Portability", "Performance", "Marker", "Other"
    };
    const char* typ = "Unknown";
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               typ = typeStrings[0]; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typ = typeStrings[1]; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typ = typeStrings[2]; break;
        case GL_DEBUG_TYPE_PORTABILITY:         typ = typeStrings[3]; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typ = typeStrings[4]; break;
        case GL_DEBUG_TYPE_MARKER:              typ = typeStrings[5]; break;
        case GL_DEBUG_TYPE_OTHER:               typ = typeStrings[6]; break;
    }

    // Human-readable severity
    const char* sevStrings[] = {
        "High", "Medium", "Low", "Notification"
    };
    const char* sev = "Unknown";
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:         sev = sevStrings[0]; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       sev = sevStrings[1]; break;
        case GL_DEBUG_SEVERITY_LOW:          sev = sevStrings[2]; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: sev = sevStrings[3]; break;
    }

    fprintf(stderr,
            "GL DEBUG: Source=%s, Type=%s, ID=%u, Severity=%s\n  Message: %s\n\n",
            src, typ, id, sev, message);
}

// Call this once after your context is created and made current:
void enableGLDebug() {
    // Must be an OpenGL 4.3+ context or have KHR_debug
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes callback synchronous—good for debugging
    glDebugMessageCallback(openglDebugCallback, NULL);
    // Optionally, filter messages:
    // glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
}


void initgl(struct client_state *state){
    enableGLDebug();
    // REGULAR RENDERING CODE
    vec3 temp_position = {0.0f, 0.0f, 3.0f};
    init_camera(&global_camera, temp_position);

    char *goal = "活動限界まで内部主エネルギー供給システムやめるスロー正常レースあと";
    init_font(&matisse, &text_shader, "/home/hiatus/Documents/waylandplaying/include/graphics/matias.otf", goal, 48*2, 1.0f, 1.0f);
    init_font(&timer, &text_shader, "/home/hiatus/Documents/waylandplaying/include/graphics/Digital-Display.ttf", goal, 300, 1.0f, 2.0f);
    init_font(&helvetica, &text_shader, "/home/hiatus/Documents/waylandplaying/include/graphics/Helvetica.ttf", goal, 48*1.5, 1.0f, 1.0f);

    // float colors[][3] = {{0.745, 0.341, 0.254}, {0.67f, 0.792f, 0.301f}, {0.227, 0.5686, 0.2901}};
    float colors[][3] = {{0.54, 0.06, 0.03}, {0.45f, 0.74f, 0.06f}, {0.03, 0.27, 0.06}};

    int length;
    float* main_eva_gradient = generate_gradient(3, colors, &length);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    init_shader(&texture_shader, "/shaders/vertex_texture.vs", "/shaders/texture.fs"); // takes in two textures and combines them into one output
    init_shader(&global_shader, "/shaders/vertex.vs", "/shaders/fragment.fs");
    init_shader(&text_shader, "/shaders/text_vertex.vs", "/shaders/text_fragment.fs");

    // BLOOOM RELATED CODE
    init_shader(&global_shader_bloom, "/shaders/vertex.vs", "/shaders/bloom_fragment.fs"); // takes in two textures and combines them into one output
    init_shader(&b_shader, "/shaders/text_bloom.vs", "/shaders/text_bloom.fs"); // renders a text field only to be blurred
    init_shader(&gaussian, "/shaders/gaussian.vs", "/shaders/gaussian.fs"); // does ping pong gaussian blurring on the texture several times
    init_shader(&final, "/shaders/final.vs", "/shaders/final.fs"); // takes in two textures and combines them into one output

    init_font(&matisse_bloom, &b_shader, "/home/hiatus/Documents/waylandplaying/include/graphics/matias.otf", goal, 48*2, 1.0f, 1.3f);
    init_font(&clock_bloom, &b_shader, "/home/hiatus/Documents/waylandplaying/include/graphics/7-segment-mono.otf", goal, 48*32, 0.6f, 1.1f);

    // init_entity_texture(&main_panel, &global_camera, &texture_shader, VERTICES_COLOR_TEXTURE, main_eva_gradient, length * sizeof(float), GL_TRIANGLES, "./textures/awesomeface.png");
    init_entity_texture(&main_panel, &global_camera, &texture_shader, VERTICES_COLOR_TEXTURE, quad, sizeof(quad), GL_TRIANGLES, "./textures/awesomeface.png");
    // init_entity(&main_gradient, &global_camera, &global_shader, VERTICES_COLOR, eva_gradient2, sizeof(eva_gradient2), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    init_entity(&main_gradient, &global_camera, &global_shader, VERTICES_COLOR, main_eva_gradient, length * sizeof(float), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    init_entity(&test_entity, &global_camera, &global_shader, VERTICES_COLOR, main_eva_gradient, length * sizeof(float), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    init_entity(&test_entity_2, &global_camera, &global_shader_bloom, VERTICES_COLOR, main_eva_gradient, length * sizeof(float), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    init_entity(&main_border, &global_camera, &global_shader_bloom, VERTICES_COLOR, quad_color,  sizeof(quad_color), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    init_entity(&slant, &global_camera, &global_shader_bloom, VERTICES_COLOR, quad_color,  sizeof(quad_color), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static

    glm_translate(main_gradient.model, (vec3){0.0, 0.0, -0.01});
    glm_scale(main_gradient.model, (vec3){2*1.0f, 2*1080.0f/1920.0f, 0.01f});
    glm_scale(main_panel.model, (vec3){2*1.0f, 2*1080.0f/1920.0f, 0.01f});
    glm_translate(test_entity_2.model, (vec3){-1.0, 0.0, 1.0});
    glm_translate(main_panel.model, (vec3){0.0, 0.0, -0.001f});
    glm_scale(test_entity_2.model, (vec3){0.2f, 0.2f, 0.01f});

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(2, colorBuffers);

    for (unsigned int i = 0; i < 2; i++){
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, SRC_WIDTH, SRC_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SRC_WIDTH, SRC_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    glDrawBuffers(2, attachments);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // The original global_shader is what will be used to merge into the final.fs, and that is what will get displayed

    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffer);
    for (unsigned int i = 0; i < 2; i++){
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SRC_WIDTH, SRC_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

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
