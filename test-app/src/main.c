#include <pebble.h>

#include "universal_fb/universal_fb.h"

static Window *s_window;
static Layer *s_layer;

static void invert(GContext *ctx, GColor c1, GColor c2) {
  GBitmap *fb = graphics_capture_frame_buffer(ctx);

  universal_fb_swap_colors(fb, c1, c2);

  universal_fb_set_pixel_color(fb, GPoint(70, 70), PBL_IF_COLOR_ELSE(GColorBlue, GColorBlack));

  // Release framebuffer
  graphics_release_frame_buffer(ctx, fb);
}

static void update_proc(Layer *layer, GContext *ctx) {
  GColor c1 = PBL_IF_COLOR_ELSE(GColorRed, GColorBlack);
  GColor c2 = PBL_IF_COLOR_ELSE(GColorGreen, GColorWhite);

  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorRed, GColorBlack));
  graphics_fill_rect(ctx, GRect(30, 30, 30, 30), GCornerNone, 0);

  invert(ctx, c1, c2);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_layer = layer_create(bounds);
  layer_set_update_proc(s_layer, update_proc);
  layer_add_child(window_layer, s_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_layer);

  window_destroy(s_window);
}

static void init() {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

static void deinit() { }

int main() {
  init();
  app_event_loop();
  deinit();
}
