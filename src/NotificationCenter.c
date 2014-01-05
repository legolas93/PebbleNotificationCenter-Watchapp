#include <pebble.h>
#include <pebble_fonts.h>
#include "NotificationsWindow.h"
#include "MainMenu.h"
#include "NotificationList.h"

uint8_t curWindow = 0;
bool gotNotification = false;

InverterLayer *inverter_layer;
uint8_t isNotificationListenerSupported;

uint8_t getCurWindow()
{
	return curWindow;
}

void setCurWindow(uint8_t newWindow)
{
	curWindow = newWindow;
}

void switchWindow(uint8_t newWindow)
{
	switch(newWindow)
	{
	case 0:
		curWindow = 0;
		init_menu_window();
		break;
	case 1:
		curWindow = 1;
		notification_window_init(false);
		break;
	}
}

void received_data(DictionaryIterator *received, void *context) {
	gotNotification = true;

	uint8_t packetId = dict_find(received, 0)->value->uint8;
	if(packetId == 3){
	  //Retrieve isNotificationListenerSupported;
	  isNotificationListenerSupported = dict_find(received, 3)->value->uint8;
	}
 
  if ((packetId == 3) || (packetId == 10))
	{
		//Retrieve options when loading main menu or when they are changed
		options_data_received(received);
	}
 
	if (packetId == 0 && curWindow > 1)
	{
		switchWindow(1);
	}
	
	switch (curWindow)
	{
	case 0:
		menu_data_received(packetId, received);
		break;
	case 1:
		notification_received_data(packetId, received);
		break;
	case 2:
		list_data_received(packetId, received);
		break;
	}
	

	app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
	app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);
}

void data_sent(DictionaryIterator *received, void *context)
{
	switch (curWindow)
	{
	case 1:
		notification_data_sent(received, context);
		break;
	}
}

void timerTriggered(void* context)
{
	if (!gotNotification)
	{
		DictionaryIterator *iterator;
		app_message_outbox_begin(&iterator);
		dict_write_uint8(iterator, 0, 0);
		app_message_outbox_send();

		app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
		app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);

	}
}


int main(void) {
	app_message_register_inbox_received(received_data);
	app_message_register_outbox_sent(data_sent);
	app_message_open(124, 50);

  
	inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));
	switchWindow(0);

	app_timer_register(300, timerTriggered, NULL);

	app_event_loop();

  inverter_layer_destroy(inverter_layer);
	window_stack_pop_all(false);
	return 0;
}
