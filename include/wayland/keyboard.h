#pragma once 
#include <sys/mman.h>
#include <wayland-client-core.h>
#include <xkbcommon/xkbcommon.h> 
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <assert.h>
#include <client_state.h>

static void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size){
    struct client_state *client_state = data; 
    assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1); 
    
    char *map_shm = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0); 
    assert(map_shm != MAP_FAILED); 

    
    struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(client_state->xkb_context, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS); 
    munmap(map_shm, size); // We use mmap to do stuff idk
    
    struct xkb_state *xkb_state = xkb_state_new(xkb_keymap); 
    xkb_keymap_unref(client_state->xkb_keymap); 
    xkb_state_unref(client_state->xkb_state); 
    
    client_state->xkb_keymap = xkb_keymap; 
    client_state->xkb_state = xkb_state; 
}

static void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys){
    struct client_state *client_state = data; 
    fprintf(stderr, "keyboadr enter; keys pressed are:\n"); 
    uint32_t *key; 
    wl_array_for_each(key,keys){
        char buf[128]; 
        xkb_keysym_t sym = xkb_state_key_get_one_sym(client_state->xkb_state, *key + 8); 
        xkb_keysym_get_name(sym, buf, sizeof(buf)); 
        fprintf(stderr, "sym: %-12s (%d),", buf, sym); 
        xkb_state_key_get_utf8(client_state->xkb_state, *key + 8, buf, sizeof(buf)); 
        fprintf(stderr, "utf8: '%s'\n", buf); 
    }
}

static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state){
    struct client_state *client_state = data; 
    char buf[128]; 
    uint32_t keycode = key + 8; 
    xkb_keysym_t sym = xkb_state_key_get_one_sym(client_state->xkb_state, keycode); 
    xkb_keysym_get_name(sym,buf,sizeof(buf)); 
    
    const char *action = state == WL_KEYBOARD_KEY_STATE_PRESSED ? "press" : "release"; 
    fprintf(stderr, "key %s: sym: %-12s %d", action, buf, sym);
   
    xkb_state_key_get_utf8(client_state->xkb_state, keycode, buf, sizeof(buf)); 
    fprintf(stderr, "utf8: '%s'\n", buf); 
}

static void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface){
    fprintf(stderr, "keyboard leave!\n"); 
}

static void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard, 
    uint32_t serial, uint32_t mods_depressed, 
    uint32_t mods_latched, uint32_t mods_locked, 
    uint32_t group){
    struct client_state *client_state = data; 
    xkb_state_update_mask(client_state->xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group); 
}

static void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay){
    /* idk what to do here cuh  */
}

static const struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap = wl_keyboard_keymap,
    .enter = wl_keyboard_enter, 
    .leave = wl_keyboard_leave, 
    .key = wl_keyboard_key, 
    .modifiers = wl_keyboard_modifiers, 
    .repeat_info = wl_keyboard_repeat_info 
}; 