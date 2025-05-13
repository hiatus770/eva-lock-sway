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


struct shader globalShader;
unsigned int VAO, VBO;

/* Wayland Code */
struct client_state
{
    struct wl_display *wl_display;
    struct wl_compositor *compositor;
    struct wl_shm *wl_shm;
    struct wl_registry *wl_registry;
    struct xdg_wm_base *xdg_wm_base;

    // Objects
    struct wl_surface *wl_surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;

    int width, height;
    bool closed;

    // State
    float offset;
    uint32_t last_frame;

    // Egl things
    EGLContext egl_context;
    EGLSurface egl_surface;
    struct wl_egl_window *egl_window;
    EGLDisplay egl_display;
};

// buffer listener

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
    wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release,
};

static struct wl_buffer *draw_frame(struct client_state *state)
{
    // Drawing code
    const int width = 640, height = 480;
    int stride = width * 4;
    int size = stride * height;

    int fd = allocate_shm_file(size);
    if (fd == -1)
    {
        return NULL;
    }

    uint32_t *data = mmap(NULL, size,
                          PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED)
    {
        close(fd);
        return NULL;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(state->wl_shm, fd, size);
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0,
                                                         width, height, stride, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);

    /* Draw checkerboxed background */
    int offset = (int)state->offset % 8;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            if (((x + offset) + (y + offset) / 8 * 8) % 16 < 8)
                data[y * width + x] = 0xFF666666;
            else
                data[y * width + x] = 0xFFEEEEEE;
        }
    }

    munmap(data, size);
    wl_buffer_add_listener(buffer, &wl_buffer_listener, NULL);
    return buffer;
}

// Surface listener

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
    struct client_state *state = data;

    xdg_surface_ack_configure(xdg_surface, serial);

    if (state->height != 0 && state->width != 0){
        glViewport(0, 0, state->width, state->height);
    }
    glClearColor(0.0, 1.0, 0.0, 1.0); // Example: green background
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(state->egl_display, state->egl_surface);
    wl_surface_commit(state->wl_surface);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};


// toplevel listener
static void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states){
    struct client_state *state = data;
    if (width == 0 || height == 0){
        return;
    }
    if (state->egl_window){
        wl_egl_window_resize(state->egl_window, width, height, 0, 0);
        wl_surface_commit(state->wl_surface);
    }
    state->width = width;
    state->height = height;
}

static void xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel){
    struct client_state *state = data;
    state->closed = true;
    ifd {
        fprintf(stderr, "WANTING TO CLOSE!");
    }
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close = xdg_toplevel_close
};

// xdg wm base

static void
xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

// Frame call back stuff

static const struct wl_callback_listener wl_surface_frame_listener;

/**
* This is the important function called when they want us to render a frame again!
*/
static void wl_surface_frame_done(void *data, struct wl_callback *cb, uint32_t time){
    // Destroy this callback
    wl_callback_destroy(cb);

    // Request another frame
    struct client_state *state = data;
    cb = wl_surface_frame(state->wl_surface);
    wl_callback_add_listener(cb, &wl_surface_frame_listener, state);

    // update scroll amount at 24 pix / second
    if (state->last_frame != 0){
        int elapsed = time - state->last_frame;
        state->offset += elapsed / 1000.0 * 24;
    }

    // struct wl_buffer *buffer = draw_frame(state);
    // wl_surface_attach(state->wl_surface, buffer, 0, 0);
    // wl_surface_damage_buffer(state->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
    // wl_surface_commit(state->wl_surface);

    glViewport(0, 0, state->width, state->height);
    glClearColor(0.0, 0.4, 0.0, 0.5);
    glClear(GL_COLOR_BUFFER_BIT);

    globalShader.use(&globalShader);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    eglSwapBuffers(state->egl_display, state->egl_surface);

    state->last_frame = time;
}

static const struct wl_callback_listener wl_surface_frame_listener = {
    .done = wl_surface_frame_done,
};

// Registry object handling

static void
registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    struct client_state *state = data;
    printf("interface: '%s', version: %d, name: %d\n",
           interface, version, name);

    if (strcmp(interface, wl_shm_interface.name) == 0)
    {
        state->wl_shm = wl_registry_bind(registry, name, &wl_shm_interface, 2);
    }

    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        state->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 6);
    }

    if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        state->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(state->xdg_wm_base, &xdg_wm_base_listener, state);
    }
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{

    // This space deliberately left blank
}

static const struct wl_registry_listener
    registry_listener = {
        .global = registry_handle_global,
        .global_remove = registry_handle_global_remove,
};


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
