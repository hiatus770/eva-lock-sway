#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <wayland-client-core.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
// #include <GL/gl.h>
// #include <glad/glad.h>
#include "../include/glad/glad.h"
#include "xdg-shell-client-protocol.h"
#include "globals.h"
#include "shader.h"
#include "memory.h"
#include <assert.h>

/* Wayland Code */
#include "../include/client_state.h" // Main client state used for the program
#include "../include/wayland/wl_buffer.h" // Buffer listener 
#include "../include/wayland/xdg_surface.h" // Xdg surface listener struct used in the program 
#include "../include/wayland/xdg_toplevel.h" // Top level application stuff handles closing and resizing 
#include "../include/wayland/wm_base.h" // Base wm
#include "../include/wayland/wl_callback_listener.h"
#include "../include/wayland/registry_handler.h"
#include "../include/graphics.h"

// Window creation and opengl
static void create_window(struct client_state *state, int32_t width, int32_t height){
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

    ifd
        fprintf(stderr, "VERSIONS: %d %d\n", major, minor);

    eglChooseConfig(state->egl_display, attributes, &config, 1, &num_config);
    state->egl_context = eglCreateContext(state->egl_display, config, EGL_NO_CONTEXT, NULL);

    state->egl_window = wl_egl_window_create(state->wl_surface, width, height);
    state->egl_surface = eglCreateWindowSurface(state->egl_display, config, state->egl_window, NULL);
    eglMakeCurrent(state->egl_display, state->egl_surface, state->egl_surface, state->egl_context);

    ifd
        fprintf(stderr, "Tried to make current");
}


int main(int argc, char *argv[])
{
    struct client_state state = {0};
    state.width = 1920;
    state.height = 1080;
    state.wl_display = wl_display_connect(NULL);
    state.wl_registry = wl_display_get_registry(state.wl_display);

    if (!state.wl_display)
    {
        fprintf(stderr, "Failed to connect to wayland display.\n");
        return 1;
    }

    wl_registry_add_listener(state.wl_registry, &registry_listener, &state);
    wl_display_roundtrip(state.wl_display);
    fprintf(stderr, "Connected to wayland display.\n");

    state.wl_surface = wl_compositor_create_surface(state.compositor);
    state.xdg_surface = xdg_wm_base_get_xdg_surface(state.xdg_wm_base, state.wl_surface); // Pass our surface to make a xdg surface

    xdg_surface_add_listener(state.xdg_surface, &xdg_surface_listener, &state);

    state.xdg_toplevel = xdg_surface_get_toplevel(state.xdg_surface);

    xdg_toplevel_add_listener(state.xdg_toplevel, &xdg_toplevel_listener, &state);

    // Creates the necessary EGL context and information, it will initailize the egl window and it can start the opengl context
    create_window(&state, state.width, state.height);


    // Initialize GLAD after EGL context is current
    if (!gladLoadGLLoader((GLADloadproc)eglGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return 1;
    }

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    init_shader(&globalShader, "/shaders/vertex.vs", "/shaders/fragment.fs");


    // XDG Shell making the actual window
    xdg_toplevel_set_title(state.xdg_toplevel, "CLIENT!");
    wl_surface_commit(state.wl_surface);

    struct wl_callback *cb = wl_surface_frame(state.wl_surface);
    wl_callback_add_listener(cb, &wl_surface_frame_listener, &state);


    if (!eglMakeCurrent(state.egl_display, state.egl_surface, state.egl_surface, state.egl_context)) {
        fprintf(stderr, "Failed to make EGL context current: %d\n", eglGetError());
    }

    while (wl_display_dispatch(state.wl_display) != -1)
    {
        if (state.closed){
            break;
        }
    }

    eglDestroySurface(state.egl_display, state.egl_surface);
    eglDestroyContext(state.egl_display, state.egl_context);
    wl_egl_window_destroy(state.egl_window);
    xdg_toplevel_destroy(state.xdg_toplevel);
    xdg_surface_destroy(state.xdg_surface);
    wl_surface_destroy(state.wl_surface);
    wl_display_disconnect(state.wl_display);

    return 0;
}
