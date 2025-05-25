#pragma once 

#include <wayland-client-core.h> 
#include <wayland-client-protocol.h> 
#include <wayland-egl-core.h> 
#include <EGL/egl.h> 
#include <wayland-egl.h> 

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer){
    wl_buffer_destroy(wl_buffer); 
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release 
}; 

