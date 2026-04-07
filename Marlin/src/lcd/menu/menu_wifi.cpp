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
// Wi-Fi SSID Input - Simple character-by-character editor
//
static uint8_t ssid_edit_idx = 0;
static char ssid_edit_buf[WIFI_SSID_MAX_LEN + 1];

static void menu_wifi_ssid_edit() {
  if (ui.use_click()) {
    // Confirm current character
    if (ssid_edit_idx < WIFI_SSID_MAX_LEN - 1 && ssid_edit_buf[ssid_edit_idx] != '\0') {
      ssid_edit_idx++;
      ssid_edit_buf[ssid_edit_idx] = '\0';
    } else if (ssid_edit_idx > 0 || (ssid_edit_idx == 0 && ssid_edit_buf[0] != '\0')) {
      // Done editing
      mks_wifi_set_ssid(ssid_edit_buf);
      ssid_edit_idx = 0;
      ui.go_back();
    }
    return;
  }

  START_SCREEN();
  STATIC_ITEM_F(F("Edit SSID"), SS_DEFAULT|SS_INVERT);
  STATIC_ITEM_F(nullptr, SS_CENTER, ssid_edit_buf);
  STATIC_ITEM_F(F("Letters, numbers"), SS_FULL);
  STATIC_ITEM_F(F("- _ allowed"), SS_FULL);

  END_SCREEN();
}

//
// Wi-Fi SSID Input (view/edit submenu)
//
static void menu_wifi_ssid() {
  if (ui.use_click()) return ui.go_back();

  char ssid[WIFI_SSID_MAX_LEN];
  mks_wifi_get_ssid_buffer(ssid, sizeof(ssid));

  START_MENU();
  STATIC_ITEM_F(F("Network Name"), SS_DEFAULT|SS_INVERT);
  
  if (ssid[0] == '\0') {
    STATIC_ITEM_F(F("Not set"), SS_CENTER);
  } else {
    STATIC_ITEM_F(nullptr, SS_FULL, ssid);
  }
  
  STATIC_ITEM_F(nullptr, SS_FULL, "");
  ACTION_ITEM_F(F("Edit"), []() {
    mks_wifi_get_ssid_buffer(ssid_edit_buf, sizeof(ssid_edit_buf));
    ssid_edit_idx = strlen(ssid_edit_buf);
    ui.goto_screen(menu_wifi_ssid_edit);
  });

  END_MENU();
}

//
// Wi-Fi Password Input - Simple editor
//
static uint8_t pass_edit_idx = 0;
static char pass_edit_buf[WIFI_PASS_MAX_LEN + 1];

static void menu_wifi_password_edit() {
  if (ui.use_click()) {
    // Done editing
    if (pass_edit_idx > 0 || pass_edit_buf[0] != '\0') {
      mks_wifi_set_password(pass_edit_buf);
      pass_edit_idx = 0;
      ui.go_back();
    }
    return;
  }

  START_SCREEN();
  STATIC_ITEM_F(F("Edit Password"), SS_DEFAULT|SS_INVERT);
  
  // Show password length
  char len_str[16];
  sprintf_P(len_str, PSTR("Length: %d"), strlen(pass_edit_buf));
  STATIC_ITEM_F(nullptr, SS_CENTER, len_str);
  
  STATIC_ITEM_F(F("Use serial or"), SS_FULL);
  STATIC_ITEM_F(F("external input"), SS_FULL);

  END_SCREEN();
}

//
// Wi-Fi Password Input (view/edit submenu)
//
static void menu_wifi_password() {
  if (ui.use_click()) return ui.go_back();

  char password[WIFI_PASS_MAX_LEN];
  mks_wifi_get_password_buffer(password, sizeof(password));

  START_MENU();
  STATIC_ITEM_F(F("Password"), SS_DEFAULT|SS_INVERT);
  
  // Show password as dots
  uint8_t len = strlen(password);
  char dots[WIFI_PASS_MAX_LEN + 1];
  if (len > 0) {
    for (uint8_t i = 0; i < len; i++) {
      dots[i] = '*';
    }
    dots[len] = '\0';
  } else {
    strcpy_P(dots, PSTR("Not set"));
  }
  STATIC_ITEM_F(nullptr, SS_FULL, dots);
  
  STATIC_ITEM_F(nullptr, SS_FULL, "");
  ACTION_ITEM_F(F("Edit"), []() {
    mks_wifi_get_password_buffer(pass_edit_buf, sizeof(pass_edit_buf));
    pass_edit_idx = strlen(pass_edit_buf);
    ui.goto_screen(menu_wifi_password_edit);
  });

  END_MENU();
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
// Scanning in progress screen
//
static void menu_wifi_scanning() {
  if (ui.use_click()) return ui.go_back();

  uint8_t state = mks_wifi_get_state();
  
  START_SCREEN();
  STATIC_ITEM_F(F("Scanning Networks"), SS_DEFAULT|SS_INVERT);
  
  if (state == WIFI_STATE_SCANNING) {
    STATIC_ITEM_F(F("Scanning..."), SS_CENTER|SS_INVERT);
  } else if (state == WIFI_STATE_SCAN_DONE) {
    STATIC_ITEM_F(F("Done!"), SS_CENTER);
    ui.go_back();
  }
  
  END_SCREEN();
}

//
// Scan results summary screen
//
static void menu_wifi_scan_results() {
  if (ui.use_click()) return ui.go_back();

  char count_str[16];
  uint8_t scan_count = mks_wifi_get_scan_count();
  sprintf_P(count_str, PSTR("Found: %d"), scan_count);

  START_SCREEN();
  STATIC_ITEM_F(F("Scan Results"), SS_DEFAULT|SS_INVERT);
  STATIC_ITEM_F(nullptr, SS_CENTER, count_str);
  
  if (scan_count > 0) {
    STATIC_ITEM_F(F("Use Select Network"), SS_FULL);
    STATIC_ITEM_F(F("to choose one"), SS_FULL);
  } else {
    STATIC_ITEM_F(F("No networks found"), SS_FULL);
  }

  END_SCREEN();
}

//
// Select network from scan results
//
static void menu_wifi_select_from_scan() {
  if (ui.use_click()) return ui.go_back();

  START_MENU();
  STATIC_ITEM_F(F("Select Network"), SS_DEFAULT|SS_INVERT);

  uint8_t scan_count = mks_wifi_get_scan_count();
  if (scan_count == 0) {
    STATIC_ITEM_F(F("No networks found"), SS_CENTER);
  } else {
    // Display each found network - simple list only
    char ssid_str[WIFI_SSID_MAX_LEN];
    for (uint8_t i = 0; i < scan_count; i++) {
      mks_wifi_get_scan_ssid(i, ssid_str, WIFI_SSID_MAX_LEN);
      if (ssid_str[0] != '\0') {
        STATIC_ITEM_F(nullptr, SS_FULL, ssid_str);
      }
    }
  }

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
    
    ACTION_ITEM_F(F("Scan Networks"), []() {
      mks_wifi_request_scan();
      ui.goto_screen(menu_wifi_scanning);
    });
    
    if (mks_wifi_has_scan_results()) {
      SUBMENU_F(F("Scan Results"), menu_wifi_scan_results);
      SUBMENU_F(F("Select Network"), menu_wifi_select_from_scan);
    }
    
    STATIC_ITEM_F(nullptr, SS_FULL, "");
    
    ACTION_ITEM_F(F("Reconnect"), []() {
      mks_wifi_reconnect();
      ui.return_to_status();
    });
  }

  END_MENU();
}

#endif // HAS_MARLINUI_MENU


