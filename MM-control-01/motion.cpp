//!
//! @file
//! @author Marek Bel

#include "motion.h"
#include "stepper.h"
#include "permanent_storage.h"
#include <Arduino.h>
#include "main.h"
#include "config.h"
#include "tmc2130.h"
#include "shr16.h"
#include "mmctl.h"
#include "display.h"

static uint8_t s_idler = 0;
static uint8_t s_selector = 0;
static bool s_selector_homed = false;
static bool s_idler_engaged = true;
static bool s_has_door_sensor = false;

void rehome()
{
    s_idler = 0;
    s_selector = 0;
    shr16_set_ena(0);
    delay(10);
    shr16_set_ena(7);
    tmc2130_init(tmc2130_mode);
    home();
    if (s_idler_engaged) park_idler(true);
}

static void rehome_idler()
{
    shr16_set_ena(0);
    delay(10);
    shr16_set_ena(7);
    tmc2130_init(tmc2130_mode);
    home_idler();
    int idler_steps = get_idler_steps(0, s_idler);
    move(idler_steps, 0, 0);
    if (s_idler_engaged) park_idler(true);
}

void motion_set_idler_selector(uint8_t idler_selector)
{
    motion_set_idler_selector(idler_selector, idler_selector);
}

//! @brief move idler and selector to desired location
//!
//! In case of drive error re-home and try to recover 3 times.
//! If the drive error is permanent call unrecoverable_error();
//!
//! @param idler idler
//! @param selector selector
void motion_set_idler_selector(uint8_t idler, uint8_t selector)
{
    if (!s_selector_homed)
    {
            home();
            s_selector = 0;
            s_idler = 0;
            s_selector_homed = true;
    }
#ifdef SSD_DISPLAY
    display_message(MSG_SELECTING);
    display_extruder(-1);
#endif
    
    const uint8_t tries = 2;
    for (uint8_t i = 0; i <= tries; ++i)
    {
        int idler_steps = get_idler_steps(s_idler, idler);
        int selector_steps = get_selector_steps(s_selector, selector);

        move(idler_steps, selector_steps, 0);
        s_idler = idler;
        s_selector = selector;

        if (!tmc2130_read_gstat()) break;
        else
        {
            if (tries == i) unrecoverable_error();
            drive_error();
            rehome();
        }
    }
#ifdef SSD_DISPLAY
    display_message(MSG_IDLE);
    display_extruder();
#endif
}

static void check_idler_drive_error()
{
    const uint8_t tries = 2;
    for (uint8_t i = 0; i <= tries; ++i)
    {
        if (!tmc2130_read_gstat()) break;
        else
        {
            if (tries == i) unrecoverable_error();
            drive_error();
            rehome_idler();
        }
    }
}

void motion_engage_idler()
{
    s_idler_engaged = true;
    park_idler(true);
    check_idler_drive_error();
}

void motion_disengage_idler()
{
    s_idler_engaged = false;
    park_idler(false);
    check_idler_drive_error();
}

//! @brief unload until FINDA senses end of the filament
static void unload_to_finda()
{
#ifdef SSD_DISPLAY
    display_message(MSG_UNLOADING);
#endif
    uint16_t steps = get_pulley_steps(FILAMENT_BOWDEN_MM);
    uint16_t steps_acc = get_pulley_acceleration_steps(PULLEY_DELAY_PRIME, PULLEY_DELAY_UNLOAD);
    uint16_t steps_dec = get_pulley_acceleration_steps(PULLEY_DELAY_UNLOAD, PULLEY_DELAY_PRIME);
    uint16_t steps_extra = get_pulley_steps(15);
    uint8_t _endstop_hit = 0;
    
    set_pulley_dir_pull();
    uint16_t delay = PULLEY_DELAY_PRIME;
    uint16_t stepPeriod = PULLEY_DELAY_PRIME;
    uint16_t _steps = steps + steps_extra;

    while (_endstop_hit < finda_limit && _steps > 0)
    {
        delayMicroseconds(delay);
        unsigned long now = micros();
        
        do_pulley_step();

        if (_steps > steps-steps_acc  &&  stepPeriod > PULLEY_DELAY_UNLOAD)  { stepPeriod = (float)stepPeriod * PULLEY_ACCELERATION_X; }
        if (_steps < steps_dec+steps_extra  &&  stepPeriod < PULLEY_DELAY_PRIME)  { stepPeriod = (float)stepPeriod / PULLEY_ACCELERATION_X; }

        if (digitalRead(A1) == 0) _endstop_hit++;
        delay = stepPeriod - (micros() - now);
        _steps--;
    }
}

