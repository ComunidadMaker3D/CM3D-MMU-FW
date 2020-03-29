// @file
// @brief Low level stepper routines

#ifndef STEPPER_H
#define STEPPER_H

#include "config.h"
#include <inttypes.h>

extern int8_t filament_type[EXTRUDERS];

void home();
bool home_idler();

int get_pulley_steps(float mm);
int get_pulley_acceleration_steps(int16_t delay_start, int16_t delay_end);
int get_idler_steps(int current_filament, int next_filament);
int get_selector_steps(int current_filament, int next_filament);

void park_idler(bool _unpark);

void do_pulley_step();
void set_pulley_dir_pull();
void set_pulley_dir_push();
void move(int _idler, int _selector, int _pulley);

#endif //STEPPER_H
