#include <Stepper.h>
#include <ESP32Servo.h>
#include <LiquidCrystal.h>

const int STEPS_PER_REV = 2048;
const int STEPPER_RPM = 12;
const int NUM_LANES = 3;
const int LANE_PINS[NUM_LANES][4] = {
  {4, 5, 6, 7},    
  {15, 16, 17, 18}, 
  {9, 10, 11, 12}   
};
Stepper steppers[NUM_LANES] = { 
  Stepper(STEPS_PER_REV, 4, 6, 5, 7),
  Stepper(STEPS_PER_REV, 15, 17, 16, 18),
  Stepper(STEPS_PER_REV, 9, 11, 10, 12)
};
const char* LANE_NAMES[NUM_LANES] = {"Doom", "Love", "Work"};
const int FULL_STOCK = 5;
int stock[NUM_LANES] = {FULL_STOCK, FULL_STOCK, FULL_STOCK};

const int VEND_PIN = 38;               
const unsigned long DEBOUNCE_MS = 30;
bool vendWasPressed = false;

const int JOY_X_PIN = 1;
const int JOY_SW_PIN = 41;           
const int JOY_LOW = 1000, JOY_HIGH = 3000;
bool joyWasPressed = false;
unsigned long lastMove = 0;
int selected = 0;

const int SERVO_PIN = 8;
int FLAP_CLOSED = 108;                  
int FLAP_OPEN = 12;
const int DOOR_WAIT_MS = 1200;         
const int FLAP_HOLD_MS = 4000;        
const int DOOR_MOVE_MS = 600;         
Servo servo;
int servoPos = 0;

LiquidCrystal lcd(13, 14, 21, 47, 42, 2);

void moveServo(int target, int ms) {  
  target = constrain(target, 0, 180);
  int start = servoPos;
  int steps = abs(target - start);
  if (steps == 0) return;
  for (int i = 1; i <= steps; i++) {
    servo.write(start + (target > start ? i : -i));
    delay(ms / steps);
  }
  servoPos = target;
}

void setServoAngle(int a) {         
  a = constrain(a, 0, 180);
  servo.write(a);
  servoPos = a;
  Serial.printf("Servo angle: %d\n", a);
}

void showMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("< A%d %-5s    >", selected + 1, LANE_NAMES[selected]);
  lcd.setCursor(0, 1);
  if (stock[selected] > 0) lcd.printf("%d left  Press!", stock[selected]);
  else lcd.print("   SOLD OUT");
}

void showMessage(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(line1);
  lcd.setCursor(0, 1); lcd.print(line2);
}

void releaseCoils(int lane) {
  for (int i = 0; i < 4; i++) digitalWrite(LANE_PINS[lane][i], LOW);
}

void vend(int lane) {
  if (stock[lane] <= 0) {
    showMessage(LANE_NAMES[lane], "is SOLD OUT");
    Serial.printf("%s sold out\n", LANE_NAMES[lane]);
    delay(1500);
    showMenu();
    return;
  }
  showMessage("Consulting the", "cookies...");
  Serial.printf("Vending %s\n", LANE_NAMES[lane]);

  steppers[lane].step(STEPS_PER_REV);  
  releaseCoils(lane);
  stock[lane]--;

  delay(DOOR_WAIT_MS);             
  showMessage("Take your", "fortune!");
  moveServo(FLAP_OPEN, DOOR_MOVE_MS);
  delay(FLAP_HOLD_MS);
  showMessage("Thank you!", "");
  moveServo(FLAP_CLOSED, DOOR_MOVE_MS);

  showMenu();
}

