// hardware-specific configuration file


//
// Original Prusa MMU2S
// or, theoretically, a reasonable facsimile thereof
//


#ifndef CONFIG_MMU_H_
#define CONFIG_MMU_H_


//number of extruders
#define EXTRUDERS               5
#undef  ENABLE_CUTTER

//former stepper.cpp variables
#define SELECTOR_STEPS                  697.5f   // 2790/4
#define SELECTOR_STEPS_AFTER_HOMING     -3700
#define SELECTOR_STEPS_LAST             0

#define IDLER_STEPS                     355      // angle / .1125
#define IDLER_STEPS_AFTER_HOMING        -130
#define IDLER_PARKLING_STEPS            217.5f   //(idler_steps / 2) + 40

//axis parameters
#undef REVERSE_PULLEY
#undef REVERSE_IDLER
#undef REVERSE_SELECTOR

//pulley speeds (mm/s)
#define PULLEY_DIAMETER         6.2f
#define PULLEY_STEPS_PER_MM     400 / (PI * PULLEY_DIAMETER)  // 400 = motor steps(200) * pulley resolution(2)
#define PULLEY_ACCELERATION_X   0.997f

#define PULLEY_RATE_EXTRUDER    19.02f   // mm/s, direct from Firmware::mmu.h
#define PULLEY_RATE_PRIME       12.5f
#define PULLEY_RATE_LOAD        125.0f
#define PULLEY_RATE_UNLOAD      125.0f

#define PULLEY_DELAY_EXTRUDER   ceil(1000000 / (PULLEY_RATE_EXTRUDER*PULLEY_STEPS_PER_MM))
#define PULLEY_DELAY_PRIME      ceil(1000000 / (PULLEY_RATE_PRIME*PULLEY_STEPS_PER_MM))
#define PULLEY_DELAY_LOAD       ceil(1000000 / (PULLEY_RATE_LOAD*PULLEY_STEPS_PER_MM))
#define PULLEY_DELAY_UNLOAD     ceil(1000000 / (PULLEY_RATE_UNLOAD*PULLEY_STEPS_PER_MM))

//filament lengths
#define FILAMENT_RETRACT_MM     29.2f
#define FILAMENT_BOWDEN_MM      427.0f
#define FILAMENT_EJECT_MM		120.0f

//external display
#define SSD_DISPLAY

//diagnostic functions
#undef _DIAG
#undef NO_HOME


#endif //CONFIG_MMU_H_
