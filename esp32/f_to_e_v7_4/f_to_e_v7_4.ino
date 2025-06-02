#include <EEPROM.h>
#include <HardwareSerial.h>

#define EEPROM_SIZE 64

// PIN Mapping
#define OZONE_SENSOR_PIN 36
#define RELAY_PIN 12
#define FAN_PIN 14
#define LED_PIN 2

// UART2 ↔ ESP-01
HardwareSerial espSerial(2); // UART2
#define ESP01_RX 17
#define ESP01_TX 16

// State
String ssid, password, ip = "0.0.0.0", wifiMode = "";
int targetOzone = 100;
bool relayState = false;
bool fanState = false;

void setup() {
  Serial.begin(9600); // Bluetooth
  espSerial.begin(9600, SERIAL_8N1, ESP01_RX, ESP01_TX);
  EEPROM.begin(EEPROM_SIZE);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  loadConfig();
}

void loop() {
  handleBluetooth();
}

void handleBluetooth() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "HANDSHAKE") {
      Serial.println("ACK:" + wifiMode + "," + ip);
    } else if (cmd.startsWith("SET_WIFI:")) {
      parseAndConnectWiFi(cmd.substring(9));
    } else if (cmd.startsWith("CONTROL:")) {
      parseControl(cmd.substring(8));
    } else if (cmd == "STATUS") {
      sendStatus();
    }
  }
}

void parseAndConnectWiFi(String data) {
  int sep = data.indexOf(',');
  ssid = data.substring(0, sep);
  password = data.substring(sep + 1);

  sendAT("AT+RST", 2000);
  sendAT("AT+CWMODE=1", 1000);
  sendAT("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"", 5000);
  ip = sendAT("AT+CIFSR", 2000);

  wifiMode = "router";
  saveConfig();
  Serial.println("ACK:" + wifiMode + "," + ip);
}

void parseControl(String data) {
  if (data.indexOf("FAN_ON") != -1) {
    fanState = true;
    digitalWrite(FAN_PIN, HIGH);
  }
  if (data.indexOf("FAN_OFF") != -1) {
    fanState = false;
    digitalWrite(FAN_PIN, LOW);
  }
  if (data.indexOf("RELAY_ON") != -1) {
    relayState = true;
    digitalWrite(RELAY_PIN, HIGH);
  }
  if (data.indexOf("RELAY_OFF") != -1) {
    relayState = false;
    digitalWrite(RELAY_PIN, LOW);
  }
  if (data.indexOf("TARGET=") != -1) {
    int val = data.substring(data.indexOf("TARGET=") + 7).toInt();
    targetOzone = val;
  }
}

void sendStatus() {
  int ozone = analogRead(OZONE_SENSOR_PIN);
  Serial.println("STATUS:O3=" + String(ozone) + ",FAN=" + String(fanState) + ",RELAY=" + String(relayState) + ",TARGET=" + String(targetOzone));
}

// ===== EEPROM SAVE/LOAD =====

void saveConfig() {
  EEPROM.writeString(0, wifiMode);
  EEPROM.writeString(20, ip);
  EEPROM.commit();
}

void loadConfig() {
  wifiMode = EEPROM.readString(0);
  ip = EEPROM.readString(20);
}

// ====== AT Command ======

String sendAT(String cmd, int delayTime) {
  espSerial.println(cmd);
  delay(delayTime);
  String response = "";
  while (espSerial.available()) {
    response += espSerial.readString();
  }
  return response;
}
