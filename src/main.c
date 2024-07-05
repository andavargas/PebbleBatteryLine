#include "pebble.h"
  
static Window *main_window;
static TextLayer *date_textlayer;
static Layer *watchbattery_layer;
static TextLayer *time_textlayer;

int pebwidth = 144;
int pebheight = 168;

// Establish layout
int date_textlayer_height = 50;
int time_textlayer_height = 50;
int batteryline = 100;
int datecorrection = 19;
int timecorrection = -60;
int marginrightdate = 7;
int marginlefttime = 7;

// Create a long-lived buffer
static char datetext[] = "0000000000";
static char timetext[] = "00:00";

static void update_time() {
	// Get a tm structure
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);

	// Write date and time into buffer
	strftime(datetext, sizeof("0000000000"), "%a  %m/%d", tick_time);
	if (clock_is_24h_style() == true) {
		strftime(timetext, sizeof("00:00"), "%k:%M", tick_time); // Use 24 hour format
	} else {
		strftime(timetext, sizeof("00:00"), "%l:%M", tick_time); // Use 12 hour format
	}

	// Display this time on the TextLayer
	text_layer_set_text(date_textlayer, datetext);
	text_layer_set_text(time_textlayer, timetext);
}

static void watchbattery_layer_update(Layer *my_layer, GContext *ctx) {
	// Set up context
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_fill_color(ctx, GColorWhite);

	// Calculate parameters and draw
	int battery_state = battery_state_service_peek().charge_percent;
	int chargelinefill = pebwidth * battery_state / 100;
	graphics_draw_rect(ctx, GRect(0,0,chargelinefill,1));
	graphics_draw_rect(ctx, GRect(pebwidth-chargelinefill,1,chargelinefill,1));
}

static void main_window_load(Window *window) {
	// Create the date box
	date_textlayer = text_layer_create(GRect(marginrightdate, batteryline-date_textlayer_height+datecorrection, pebwidth-marginrightdate, date_textlayer_height));
	// Create the watch battery lines
	watchbattery_layer = layer_create(GRect(0, batteryline, pebwidth, 5));
	layer_set_update_proc(watchbattery_layer, watchbattery_layer_update);
	// Create the time box
	time_textlayer = text_layer_create(GRect(0, batteryline+5+time_textlayer_height+timecorrection, pebwidth-marginlefttime, time_textlayer_height));

	// Formatting
	text_layer_set_font(date_textlayer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_font(time_textlayer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	text_layer_set_text_alignment(date_textlayer, GTextAlignmentLeft);
	text_layer_set_text_alignment(time_textlayer, GTextAlignmentRight);
	text_layer_set_background_color(date_textlayer, GColorClear);
	text_layer_set_background_color(time_textlayer, GColorClear);
	text_layer_set_text_color(date_textlayer, GColorWhite);
	text_layer_set_text_color(time_textlayer, GColorWhite);

	// Set up hierarchy
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_textlayer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_textlayer));
	layer_add_child(window_get_root_layer(window), watchbattery_layer);

	// Make sure the time is displayed from the start
	update_time();
}

static void main_window_unload(Window *window) {
	// Destroy layers
	text_layer_destroy(time_textlayer);
	layer_destroy(watchbattery_layer);
	text_layer_destroy(date_textlayer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

static void init() {
	// Create main Window element and assign to pointer
	main_window = window_create();
	window_set_background_color(main_window, GColorBlack);

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});

	// Show the Window on the watch, with animated=true
	window_stack_push(main_window, true);

	// Register with TickTimerService, watch battery
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
	// Destroy Window
	window_destroy(main_window);
	tick_timer_service_unsubscribe();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
