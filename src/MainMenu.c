#include <pebble.h>
#include <pebble_fonts.h>
#include "MainMenu.h"
#include "NotificationCenter.h"
#include "NotificationsWindow.h"
#include "NotificationList.h"

Window* menuWindow;
Layer* topLayer;

SimpleMenuItem mainMenuItems[2] = {};
SimpleMenuItem mainMenuOptionItems[2] = {};
SimpleMenuSection mainMenuSection[2] = {};

GBitmap* currentIcon;
GBitmap* historyIcon;

TextLayer* menuLoadingLayer;

TextLayer* quitTitle;
TextLayer* quitText;

SimpleMenuLayer* menuLayer;

uint8_t vibrateEnabledStatus = 1;
uint8_t inverterEnabledStatus = 0;

void show_loading()
{
	layer_set_hidden((Layer *) menuLoadingLayer, false);
	layer_set_hidden((Layer *) quitTitle, true);
	layer_set_hidden((Layer *) quitText, true);
	if (menuLayer != NULL) layer_set_hidden((Layer *) menuLayer, true);
}

void show_quit()
{
	layer_set_hidden((Layer *) menuLoadingLayer, true);
	layer_set_hidden((Layer *) quitTitle, false);
	layer_set_hidden((Layer *) quitText, false);
}

void menu_picked(int index, void* context)
{
	show_loading();

	DictionaryIterator *iterator;
	app_message_outbox_begin(&iterator);
  if(isNotificationListenerSupported == 1){
	  dict_write_uint8(iterator, 0, 6);
	  dict_write_uint8(iterator, 1, index);
	} else {
	  dict_write_uint8(iterator, 0, 6);
	  dict_write_uint8(iterator, 1, 1);
	}

	app_message_outbox_send();

	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
	app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);
}

void option_picked(int index, void* context)
{
  DictionaryIterator *iterator;
	app_message_outbox_begin(&iterator);	
	
	if(index == 0){
    if (vibrateEnabledStatus == 1){
	    vibrateEnabledStatus = 0;
	  } else {
	    vibrateEnabledStatus = 1;
	  }
	  dict_write_uint8(iterator, 0, 10);
	  dict_write_uint8(iterator, 1, vibrateEnabledStatus);	
	} else if(index == 1){
	  if (inverterEnabledStatus == 1){	  
	    dict_write_uint8(iterator, 0, 11);
	    dict_write_uint8(iterator, 1, 0);	
	  } else {
	    dict_write_uint8(iterator, 0, 11);
	    dict_write_uint8(iterator, 1, 1);	
	  }
	}
	
	app_message_outbox_send();
	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
	app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);
	
}

void show_menu()
{
  mainMenuSection[0].title = "Menu";
	mainMenuSection[0].items = mainMenuItems;	
	
	mainMenuSection[1].title = "Options";
	mainMenuSection[1].items = mainMenuOptionItems;
	mainMenuSection[1].num_items = 2;
	
  if(isNotificationListenerSupported == 1){
    mainMenuSection[0].num_items = 2;
  
	  mainMenuItems[0].title = "Active";
	  mainMenuItems[0].icon = currentIcon;
	  mainMenuItems[0].callback = menu_picked;

	  mainMenuItems[1].title = "History";
	  mainMenuItems[1].icon = historyIcon;
	  mainMenuItems[1].callback = menu_picked;
  } else {
    mainMenuSection[0].num_items = 1;

	  mainMenuItems[0].title = "History";
	  mainMenuItems[0].icon = historyIcon;
	  mainMenuItems[0].callback = menu_picked;
  }
	
	mainMenuOptionItems[0].title = "Vibration";
	if (vibrateEnabledStatus == 1){
	  mainMenuOptionItems[0].subtitle = "Enabled";
	} else {
	  mainMenuOptionItems[0].subtitle = "Disabled";
	}
	mainMenuOptionItems[0].icon = NULL;
	mainMenuOptionItems[0].callback = option_picked;
	
	mainMenuOptionItems[1].title = "Inverted Color";
	if (inverterEnabledStatus == 1){
	  mainMenuOptionItems[1].subtitle = "Enabled";
	} else {
	  mainMenuOptionItems[1].subtitle = "Disabled";
	}
	mainMenuOptionItems[1].icon = NULL;
	mainMenuOptionItems[1].callback = option_picked;

	Layer* topLayer = window_get_root_layer(menuWindow);

	if (menuLayer != NULL) layer_remove_from_parent((Layer *) menuLayer);
	menuLayer = simple_menu_layer_create(GRect(0, 0, 144, 156), menuWindow, mainMenuSection, 2, NULL);
	layer_add_child(topLayer, (Layer *) menuLayer);

	layer_set_hidden((Layer *) menuLoadingLayer, true);
	layer_set_hidden((Layer *) menuLayer, false);
	layer_set_hidden((Layer *) quitTitle, true);
	layer_set_hidden((Layer *) quitText, true);
	
	if(inverterEnabledStatus)
	  layer_add_child(topLayer, inverter_layer_get_layer(inverter_layer));
}



