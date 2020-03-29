#include "stepper.h"
#include "shr16.h"
#include "tmc2130.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <Arduino.h>
#include "main.h"
#include "mmctl.h"
#include "Buttons.h"
#include "permanent_storage.h"
#include "pins.h"
#include "tmc2130.h"
#include "display.h"

int8_t filament_type[EXTRUDERS];

static bool isIdlerParked = false;

static const int selector_steps_after_homing = -4250;
static const int idler_steps_after_homing = -33;  // need <45 for 12x25.2

static const int selector_steps = 303;        // 50 steps/mm
static const int idler_steps = 224;           // 25.2° / .1125
static const int idler_parking_steps = 280;   // idler*1.5

static int set_idler_direction(int _steps);
static int set_selector_direction(int _steps);
static int set_pulley_direction(int _steps);
static void set_idler_dir_down();
static void set_idler_dir_up();
static void move(int _idler, int _selector, int _pulley);


//! @brief Compute delay based on mm/s
//! @param mms mm/s feedrate for pulley
//! @return delay in ms
int get_pulley_delay(float mms)
{
  return floor((float)1000000 / (mms*(float)PULLEY_STEPS_PER_MM));
}

//! @brief Compute pulley steps based on distance
//! @param mm total length of movement of filament
//! @return steps
int get_pulley_steps(float mm) {
  return (float)PULLEY_STEPS_PER_MM * mm;
}

//! @brief Compute steps for selector needed to change filament
//! @param current_filament Currently selected filament
//! @param next_filament Filament to be selected
//! @return selector steps
int get_selector_steps(int current_filament, int next_filament)
{
    return (((current_filament - next_filament) * selector_steps) * -1);
}

//! @brief Compute number of steps to reach full speed
//! @param delay_start Initial delay
//! @param delay_end Final delay
//! @return Steps
int get_pulley_acceleration_steps(int16_t delay_start, int16_t delay_end) {
  float delay_diff = (float)min(delay_start, delay_end) / (float)max(delay_start, delay_end);
  return ceil( log(delay_diff) / log(PULLEY_ACCELERATION_X) );
}


//! @brief Compute steps for idler needed to change filament
//! @param current_filament Currently selected filament
//! @param next_filament Filament to be selected
//! @return idler steps
int get_idler_steps(int current_filament, int next_filament)
{
    return ((current_filament - next_filament) * idler_steps);
}

void do_pulley_step()
{
    pulley_step_pin_set();
	asm("nop");
	pulley_step_pin_reset();
	asm("nop");
}




//! @brief home idler
//!
//! @param toLastFilament
//!   - true
//! Move idler to previously loaded filament and disengage. Returns true.
//! Does nothing if last filament used is not known and returns false.
//!   - false (default)
//! Move idler to filament 0 and disengage. Returns true.
//!
//! @retval true Succeeded
//! @retval false Failed
bool home_idler()
{
#ifdef SSD_DISPLAY
  display_message(MSG_HOMING);
#endif
	int _c = 0;
	int _l = 0;

	tmc2130_init(HOMING_MODE);

	move(-10, 0, 0); // move a bit in opposite direction

	for (int c = 1; c > 0; c--)  // not really functional, let's do it rather more times to be sure
	{
		delay(100);
		for (int i = 0; i < 3000; i++)
		{
			move(1, 0,0);
			delayMicroseconds(100);
			tmc2130_read_sg(0);

			_c++;
			if (i == 1000) { _l++; }
			if (_c > 100) { shr16_set_led(1 << 2 * _l); };
			if (_c > 200) { shr16_set_led(0x000); _c = 0; };
		}
	}

	move(idler_steps_after_homing, 0, 0); // move to initial position

	tmc2130_init(tmc2130_mode);

	delay(500);

    isIdlerParked = false;

	park_idler(false);

	return true;
}

bool home_selector()
{
#ifdef SSD_DISPLAY
    display_message(MSG_HOMING);
#endif
    
    // if FINDA is sensing filament do not home
    check_filament_not_present();

    tmc2130_init(HOMING_MODE);

	int _c = 0;
	int _l = 2;

	for (int c = 7; c > 0; c--)   // not really functional, let's do it rather more times to be sure
	{
		move(0, c * -18, 0);
		delay(50);
		for (int i = 0; i < 4000; i++)
		{
			move(0, 1, 0);
			uint16_t sg = tmc2130_read_sg(AX_SEL);
			if ((i > 16) && (sg < 5))	break;

			_c++;
			if (i == 3000) { _l++; }
			if (_c > 100) { shr16_set_led(1 << 2 * _l); };
			if (_c > 200) { shr16_set_led(0x000); _c = 0; };
		}
	}

	move(0, selector_steps_after_homing, 0); // move to initial position

    tmc2130_init(tmc2130_mode);

	delay(500);

	return true;
}

