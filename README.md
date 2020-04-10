# MM-control-01
MMU 3-axis stepper control

## Overview
This project was forked from the original Prusa source in order to make modifications for increasing the number of filaments handled by the MMU. Related hardware for this project can be found at [https://github.com/cjbaar/prusa-mmu-12x](https://github.com/cjbaar/prusa-mmu-12x).

I have only tested this using the Arduino IDE, so other sections below have been removed. Note that you need to copy one of the config variations in /MM-control-01/config-mmu-options to /MM-control-01/config-mmu.h. I have not tested this with the original Prusa 5-filament hardware, but it should theoretically work if you just want the display.

## Display
This code has support for an optional OLED display (SSD1306, 128x64). I added this to help provide more information on what the MMU is actually thinking as it makes tool changes, and to improve the options for recovering from load/unload failures and jams. The display can be attached to the unused "SENSOR" port on the Prusa MMU2 control board, which is an exposed I2C interface. Connect jumper wires as below. Pin 1 of the SENSOR port is next to the power connector.
```
  1 <-> VCC (+5v)
  2 <-> GND
  3 <-> SCL
  4 <-> SDA
```
### Data
The display will show a general message describing what state/function the MMU is currently in, along with the most received serial command at the top. The center will show the current filament, or the current change in progress from a T* command. The bottom is just a quick status overview that keeps count of issues.
<img src="https://github.com/cjbaar/prusa-mmu-12x/blob/master/img/change.jpeg" width="480" />
The data displayed includes:
```
  L:n/n  Priming failures / load failures
  U:n/n  Retract failures / unload failures
  T:n    Total tool changes completed
```

### Menu
When a load or unload failure is detected, or when the unit receives a wait (W0) command from the printer, the unit will go into an "enhanced recovery menu." This is fairly basic, as it relies on the three available buttons, but it does provide the ability to tune the stepper motors if something got jammed and is not in the right place. It operates in four "modes." In each case, the center button will switch to the next mode.
```
  + Main
      < Rehome the idler and selector
      > OK to continue print
  + Pulley
      < Move the pulley back 1mm
      > Move the pulley forward 1m
  + Selector
      < Move the selector left 0.5mm
      > Move the pulley right 0.5mm
  + Idler
      < Move the idler back 1°
      > Move the idler forward 1°
```
This is still a work in progress, and hope to get ideas for further improvements in the future.

## Building
### Arduino
Recomended version is arduino 1.8.5.  
in MM-control-01 subfolder create file version.h
in MM-control-01 subfolder copy config-mmu.h from selection in ./config-mmu-options
use version.h.in as template, replace ${value} with numbers or strings according to comments in template file.  

#### Adding MMUv2 board
In Arduino IDE open File / Settings  
Set Additional boards manager URL to:  
https://raw.githubusercontent.com/prusa3d/Arduino_Boards/master/IDE_Board_Manager/package_prusa3d_index.json  
Open Tools / Board: / Boards manager...
Install Prusa Research AVR Boards by Prusa Research  
which contains only one board:  
Original Prusa i3 MK3 Multi Material 2.0

Select board Original Prusa i3 MK3 Multi Material 2.0

Bootloader binary is shipped with the board, source is located at https://github.com/prusa3d/caterina
#### Build
click verify to build

## Building documentation
Run doxygen in MM-control-01 folder.
Documentation is generated in Doc subfolder.
