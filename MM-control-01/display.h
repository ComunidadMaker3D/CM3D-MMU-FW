#include "config.h"


#ifndef DISPLAY_H
#define DISPLAY_H


#ifdef SSD_DISPLAY

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include "fonts/Verdana_custom_24.h"

#define I2C_ADDRESS 0x3C
#define OLED_RESET -1     // if using 5-wire with reset pin, this should be 7

extern const char MSG_IDLE[];
extern const char MSG_PRINTING[];
extern const char MSG_INITIALIZING[];
extern const char MSG_HOMING[];
extern const char MSG_LOADING[];
extern const char MSG_LOADED[];
extern const char MSG_UNLOADING[];
extern const char MSG_EJECTING[];
extern const char MSG_CONTINUING[];
extern const char MSG_RECOVERING[];
extern const char MSG_WAITING[];
extern const char MSG_CUTTING[];
extern const char MSG_SELECTING[];
extern const char MSG_PRIMING[];
extern const char MSG_RETRACTING[];
extern const char MSG_ERROR[];
extern const char MSG_LOADERROR[];
extern const char MSG_UNLOADERROR[];
extern const char MSG_F[];
extern const char MSG_AXIS_PUL[];
extern const char MSG_AXIS_SEL[];
extern const char MSG_AXIS_IDL[];

extern const char OPT_MENU_DECR[];
extern const char OPT_MENU_INCR[];
extern const char OPT_MENU_REHOME[];
extern const char OPT_MENU_OK[];
extern const char OPT_MENU_MAIN[];
extern const char OPT_MENU_PUL[];
extern const char OPT_MENU_SEL[];
extern const char OPT_MENU_IDL[];

enum class COUNTER : uint8_t {LOAD_RETRY, LOAD_FAIL, UNLOAD_RETRY, UNLOAD_FAIL, SUCCESS};

extern void display_init();
extern void display_test();
extern void display_command();
extern void display_command(char c, uint8_t v, boolean force);
extern void display_extruder();
extern void display_extruder(int8_t v);
extern void display_extruder_change(int8_t new_extruder);
extern void display_message(const char* msg);
extern void display_message(const char* msg, int8_t v);
extern void display_message(const char* msg, int8_t v, boolean err);
extern void display_error(const char* msg);
extern void display_error(const char* msg, int8_t v);
extern void display_menu_options(const char* opta, const char* optb, const char* optc);
extern void display_status();
extern void display_count_incr(COUNTER i);

#endif // SSD_DISPLAY
#endif // DISPLAY_H
