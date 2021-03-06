#include <pebble.h>

// invert colors?
// by default display 'white' theme
static bool invertColors = true;
// windows, layers etc.
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_text_layer;
static InverterLayer *s_invert_layer;
static BitmapLayer *s_logo_layer;
static GBitmap *s_logo_bitmap;

// refresh the time
static void update_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
 
  static char buffer[] = "00:00";
 
  if(clock_is_24h_style() == true) {
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
 
  text_layer_set_text(s_time_layer, buffer);
}

// show time again after the date
static void show_date_callback(void *data) {
  update_time();
}

// show date by jog
static void tap_handler(AccelAxisType axis, int32_t direction) {
  static char date_buffer[] = "02/20";
  time_t temp = time(NULL); 
  strftime(date_buffer, sizeof("02/20"), "%m/%d", localtime(&temp));
  text_layer_set_text(s_time_layer, date_buffer);
  app_timer_register(2000, show_date_callback, NULL);
}

// prepare all layers and window
static void main_window_load(Window *window) { 
  // logo
  s_logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOGO);
  s_logo_layer = bitmap_layer_create(GRect(3, 3, 144, 66));
  bitmap_layer_set_bitmap(s_logo_layer, s_logo_bitmap);
  
  // VIAcode text
  s_text_layer = text_layer_create(GRect(0, 66, 144, 120));
  text_layer_set_background_color(s_text_layer, GColorWhite);
  text_layer_set_text_color(s_text_layer, GColorBlack);
  text_layer_set_text(s_text_layer, "VIAcode");
  
  // time
  s_time_layer = text_layer_create(GRect(0, 115, 144, 168));
  text_layer_set_background_color(s_time_layer, GColorWhite);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  
  // font and aligment
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  
  // add layers to window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_text_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_logo_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // invert colors if needed
  if (invertColors) {
    // screen size 144x168
    s_invert_layer = inverter_layer_create(GRect(0, 0, 144, 168));
    layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(s_invert_layer));
  }
  
  // show time
  update_time();
}
 
// destroy objects
static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_text_layer);
  gbitmap_destroy(s_logo_bitmap);
  bitmap_layer_destroy(s_logo_layer);
  inverter_layer_destroy(s_invert_layer);
  accel_tap_service_unsubscribe();
}
 
// update time handler
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
  
// create window and init
static void init() {
  s_main_window = window_create();
 
  window_set_background_color(s_main_window, GColorWhite);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
 
  window_stack_push(s_main_window, true);
  // subscribe for time change every 1 minute
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // subscribe for tap to display date by jog
  accel_tap_service_subscribe(tap_handler);
}
 
// destroy window
static void deinit() {
  window_destroy(s_main_window);
}
 
int main(void) {
  init();
  app_event_loop();
  deinit();
}