const int PINS[] = {4, 5, 6};
void setup() {
    Serial.begin(9600);
    pinMode(4, INPUT_PULLDOWN);
    pinMode(6, INPUT_PULLDOWN);
}

void loop() {
//   for (int p : PINS) {
// }
    Serial.printf("GPIO%d: %4d   ", 4, digitalRead(4));
    Serial.printf("GPIO%d: %4d   ", 5, analogRead(5));
    Serial.printf("GPIO%d: %4d   ", 6, digitalRead(6));
  Serial.println();
  delay(100);
}