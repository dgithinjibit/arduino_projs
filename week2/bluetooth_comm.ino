#include <SoftwareSerial.h>

#define BT_TX 10  // Arduino TX -> HC-05 RX
#define BT_RX 11  // Arduino RX -> HC-05 TX

SoftwareSerial BTSerial(BT_RX, BT_TX);

void setup() {
  Serial.begin(9600);       // Serial monitor
  BTSerial.begin(9600);     // Bluetooth module
  Serial.println("Bluetooth device ready. Pair your phone and open a serial terminal app.");
}

void loop() {
  // Send data to smartphone
  BTSerial.println("Hello from Arduino via Bluetooth!");
  delay(2000);

  // Check if data received from smartphone
  if (BTSerial.available()) {
    String incoming = BTSerial.readString();
    Serial.print("Received from phone: ");
    Serial.println(incoming);

    // Echo back confirmation
    BTSerial.print("Arduino received: ");
    BTSerial.println(incoming);
  }
}
