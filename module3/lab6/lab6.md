- Coby and Jam
- First task was to get the MAC address which we got in previous lab
	- 80:45:6B:2A:47:D8
- The next task is to build the circuit. Dealt with some confusion about voltage dividing but we're trying
- Build a simple circuit with photoresistor connecting to 3v3 and a resistor, then the resistor connects to ground, and finally a cable connects the photoresistor/resistor to GPIO10
- After building the circuit we checked it with a multimeter then wrote a simple loop for it to send out the analog input through the wifi to the raspberry pi. It reads input and sends it to the Pi's IP over yale wireless with a specified port
```c++

//TODO put in the right pins
const int joystick1xpin = 4;
//const int joystick1yPin = 0;
const int joystick2xpin = 5;
//const int joystick2yPin = 0;
const int joystick3xpin = 6;
//const int joystick3yPin = 0;


void setup() {
  Serial.begin(9600);
}

void loop() {
  int xval1 = analogRead(joystick1xpin);
  //int yval1 = analogRead(joystick1yPin);
  int xval2 = analogRead(joystick2xpin);
  //int yval2 = analogRead(joystick2yPin);
  int xval3 = analogRead(joystick3xpin);
  //int yval3 = analogRead(joystick3yPin);

  Serial.printf("%d %d %d\n", xval1, xval2, xval3);
  delay(50);
}
```
```C++ 
#include "WiFi.h"

  

const char* ssid = "yale wireless";

const char* password = "";

const char* piHost = "10.67.69.8";

int piPort = 5000;

  

WiFiClient client;

  

void setup() {

Serial.begin(9600);

WiFi.mode(WIFI_MODE_STA);

Serial.print("Connecting to ");

Serial.println(ssid);

WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {

delay(500);

Serial.print(".");

}

Serial.println("\nWifi Connected");

Serial.println(WiFi.localIP());

}

  

void loop() {

Serial.println(WiFi.macAddress());

while (client.connect(piHost, piPort)) {

int rawValue = analogRead(10);

client.println(rawValue);

delay(500);

}

}
```
- Flashing the code onto the ESP32 is the bane of my existence. 
- From here we wrote the python code to receive this input. We used the socket library and TCP
```python
import socket
import tkinter as tk
import random
HOST = "0.0.0.0"
PORT = 5000

root = tk.Tk()
root.geometry("400x300")
    
def handle_client():
    value = random.randint(0, 4096)
    value /= 16
    rgb = (value, value, value)
    hex_color = f"#{int(rgb[0]):02x}{int(rgb[1]):02x}{int(rgb[2]):02x}"
    root.configure(bg=hex_color)
    root.after(50, handle_client)
    print(hex_color)

handle_client()
root.mainloop()

#     with conn:
#         conn.settimeout(5)
#         buffer = ""
#         try:
#             while True:
#                 chunk = conn.recv(1024)
#                 if not chunk:
#                     break
#                 buffer += chunk;
#             print(buffer)
#         except socket.timeout:
#             print("Timed out")

# with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
#         server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
#         server.bind((HOST, PORT))
#         server.listen(5)
#         print(f"Listening on port {PORT}")
#         
#         try:
#             while True:
#                 conn, addr = server.accept();
#                 handle_client(conn, addr)
#         except:
#             pass

```
- For the visualization we decided to keep it simple given how much time everything else is taking us. So we are just coloring the screen to match the light levels. It's a very simple 1:1 