void surpriseMe() {
  int available[NUM_LANES], n = 0;
  for (int l = 0; l < NUM_LANES; l++) if (stock[l] > 0) available[n++] = l;
  if (n == 0) {
    showMessage("Fate is", "SOLD OUT");
    delay(1500);
    showMenu();
    return;
  }
  int choice = available[random(n)];
  Serial.printf("Surprise -> %s\n", LANE_NAMES[choice]);

  int shown = random(NUM_LANES);
  int spins = 12 + random(NUM_LANES);
  for (int i = 0; i < spins; i++) {
    shown = (shown + 1) % NUM_LANES;
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print(" Fate chooses...");
    lcd.setCursor(5, 1); lcd.print(LANE_NAMES[shown]);
    delay(60 + i * 25);
  }
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(" Fate chooses...");
  lcd.setCursor(4, 1); lcd.printf(">%s<", LANE_NAMES[choice]);
  delay(1200);

  selected = choice;
  vend(choice);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  randomSeed(esp_random());

  for (int l = 0; l < NUM_LANES; l++) {
    steppers[l].setSpeed(STEPPER_RPM);
    for (int i = 0; i < 4; i++) pinMode(LANE_PINS[l][i], OUTPUT);
    releaseCoils(l);
  }
  pinMode(VEND_PIN, INPUT_PULLUP);
  pinMode(JOY_SW_PIN, INPUT_PULLUP);

  ESP32PWM::allocateTimer(0);
  servo.setPeriodHertz(50);
  servo.attach(SERVO_PIN, 500, 2400);
  servo.write(FLAP_CLOSED);
  servoPos = FLAP_CLOSED;

  lcd.begin(16, 2);
  showMessage("FORTUNE COOKIES", "warming up...");
  delay(1000);
  showMenu();

  Serial.println("Ready.");
  Serial.println("Vend:  1/2/3 = vend lane, v = vend selected, s = surprise, r = restock");
  Serial.println("Door:  o = open, c = close");
  Serial.println("Calib: ] / [ = +/-1 deg, } / { = +/-5 deg, a45 = go to 45 deg, p = print");
  Serial.println("       C = save current angle as CLOSED, O = save current angle as OPEN");
}

void loop() {
  // joystick left/right to change lane
  int x = analogRead(JOY_X_PIN);
  if (millis() - lastMove > 300) {
    if (x < JOY_LOW)  { selected = (selected + NUM_LANES - 1) % NUM_LANES; lastMove = millis(); showMenu(); }
    if (x > JOY_HIGH) { selected = (selected + 1) % NUM_LANES;             lastMove = millis(); showMenu(); }
  }

  // vend button vends the selected lane
  bool vPressed = (digitalRead(VEND_PIN) == LOW);
  if (vPressed && !vendWasPressed) {
    delay(DEBOUNCE_MS);
    if (digitalRead(VEND_PIN) == LOW) vend(selected);
  }
  vendWasPressed = vPressed;

  // joystick press for "surprise me" functionality
  bool joyPressed = (digitalRead(JOY_SW_PIN) == LOW);
  if (joyPressed && !joyWasPressed) {
    delay(DEBOUNCE_MS);
    if (digitalRead(JOY_SW_PIN) == LOW) surpriseMe();
  }
  joyWasPressed = joyPressed;

  // commands for real-time control
  if (Serial.available()) {
    char c = Serial.read();
    // vending
    if (c >= '1' && c <= '3') { selected = c - '1'; vend(selected); }
    if (c == 'v') vend(selected);
    if (c == 's') surpriseMe();
    if (c == 'r') {
      for (int l = 0; l < NUM_LANES; l++) stock[l] = FULL_STOCK;
      showMenu();
      Serial.println("Restocked");
    }
    
    if (c == 'o') { moveServo(FLAP_OPEN, DOOR_MOVE_MS);   Serial.printf("Door open (%d)\n", FLAP_OPEN); }
    if (c == 'c') { moveServo(FLAP_CLOSED, DOOR_MOVE_MS); Serial.printf("Door closed (%d)\n", FLAP_CLOSED); }

    if (c == ']') setServoAngle(servoPos + 1);
    if (c == '[') setServoAngle(servoPos - 1);
    if (c == '}') setServoAngle(servoPos + 5);
    if (c == '{') setServoAngle(servoPos - 5);
    if (c == 'a') setServoAngle(Serial.parseInt());
    if (c == 'p') Serial.printf("Servo angle: %d  (closed=%d, open=%d)\n", servoPos, FLAP_CLOSED, FLAP_OPEN);
    if (c == 'C') { FLAP_CLOSED = servoPos; Serial.printf("FLAP_CLOSED set to %d\n", FLAP_CLOSED); }
    if (c == 'O') { FLAP_OPEN = servoPos;   Serial.printf("FLAP_OPEN set to %d\n", FLAP_OPEN); }
  }
}