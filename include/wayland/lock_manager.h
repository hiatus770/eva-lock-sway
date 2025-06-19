#pragma once

#include <wayland-client-core.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include "../ext-session-lock-v1-protocol.h"
#include "../client_state.h"

static void handle_locked(void *data, struct ext_session_lock_v1 *lock){
    fprintf(stderr, "Locked!");
    struct client_state *state = data;
}

static void handle_finished(void *data, struct ext_session_lock_v1 *lock){
     fprintf(stderr, "Exiting lock! OR Failed to lock!");
};

static const struct ext_session_lock_v1_listener ext_session_lock_v1_listener  = {
    .locked = handle_locked,
    .finished = handle_finished
};
