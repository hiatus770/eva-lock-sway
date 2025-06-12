#include "../include/graphics/graphics.h"
#include "../include/glad/glad.h"
#include "../include/globals.h"
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
#include "../include/xdg-shell-client-protocol.h" // TODO FIX THESE IMPORTS SO THEY AREN"T THIS UGLY!
#include "../include/client_state.h"
#include "../include/eva.h"
#include "../include/graphics/map.h"
#include "../include/graphics/shader.h"
#include "../include/graphics/camera.h"
#include "../include/graphics/entity.h"
#include "../include/graphics/text.h"

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
struct entity black_box;
struct entity red_box;
struct entity slant;
struct entity colorable_slant;
struct entity border_1; // rendered once
struct entity border_2; // rendered once
struct entity border_bottom;  // rendered several times
struct entity red_stripe;  // rendered several times
struct entity blue_box;

// Bloom related code
unsigned int fbo;
unsigned int pingpongFBO[2];
unsigned int pingpongBuffer[2]; // This is the texture passed into each other1
unsigned int colorBuffers[2];
unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

font matisse_bloom;
struct shader b_shader;
struct shader global_shader_bloom; // same as global shader but incorporates bloom effect into the object
struct shader global_shader_bloom_mono; // same as global shader but incorporates bloom effect into the object but uses the mono color fragment shader
struct shader gaussian;  /// Eventually adding more post processing effects!
struct shader final; // combines several or more textures from the framebuffers into one final output
struct shader texture_shader;
struct shader texture_shader_bloom;

// Main fonts used throughout
font matisse;
font timer;
font helvetica;
font helvetica_bloom;
font clock_bloom;

float radius_global; 

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
     1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   1.0f, 1.0f,   // top right
     1.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -1.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,   // bottom left
    -1.0f, -1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,   // bottom left
    -1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,    // top left
    1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f  // top right
};

float quad_vert[] = {
    // positions          // colors           // texture coords
     1.0f,  1.0f, 0.0f,   // top right
     1.0f, -1.0f, 0.0f,   // bottom right
    -1.0f, -1.0f, 0.0f,   // bottom left
    -1.0f, -1.0f, 0.0f,   // bottom left
    -1.0f,  1.0f, 0.0f,   // top left
    1.0f,  1.0f, 0.0f,    // top right
};
float quad_color[] = {
    0.5f,  0.5f, 0.0f, CLOCK_COLOR_RAW   // top right
    0.5f, -0.5f, 0.0f, CLOCK_COLOR_RAW   // bottom right
   -0.5f, -0.5f, 0.0f, CLOCK_COLOR_RAW
   -0.5f, -0.5f, 0.0f, CLOCK_COLOR_RAW
   -0.5f,  0.5f, 0.0f, CLOCK_COLOR_RAW
    0.5f,  0.5f, 0.0f, CLOCK_COLOR_RAW
};

float quad_color_black[] = {
    0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,   // top right
    0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,   // bottom right
   -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
   -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
   -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,
};

float blue_quad_vert[] = {
    0.5f,  0.5f, 0.0f, BLUE_COLOR_RAW,   // top right
    0.5f, -0.5f, 0.0f, BLUE_COLOR_RAW,   // bottom right
   -0.5f, -0.5f, 0.0f, BLUE_COLOR_RAW,
   -0.5f, -0.5f, 0.0f, BLUE_COLOR_RAW,
   -0.5f,  0.5f, 0.0f, BLUE_COLOR_RAW,
    0.5f,  0.5f, 0.0f, BLUE_COLOR_RAW
};

float red_quad_vert[] = {
    0.5f,  0.5f, 0.0f, GLOW_COLOR_RED_RAW   // top right
    0.5f, -0.5f, 0.0f, GLOW_COLOR_RED_RAW   // bottom right
   -0.5f, -0.5f, 0.0f, GLOW_COLOR_RED_RAW
   -0.5f, -0.5f, 0.0f, GLOW_COLOR_RED_RAW
   -0.5f,  0.5f, 0.0f, GLOW_COLOR_RED_RAW
    0.5f,  0.5f, 0.0f, GLOW_COLOR_RED_RAW
};

float red_quad[] = {
    // positions          // colors           // texture coords
     1.0f,  1.0f, 0.0f,   GLOW_COLOR_RED_RAW   1.0f, 1.0f,   // top right
     1.0f, -1.0f, 0.0f,   GLOW_COLOR_RED_RAW   1.0f, 0.0f,   // bottom right
    -1.0f, -1.0f, 0.0f,   GLOW_COLOR_RED_RAW   0.0f, 0.0f,   // bottom left
    -1.0f, -1.0f, 0.0f,   GLOW_COLOR_RED_RAW   0.0f, 0.0f,   // bottom left
    -1.0f,  1.0f, 0.0f,   GLOW_COLOR_RED_RAW   0.0f, 1.0f,    // top left
    1.0f,  1.0f, 0.0f,   GLOW_COLOR_RED_RAW   1.0f, 1.0f  // top right
};

