#Module 4 Creative Task
===========

This folder holds records and code for Module 4. 

Final code for this project can be found in fortunetelling/fortunetelling.ino, while test code can be found in fortune_test/fortune_test.ino. 

This project can be run with the fortune vending machine by turning on the switch in the back. 

To use the vending machine, select a type of fortune by moving the joystick to the right and press the button to confirm. Pressing the joystick button gives a random selection. To restock, hold the button
down for three seconds (for the selected lane) and six seconds (for all lanes).  

#include <Stepper.h>
#include <ESP32Servo.h>
#include <LiquidCrystal.h>

This project requires the Stepper, ESP32Servo and LiquidCrystal (for the LCD display) libraries for the Pi. 

The following materials/components were involved in the building of this project:

Count	Name
1	Raspberry Pi
1	ESP32 Microcontroller
3   28BYJ-48 5V Stepper Motors
3   ULN2003 Driver Boards
1   SG90 Servo
1   Switch
1   Button
1   Joystick
1   16-pin LCD display
5   Black foam board
2   MDF wooden sheets
1	Breadboard
Dozens	Wires
Dozens  3-D printed parts
Dozens  Fortune Cookies
