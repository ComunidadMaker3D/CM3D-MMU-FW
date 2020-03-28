#include "display.h"
#include "mmctl.h"


#ifdef SSD_DISPLAY
  SSD1306AsciiWire oled;
  
  const char MSG_IDLE[] = "Idle";
  const char MSG_INITIALIZING[] = "Initializing";
  const char MSG_HOMING[] = "Homing";
  const char MSG_LOADING[] = "Loading";
  const char MSG_LOADED[] = "Loaded";
  const char MSG_UNLOADING[] = "Unloading";
  const char MSG_EJECTING[] = "Ejecting";
  const char MSG_CONTINUING[] = "Continuing";
  const char MSG_RECOVERING[] = "Recovering";
  const char MSG_WAITING[] = "Waiting";
  const char MSG_CUTTING[] = "Cutting";
  const char MSG_SELECTING[] = "Selecting";
  const char MSG_PRIMING[] = "Priming";
  const char MSG_RETRACTING[] = "Retracting";
  const char MSG_ERROR[] = "ERROR";
  const char MSG_LOADERROR[] = "Load Fail";
  const char MSG_UNLOADERROR[] = "Unload Fail";
  const char MSG_F[] = "F";
  
  uint16_t current_display_cmd = 0;
  uint16_t current_display_counts[5] = {0,0,0,0,0};
  boolean current_display_error = false;
  boolean display_transition = false;
  
  
  void display_init() {
    //sdd1306 display
    Wire.begin();
    Wire.setClock(400000L);
    oled.begin(&Adafruit128x64, I2C_ADDRESS, OLED_RESET);
    oled.clear();
  
    display_test();
  }
  
  
  void display_test() {
    oled.clear();
  
    display_message(MSG_INITIALIZING);
    display_command('X', 0);
    display_extruder(-1);
    display_status();
  }
  
  void display_error(char *msg) {
    display_message(msg, -1, true);
  }
  
  void display_error(char *msg, int8_t v) {
    display_message(msg, v, true);
  }
  
  void display_message(char *msg) {
    display_message(msg, -1, false);
  }
  
  void display_message(char *msg, int8_t v) {
    display_message(msg, v, false);
  }
  
  void display_message(char *msg, int8_t v, boolean err) {
    char text[3];
    
    if (current_display_error != err) {
      current_display_error = err;
      display_command();
    }
    
    oled.setFont(Arial_bold_14);
    oled.setInvertMode(current_display_error);
    oled.setCursor(0, 0);
    oled.write("                    ");
    oled.setCursor(2, 0);
    oled.write(msg);
  
    if (v >= 0) {
      sprintf(text, "%d", v);
      oled.setCursor(oled.col()+4, 0);
      oled.write("(");
      oled.setCursor(oled.col(), 0);
      oled.write(text);
      oled.setCursor(oled.col(), 0);
      oled.write(")");
    }
    
    oled.setInvertMode(false);
  }
  
  void display_command() {
    display_command(current_display_cmd>>8, current_display_cmd&0xFF, true);
  }
  
  void display_command(char c, uint8_t v, boolean force=false) {
    uint16_t command = (c<<8) | v;
    
    if (command != current_display_cmd  ||  force) {
      current_display_cmd = command;
      char text[4];
      sprintf(text, "%c%d", c, v);
  
      oled.setFont(Arial_bold_14);
      oled.setInvertMode(current_display_error);
      oled.setCursor(127-oled.strWidth("     ")-2, 0);
      oled.write("      ");
      oled.setCursor(127-oled.strWidth(text)-1, 0);
      oled.write(text);
      oled.setInvertMode(false);
    }
  }
  
  
  void display_extruder() {
    display_extruder(active_extruder+1);
  }
  
  void display_extruder(int8_t v) {
    if (display_transition) {
      return;
    }
    
    char text[3];
    if (v >= 0) {
      sprintf(text, "%d", v);
    } else {
      sprintf(text, "=");
    }
    //Serial.println(text);
    
    oled.setFont(Verdana_digits_24);
    oled.setCursor(0, 3);
    oled.write(";;;;;;;;;;;;;;;;");
    uint8_t x = (127-oled.strWidth(text)-11) / 2;
    
    oled.setCursor(x, 4);
    oled.setFont(Arial_bold_14);
    oled.write("F");
    
    oled.setCursor(oled.col()+2, 3);
    oled.setFont(Verdana_digits_24);
    oled.write(text);
  }
  
  
  void display_extruder_change(int8_t new_extruder) {
    if (new_extruder < 0) {
      display_transition = false;
      display_extruder();
      return;
    }
    
    char text_old[3];
    char text_new[3];
    sprintf(text_old, "%d", active_extruder+1);
    sprintf(text_new, "%d", new_extruder+1);
    
    oled.setFont(Verdana_digits_24);
    oled.setCursor(0, 3);
    oled.write(";;;;;;;;;;;;;;;;");
    uint8_t x = (127-oled.strWidth(text_old)-oled.strWidth(text_new)-11-11-12-8) / 2;
  
    oled.setCursor(x, 4);
    oled.setFont(Arial_bold_14);
    oled.write(MSG_F);
    oled.setCursor(oled.col()+2, 3);
    oled.setFont(Verdana_digits_24);
    oled.write(text_old);
    oled.setCursor(oled.col()+6, oled.row());
    oled.write(">");
    oled.setCursor(oled.col()+6, 4);
    oled.setFont(Arial_bold_14);
    oled.write(MSG_F);
    oled.setCursor(oled.col()+2, 3);
    oled.setFont(Verdana_digits_24);
    oled.write(text_new);
  
    display_transition = true;
  }
  
  
  void display_status() {
    char text[22];
    sprintf(text, "L:%d/%d U:%d/%d S:%d", current_display_counts[0], current_display_counts[1], current_display_counts[2], current_display_counts[3], current_display_counts[4]);
    
    oled.setFont(Adafruit5x7);
    uint8_t x = (127-oled.strWidth(text)) / 2;
    
    oled.setCursor(x, 7);
    oled.write(text);
  }
  
  
  void display_count_incr(COUNTER i) {
    current_display_counts[(uint8_t)i]++;
    display_status();
  }

#endif
