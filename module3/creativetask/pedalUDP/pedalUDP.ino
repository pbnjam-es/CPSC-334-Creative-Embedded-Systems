#include <WiFi.h>
#include <WiFiUdp.h>

// ---- network ----
const char* SSID     = "yale wireless";   // must match the network name exactly
const char* PASSWORD = "";                // leave empty if it's an open/registered network
const char* PI_IP    = "10.67.69.8";
const int   PI_PORT  = 5005;

// ---- sensor ----
const int PAD = 4;
const uint32_t BASELINE = 19200;          // your hardcoded value
const int OPEN_DELTA  = 6000;
const int CLOSE_DELTA = 3000;

WiFiUDP udp;
bool isOpen = false;
unsigned long openedAt = 0, lastBeat = 0;
float smooth = BASELINE;

uint32_t median5() {
  uint32_t r[5];
  for (int i = 0; i < 5; i++) r[i] = touchRead(PAD);
  for (int i = 0; i < 4; i++)
    for (int j = i + 1; j < 5; j++)
      if (r[j] < r[i]) { uint32_t t = r[i]; r[i] = r[j]; r[j] = t; }
  return r[2];
}

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

void setup() {
  Serial.begin(115200);
  delay(1000);
  for (int i = 0; i < 30; i++) { touchRead(PAD); delay(20); }  // warm-up
  connectWiFi();
  send("HELLO");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();

  smooth = 0.7 * smooth + 0.3 * median5();
  long delta = (long)smooth - (long)BASELINE;

  if (!isOpen && delta > OPEN_DELTA) {
    isOpen = true; openedAt = millis();
    send("OPEN");
  } else if (isOpen && delta < CLOSE_DELTA) {
    isOpen = false;
    send("CLOSED " + String(millis() - openedAt));
  }

  if (millis() - lastBeat > 5000) {          // heartbeat so you know it's alive
    lastBeat = millis();
    send("HB " + String(delta));
  }
  delay(30);
}