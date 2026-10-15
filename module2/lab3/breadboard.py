from RPi import GPIO
import time

GPIO.setmode(GPIO.BCM)

GPIO.setup(17, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(27, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(22, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)


while True:
    state17 = GPIO.input(17)
    state27 = GPIO.input(27)
    state22 = GPIO.input(22)
    print(state17, state27, state22)
    time.sleep(0.1)
    
    