#include <pebble.h>

#define NUM_ITEMS 5

static Window *s_main_window;
static TextLayer *s_label_layer;
static ActionBarLayer *s_action_bar;

static GBitmap *s_ellipsis_bitmap;
static GColor s_color, s_visible_color;

static ActionMenu *s_action_menu;
static ActionMenuLevel *s_root_level;

/********************************* ActionMenu *********************************/

static void action_performed_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
#ifdef PBL_COLOR
  // An action was selected, determine which one
  s_color = *(GColor*)action_menu_item_get_action_data(action);
#else
  text_layer_set_text(s_label_layer, (char*)action_menu_item_get_action_data(action));
#endif

  // Update background color
  window_set_background_color(s_main_window, s_color);

  // Update legible foreground color
  s_visible_color = gcolor_legible_over(s_color);
  text_layer_set_text_color(s_label_layer, s_visible_color);
}

static void init_action_menu() {
  // Create the root level
  s_root_level = action_menu_level_create(NUM_ITEMS);

  // Set up the actions for this level, using action context to pass colors
  action_menu_level_add_action(s_root_level, "Red", action_performed_callback,
                               PBL_IF_COLOR_ELSE(&GColorRed, (char*)"Red"));
  action_menu_level_add_action(s_root_level, "Blue", action_performed_callback,
                               PBL_IF_COLOR_ELSE(&GColorBlue, (char*)"Blue"));
  action_menu_level_add_action(s_root_level, "Yellow", action_performed_callback,
                               PBL_IF_COLOR_ELSE(&GColorChromeYellow, (char*)"Yellow"));
  action_menu_level_add_action(s_root_level, "Green", action_performed_callback,
                               PBL_IF_COLOR_ELSE(&GColorGreen, (char*)"Green"));
  action_menu_level_add_action(s_root_level, "White", action_performed_callback,
                               PBL_IF_COLOR_ELSE(&GColorWhite, (char*)"White"));
}

/*********************************** Clicks ***********************************/

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Configure the ActionMenu Window about to be shown
  ActionMenuConfig config = (ActionMenuConfig) {
    .root_level = s_root_level,
    .colors = {
      .background = s_color,
      .foreground = s_visible_color,
    },
    .align = ActionMenuAlignCenter
  };

  // Show the ActionMenu
  s_action_menu = action_menu_open(&config);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

/******************************** Main Window *********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_ellipsis_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ELLIPSIS);

  s_action_bar = action_bar_layer_create();
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
  action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_ellipsis_bitmap);
  action_bar_layer_add_to_window(s_action_bar, window);

  s_label_layer = text_layer_create(GRect(bounds.origin.x, bounds.origin.y, bounds.size.w - ACTION_BAR_WIDTH, bounds.size.h));
  text_layer_set_text(s_label_layer, PBL_IF_COLOR_ELSE("Choose a background color.", "Choose a color."));
  text_layer_set_font(s_label_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_color(s_label_layer, s_visible_color);
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_label_layer));

#if defined(PBL_ROUND)
  // Center vertically
  GSize text_size = text_layer_get_content_size(s_label_layer);
  layer_set_frame(text_layer_get_layer(s_label_layer), GRect(
    bounds.origin.x, (bounds.size.h - text_size.h) / 2, 
    bounds.size.w - ACTION_BAR_WIDTH, bounds.size.h
  ));

  // Enable paging
  text_layer_enable_screen_text_flow_and_paging(s_label_layer, 3);
#endif
}

static void window_unload(Window *window) {
  text_layer_destroy(s_label_layer);
  action_bar_layer_destroy(s_action_bar);
  gbitmap_destroy(s_ellipsis_bitmap);

  action_menu_hierarchy_destroy(s_root_level, NULL, NULL);
}

/************************************ App *************************************/

static void init() {
  s_color = GColorChromeYellow;
  s_visible_color = gcolor_legible_over(s_color);

  s_main_window = window_create();
  window_set_background_color(s_main_window, s_color);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

  init_action_menu();
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
