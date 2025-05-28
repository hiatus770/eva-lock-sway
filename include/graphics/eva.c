#include "eva.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../logger.h"
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include "text.h"
#include "../../src/globals.h"
#include "entity.h"

void render_clock(font* timer_font, struct camera global_camera){

    time_t now;
    struct tm *tm;
    struct timeval tv;
    now = time(0);

    gettimeofday(&tv, NULL);

    if ((tm = localtime (&now)) == NULL) {
        log_error("Error extracting time stuff\n");
        return;
    }

    // printf("Current time: %02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);

    float x = -0.55f, y = -0.65f;

    char* out_hours_minutes = malloc(20 * sizeof(char));
    char* out_seconds = malloc(20 * sizeof(char));
    sprintf(out_hours_minutes, "%01d:%02d", tm->tm_min%10, tm->tm_sec);
    sprintf(out_seconds, ":%02d", (int) (tv.tv_usec / 1000) % 100);

    render_font(timer_font, out_hours_minutes, x, y, 0.0008*0.9, CLOCK_TEXT_COLOR, global_camera);
    render_font(timer_font, out_seconds, x + 1.27, y, 0.0004*0.9, CLOCK_TEXT_COLOR, global_camera);
}

float* generate_gradient(int color_count, float (*arr)[3], int* length){
    *length = (color_count - 1) * 2 * 3 * 6;
    // Figure out how many columns we are going to have
    assert(color_count > 1);
    float (*rarr)[6] = malloc(sizeof(float[3*(color_count * 2 - 2)][6]));  // we have 6 entries per point, and we need 3 poinst per triangle

    float width = 2.0/(float)(color_count - 1);

    // Now start generating triangles
    int triangle_index = 0;
    for (int i = 0; i < color_count-1; i++){
        // This is iterating column wise
        float temp_arr1[6] = {-1.0f + width * i, 1.0f, 0.0f, arr[i][0], arr[i][1], arr[i][2]};
        float temp_arr2[6] = {-1.0f + width * i, -1.0, 0.0f, arr[i][0], arr[i][1], arr[i][2]};
        float temp_arr3[6] = {-1.0f + width * i + width, 1.0f, 0.0f, arr[i+1][0], arr[i+1][1], arr[i+1][2]};

        float temp_arr4[6] = {-1.0f + width * i + width, -1.0f, 0.0f, arr[i+1][0], arr[i+1][1], arr[i+1][2]};
        float temp_arr5[6] = {-1.0f + width * i + width, 1.0f,  0.0f, arr[i+1][0], arr[i+1][1], arr[i+1][2]};
        float temp_arr6[6] = {-1.0f + width * i, -1.0f, 0.0f, arr[i][0], arr[i][1], arr[i][2]};

        memcpy(rarr[triangle_index++], temp_arr1, sizeof(temp_arr1));
        memcpy(rarr[triangle_index++], temp_arr2, sizeof(temp_arr1));
        memcpy(rarr[triangle_index++], temp_arr3, sizeof(temp_arr1));
        memcpy(rarr[triangle_index++], temp_arr4, sizeof(temp_arr1));
        memcpy(rarr[triangle_index++], temp_arr5, sizeof(temp_arr1));
        memcpy(rarr[triangle_index++], temp_arr6, sizeof(temp_arr1));
    }

    float* flat = (float*)rarr;
    for (int i = 0; i < *length; i++) {
        fprintf(stderr, "%f ", flat[i]);
        if (i % 6 == 5) fprintf(stderr, "\n");
    }


    return (float*)rarr;
}

void draw_line(struct entity* quad, float line_x, float line_y, float width, float height){
    // now we draw the line
    glm_mat4_identity(quad->model);
    glm_scale(quad->model, (vec3){width, height, 1.0f});
    // glm_translate(quad->model, (vec3){line_x + width/2, line_y + height/2, 0.0f});
    glm_translate(quad->model, (vec3){line_x/width + 0.5, line_y/height + 0.5, 0.0f});
    quad->render(quad);
}

void draw_slant(struct entity* quad, float line_x, float line_y, float height, float line_x_f, float line_y_f){
    glm_mat4_identity(quad->model);
    // glm_spin(quad->model, angle, (vec3){0.0, 0.0, 1.0f});
    // glm_translate(quad->model, (vec3){line_x, line_y, 0.0f});
    // glm_rotate_at(quad->model, (vec3){line_x, line_y, 0.0f}, angle, (vec3){0.0, 0.0, 1.0f});
    // glm_scale(quad->model, (vec3){length, width, 1.0f});
    //
    float quad_color[] = {
        line_x_f,  line_y_f + height, 0.0f, CLOCK_COLOR_RAW   // top right
        line_x_f, line_y_f, 0.0f, CLOCK_COLOR_RAW   // bottom right
        line_x, line_y, 0.0f, CLOCK_COLOR_RAW  // bottom left
        line_x, line_y, 0.0f, CLOCK_COLOR_RAW  // bottom left
        line_x, line_y + height, 0.0f, CLOCK_COLOR_RAW  // top left
        line_x_f, line_y_f + height, 0.0f, CLOCK_COLOR_RAW  //  top right
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, quad->VBO); 
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_color), quad_color);
    


    // glm_rotate(vec4 *m, float angle, float *axis)
    // glm_translate(quad->model, (vec3){line_x + width/2, line_y + height/2, 0.0f});
    quad->render(quad);
}
