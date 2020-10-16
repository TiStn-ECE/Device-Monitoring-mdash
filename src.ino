#include <ModbusMaster.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <WiFi.h>
#include <mDash.h>

#define MDASH_APP_NAME "deviceName"
#define DEVICE_PASSWORD "devicePassword"

#define MAX485_DE     4
#define MAX485_RE_NEG 0
#define RX2 16
#define TX2 17
uint8_t result;
float temp, hum;
long interval = 15000;
long previousMillis = 0;
int statusOTA = 1;

ModbusMaster node;
WiFiManager wifiManager;
void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void configModeCallback (WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);
  node.begin(1, Serial2);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setTimeout(60);
  if (!wifiManager.autoConnect("ESP32"))
  {
    Serial.println("failed to connect and hit timeout");
  }
  else
  {
    Serial.println("Successfully connected");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  mDashBegin(DEVICE_PASSWORD);
}

void loop() {
  unsigned long currentMillis = millis();
  result = node.readInputRegisters(0x0001, 2);
  if (WiFi.status() == WL_CONNECTED)
  {
    if (result == node.ku8MBSuccess)
    {
      temp = node.getResponseBuffer(0) / 10.0f;
      hum = node.getResponseBuffer(1) / 10.0f;
      Serial.print("Temp: "); Serial.print(temp); Serial.print("\t");
      Serial.print("Hum: "); Serial.print(hum);
      Serial.println();
      if (currentMillis - previousMillis > interval)
      {
        mDashNotify("DB.Save", "{%Q:%f, %Q:%f, %Q:%Q}", "Suhu", temp, "Kelembapan", hum, "OTA", "ON");
        previousMillis = currentMillis;
      }
    }
  }
}
