#pragma once
#include <stdint.h>

#include <wayland-client-core.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include "../client_state.h"
#include "logger.h"

static void wl_seat_capabilities(void* data, struct wl_seat *wl_seat, uint32_t capabilities){
    struct client_state *state = data;
}

// Just prints the name for now
static void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name){
    fprintf(stderr, "Seat name: %s\n", name);
}

static const struct wl_seat_listener wl_seat_listener = {
    .capabilities = wl_seat_capabilities,
    .name = wl_seat_name
};
