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
    move_proportional(idler_steps, 0);
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
    const uint8_t tries = 2;
    for (uint8_t i = 0; i <= tries; ++i)
    {
        int idler_steps = get_idler_steps(s_idler, idler);
        int selector_steps = get_selector_steps(s_selector, selector);

        move_proportional(idler_steps, selector_steps);
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
    int delay = 2000*PULLEY_SPEED_ADJ; //microstep period in microseconds

    uint8_t _endstop_hit = 0;

    int _unloadSteps = BowdenLength::get()/PULLEY_SPEED_ADJ + 250;    //~4750
    const int _first_point = (float)_unloadSteps*(float)0.2;
    const int _second_point = (float)_unloadSteps*(float)0.85;

    set_pulley_dir_pull();

    while (_endstop_hit < 100u && _unloadSteps > 0)
    {
        do_pulley_step();
        _unloadSteps--;

        if (_unloadSteps < 1400/PULLEY_SPEED_ADJ && delay < 6000/PULLEY_SPEED_ADJ) delay += 3*PULLEY_SPEED_ADJ;
        if (_unloadSteps < _first_point && delay < 2500/PULLEY_SPEED_ADJ) delay += 2*PULLEY_SPEED_ADJ;
        if (_unloadSteps < _second_point && _unloadSteps > 5000/PULLEY_SPEED_ADJ)
        {
            if (delay > 550/PULLEY_SPEED_ADJ) delay -= 1*PULLEY_SPEED_ADJ;
            if (delay > 330/PULLEY_SPEED_ADJ && (NORMAL_MODE == tmc2130_mode)) delay -= 1*PULLEY_SPEED_ADJ;
        }

        delayMicroseconds(delay);
        if (digitalRead(A1) == 0) _endstop_hit++;

    }
}

void motion_feed_to_bondtech()
{
    int stepPeriod = 5500*PULLEY_SPEED_ADJ; //inital microstep period in microseconds
    uint16_t steps = BowdenLength::get()/PULLEY_SPEED_ADJ;

    const uint8_t tries = 2;
    for (uint8_t tr = 0; tr <= tries; ++tr)
    {
        set_pulley_dir_push();
        unsigned long delay = 5500*PULLEY_SPEED_ADJ;

        for (uint16_t i = 0; i < steps; i++)
        {
            delayMicroseconds(delay);
            unsigned long now = micros();

            if (i < 4000/PULLEY_SPEED_ADJ)
            {
                if (stepPeriod > 2600/PULLEY_SPEED_ADJ) stepPeriod -= 4*PULLEY_SPEED_ADJ;
                if (stepPeriod > 1300/PULLEY_SPEED_ADJ) stepPeriod -= 2*PULLEY_SPEED_ADJ;
                if (stepPeriod > 650/PULLEY_SPEED_ADJ) stepPeriod -= 1*PULLEY_SPEED_ADJ;
                if (stepPeriod > 350/PULLEY_SPEED_ADJ && (NORMAL_MODE == tmc2130_mode) && s_has_door_sensor) stepPeriod -= 1*PULLEY_SPEED_ADJ;
            }
            if (i > (steps - 800/PULLEY_SPEED_ADJ) && stepPeriod < 2600/PULLEY_SPEED_ADJ) stepPeriod += 10*PULLEY_SPEED_ADJ;
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
    int idler_steps = get_idler_steps(0, idler);
    move_proportional(idler_steps, 0);
    s_idler = idler;
}
