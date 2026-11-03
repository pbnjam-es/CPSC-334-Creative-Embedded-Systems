#Module 2 Technical Task
===========

This project is a five-mode device implemented in pygame/Python with the Raspberry Pi. It can be run with the following command:
```python3 technicaltask.py ```
on the Raspberry Pi terminal. 

There are two separate game modes:
  Switch ON  = MENU: button -> next mode of operation, joystick X push -> previous mode of operation
  Switch OFF = PLAY: button + joystick do whatever the current mode defines

Joystick is read digitally, with each axis giving 1 when pushed hard
toward its "high" side, 0 otherwise. That creates two usable joystick bits.
The five modes of operation are paint, stamps, spinner, flash, and counter, each of which use the button and joystick in some combination to create distinct effects. 
In Python, a while loop enables inputs to be constantly monitored and processed for updating by pygame, leading to real-time responsiveness. 

The following materials/components were involved in the building of this project:

Count	Name
1	Raspberry Pi
1   Joystick
1   Button
1   Switch
1   Breadboard
12  Wires

A demo video can be viewed [here](https://drive.google.com/file/d/1FwJWLbi5Wsr8a1ToRfxb7kV718xHTUr7/view?usp=sharing)