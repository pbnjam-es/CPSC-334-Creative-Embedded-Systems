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