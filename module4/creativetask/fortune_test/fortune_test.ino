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

// button: one leg to BUTTON_PIN, other leg to GND (uses the internal pull-up)
const int BUTTON_PIN = 38;
const unsigned long DEBOUNCE_MS = 30;

void releaseCoils() {
  // The Stepper library leaves the coils powered; turn them off so the motor stays cool
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void oneRevolution() {
  Serial.println("Button -> one revolution");
  stepper.step(STEPS_PER_REV);           // one full 360° revolution
  releaseCoils();

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

void setup() {
  Serial.begin(115200);
  delay(1000);

  stepper.setSpeed(STEPPER_RPM);
  releaseCoils();

  pinMode(BUTTON_PIN, INPUT_PULLUP);     // reads HIGH normally, LOW when pressed

  ESP32PWM::allocateTimer(0);
  servo.setPeriodHertz(50);              // standard 50 Hz servo signal
  servo.attach(SERVO_PIN, 500, 2400);    // pulse range in µs
  servo.write(servoAngle);
  delay(500);

  Serial.println("Start: servo at 0. Press the button to turn one revolution.");
}

void loop() {
  static bool wasPressed = false;
  bool pressed = (digitalRead(BUTTON_PIN) == LOW);

  if (pressed && !wasPressed) {          // only on the moment the button goes down
    delay(DEBOUNCE_MS);
    if (digitalRead(BUTTON_PIN) == LOW) {
      oneRevolution();                   // holding the button won't repeat; release and press again
    }
  }
  wasPressed = pressed;
}