- Plugged the ESP32 into the pi and downloaded the esp32 library on arduiono
- ssh'd onto pi and downloaded the arduino-cli
- Used the following commands to setup arduino-cli
	- ```bash
	  mkdir -p ~/.local/bin && curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=~/.local/bin sh
	  echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc && source ~/.bashrc
	  arduino-cli config init
	  arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
	  arduino-cli core update-index
	  arduino-cli core install esp32:esp32
	  arduino-cli core install esp32:esp32@3.2.0
	  
	  ```
- Wrote a simple .ino file to print the mac address
- ```
  mkdir -p ~/Barrow-CES3340-2026/module2/lab3/macaddress && cat > ~/Barrow-CES3340-2026/module2/lab3/macaddress/macaddress.ino << 'EOF'

#include "esp_mac.h"

uint8_t baseMac[6];

void setup() {
	
	Serial.begin(9600);
	
	esp_read_mac(baseMac, ESP_MAC_WIFI_STA);

}

void loop() {
	
	Serial.print("Station MAC: ");
	
	for (int i = 0; i < 5; i++) {
	
	Serial.printf("%02X:", baseMac[i]);
	
	}
	
	Serial.printf("%02X\n", baseMac[5]);
	
	delay(1000);

}
  ```
- compiled with
``` bash
cd ~/Barrow-CES3340-2026/module2/lab3 && arduino-cli compile --fqbn esp32:esp32:esp32s3 macaddress && arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32s3 macaddress
```
- executed with
```bash
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=9600
```
- Station MAC: 80:45:6B:2A:47:D8
- Once the interface and mac address were setup we wired the circuits
- Using the esp32s3 pinout, we connected the button, switch, and joystick to the esp32 pins
- Wrote a ino file to read in inputs
``` c
const int PIN_X = 4, PIN_Y = 5;              // ADC1 pins
const int PIN_JOYBTN = 6, PIN_BUTTON = 7, PIN_SWITCH = 15;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);               
  analogSetAttenuation(ADC_11db);           
  pinMode(PIN_JOYBTN, INPUT_PULLUP);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_SWITCH, INPUT_PULLUP);

}

  

void loop() {

  int x = analogRead(PIN_X);
  int y = analogRead(PIN_Y);
  int joyBtn = !digitalRead(PIN_JOYBTN);     // 1 = pressed
  int button = !digitalRead(PIN_BUTTON);
  int sw = !digitalRead(PIN_SWITCH);
  
  Serial.print(x); Serial.print(',');
  Serial.print(y); Serial.print(',');
  Serial.print(joyBtn); Serial.print(',');
  Serial.print(button); Serial.print(',');
  Serial.println(sw);

}
```
- Flashed to esp32 with 
	```bash
	cd ~/Barrow-CES3340-2026/module2/lab3 && arduino-cli compile --fqbn esp32:esp32:esp32s3 joystick && arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32s3 joystick
	```
- Ran the esp32 with
```bash
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200
```
- This outputted the x,y, joysw, button,  and switch values
- From here we made a python file to take in the serial data and output it
```python
import serial

PORT = "/dev/ttyUSB0"

BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=1)

ser.reset_input_buffer() 

print("Reading from", PORT, "- Ctrl+C to quit")

try:

    while True:

        line = ser.readline().decode("utf-8", errors="ignore").strip()
        parts = line.split(",")
        if len(parts) != 5:
            continue  # skip malformed lines
        try:
            x, y, joy_btn, button, switch = map(int, parts)
        except ValueError:
            continue

        print(f"X={x:4d}  Y={y:4d}  joyBtn={joy_btn}  button={button}  switch={switch}")
except KeyboardInterrupt:
	print("\nGoodbye world")
finall:
	ser.close()
```