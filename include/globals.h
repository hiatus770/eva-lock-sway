#pragma once

#define PROJECT_DIRECTORY "/home/hiatus/Documents/waylandplaying"
#define DEBUG true
#define ifd if(DEBUG)

// #define CLOCK_TEXT_COLOR (vec3){0.9933f, 0.647f, 0.0666fmake
// 
#define CLOCK_TEXT_COLOR (vec3){0.59, 0.23f, 0.00f}
#define CLOCK_COLOR_RAW 0.59, 0.23f, 0.00f,
#define BLUE_COLOR_RAW 0.12, 0.39, 0.49
#define GLOW_COLOR_RED_RAW 0.27, 0.02, 0.03,
// #define CLOCK_TEXT_COLOR (vec3){0.5f, 0.5f, 0.0f}
extern int SRC_HEIGHT, SRC_WIDTH;

typedef enum clock_state {
    NORMAL, ALARM
} clock_state;
