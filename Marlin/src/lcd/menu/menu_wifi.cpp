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
// Wi-Fi Password Input - Keyboard editor
//
static uint8_t pass_edit_idx = 0;
static char pass_edit_buf[WIFI_PASS_MAX_LEN + 1];
static uint8_t pass_kbd_pos = 0;  // Position in keyboard (0-59)

// Keyboard layout: 10 columns x 5 rows = 50 chars + 4 actions
static const char pass_keyboard[] = "abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-";
const uint8_t pass_kbd_size = 50;  // 50 characters exactly (10x5 grid)
// Character positions: 0-49
// Action positions: 50=Bksp, 51=Clr, 52=OK, 53=X

static void menu_wifi_password_edit() {
  // Handle character selection and actions
  if (ui.use_click()) {
    uint8_t pos = pass_kbd_pos;
    
    // Regular characters (0-49)
    if (pos < pass_kbd_size) {
      // Add character to buffer
      uint8_t buf_len = strlen(pass_edit_buf);
      if (buf_len < WIFI_PASS_MAX_LEN - 1) {
        pass_edit_buf[buf_len] = pass_keyboard[pos];
        pass_edit_buf[buf_len + 1] = '\0';
      }
    }
    // Position 50: Backspace
    else if (pos == 50) {
      uint8_t buf_len = strlen(pass_edit_buf);
      if (buf_len > 0) {
        pass_edit_buf[buf_len - 1] = '\0';
      }
    }
    // Position 51: Clear all
    else if (pos == 51) {
      pass_edit_buf[0] = '\0';
    }
    // Position 52: Confirm
    else if (pos == 52) {
      mks_wifi_set_password(pass_edit_buf);
      pass_kbd_pos = 0;
      ui.go_back();
      return;
    }
    // Position 53: Cancel
    else if (pos == 53) {
      mks_wifi_get_password_buffer(pass_edit_buf, sizeof(pass_edit_buf));
      pass_kbd_pos = 0;
      ui.go_back();
      return;
    }
  }

  // Handle encoder navigation
  if (ui.encoderPosition) {
    int8_t diff = ui.encoderPosition;
    pass_kbd_pos = constrain(pass_kbd_pos + diff, 0, 53);
    ui.encoderPosition = 0;
  }

  START_SCREEN();
  STATIC_ITEM_F(F("Edit Password"), SS_DEFAULT|SS_INVERT);
  
  // Show current password (as dots)
  char display_buf[WIFI_PASS_MAX_LEN + 1];
  uint8_t pwd_len = strlen(pass_edit_buf);
  if (pwd_len > 0) {
    for (uint8_t i = 0; i < pwd_len; i++) {
      display_buf[i] = '*';
    }
    display_buf[pwd_len] = '\0';
  } else {
    strcpy_P(display_buf, PSTR("(empty)"));
  }
  STATIC_ITEM_F(nullptr, SS_CENTER, display_buf);

  // Show keyboard with selection highlight
  STATIC_ITEM_F(nullptr, SS_FULL, "");
  
  // Keyboard rows (10 chars each)
  for (uint8_t row = 0; row < 5; row++) {
    char kbd_line[32];
    kbd_line[0] = '\0';
    
    for (uint8_t col = 0; col < 10; col++) {
      uint8_t idx = row * 10 + col;
      if (idx >= pass_kbd_size) break;
      
      char ch = pass_keyboard[idx];
      uint8_t len = strlen(kbd_line);
      
      if (pass_kbd_pos == idx) {
        kbd_line[len] = '[';
        kbd_line[len + 1] = ch;
        kbd_line[len + 2] = ']';
        kbd_line[len + 3] = ' ';
        kbd_line[len + 4] = '\0';
      } else {
        kbd_line[len] = ch;
        kbd_line[len + 1] = ' ';
        kbd_line[len + 2] = '\0';
      }
    }
    
    STATIC_ITEM_F(nullptr, SS_FULL, kbd_line);
  }

  // Action buttons row
  STATIC_ITEM_F(nullptr, SS_FULL, "");
  char action_line[64];
  strcpy_P(action_line, PSTR(""));
  
  const char *actions[] = { "Bksp", "Clr", "OK", "X" };
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t pos = 50 + i;
    uint8_t len = strlen(action_line);
    
    if (pass_kbd_pos == pos) {
      action_line[len] = '[';
      strcat(action_line, actions[i]);
      strcat(action_line, "] ");
    } else {
      strcat(action_line, actions[i]);
      strcat(action_line, " ");
    }
  }
  
  STATIC_ITEM_F(nullptr, SS_FULL, action_line);

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
    pass_kbd_pos = 0;  // Start at 'a' in keyboard
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
// Select network from scan results - Interactive with scrolling
//
static uint8_t wifi_selected_network = 0;

static void menu_wifi_select_from_scan() {
  // Handle selection
  if (ui.use_click()) {
    uint8_t scan_count = mks_wifi_get_scan_count();
    if (wifi_selected_network < scan_count) {
      char ssid_selected[WIFI_SSID_MAX_LEN];
      mks_wifi_get_scan_ssid(wifi_selected_network, ssid_selected, WIFI_SSID_MAX_LEN);
      mks_wifi_set_ssid(ssid_selected);
      
      // Show confirmation and return
      ui.go_back();
      ui.go_back();
      return;
    }
  }

  // Navigate with encoders
  uint8_t scan_count = mks_wifi_get_scan_count();
  if (scan_count > 0) {
    // Encoder navigation
    if (ui.encoderPosition) {
      int8_t diff = ui.encoderPosition;
      wifi_selected_network = constrain(wifi_selected_network + diff, 0, scan_count - 1);
      ui.encoderPosition = 0;
    }
  }

  START_MENU();
  STATIC_ITEM_F(F("Select Network"), SS_DEFAULT|SS_INVERT);

  if (scan_count == 0) {
    STATIC_ITEM_F(F("No networks found"), SS_CENTER);
  } else {
    // Display found networks with selection indicator
    uint8_t start_idx = 0;
    if (wifi_selected_network > 2) {
      start_idx = wifi_selected_network - 2;
    }
    uint8_t end_idx = _MIN(start_idx + 5, scan_count);
    
    char ssid_display[WIFI_SSID_MAX_LEN + 4];
    for (uint8_t i = start_idx; i < end_idx; i++) {
      mks_wifi_get_scan_ssid(i, ssid_display, WIFI_SSID_MAX_LEN);
      int8_t rssi = mks_wifi_get_scan_rssi(i);
      
      // Add signal strength indicator
      const char *signal = "";
      if (rssi > -30) signal = " [***]";
      else if (rssi > -60) signal = " [** ]";
      else if (rssi > -80) signal = " [*  ]";
      else signal = " [   ]";
      
      strcat(ssid_display, signal);
      
      if (i == wifi_selected_network) {
        char selected_str[WIFI_SSID_MAX_LEN + 6];
        strcpy_P(selected_str, PSTR("> "));
        strcat(selected_str, ssid_display);
        STATIC_ITEM_F(nullptr, SS_INVERT|SS_FULL, selected_str);
      } else {
        char unselected_str[WIFI_SSID_MAX_LEN + 6];
        strcpy_P(unselected_str, PSTR("  "));
        strcat(unselected_str, ssid_display);
        STATIC_ITEM_F(nullptr, SS_FULL, unselected_str);
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


