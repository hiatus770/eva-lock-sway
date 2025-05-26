#pragma once

#include <math.h>
#include "../cglm/cglm.h"
#include "../cglm/quat.h"
#include "../client_state.h"

struct camera {
    vec3 direction;
    vec3 up;
    vec3 position;
    mat4 projection;
    mat4 view_matrix;

    void (*set_position)(struct camera *camera, vec3 newPos);

    mat4* (*get_view_matrix)(struct camera *camera);

    mat4* (*get_projection_matrix)(struct camera *camera);

    void (*look_at)(struct camera *camera, vec3 look_at_pos);
};

void set_position(struct camera *camera, vec3 newPos);

void look_at(struct camera *camera, vec3 look_at_pos);

mat4 *get_projection(struct camera *camera);

mat4* get_view_matrix(struct camera *camera);

void init_camera(struct camera *camera, vec3 pos);
