// @brief High level multimaterial switcher control

#ifndef _MMCTL_H
#define _MMCTL_H

#include <inttypes.h>

extern int active_extruder;
extern bool isFilamentLoaded;
extern const uint8_t finda_limit;

void retract_filament(int extra_steps = 0);
void switch_extruder_withSensor(int new_extruder);
void select_extruder(int new_extruder);
bool feed_filament(bool timeout = false);
void enhanced_interactive_menu();
void load_filament_withSensor(bool disengageIdler);
void load_filament_inPrinter();
void unload_filament_withSensor(bool disengageIdler);
void eject_filament(uint8_t filament);
void recover_after_eject();
void mmctl_cut_filament(uint8_t filament);
bool mmctl_IsOk();

#endif //_MMCTL_H
