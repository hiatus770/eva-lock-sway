#include "eva.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../logger.h"
#include <time.h>
#include <stdio.h>
#include "text.h"
#include "../../src/globals.h"

void render_clock(font* timer_font, struct camera global_camera){
    time_t now;
    struct tm *tm;
    now = time(0);

    if ((tm = localtime (&now)) == NULL) {
        log_error("Error extracting time stuff\n");
        return;
    }

    // printf("Current time: %02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);

    float x = 0.0f, y = 0.5f;

    char* out_hours_minutes = malloc(20 * sizeof(char));
    char* out_seconds = malloc(20 * sizeof(char));
    sprintf(out_hours_minutes, "%02d:%02d", tm->tm_hour, tm->tm_min);
    sprintf(out_seconds, ":%02d", tm->tm_sec);
    // char *test = "htesttst";

    render_font(timer_font, out_hours_minutes, x, y, 0.002, CLOCK_TEXT_COLOR, global_camera);
    render_font(timer_font, out_seconds, x + 1.16, y, 0.001, CLOCK_TEXT_COLOR, global_camera);
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

    // if we have 3 colors in the array
    // a b c d
    // a b c d




    return (float*)rarr;
}