void menu_data_received(int packetId, DictionaryIterator* data)
{
	switch (packetId)
	{
	case 0:
		show_quit();
		notification_window_init(true);
		notification_received_data(packetId, data);
		break;
	case 2:
		window_stack_pop(true);
		init_notification_list_window();
		list_data_received(packetId, data);
		break;
	case 3:
		show_menu();
		break;
	}
}
void options_data_received(DictionaryIterator* data)
{
	vibrateEnabledStatus = dict_find(data, 1)->value->uint8;
	uint8_t oldInverterEnabledStatus = inverterEnabledStatus;
	inverterEnabledStatus = dict_find(data, 2)->value->uint8;
	if (vibrateEnabledStatus == 1){
	  mainMenuOptionItems[0].subtitle = "Enabled";	  
	} else {
	  mainMenuOptionItems[0].subtitle = "Disabled";
	}
	if (inverterEnabledStatus == 1){
	  mainMenuOptionItems[1].subtitle = "Enabled";	  
	} else {
	  mainMenuOptionItems[1].subtitle = "Disabled";
	}
	if(oldInverterEnabledStatus != inverterEnabledStatus){
	  if(inverterEnabledStatus == 1){
	    layer_add_child(topLayer, inverter_layer_get_layer(inverter_layer));	    
	  } else{
	    layer_remove_from_parent(inverter_layer_get_layer(inverter_layer));
	  }
	}
	if(menuLayer != NULL)
	  menu_layer_reload_data((struct MenuLayer *)menuLayer);
}

void window_unload(Window* me)
{
	gbitmap_destroy(currentIcon);
	gbitmap_destroy(historyIcon);

	text_layer_destroy(menuLoadingLayer);
	text_layer_destroy(quitTitle);
	text_layer_destroy(quitText);

	window_destroy(me);
	me = NULL;
}

void window_load(Window *me) {
	currentIcon = gbitmap_create_with_resource(RESOURCE_ID_ICON);
	historyIcon = gbitmap_create_with_resource(RESOURCE_ID_RECENT);

	topLayer = window_get_root_layer(menuWindow);

	menuLoadingLayer = text_layer_create(GRect(0, 10, 144, 168 - 16));
	text_layer_set_text_alignment(menuLoadingLayer, GTextAlignmentCenter);
	text_layer_set_text(menuLoadingLayer, "Loading...");
	text_layer_set_font(menuLoadingLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	layer_add_child(topLayer, (Layer*) menuLoadingLayer);

	quitTitle = text_layer_create(GRect(0, 70, 144, 50));
	text_layer_set_text_alignment(quitTitle, GTextAlignmentCenter);
	text_layer_set_text(quitTitle, "Press back again if app does not close in several seconds");
	layer_add_child(topLayer, (Layer*) quitTitle);

	quitText = text_layer_create(GRect(0, 10, 144, 50));
	text_layer_set_text_alignment(quitText, GTextAlignmentCenter);
	text_layer_set_text(quitText, "Quitting...\n Please wait");
	text_layer_set_font(quitText, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	layer_add_child(topLayer, (Layer*) quitText);
	
	setCurWindow(0);
}

void close_menu_window()
{
	if (menuWindow != NULL)
		window_stack_remove(menuWindow, false);
}


void init_menu_window()
{
	menuWindow = window_create();
	
	window_set_window_handlers(menuWindow, (WindowHandlers){
		.appear = window_load,
		.unload = window_unload
	});

	window_stack_push(menuWindow, true /* Animated */);

	show_loading();
}

