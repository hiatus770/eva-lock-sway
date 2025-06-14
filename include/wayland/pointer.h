#pragma once
#include <stdbool.h>
#include <stdint.h>

#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-util.h>
#include "../client_state.h"
#include "pointer_struct.h"

enum pointer_event_mask {
  POINTER_EVENT_ENTER = 1 << 0,
  POINTER_EVENT_LEAVE = 1 << 1,
  POINTER_EVENT_MOTION = 1 << 2,
  POINTER_EVENT_BUTTON = 1 << 3,
  POINTER_EVENT_AXIS = 1 << 4,
  POINTER_EVENT_AXIS_SOURCE = 1 << 5,
  POINTER_EVENT_AXIS_STOP = 1 << 6,
  POINTER_EVENT_AXIS_DISCRETE = 1 << 7,
};

static void wl_pointer_enter(void *data, struct wl_pointer *wl_pointer,
                             uint32_t serial, struct wl_surface *surface,
                             wl_fixed_t surface_x, wl_fixed_t surface_y) {
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_ENTER;
  client_state->pointer_event.serial = serial;
  client_state->pointer_event.surface_x = surface_x,
  client_state->pointer_event.surface_y = surface_y;
}

static void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
                             uint32_t serial, struct wl_surface *surface) {
  struct client_state *client_state = data;
  client_state->pointer_event.serial = serial;
  client_state->pointer_event.event_mask |= POINTER_EVENT_LEAVE;
}

static void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer,
                              uint32_t time, wl_fixed_t surface_x,
                              wl_fixed_t surface_y) {
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_MOTION;
  client_state->pointer_event.time = time;
  client_state->pointer_event.surface_x = surface_x,
  client_state->pointer_event.surface_y = surface_y;
}

static void wl_pointer_button(void *data, struct wl_pointer *wl_pointer,
                              uint32_t serial, uint32_t time, uint32_t button,
                              uint32_t state) {
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_BUTTON;
  client_state->pointer_event.time = time;
  client_state->pointer_event.serial = serial;
  client_state->pointer_event.button = button;
  client_state->pointer_event.button = button,
  client_state->pointer_event.state = state;
}

static void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer,
                            uint32_t time, uint32_t axis, wl_fixed_t value) {
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS;
  client_state->pointer_event.time = time;
  client_state->pointer_event.axes[axis].valid = true;
  client_state->pointer_event.axes[axis].value = value;
}

static void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
                                   uint32_t axis_source) {
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_SOURCE;
  client_state->pointer_event.axis_source = axis_source;
}

static void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
                                 uint32_t time, uint32_t axis) {
  struct client_state *client_state = data;
  client_state->pointer_event.time = time;
  client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_STOP;
  client_state->pointer_event.axes[axis].valid = true;
}

static void wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
                                     uint32_t axis, int32_t discrete) {
  struct client_state *client_state = data;
  client_state->pointer_event.event_mask |= POINTER_EVENT_AXIS_DISCRETE;
  client_state->pointer_event.axes[axis].valid = true;
  client_state->pointer_event.axes[axis].discrete = discrete;
}


static void wl_pointer_frame(void *data, struct wl_pointer *wl_pointer) {
  struct client_state *client_state = data;
  struct pointer_event *event = &client_state->pointer_event;
  fprintf(stderr, "pointer frame @ %d", event->time);

  if (event->event_mask & POINTER_EVENT_ENTER) {
    fprintf(stderr, "entered %f %f", wl_fixed_to_double(event->surface_x),
            wl_fixed_to_double(event->surface_y));
  }

  if (event->event_mask & POINTER_EVENT_LEAVE) {
    fprintf(stderr, "pointer leave event");
  }

  if (event->event_mask & POINTER_EVENT_MOTION) {
    fprintf(stderr, "motion %f %f", wl_fixed_to_double(event->surface_x),
            wl_fixed_to_double(event->surface_y));
  }

  if (event->event_mask & POINTER_EVENT_BUTTON) {
    char *state = event->state == WL_POINTER_BUTTON_STATE_RELEASED ? "released"
                                                                   : "pressed";
    fprintf(stderr, "Button %d %s", event->button, state);
  }

  uint32_t axis_events = POINTER_EVENT_AXIS | POINTER_EVENT_AXIS_SOURCE |
                         POINTER_EVENT_AXIS_STOP | POINTER_EVENT_AXIS_DISCRETE;
  char *axis_name[2] = {[WL_POINTER_AXIS_VERTICAL_SCROLL] = "vertical",
                        [WL_POINTER_AXIS_HORIZONTAL_SCROLL] = "horizontal"};
  char *axis_source[4] = {[WL_POINTER_AXIS_SOURCE_WHEEL] = "wheel",
                          [WL_POINTER_AXIS_SOURCE_FINGER] = "finger",
                          [WL_POINTER_AXIS_SOURCE_CONTINUOUS] = "continuous",
                          [WL_POINTER_AXIS_SOURCE_WHEEL_TILT] = "wheel tilt"};

  if (event->event_mask & axis_events) {
    for (size_t i = 0; i < 2; i++) {
      if (!event->axes[i].valid) {
        continue;
      }
      fprintf(stderr, "%s axis ", axis_name[i]);
      if (event->event_mask & POINTER_EVENT_AXIS) {
        fprintf(stderr, "value %f ", wl_fixed_to_double(event->axes[i].value));
      }
      if (event->event_mask & POINTER_EVENT_AXIS_DISCRETE) {
        fprintf(stderr, "discrete %d ", event->axes[i].discrete);
      }
      if (event->event_mask & POINTER_EVENT_AXIS_SOURCE) {
        fprintf(stderr, "via %s ", axis_source[event->axis_source]);
      }
      if (event->event_mask & POINTER_EVENT_AXIS_STOP) {
        fprintf(stderr, "(stopped) ");
      }
    }
  }
  fprintf(stderr, "\n");
  memset(event, 0, sizeof(*event)); // so cool
}

static void wl_axis_value120(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t value120){
    // Do nothing since we aren't using this functionality 
    return; 
}

static void wl_axis_relative_direction(void *data, struct wl_pointer *wl_pointer, uint32_t axis, uint32_t direction){
    return; // Also do nothing for this function since we don't use its functionality 
}


static const struct wl_pointer_listener wl_pointer_listener = {
    .enter = wl_pointer_enter,
    .leave = wl_pointer_leave,
    .motion = wl_pointer_motion,
    .button = wl_pointer_button,
    .axis = wl_pointer_axis,
    .frame = wl_pointer_frame,
    .axis_source = wl_pointer_axis_source,
    .axis_stop = wl_pointer_axis_stop,
    .axis_discrete = wl_pointer_axis_discrete,
    .axis_value120 = wl_axis_value120, 
    .axis_relative_direction = wl_axis_relative_direction 
};
