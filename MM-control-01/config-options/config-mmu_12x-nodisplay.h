// hardware-specific configuration file


//
// MMU 12x by cjbaar
// https://github.com/cjbaar/prusa-mmu-12x
//


#ifndef CONFIG_MMU_H_
#define CONFIG_MMU_H_


//number of extruders
#define EXTRUDERS               12
#undef  ENABLE_CUTTER           // defined for original PrusaMMU2

//former stepper.cpp variables
#define SELECTOR_STEPS                  303     // 50 steps/mm
#define SELECTOR_STEPS_AFTER_HOMING     -4250
#define SELECTOR_STEPS_LAST             450     // extra steps for close filaments

#define IDLER_STEPS                     224     // angle / .1125
#define IDLER_STEPS_AFTER_HOMING        -33     // need <45 for 12x25.2
#define IDLER_PARKLING_STEPS            280     // idler*1.5

//axis parameters
#define REVERSE_PULLEY          // undefined for original PrusaMMU2
#define REVERSE_IDLER           // undefined for original PrusaMMU2
#undef  REVERSE_SELECTOR        // undefined for original PrusaMMU2

//pulley speeds (mm/s)
#define PULLEY_DIAMETER         11.9f    // original PrusaMMU2 = 6.2f
#define PULLEY_STEPS_PER_MM     400 / (PI * PULLEY_DIAMETER)  // 400 = motor steps(200) * pulley resolution(2)
#define PULLEY_ACCELERATION_X   0.995f

#define PULLEY_RATE_EXTRUDER    19.02f   // mm/s, direct from Firmware::mmu.h
#define PULLEY_RATE_PRIME       12.5f
#define PULLEY_RATE_LOAD        125.0f
#define PULLEY_RATE_UNLOAD      125.0f

#define PULLEY_DELAY_EXTRUDER   ceil(1000000 / (PULLEY_RATE_EXTRUDER*PULLEY_STEPS_PER_MM))
#define PULLEY_DELAY_PRIME      ceil(1000000 / (PULLEY_RATE_PRIME*PULLEY_STEPS_PER_MM))
#define PULLEY_DELAY_LOAD       ceil(1000000 / (PULLEY_RATE_LOAD*PULLEY_STEPS_PER_MM))
#define PULLEY_DELAY_UNLOAD     ceil(1000000 / (PULLEY_RATE_UNLOAD*PULLEY_STEPS_PER_MM))

//filament lengths
#define FILAMENT_RETRACT_MM     5.0f   // original PrusaMMU2 = ~29.2?
#define FILAMENT_BOWDEN_MM      427.0f   // original PrusaMMU2 = ~427
#define FILAMENT_EJECT_MM		120.0f

//external display
#undef SSD_DISPLAY             // undefined for original PrusaMMU2

//diagnostic functions
#undef _DIAG
#undef NO_HOME


#endif //CONFIG_MMU_H_
