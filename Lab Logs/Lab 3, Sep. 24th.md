- Starting with the rpi-lgpio package, and reading input
- Wrote a simple
 ```python
 from RPi import GPIO
 
 GPIO.setmode(GPIO.BCM)
 
 GPIO.setup(14, GPIO>IN)
 while(True):
	 result = GPIO.input(14)
	 print(result)
 ```
- Lots of struggles with the initial setup but eventually figured it out