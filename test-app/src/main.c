#include <pebble.h>

#include "universal_fb/universal_fb.h"

static Window *s_window;
static Layer *s_layer;

static void test(bool condition, char *name) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Test \"%s\" %s", 
          name, condition ? "PASSED" : "FAILED");
}

static void test_drawing(GBitmap *fb, GColor c1, GColor c2) {
  GPoint test_point = GPoint(30, 30);

  // Test set and get
  universal_fb_set_pixel_color(fb, test_point, c1);
  test(gcolor_equal(universal_fb_get_pixel_color(fb, test_point), c1), 
       "universal_fb_set/get_pixel_color");

  // Test swap
  universal_fb_swap_colors(fb, GRect(test_point.x, test_point.y, 1, 1), c1, c2);
  test(gcolor_equal(universal_fb_get_pixel_color(fb, test_point), c2), 
       "universal_fb_swap_colors");
}

static void update_proc(Layer *layer, GContext *ctx) {
  GColor c1 = PBL_IF_COLOR_ELSE(GColorRed, GColorBlack);
  GColor c2 = PBL_IF_COLOR_ELSE(GColorGreen, GColorWhite);

  GBitmap *fb = graphics_capture_frame_buffer(ctx);
  test_drawing(fb, c1, c2);
  graphics_release_frame_buffer(ctx, fb);
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
