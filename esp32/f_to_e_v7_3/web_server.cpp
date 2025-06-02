// File: web_server.cpp
#include "web_server.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include "utils.h"

const char* pairing_pin = "865712";
String auth_token = "";

bool relayStatus = false;
bool fanStatus = false;
float ozoneTarget = 5.0;
float currentOzone = 4.32;

WebServer server(80);

void WiFiEvent(WiFiEvent_t event) {
  if (event == WIFI_EVENT_AP_STACONNECTED) {
    logEvent("WIFI", "Android connected.");
  } else if (event == WIFI_EVENT_AP_STADISCONNECTED) {
    logEvent("WIFI", "Android disconnected.");
  }
}

void setupWiFiAccessPoint() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32_OZONE", "12345678");
  IPAddress ip = WiFi.softAPIP();
  logEvent("WIFI", "SoftAP Started");
  logEvent("WIFI", "AP IP address: " + ip.toString());
  WiFi.onEvent(WiFiEvent);
}

void sendJSON(int code, JsonDocument& doc) {
  String json;
  serializeJson(doc, json);
  server.send(code, "application/json", json);
}

void handleHandshake() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  StaticJsonDocument<256> body;
  DeserializationError err = deserializeJson(body, server.arg("plain"));

  if (err) {
    server.send(400, "text/plain", "Bad JSON");
    return;
  }

  const char* pin = body["pin"];
  if (String(pin) != pairing_pin) {
    server.send(401, "text/plain", "Unauthorized");
    return;
  }

  auth_token = "TKN" + String(millis());
  StaticJsonDocument<128> response;
  response["status"] = "ok";
  response["auth_token"] = auth_token;
  sendJSON(200, response);
}

void handleControl() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  StaticJsonDocument<256> body;
  DeserializationError err = deserializeJson(body, server.arg("plain"));
  if (err) {
    server.send(400, "text/plain", "Bad JSON");
    return;
  }

  if (!body.containsKey("auth_token") || body["auth_token"] != auth_token) {
    server.send(401, "text/plain", "Unauthorized");
    return;
  }

  if (body.containsKey("relay")) {
    relayStatus = body["relay"];
    logEvent("RELAY", relayStatus ? "ON" : "OFF");
  }
  if (body.containsKey("fan")) {
    fanStatus = body["fan"];
    logEvent("FAN", fanStatus ? "ON" : "OFF");
  }
  if (body.containsKey("target_ozone")) {
    ozoneTarget = body["target_ozone"];
    logEvent("TARGET", String(ozoneTarget, 2) + " ppm");
  }

  StaticJsonDocument<64> resp;
  resp["status"] = "ok";
  sendJSON(200, resp);
}

void handleStatus() {
  if (!server.hasArg("auth_token") || server.arg("auth_token") != auth_token) {
    server.send(401, "text/plain", "Unauthorized");
    return;
  }

  StaticJsonDocument<256> doc;
  doc["relay"] = relayStatus;
  doc["fan"] = fanStatus;
  doc["target"] = ozoneTarget;
  doc["ozone"] = currentOzone;
  sendJSON(200, doc);
}

void setupServer() {
  setupWiFiAccessPoint();
  server.on("/handshake", handleHandshake);
  server.on("/control", handleControl);
  server.on("/status", handleStatus);
  server.begin();
  logEvent("HTTP", "Server started");
}
