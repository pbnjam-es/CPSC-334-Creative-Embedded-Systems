#include <WiFi.h>
#include <WiFiUdp.h>

// ---------- network ----------
const char* SSID     = "yale wireless";
const char* PASSWORD = "";
const char* PI_IP    = "10.67.69.8";
const int   PI_PORT  = 5005;

// ---------- pins ----------
const int PEDAL = 4, PIEZO = 5;
const int LIGHT[3] = {6, 7, 8};               // top, middle, bottom

// ---------- pedal calibration (parallel-plate sensor) ----------
float       REST       = 35500;   // measured automatically at boot (keep pedal up!)
const float FULL       = 70000;   // pedal resting on the block
const float PUSH_RANGE = 27000;   // extra reading when pressed hard into the block
const float OPEN_AT    = 0.30;    // depth that counts as "lid open"
const float CLOSE_AT   = 0.15;    // depth that counts as "closed" (hysteresis)

// ---------- piezo ----------
const int PIEZO_IDLE = 70;                     // remeasure once mounted
const unsigned long IGNORE_AFTER_OPEN = 300;    // skip lid-swing vibration

WiFiUDP udp;
float smooth;
bool isOpen = false;
unsigned long openedAt = 0, peakAt = 0, reachedAt = 0, lastBeat = 0;
float maxDepth = 0, maxPush = 0;
int maxThud = 0, maxLight[3];

// ---------- helpers ----------
uint32_t median5() {
  uint32_t r[5];
  for (int i = 0; i < 5; i++) r[i] = touchRead(PEDAL);
  for (int i = 0; i < 4; i++)
    for (int j = i + 1; j < 5; j++)
      if (r[j] < r[i]) { uint32_t t = r[i]; r[i] = r[j]; r[j] = t; }
  return r[2];
}

int piezoPeak(int ms) {
  int peak = 0;
  unsigned long t = millis();
  while (millis() - t < (unsigned long)ms) {
    int v = analogRead(PIEZO);
    if (v > peak) peak = v;
  }
  return peak;
}

float depthOf(float v) { return constrain((v - REST) / (FULL - REST), 0.0f, 1.0f); }
float pushOf(float v)  { return constrain((v - FULL) / PUSH_RANGE, 0.0f, 1.0f); }

void send(const String& msg) {
  udp.beginPacket(PI_IP, PI_PORT);
  udp.print(msg);
  udp.endPacket();
  Serial.println("sent: " + msg);
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  if (strlen(PASSWORD)) WiFi.begin(SSID, PASSWORD); else WiFi.begin(SSID);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.printf("\nconnected, my IP: %s\n", WiFi.localIP().toString().c_str());
}

void calibrateRest() {
  for (int i = 0; i < 30; i++) { touchRead(PEDAL); delay(20); }   // discard junk reads
  uint32_t s[21];
  for (int i = 0; i < 21; i++) { s[i] = median5(); delay(20); }
  for (int i = 0; i < 20; i++)
    for (int j = i + 1; j < 21; j++)
      if (s[j] < s[i]) { uint32_t t = s[i]; s[i] = s[j]; s[j] = t; }
  REST = s[10];
  smooth = REST;
  Serial.printf("REST calibrated: %.0f (FULL %.0f)\n", REST, FULL);
}

// ---------- main ----------
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  delay(1000);
  calibrateRest();
  connectWiFi();
  send("HELLO " + String(REST, 0));
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();

  int thud = max(0, piezoPeak(20) - PIEZO_IDLE);
  smooth = 0.7 * smooth + 0.3 * median5();
  float depth = depthOf(smooth);
  float push  = pushOf(smooth);

  // ---- lid opens ----
  if (!isOpen && depth > OPEN_AT) {
    isOpen = true;
    openedAt = peakAt = millis();
    reachedAt = 0;
    maxDepth = depth; maxPush = 0; maxThud = 0;
    for (int i = 0; i < 3; i++) maxLight[i] = 0;
    send("OPEN");
  }

  // ---- while open: collect everything ----
  if (isOpen) {
    if (depth > maxDepth) { maxDepth = depth; peakAt = millis(); }
    if (push > maxPush) maxPush = push;
    if (reachedAt == 0 && depth >= 0.9) reachedAt = millis();
    if (millis() - openedAt > IGNORE_AFTER_OPEN && thud > maxThud) maxThud = thud;
    for (int i = 0; i < 3; i++) maxLight[i] = max(maxLight[i], analogRead(LIGHT[i]));
  }

  // ---- lid closes: send the deposit ----
  if (isOpen && depth < CLOSE_AT) {
    isOpen = false;
    unsigned long openMs = millis() - openedAt;
    unsigned long riseMs = (reachedAt ? reachedAt : peakAt) - openedAt;
    send("DEPOSIT " + String(openMs) + " " + String(maxThud) + " " +
         String(maxDepth, 2) + " " + String(maxPush, 2) + " " + String(riseMs) + " " +
         String(maxLight[0]) + " " + String(maxLight[1]) + " " + String(maxLight[2]));
  }

  // ---- heartbeat with live values (for calibration) ----
  if (millis() - lastBeat > 5000) {
    lastBeat = millis();
    send("HB " + String(smooth, 0) + " " + String(depth, 2) + " " + String(push, 2) + " " +
         String(thud) + " " + String(analogRead(LIGHT[0])) + " " +
         String(analogRead(LIGHT[1])) + " " + String(analogRead(LIGHT[2])));
  }
}