//! @brief Home both idler and selector if already not done
void home()
{
#ifndef NO_HOME
    home_idler();
    home_selector();
#else
    isIdlerParked = true;
    park_idler(false);
#endif
    shr16_set_led(0x155);
    shr16_set_led(0x000);
    shr16_set_led(1 << 2 * (4-active_extruder));
}
 

void move(int _idler, int _selector, int _pulley)
{
	int _acc = (max(_idler, _selector)>1) ? 120 : 0;
  int delay = 1200;

	// gets steps to be done and set direction
	_idler = set_idler_direction(_idler); 
	_selector = set_selector_direction(_selector);
	_pulley = set_pulley_direction(_pulley);

	do
	{
		if (_idler > 0) { idler_step_pin_set(); }
		if (_selector > 0) { selector_step_pin_set();}
		if (_pulley > 0) { pulley_step_pin_set(); }
		asm("nop");
		if (_idler > 0) { idler_step_pin_reset(); _idler--; }
		if (_selector > 0) { selector_step_pin_reset(); _selector--; }
		if (_pulley > 0) { pulley_step_pin_reset(); _pulley--;  }
		asm("nop");
       
    delayMicroseconds(delay);
    //if (delay > 1000) { delay -= 10; }
    //if (delay < 2000  && (_selector < _acc || _idler < _acc)) { delay += 10; }

    if (_acc > 0) { delayMicroseconds(_acc*10); _acc -= 1; }; // super pseudo acceleration control

	} while (_selector != 0 || _idler != 0 || _pulley != 0);
}


void set_idler_dir_down()
{
	shr16_set_dir(shr16_get_dir() & ~4);
	//shr16_set_dir(shr16_get_dir() | 4);
}
void set_idler_dir_up()
{
	shr16_set_dir(shr16_get_dir() | 4);
	//shr16_set_dir(shr16_get_dir() & ~4);
}


int set_idler_direction(int _steps)
{
#ifdef REVERSE_IDLER
	if (_steps > 0)
	{
		set_idler_dir_down();
	}
	else 
	{
    _steps = _steps * -1;
		set_idler_dir_up();
	}
#else
  if (_steps < 0)
  {
    _steps = _steps * -1;
    set_idler_dir_down();
  }
  else 
  {
    set_idler_dir_up();
  }
#endif

  return _steps;
}


int set_selector_direction(int _steps)
{
#ifdef REVERSE_SELECTOR
	if (_steps > 0)
	{
		shr16_set_dir(shr16_get_dir() & ~2);
	}
	else
	{
    _steps = _steps * -1;
		shr16_set_dir(shr16_get_dir() | 2);
	}
#else
  if (_steps < 0)
  {
    _steps = _steps * -1;
    shr16_set_dir(shr16_get_dir() & ~2);
  }
  else
  {
    shr16_set_dir(shr16_get_dir() | 2);
  }
#endif

	return _steps;
}


int set_pulley_direction(int _steps)
{
  if (_steps < 0)
  {
    _steps = _steps * -1;
    set_pulley_dir_pull();
  }
  else
  {
    set_pulley_dir_push();
  }
  
	return _steps;
}


void set_pulley_dir_push()
{
#ifdef REVERSE_PULLEY
  shr16_set_dir(shr16_get_dir() | 1);
#else  
	shr16_set_dir(shr16_get_dir() & ~1);
#endif
}
void set_pulley_dir_pull()
{
#ifdef REVERSE_PULLEY
  shr16_set_dir(shr16_get_dir() & ~1);
#else  
	shr16_set_dir(shr16_get_dir() | 1);
#endif
}


//! @brief Park idler
//! each filament selected has its park position, there is no park position for all filaments.
//! @param _unpark
//!  * false park
//!  * true engage
void park_idler(bool _unpark)
{
    if (_unpark && isIdlerParked) // get idler in contact with filament
    {
        move(idler_parking_steps, 0, 0);
        isIdlerParked = false;
    }
    else if (!_unpark && !isIdlerParked) // park idler so filament can move freely
    {
        move(idler_parking_steps*-1, 0, 0);
        isIdlerParked = true;
    }
}