void enableGLDebug();

// Main render function of the program
void render(struct client_state *state){
    glViewport(0,0, SRC_WIDTH, SRC_HEIGHT);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vec3 look_at_goal = {-0.5, -0.00, -1.0f};
    vec3 camera_position = {1.5f, 0.0, 3.0f};
    glm_vec3_copy(look_at_goal, global_camera.direction);
    glm_vec3_copy(camera_position, global_camera.position);

    // TODO make this call all the entity's render function in the program!
    char *goal = "活動限界まで内部主エネルギー供給システムやめるスロー正常レース";
    char *top_left = "活動限界まで";
    char *top_left_secondary = "あと";
    char *active_time = "ACTIVE TIME REMAINING:";

    float color[] = {1.0, 0.5, 0.0};

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0,0, SRC_WIDTH, SRC_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float stripe_x = 1.82, stripe_y = 0.30;
    glm_mat4_identity(red_stripe.model);
    glm_scale(red_stripe.model, (vec3){0.5, 1.0f, 1.0f});
    glm_scale(red_stripe.model, (vec3){0.15, 0.19f, 1.0f});
    glm_translate(red_stripe.model, (vec3){stripe_x/(0.5 * 0.15), stripe_y/(0.19), -0.00001f});

    red_stripe.render(&red_stripe);
    main_gradient.render(&main_gradient); // maps to texture 0  -- no bloom
    main_panel.render(&main_panel);

    // Code for rendering the clock
    float x_top_left = -0.75, y_top_left = 0.49;
    render_clock(&clock_bloom, global_camera);

    // Active time remaining
    render_font(&helvetica_bloom, active_time, x_top_left + 0.80, y_top_left-0.04, (0.005/4)*1.08, CLOCK_TEXT_COLOR, global_camera);

    // Code for text top left of clock
    render_font(&matisse_bloom, top_left, x_top_left, y_top_left, (0.005/4) *1.08, CLOCK_TEXT_COLOR, global_camera); // maps to texture 1 -- will get bloomed on
    render_font(&matisse_bloom, top_left_secondary, x_top_left, y_top_left - 0.16, (0.005/4)*1.08, CLOCK_TEXT_COLOR, global_camera); // maps to texture 1 -- will get bloomed on
    // test_entity_2.render(&test_entity_2); // maps to texture 0  -- no bloom

    // Internal and other 2 characters
    matisse_bloom.scale_x *= 1.10;
    render_font(&matisse_bloom, "内部" , stripe_x - 0.6, stripe_y - 0.06, 0.005/2.1, CLOCK_TEXT_COLOR, global_camera);
    matisse_bloom.scale_x /= 1.10;
    render_font(&helvetica_bloom, "INTERNAL" , stripe_x - 0.6, stripe_y - 0.17, 0.005/2.5, CLOCK_TEXT_COLOR, global_camera);

    // Main energy supply system font is rendered here
    render_font(&matisse_bloom, "主電源供給システム" , stripe_x - 0.6, stripe_y - 0.34, 0.005/5, CLOCK_TEXT_COLOR, global_camera);
    helvetica_bloom.scale_x *= 0.80;
    render_font(&helvetica_bloom, "MAINENERGY SUPPLY SYSTEM" , stripe_x - 0.6, stripe_y - 0.4, 0.005/4.5, CLOCK_TEXT_COLOR, global_camera);
    helvetica_bloom.scale_x /= 0.80;

    // Code for the main border
    float b_w = 0.01; float b_h = 0.02;
    float bl_x = -0.56f, bl_y = -0.7f;
    float line_w = 0.02f;
    draw_line(&main_border, bl_x, bl_y, 2.52f, line_w);
    draw_line(&blue_box, bl_x - 0.04, bl_y + 0.03, b_w, b_h);
    draw_line(&blue_box, bl_x - 0.20, bl_y + 0.03, b_w, b_h);
    draw_line(&blue_box, bl_x - 0.04, bl_y + 0.85, b_w, b_h);
    draw_line(&blue_box, bl_x - 0.20, bl_y + 0.85, b_w, b_h);
    float l_h = 0.9f;
    draw_line(&main_border, bl_x, bl_y, line_w, l_h);
    float l_w = 0.22f;
    draw_line(&main_border, bl_x - l_w, bl_y + l_h, l_w + line_w, line_w);

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

    // colorable_slant.shader->use(colorable_slant.shader);
    // glUniform3f(glGetUniformLocation(colorable_slant.shader->ID, "color"), BLUE_COLOR_RAW);
    // colorable_slant.render(&colorable_slant);

    temp_h = 2.0;
    draw_line(&main_border, cur_x, cur_y, 1.96 - cur_x, line_w);
    cur_x = 1.96;

    temp_h = 1.24;
    draw_line(&main_border, cur_x, cur_y - temp_h, line_w, temp_h+line_w);

    // Box code
    // INTERNAL + main power supply system boxes
    draw_box(&main_border, 1.2, 0.1, 0.7, 0.4, line_w/2);
    draw_box(&main_border, 1.2, 0.1 - 0.20 - line_w*2, 0.7, 0.20, line_w/2);
    draw_line(&blue_box, 1.2 - 0.02, 0.1 - 0.00, b_w, b_h);
    draw_line(&blue_box, 1.2 - 0.02 + 0.7 + 0.02 + 0.02, 0.1 - 0.00, b_w, b_h);
    draw_line(&blue_box, 1.2 - 0.02, 0.1 - 0.00 + 0.4, b_w, b_h);
    draw_line(&blue_box, 1.2 - 0.02 + 0.7 + 0.02 + 0.02, 0.1 - 0.00 + 0.4, b_w, b_h);
   
    float gap_between_dots = 0.3; 
    for (float x = -2.0f; x <= 2.0f; x+=gap_between_dots){
        draw_line(&blue_box, x, 1.12f, b_w, b_h);
        draw_line(&blue_box, x + 0.03, 1.12f, b_w, b_h);
        draw_line(&black_box, x + 0.03, 1.08f, b_w, b_h);
    }
    
    
    for (float y = -1.0f; y <= 1.0f; y+=gap_between_dots){
        draw_line(&blue_box, -2.1f, y, b_w, b_h);
        draw_line(&black_box, -1.96f, y, b_w, b_h);
    }
    

    // Box code for STOP, SLOW, NORMAL, and RACING
    float box_x = -0.4f; float box_y = -0.95f;
    float box_stop = 0.25, box_height = 0.18;
    float box_slow = 0.25;
    float box_normal = 0.36;
    float box_racing = 0.35;
    float gap = 0.08;
    float tx_height = 0.06;
    float font_scale = 0.005/3;
    draw_box(&main_border, box_x, box_y, box_stop, box_height, line_w/2);
    draw_line(&black_box, box_x, box_y, box_stop + line_w/2, box_height + line_w/2);
    render_font(&helvetica_bloom, "STOP" , box_x + line_w, box_y + box_height- tx_height, font_scale, CLOCK_TEXT_COLOR, global_camera);
    box_x += box_stop + gap;
    draw_box(&main_border, box_x, box_y, box_slow, box_height, line_w/2);
    draw_line(&black_box, box_x, box_y, box_slow + line_w/2, box_height + line_w/2);
    render_font(&helvetica_bloom, "SLOW" , box_x + line_w, box_y + box_height- tx_height, font_scale, CLOCK_TEXT_COLOR, global_camera);
    box_x += box_slow+ gap;
    draw_box(&main_border, box_x, box_y, box_normal, box_height, line_w/2);
    draw_line(&black_box, box_x, box_y, box_normal+ line_w/2, box_height + line_w/2);
    render_font(&helvetica_bloom, "NORMAL" , box_x + line_w, box_y + box_height- tx_height, font_scale, CLOCK_TEXT_COLOR, global_camera);
    box_x += box_normal + gap;
    draw_box(&main_border, box_x, box_y, box_racing, box_height, line_w/2);
    draw_line(&red_box, box_x, box_y, box_racing + line_w/2, box_height/2 + line_w/2);
    draw_line(&black_box, box_x, box_y, box_racing + line_w/2, box_height + line_w/2);
    render_font(&helvetica_bloom, "RACING" , box_x + line_w, box_y + box_height - tx_height, font_scale, CLOCK_TEXT_COLOR, global_camera);

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
    if (state->state == NORMAL){
        radius_global = 0.0f; 
        final.set_float(&final, "radius", radius_global); 
    } else {
        if (radius_global >= 5){
            radius_global = 5; 
        } else {
            radius_global += 0.1; 
            radius_global *= 4; 
        }
        final.set_float(&final, "radius", radius_global); 
    }
    
    glActiveTexture(GL_TEXTURE0); // Prepare to bind color buffer from our fbo to texture
    glBindTexture(GL_TEXTURE_2D, colorBuffers[0]); // take our original source
    // glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);

    final.use(&final);
    render_quad();

    eglSwapBuffers(state->egl_display, state->egl_surface);
}

