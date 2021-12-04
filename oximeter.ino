#include<SoftwareSerial.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const int timeout = 120;
SoftwareSerial mySerial (D1, D2);
WiFiManager wm;
const byte numChars = 100;
String nmwifi, wifi;
char receivedChars[numChars];
boolean newData = false;
BearSSL::WiFiClientSecure net;
PubSubClient client(net);
static const char fp[] PROGMEM = "FINGER_SHA1-SERVER";
const char HOSTNAME[] = "NAMASERVER";
const char MQTT_HOST[] = "IPSERVER";
const int MQTT_PORT = 8883;
const char MQTT_USER[] = "USER";
const char MQTT_PASS[] = "PASSWORD";


void mqtt_connect()
{
  if (!client.connect(HOSTNAME, MQTT_USER, MQTT_PASS)) {
    Serial.print("failed, status code =");
    Serial.print(client.state());
    delay(5000);
  }
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println();
  mySerial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println();
  wifi = GetMyMacAddress();
  nmwifi = "NATRYA" + wifi;
  std::vector<const char *> menu = {"wifi", "sep", "restart", "exit"};
  wm.setMenu(menu);
  wm.setHostname(HOSTNAME);
  wm.setConfigPortalTimeout(timeout);
  wm.setDebugOutput(false);
  if (wm.autoConnect(nmwifi.c_str(), "PASSWORDWIFI")) {
    net.setFingerprint(fp);
    client.setServer(MQTT_HOST, MQTT_PORT);
    delay(2000);
    mqtt_connect();
  }
}


void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    wm.setConfigPortalTimeout(timeout);
    wm.setDebugOutput(false);
    if (!wm.autoConnect(nmwifi.c_str(), "PASSWORDWIFI")) {
      delay(5000);
      ESP.restart();
    }
  } else {
    recvWithEndMarker();
    showNewData();
  }
  if (newData) {
    newData = false;
  }
  mqtt_connect();
  client.loop();
}

void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;
  while (mySerial.available() > 0 && newData == false) {
    rc = mySerial.read();
    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0';
      ndx = 0;
      newData = true;
    }
  }
}
void showNewData() {
  if (newData == true) {
    if (receivedChars[1] == 'S' && receivedChars[6] != ' ' && receivedChars[7] != ' ') {
      if (receivedChars[11] == '=' && receivedChars[12] != ' ' && receivedChars[13] != ' ') {
        String SP,DJ;
        SP=String(receivedChars[6])+String(receivedChars[7]);
        DJ=String(receivedChars[12])+String(receivedChars[13]);
        client.publish(String("/" + wifi + "/oxy").c_str(), SP.c_str(), false);
        client.publish(String("/" + wifi + "/bpm").c_str(), DJ.c_str(), false);
      }
    }
    newData = false;
  }
}

String GetMyMacAddress()
{
  uint8_t mac[6];
  char macStr[18] = {0};
  WiFi.macAddress(mac);
  sprintf(macStr, "%02X%02X%02X%02X%02X%02X", mac[0],  mac[1], mac[2], mac[3], mac[4], mac[5]);
  return  String(macStr);
}
