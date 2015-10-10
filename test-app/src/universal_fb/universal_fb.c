/**
 * Universal set, get, and swap colors for Pebble SDK 3.0
 * Author: Chris Lewis
 * License: MIT
 */
 
#include "universal_fb.h"

/******************************* Compatibility ********************************/

// Provide GBitmapDataRowInfo API on 2.x
#if defined(PBL_SDK_2)
typedef struct {
  int min_x;
  int max_x;
  uint8_t *data;
} GBitmapDataRowInfo;

static GBitmapDataRowInfo gbitmap_get_data_row_info(GBitmap *bitmap, int y) {
  uint8_t *ptr = gbitmap_get_data(bitmap);
  int rsb = gbitmap_get_bytes_per_row(bitmap);
  ptr = &ptr[y * rsb];
  return (GBitmapDataRowInfo) {
    .min_x = 0, .max_x = 143, .data = ptr // Start of this row
  };
}
#endif

/********************************** Internal **********************************/

// "0000 0010 get bit 1" --> 1
static bool byte_get_bit(uint8_t *byte, uint8_t bit) {
  return ((*byte) >> bit) & 1;
}

static void byte_set_bit(uint8_t *byte, uint8_t bit, bool value) {
  *byte = (value << bit) | *byte;
}

/************************************ API *************************************/

GColor universal_fb_get_pixel_color(GBitmap *fb, GPoint point) {
  GRect bounds = gbitmap_get_bounds(fb);
  GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, point.y);
  if(point.x >= info.min_x && point.x <= info.max_x
  && point.y >= bounds.origin.y && point.y <= bounds.origin.y + bounds.size.h) {
#if defined(PBL_COLOR)
    return (GColor){ .argb = info.data[point.x] };
#elif defined(PBL_BW)
    uint8_t byte = point.x / 8;
    uint8_t bit = point.x % 8; // fb: bwbb bbbb -> byte: 0000 0010
    return byte_get_bit(&byte, bit) ? GColorWhite : GColorBlack;
#endif
  } else {
    // Out of bounds
    APP_LOG(APP_LOG_LEVEL_ERROR, "Pixel %d,%d out of bounds on this display", point.x, point.y);
    return GColorClear;
  }
}

void universal_fb_set_pixel_color(GBitmap *fb, GPoint point, GColor color) {
  GRect bounds = gbitmap_get_bounds(fb);
  GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, point.y);
  if(point.x >= info.min_x && point.x <= info.max_x
  && point.y >= bounds.origin.y && point.y <= bounds.origin.y + bounds.size.h) {
#if defined(PBL_COLOR)
    memset(&info.data[point.x], color.argb, 1);
#elif defined(PBL_BW)
    uint8_t byte = point.x / 8;
    uint8_t bit = point.x % 8; // fb: bwbb bbbb -> byte: 0000 0010
    uint8_t value = (color == GColorWhite) ? 1 : 0;   // '1 is white'
    byte_set_bit(&byte, bit, value);

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting pixel %d,%d (byte %d bit %d) to %s",
      point.x, point.y, byte, bit, value == 1 ? "GColorWhite" : "GColorBlack");
#endif
  } else {
    // Out of bounds
    APP_LOG(APP_LOG_LEVEL_ERROR, "Pixel %d,%d out of bounds on this display", point.x, point.y);
    return;
  }
}

void universal_fb_swap_colors(GBitmap *fb, GColor c1, GColor c2) {
  GRect bounds = gbitmap_get_bounds(fb);

  for(int y = 0; y < bounds.size.h; y++) {
    GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, y);
    for(int x = info.min_x; x < info.max_x; x++) {
      if(gcolor_equal(universal_fb_get_pixel_color(fb, GPoint(x, y)), c1)) {
        // Replace c1 with c2
        universal_fb_set_pixel_color(fb, GPoint(x, y), c2);
      } else if(gcolor_equal(universal_fb_get_pixel_color(fb, GPoint(x, y)), c2)) {
        // Vice versa
        universal_fb_set_pixel_color(fb, GPoint(x, y), c1);
      }
    }
  }
}
