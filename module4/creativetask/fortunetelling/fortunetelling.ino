#include <Stepper.h>
#include <ESP32Servo.h>
#include <LiquidCrystal.h>

// ---------- lanes ----------
const int STEPS_PER_REV = 2048;
const int STEPPER_RPM = 12;
const int NUM_LANES = 3;
const int LANE_PINS[NUM_LANES][4] = {
  {4, 5, 6, 7},       // lane A
  {15, 16, 17, 18},   // lane B
  {9, 10, 11, 12}     // lane C
};
Stepper steppers[NUM_LANES] = {   // 28BYJ-48 order: IN1, IN3, IN2, IN4
  Stepper(STEPS_PER_REV, 4, 6, 5, 7),
  Stepper(STEPS_PER_REV, 15, 17, 16, 18),
  Stepper(STEPS_PER_REV, 9, 11, 10, 12)
};
const char* LANE_NAMES[NUM_LANES] = {"Love", "Work", "Doom"};
const int FULL_STOCK = 5;
int stock[NUM_LANES] = {FULL_STOCK, FULL_STOCK, FULL_STOCK};

// ---------- vend button ----------
const int VEND_PIN = 38;                 // one leg to 38, other leg to GND
const unsigned long DEBOUNCE_MS = 30;
bool vendWasPressed = false;

// ---------- joystick ----------
const int JOY_X_PIN = 1;
const int JOY_SW_PIN = 41;               // pressing the stick = Surprise me
const int JOY_LOW = 1000, JOY_HIGH = 3000;   // ESP32 reads 0..4095, centre ~2048
bool joyWasPressed = false;
unsigned long lastMove = 0;
int selected = 0;

// ---------- servo ----------
const int SERVO_PIN = 8;
const int SERVO_STEP_DEG = 10;
const int SERVO_MAX_DEG = 180;
Servo servo;
int servoAngle = 0;

// ---------- 16x2 display (RS, E, D4, D5, D6, D7) ----------
LiquidCrystal lcd(13, 14, 21, 47, 42, 2);

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

  steppers[lane].step(STEPS_PER_REV);   // one revolution = one cookie
  releaseCoils(lane);
  stock[lane]--;

  servoAngle += SERVO_STEP_DEG;
  servo.write(servoAngle);
  if (servoAngle >= SERVO_MAX_DEG) {
    delay(500);
    servoAngle = 0;
    servo.write(servoAngle);
  }

  showMessage("Take your", "fortune!");
  delay(2000);
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

  // slot-machine spin that slows down and lands on the choice
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
  randomSeed(esp_random());              // ESP32 hardware random number generator

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
  servo.write(servoAngle);

  lcd.begin(16, 2);
  showMessage("FORTUNE COOKIES", "warming up...");
  delay(1000);
  showMenu();
  Serial.println("Ready. Joystick L/R selects, button vends, stick press = surprise. Serial: 1/2/3, v, s, r.");
}

void loop() {
  // joystick left/right: change lane
  int x = analogRead(JOY_X_PIN);
  if (millis() - lastMove > 300) {
    if (x < JOY_LOW)  { selected = (selected + NUM_LANES - 1) % NUM_LANES; lastMove = millis(); showMenu(); }
    if (x > JOY_HIGH) { selected = (selected + 1) % NUM_LANES;             lastMove = millis(); showMenu(); }
  }

  // vend button: vend the selected lane
  bool vPressed = (digitalRead(VEND_PIN) == LOW);
  if (vPressed && !vendWasPressed) {
    delay(DEBOUNCE_MS);
    if (digitalRead(VEND_PIN) == LOW) vend(selected);
  }
  vendWasPressed = vPressed;

  // joystick press: surprise me (delete this block if you don't want it)
  bool joyPressed = (digitalRead(JOY_SW_PIN) == LOW);
  if (joyPressed && !joyWasPressed) {
    delay(DEBOUNCE_MS);
    if (digitalRead(JOY_SW_PIN) == LOW) surpriseMe();
  }
  joyWasPressed = joyPressed;

  // serial: 1/2/3 vend that lane, v vend selected, s surprise, r restock
  if (Serial.available()) {
    char c = Serial.read();
    if (c >= '1' && c <= '3') { selected = c - '1'; vend(selected); }
    if (c == 'v') vend(selected);
    if (c == 's') surpriseMe();
    if (c == 'r') {
      for (int l = 0; l < NUM_LANES; l++) stock[l] = FULL_STOCK;
      showMenu();
      Serial.println("Restocked");
    }
  }
}