void initgl(struct client_state *state){
    enableGLDebug();
    // REGULAR RENDERING CODE
    vec3 temp_position = {0.0f, 0.0f, 3.0f};
    init_camera(&global_camera, temp_position);

    char *goal = "活動限界まで内部主エネルギー供給システムやめるスロー正常レースあと";
    init_font(&matisse, &text_shader, "matias.otf", goal, 48*2, 1.0f, 1.0f);
    init_font(&timer, &text_shader, "Digital-Display.ttf", goal, 300, 1.0f, 2.0f);
    init_font(&helvetica, &text_shader, "Helvetica.ttf", goal, 48*1.5, 1.0f, 1.0f);

    // float colors[][3] = {{0.745, 0.341, 0.254}, {0.67f, 0.792f, 0.301f}, {0.227, 0.5686, 0.2901}};
    float colors[][3] = {{0.54, 0.06, 0.03}, {0.45f, 0.74f, 0.06f}, {0.03, 0.27, 0.06}};

    int length;
    float* main_eva_gradient = generate_gradient(3, colors, &length);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    init_shader(&texture_shader_bloom, "vertex_texture.vs", "texture_bloom.fs"); // takes in two textures and combines them into one output
    init_shader(&texture_shader, "vertex_texture.vs", "texture.fs"); // takes in two textures and combines them into one output
    init_shader(&global_shader, "vertex.vs", "fragment.fs");
    init_shader(&text_shader, "text_vertex.vs", "text_fragment.fs");

    // BLOOOM RELATED CODE
    init_shader(&global_shader_bloom, "vertex.vs", "bloom_fragment.fs"); // takes in two textures and combines them into one output
    init_shader(&global_shader_bloom_mono, "vertex.vs", "bloom_fragment_mono.fs"); // takes in two textures and combines them into one output
    init_shader(&b_shader, "text_bloom.vs", "text_bloom.fs"); // renders a text field only to be blurred
    init_shader(&gaussian, "gaussian.vs", "gaussian.fs"); // does ping pong gaussian blurring on the texture several times
    init_shader(&final, "final.vs", "final.fs"); // takes in two textures and combines them into one output

    init_font(&matisse_bloom, &b_shader, "matias.otf", goal, 48*2, 1.0f, 1.3f);
    init_font(&clock_bloom, &b_shader, "7-segment-mono.otf", goal, 48*32, 0.6f, 1.1f);
    init_font(&helvetica_bloom, &b_shader, "Helvetica.ttf", goal, 48*1, 1.0f, 1.0f);

    init_entity_texture(&main_panel, &global_camera, &texture_shader_bloom, VERTICES_COLOR_TEXTURE, quad, sizeof(quad), GL_TRIANGLES, "clock_5.png");
    init_entity_texture(&red_stripe, &global_camera, &texture_shader_bloom, VERTICES_COLOR_TEXTURE, red_quad, sizeof(red_quad), GL_TRIANGLES, "stripe.png");
    // init_entity_texture(&main_panel, &global_camera, &texture_shader, VERTICES_COLOR_TEXTURE, red_quad, sizeof(red_quad), GL_TRIANGLES, "awesomeface.png");
    init_entity(&main_gradient, &global_camera, &global_shader, VERTICES_COLOR, main_eva_gradient, length * sizeof(float), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    init_entity(&test_entity, &global_camera, &global_shader, VERTICES_COLOR, main_eva_gradient, length * sizeof(float), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    init_entity(&test_entity_2, &global_camera, &global_shader_bloom, VERTICES_COLOR, main_eva_gradient, length * sizeof(float), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    init_entity(&main_border, &global_camera, &global_shader_bloom, VERTICES_COLOR, quad_color,  sizeof(quad_color), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    init_entity(&black_box, &global_camera, &global_shader_bloom, VERTICES_COLOR, quad_color_black,  sizeof(quad_color_black), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    init_entity(&red_box, &global_camera, &global_shader_bloom, VERTICES_COLOR, red_quad_vert,  sizeof(red_quad_vert), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    init_entity(&blue_box, &global_camera, &global_shader_bloom, VERTICES_COLOR, blue_quad_vert,  sizeof(blue_quad_vert), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    init_entity(&slant, &global_camera, &global_shader_bloom, VERTICES_COLOR, quad_color,  sizeof(quad_color), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static
    init_entity(&colorable_slant, &global_camera, &global_shader_bloom_mono, VERTICES, quad_vert,  sizeof(quad_vert), GL_TRIANGLES); // ran into issue where initializing size is based on if u have the pointer or not to it, or if i is static

    glm_translate(main_gradient.model, (vec3){0.0, 0.0, -0.01});
    glm_translate(main_panel.model, (vec3){0.0, 0.0, -0.001f});
    glm_scale(main_gradient.model, (vec3){2*1.0f, 2*1080.0f/1920.0f, 1.0f});
    glm_scale(main_panel.model, (vec3){2*1.0f, 2*1080.0f/1920.0f, 1.0f});
    glm_translate(test_entity_2.model, (vec3){-1.0, 0.0, 1.0});
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
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
