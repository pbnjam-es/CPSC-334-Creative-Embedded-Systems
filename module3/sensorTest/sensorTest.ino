const int PEDAL = 4;
const int PIEZO = 5;
const int LIGHT[3] = {6, 7, 8};

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
    for (int i = 0; i < 30; i++) {
        touchRead(PEDAL);
        delay(10);
    }
}

void loop() {
    int peak = 0;
    unsigned long t = millis();
    while (millis() - t < 50) {
        int v = analogRead(PIEZO);
        if (v> peak) {
            peak = v;
        }
    }
    Serial.printf("peda=%6u piezo=%4d light t/m/b = %4d %4d %4d\n",
        touchRead(PEDAL), peak, 
        analogRead(LIGHT[0]), analogRead(LIGHT[1]), analogRead(LIGHT[2]));
}