/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

//
// Wi-Fi Configuration Menu
//

#include "../../inc/MarlinConfigPre.h"

#if HAS_MARLINUI_MENU

#include "menu_item.h"
#include "../../module/mks_wifi/mks_wifi.h"

//
// Wi-Fi Status Screen
//
static void menu_wifi_status() {
  if (ui.use_click()) return ui.go_back();

  char buffer[32];

  START_SCREEN();
  
  STATIC_ITEM_F(F("Wi-Fi Status"), SS_DEFAULT|SS_INVERT);

  // Connection status
  if (mks_wifi_is_connected()) {
    PSTRING_ITEM(MSG_WIFI_CONNECTED, GET_TEXT(MSG_YES), SS_FULL);
  } else {
    PSTRING_ITEM(MSG_WIFI_CONNECTED, GET_TEXT(MSG_NO), SS_FULL);
  }

  // Current Network
  mks_wifi_get_current_ssid(buffer, sizeof(buffer));
  if (buffer[0] != '\0') {
    PSTRING_ITEM(MSG_WIFI_NETWORK, buffer, SS_FULL);
  }

  // IP Address
  if (mks_wifi_has_ip()) {
    mks_wifi_get_ip_string(buffer, sizeof(buffer));
    PSTRING_ITEM(MSG_WIFI_ADDRESS, buffer, SS_FULL);
  }

  // Wi-Fi Mode
  uint8_t mode = mks_wifi_get_mode();
  if (mode == WIFI_MODE_STA) {
    PSTRING_ITEM(MSG_WIFI_MODE, "STA", SS_FULL);
  } else if (mode == WIFI_MODE_AP) {
    PSTRING_ITEM(MSG_WIFI_MODE, "AP", SS_FULL);
  } else {
    PSTRING_ITEM(MSG_WIFI_MODE, "Not set", SS_FULL);
  }

  END_SCREEN();
}

//
// Wi-Fi SSID Input (simplified for Phase A - just display)
//
static void menu_wifi_ssid() {
  if (ui.use_click()) return ui.go_back();

  char ssid[WIFI_SSID_MAX_LEN];
  mks_wifi_get_ssid_buffer(ssid, sizeof(ssid));

  START_SCREEN();
  STATIC_ITEM_F(F("Network Name"), SS_DEFAULT|SS_INVERT);
  if (ssid[0] == '\0') {
    STATIC_ITEM_F(F("Not set"), SS_FULL);
  } else {
    STATIC_ITEM_F(nullptr, SS_FULL, ssid);
  }
  END_SCREEN();
}

//
// Wi-Fi Password Input
//
static void menu_wifi_password() {
  if (ui.use_click()) return ui.go_back();

  char password[WIFI_PASS_MAX_LEN];
  mks_wifi_get_password_buffer(password, sizeof(password));

  // Hide actual password, show dots
  uint8_t len = strlen(password);
  char dots[WIFI_PASS_MAX_LEN];
  if (len > 0) {
    for (uint8_t i = 0; i < len; i++) {
      dots[i] = '*';
    }
    dots[len] = '\0';
  } else {
    strcpy_P(dots, PSTR("Not set"));
  }

  START_SCREEN();
  STATIC_ITEM_F(F("Password"), SS_DEFAULT|SS_INVERT);
  STATIC_ITEM_F(nullptr, SS_FULL, dots);
  END_SCREEN();
}

//
// Manual Credential Entry Submenu
//
static void menu_wifi_manual_entry() {
  if (ui.use_click()) return ui.go_back();

  START_MENU();
  STATIC_ITEM_F(F("Wi-Fi Setup"), SS_DEFAULT|SS_INVERT);
  
  SUBMENU_F(F("Network"), menu_wifi_ssid);
  SUBMENU_F(F("Password"), menu_wifi_password);
  
  STATIC_ITEM_F(nullptr, SS_FULL, "");
  ACTION_ITEM_F(F("Connect"), []() {
    mks_wifi_connect();
    ui.return_to_status();
  });

  END_MENU();
}

//
// Main Wi-Fi Menu
//
void menu_wifi() {
  const bool busy = printer_busy();

  if (ui.use_click()) return ui.go_back();

  START_MENU();
  STATIC_ITEM_F(F("Wi-Fi"), SS_DEFAULT|SS_INVERT);
  
  SUBMENU_F(F("Status"), menu_wifi_status);
  
  if (!busy) {
    SUBMENU_F(F("Setup"), menu_wifi_manual_entry);
    
    STATIC_ITEM_F(nullptr, SS_FULL, "");
    
    ACTION_ITEM_F(F("Reconnect"), []() {
      mks_wifi_reconnect();
      ui.return_to_status();
    });
  }

  END_MENU();
}

#endif // HAS_MARLINUI_MENU


