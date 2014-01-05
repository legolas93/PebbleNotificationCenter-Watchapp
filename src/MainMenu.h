/*
 * MainMenu.h
 *
 *  Created on: Aug 25, 2013
 *      Author: matej
 */

#ifndef MAINMENU_H_
#define MAINMENU_H_

extern uint8_t vibrateEnabledStatus;
extern uint8_t inverterEnabledStatus;
extern InverterLayer *inverter_layer;
extern uint8_t isNotificationListenerSupported;

void init_menu_window();
void menu_data_received(int packetId, DictionaryIterator* data);
void options_data_received(DictionaryIterator* data);
void close_menu_window();

#endif /* MAINMENU_H_ */
