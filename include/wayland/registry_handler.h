#pragma once
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <wayland-client-core.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <string.h>
#include "../xdg-shell-client-protocol.h"
#include "wm_base.h" // Needed because in this file we use the wm_base in the last if statement in the handle global
#include "wl_seat.h" // Needed because we use the seat listener in this file
#include "lock_manager.h"
#include "../ext-session-lock-v1-protocol.h"

#include "../client_state.h"

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version){
    struct client_state *state = data;

    printf("Interface: %s, version %d, name %d\n", interface, version, name);

    if (strcmp(interface, wl_shm_interface.name) == 0){
        state->wl_shm = wl_registry_bind(registry, name, &wl_shm_interface, 2);
    }

    if (strcmp(interface, wl_compositor_interface.name) == 0){
        state->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 6); // This is the version that we use and the one that is defined in the computer wayland.xml??? idk if this is different on different computers might have to look into this later
    }

    if (strcmp(interface, xdg_wm_base_interface.name) == 0){
        state->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(state->xdg_wm_base, &xdg_wm_base_listener, state);
    }
    
    if (strcmp(interface, wl_seat_interface.name) == 0){
        state->wl_seat = wl_registry_bind(registry, name, &wl_seat_interface, 9); 
        wl_seat_add_listener(state->wl_seat, &wl_seat_listener, state); 
    }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name){
    // This is deliberately left blank!
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};
