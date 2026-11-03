const int PAD = 4;
const int OPEN_DELTA  = 6000;   // tune after looking at the output
const int CLOSE_DELTA = 3000;

uint32_t baseline;
bool isOpen = false;
unsigned long openedAt = 0;
float smooth = 0;

uint32_t median5() {                       // 5 quick reads, take the middle one
  uint32_t r[5];
  for (int i = 0; i < 5; i++) r[i] = touchRead(PAD);
  for (int i = 0; i < 4; i++)              // tiny sort
    for (int j = i + 1; j < 5; j++)
      if (r[j] < r[i]) { uint32_t t = r[i]; r[i] = r[j]; r[j] = t; }
  return r[2];
}

void setup() {
  Serial.begin(115200);
  delay(2000);                             // time to open the monitor
  Serial.println("calibrating, don't touch the bin...");
  uint32_t sum = 0;
  for (int i = 0; i < 20; i++) { sum += median5(); delay(20); }
//   baseline = sum / 20;
//   smooth = baseline;
  baseline = 19250;
  Serial.printf("baseline=%u  open>%u  close<%u\n",
                baseline, baseline + OPEN_DELTA, baseline + CLOSE_DELTA);
}

void loop() {
  uint32_t raw = median5();
  smooth = 0.7 * smooth + 0.3 * raw;       // light smoothing
  long delta = (long)smooth - (long)baseline;

  if (!isOpen && delta > OPEN_DELTA) {
    isOpen = true; openedAt = millis();
    Serial.println(">>> OPEN");
  } else if (isOpen && delta < CLOSE_DELTA) {
    isOpen = false;
    Serial.printf(">>> CLOSED after %lu ms\n", millis() - openedAt);
  }

  Serial.printf("raw=%6u  smooth=%6.0f  delta=%+6ld  %s\n",
                raw, smooth, delta, isOpen ? "OPEN" : "closed");
  delay(50);
}