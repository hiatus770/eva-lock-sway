#include "camera.h"
#include "../client_state.h"
#include "../../src/globals.h"

extern struct client_state state;

void set_position(struct camera *camera, vec3 newPos){
    glm_vec3_copy(newPos, camera->position);
}

void look_at(struct camera *camera, vec3 look_at_pos){
    glm_mat4_identity(camera->view_matrix);
    glm_look(camera->position, camera->direction, camera->up, camera->view_matrix);
}

mat4 *get_projection(struct camera *camera){
    glm_mat4_identity(camera->projection);
    // glm_perspective(glm_rad(45.0f), 1920.0f/1080.0f, 0.1f, 20.0f, camera->projection); // look into this a bit more?
    glm_perspective(glm_rad(45.0f), (float)SRC_WIDTH/(float)SRC_HEIGHT, 0.5f, 7.0f, camera->projection); // look into this a bit more?
    return &camera->projection;
}

mat4* get_view_matrix(struct camera *camera){
    glm_mat4_identity(camera->view_matrix);
    glm_look(camera->position, camera->direction, camera->up, camera->view_matrix);
    return &camera->view_matrix;
}

void init_camera(struct camera *camera, vec3 pos){
    glm_vec3_copy(pos, camera->position);
    camera->set_position = &set_position;
    camera->look_at = &look_at;
    camera->get_view_matrix = &get_view_matrix;
    camera->get_projection_matrix = &get_projection;

    vec3 up = {0.0f, 1.0f, 0.0f};
    glm_vec3_copy(up, camera->up);

    vec3 forward = {0.0f, 0.0f, -1.0f}; // forward direction by default is just forward
    glm_vec3_copy(forward, camera->direction);
}
