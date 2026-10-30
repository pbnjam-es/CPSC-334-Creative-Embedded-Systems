- Jam and Coby
- Started by writing ESP32 code for the stepper motor and servo motor
- Written on Arduino IDE and flashed to ESP32

- Connected stepper motor to ULN2003 Driver board and connected driver board to breadboard 
- Acquired 5V power supply

- First test - problems with jittery servo motor, functional stepper motor
- Decided to separate the power supply of the motors from the ESP32

- Second test - still jittery servo motor with inconsistent rotation/back and forth rotation
- Grounded ESP32 to the power supply
- Third test - resolved, both servo motor and stepper motor are now working

- Below is final Arduino code:
#include <Stepper.h>
#include <ESP32Servo.h>

// stepper
const int STEPS_PER_REV = 2048;
const int IN1 = 4;
const int IN2 = 5;
const int IN3 = 6;
const int IN4 = 7;

Stepper stepper(STEPS_PER_REV, IN1, IN3, IN2, IN4);
const int STEPPER_RPM = 12;

// servo
const int SERVO_PIN = 18;
const int SERVO_STEP_DEG = 10;
const int SERVO_MAX_DEG = 180;
Servo servo;
int servoAngle = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  stepper.setSpeed(STEPPER_RPM);

  ESP32PWM::allocateTimer(0);
  servo.setPeriodHertz(50);              // standard 50 Hz servo signal
  servo.attach(SERVO_PIN, 500, 2400);    // MG90S pulse range in µs
  servo.write(servoAngle);
  delay(500);

  Serial.println("Start: servo at 0");
}

void loop() {
  stepper.step(STEPS_PER_REV);           // one full 360° revolution

  servoAngle += SERVO_STEP_DEG;
  servo.write(servoAngle);
  Serial.printf("Stepper rev done -> servo %d deg\n", servoAngle);

  if (servoAngle >= SERVO_MAX_DEG) {
    delay(500);                          // hold at 180° briefly
    servoAngle = 0;
    servo.write(servoAngle);
    Serial.println("Reached 180 -> back to 0");
  }
}