void motion_feed_to_bondtech()
{
#ifdef SSD_DISPLAY
    display_message(MSG_LOADING);
#endif
    uint16_t steps = get_pulley_steps(FILAMENT_BOWDEN_MM);
    uint16_t steps_acc = get_pulley_acceleration_steps(PULLEY_DELAY_PRIME, PULLEY_DELAY_LOAD);
    uint16_t steps_dec = get_pulley_acceleration_steps(PULLEY_DELAY_LOAD, PULLEY_DELAY_EXTRUDER);
    uint16_t steps_extra = get_pulley_steps(10);
    
    const uint8_t tries = 2;
    for (uint8_t tr = 0; tr <= tries; ++tr)
    {
#ifdef SSD_DISPLAY
        if (tr > 0) {
          display_count_incr(COUNTER::LOAD_RETRY);
          display_error(MSG_LOADING, tr);
        }
#endif
        set_pulley_dir_push();
        uint16_t delay = PULLEY_DELAY_PRIME;
        uint16_t stepPeriod = PULLEY_DELAY_PRIME;

        for (uint16_t i = 0; i < steps+steps_extra; i++)
        {
            delayMicroseconds(delay);
            unsigned long now = micros();

            if (i < steps_acc  &&  stepPeriod > PULLEY_DELAY_LOAD)  { stepPeriod = (float)stepPeriod * PULLEY_ACCELERATION_X; }
            if (i > steps-steps_dec-steps_extra  &&  stepPeriod < PULLEY_DELAY_EXTRUDER)  { stepPeriod = (float)stepPeriod / PULLEY_ACCELERATION_X; }

           if ('A' == getc(uart_com))
            {
                s_has_door_sensor = true;
                tmc2130_disable_axis(AX_PUL, tmc2130_mode);
                motion_disengage_idler();
                return;
            }
            do_pulley_step();
            delay = stepPeriod - (micros() - now);
        }

        if (!tmc2130_read_gstat()) break;
        else
        {
            if (tries == tr) unrecoverable_error();
            drive_error();
            rehome_idler();
            unload_to_finda();
        }
    }
    
#ifdef SSD_DISPLAY
    display_error(MSG_LOADING);
    display_count_incr(COUNTER::LOAD_FAIL);
#endif
}




//! @brief unload to FINDA
//!
//! Check for drive error and try to recover 3 times.
void motion_unload_to_finda()
{
    const uint8_t tries = 2;
    for (uint8_t tr = 0; tr <= tries; ++tr)
    {
        unload_to_finda();
        if (tmc2130_read_gstat() && digitalRead(A1) == 1)
        {
            if (tries == tr) unrecoverable_error();
            drive_error();
            rehome_idler();
        }
        else
        {
            break;
        }
    }

}

void motion_door_sensor_detected()
{
    s_has_door_sensor = true;
}

void motion_set_idler(uint8_t idler)
{
    home_idler();
#ifdef SSD_DISPLAY
    display_message(MSG_SELECTING);
#endif
    int idler_steps = get_idler_steps(0, idler);
    move(idler_steps, 0, 0);
    s_idler = idler;
    
    active_extruder = idler;
#ifdef SSD_DISPLAY
    display_message(MSG_IDLE);
    display_extruder();
#endif
}
