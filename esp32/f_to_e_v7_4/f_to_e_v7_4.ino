#include <BluetoothSerial.h>
#include <EEPROM.h>

#define RELAY_PIN      12
#define FAN_PIN        14
#define LED_PIN        2
#define SENSOR_PIN     36
#define EEPROM_SIZE    256

// UART2 สำหรับ ESP-01
HardwareSerial esp01Serial(2); // UART2 (GPIO16 TX, GPIO17 RX)

BluetoothSerial SerialBT;

String wifiMode = "";
String wifiIP = "";
String authToken = "";

float targetOzone = 2.0;  // ค่าเป้าหมายเริ่มต้น

// ==== EEPROM Helpers ====
void loadConfig() {
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(0) != 0xAA) {
    wifiMode = "";
    wifiIP = "";
    authToken = "";
    EEPROM.end();
    return;
  }

  wifiMode   = EEPROMString(1, 32);
  wifiIP     = EEPROMString(33, 32);
  authToken  = EEPROMString(65, 64);
  EEPROM.end();
}

void saveConfig(String mode, String ip, String token) {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(0, 0xAA);  // magic byte
  EEPROMWriteString(1, mode, 32);
  EEPROMWriteString(33, ip, 32);
  EEPROMWriteString(65, token, 64);
  EEPROM.commit();
  EEPROM.end();
}

void clearEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < EEPROM_SIZE; i++) EEPROM.write(i, 0);
  EEPROM.commit();
  EEPROM.end();
}

String EEPROMString(int start, int len) {
  char data[len + 1];
  for (int i = 0; i < len; i++) {
    data[i] = EEPROM.read(start + i);
  }
  data[len] = 0;
  return String(data);
}

void EEPROMWriteString(int start, String value, int maxLen) {
  int len = value.length();
  if (len > maxLen) len = maxLen;
  for (int i = 0; i < len; i++) {
    EEPROM.write(start + i, value[i]);
  }
  for (int i = len; i < maxLen; i++) {
    EEPROM.write(start + i, 0);
  }
}

// ==== SETUP ====
void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32-OZONE");
  esp01Serial.begin(9600, SERIAL_8N1, 17, 16);  // RX, TX

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  loadConfig();

  Serial.println("ESP32 Ready.");
}

// ==== LOOP ====
void loop() {
  handleBluetoothCommands();
  handleESP01Commands();
  updateOzoneControl();
  delay(200);
}

// ==== Bluetooth Handler ====
void handleBluetoothCommands() {
  if (!SerialBT.available()) return;

  String cmd = SerialBT.readStringUntil('\n');
  cmd.trim();

  if (cmd == "STATUS?") {
    if (wifiMode != "") {
      SerialBT.printf("STATUS:CONNECTED|%s|%s|%s\n", wifiMode.c_str(), wifiIP.c_str(), authToken.c_str());
    } else {
      SerialBT.println("STATUS:NOT_CONFIGURED");
    }
  } else if (cmd.startsWith("SET_WIFI:")) {
    String creds = cmd.substring(9);
    esp01Serial.println("SET_WIFI:" + creds);
    SerialBT.println("FORWARDED_TO_ESP01");
  } else if (cmd.startsWith("SAVE_WIFI_MODE:")) {
    int first = cmd.indexOf(':') + 1;
    int pipe1 = cmd.indexOf('|');
    int pipe2 = cmd.indexOf('|', pipe1 + 1);
    wifiMode = cmd.substring(first, pipe1);
    wifiIP = cmd.substring(pipe1 + 1, pipe2);
    authToken = cmd.substring(pipe2 + 1);
    saveConfig(wifiMode, wifiIP, authToken);
    SerialBT.println("CONFIG_SAVED");
  } else if (cmd == "CLEAR_CONFIG") {
    clearEEPROM();
    SerialBT.println("CONFIG_CLEARED");
  }
}

// ==== ESP-01 (UART2) Command Handler ====
void handleESP01Commands() {
  if (!esp01Serial.available()) return;
  String input = esp01Serial.readStringUntil('\n');
  input.trim();

  if (input.startsWith("{") && input.endsWith("}")) {
    float t = getValue(input, "target").toFloat();
    targetOzone = t;

    if (input.indexOf("\"relay\":true") != -1) digitalWrite(RELAY_PIN, HIGH);
    else digitalWrite(RELAY_PIN, LOW);

    if (input.indexOf("\"fan\":true") != -1) digitalWrite(FAN_PIN, HIGH);
    else digitalWrite(FAN_PIN, LOW);
  }
}

// ==== Control Logic ====
void updateOzoneControl() {
  int raw = analogRead(SENSOR_PIN);
  float voltage = raw * (3.3 / 4095.0);
  digitalWrite(LED_PIN, millis() / 500 % 2);  // กระพริบ LED

  // ส่งค่า ozone ปัจจุบันกลับ ESP-01
  String json = "{\"ozone\":" + String(voltage, 2) + "}";
  esp01Serial.println(json);
}

// ==== JSON Helper ====
String getValue(String json, String key) {
  int i = json.indexOf("\"" + key + "\":");
  if (i == -1) return "";
  int start = json.indexOf(":", i) + 1;
  int end = json.indexOf(",", start);
  if (end == -1) end = json.indexOf("}", start);
  return json.substring(start, end);
}
