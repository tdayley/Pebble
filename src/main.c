#include <pebble.h>
  
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *s_day_layer;
static TextLayer *s_date_layer;
static TextLayer *s_gap_layer_one;
static TextLayer *s_gap_layer_two;
static TextLayer *s_gap_layer_three;
static TextLayer *s_gap_layer_four;
static TextLayer *s_weather_layer;
static TextLayer *s_info_layer;

static GFont s_providence_font;
static GFont s_visitor_font;

char day_text[32];
char date_text[32];
char info_text[32];


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  
  strftime(day_text, sizeof(day_text), "      %A", tick_time);
  strftime(date_text, sizeof(date_text), "%B %e     ", tick_time);
  text_layer_set_text(s_day_layer, day_text);
  text_layer_set_text(s_date_layer, date_text);
}

static void tick_handler(struct tm *tick_frame, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every 30 minutes
  if(tick_frame->tm_min % 5 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
    
    // Add update info text to TextLayer
    strftime(info_text, sizeof(info_text), "Updated:\n%I:%M", tick_frame);
    text_layer_set_text(s_info_layer, info_text);
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);
  
  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
      case KEY_TEMPERATURE:
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%d", (int)t->value->int32);
        break;
      case KEY_CONDITIONS:
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
        break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
        break;
    }
    
    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // Assemble full string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s", temperature_buffer);
  text_layer_set_text(s_weather_layer, weather_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void main_window_load(Window *window) {
  //Layer *window_layer = window_get_root_layer(window);
  //GRect window_bounds = layer_get_bounds(window_layer);
  
  s_visitor_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VISITOR_16));
  s_providence_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PROVIDENCE_48));
  
  // Create day TextLayer
  s_day_layer = text_layer_create(GRect(0, 0, 144, 22));
  text_layer_set_background_color(s_day_layer, GColorBlack);
  text_layer_set_text_color(s_day_layer, GColorWhite);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentLeft);
  text_layer_set_font(s_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  
  // Create right gap TextLayer
  s_gap_layer_one = text_layer_create(GRect(0, 22, 18, 2));
  text_layer_set_background_color(s_gap_layer_one, GColorBlack);
  // Create left gap TextLayer
  s_gap_layer_two = text_layer_create(GRect(130, 22, 30, 2));
  text_layer_set_background_color(s_gap_layer_two, GColorBlack);
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 24, 144, 60));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer, s_providence_font);
  
  // Create right gap TextLayer
  s_gap_layer_three = text_layer_create(GRect(0, 84, 18, 2));
  text_layer_set_background_color(s_gap_layer_three, GColorBlack);
  // Create left gap TextLayer
  s_gap_layer_four = text_layer_create(GRect(130, 84, 30, 2));
  text_layer_set_background_color(s_gap_layer_four, GColorBlack);
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, 86, 144, 24));
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  
  // Create weather TextLayer
  s_weather_layer = text_layer_create(GRect(5, 110, 75, 200));
  text_layer_set_background_color(s_weather_layer, GColorWhite);
  text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentLeft);
  text_layer_set_font(s_weather_layer, s_providence_font);
  
  // Create info TextLayer
  s_info_layer = text_layer_create(GRect(75, 125, 70, 200));
  text_layer_set_background_color(s_info_layer, GColorWhite);
  text_layer_set_text_color(s_info_layer, GColorBlack);
  text_layer_set_text_alignment(s_info_layer, GTextAlignmentCenter);
  text_layer_set_font(s_info_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_gap_layer_one));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_gap_layer_two));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_gap_layer_three));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_gap_layer_four));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_info_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_gap_layer_one);
  text_layer_destroy(s_gap_layer_two);
  text_layer_destroy(s_gap_layer_three);
  text_layer_destroy(s_gap_layer_four);
  text_layer_destroy(s_day_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_weather_layer);
  
  // Unload GFont
  fonts_unload_custom_font(s_providence_font);
  fonts_unload_custom_font(s_visitor_font);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
