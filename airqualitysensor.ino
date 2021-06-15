#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h"
#include "env.h"

SCD30 airSensor;
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Wire.begin();

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setTrustAnchors(&cert);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP

  if (airSensor.begin() == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    bot.sendMessage(CHATID, "Air sensor not detected. Please check wiring.", "");
    while (1)
      ;
  }

  Serial.print("Setup end");
  bot.sendMessage(CHATID, "Aufgewacht", "");
}

void loop()
{
  Serial.print("Begin Loop");
  if ((airSensor.dataAvailable()) && (airSensor.getCO2() > 1000)) {
    Serial.print("CO2 too high");
    String message = translateActualValue + String(airSensor.getCO2()) + "ppm.\n" + translateOpen;
    bot.sendMessage(CHATID, message, "");
  } else {
    Serial.print("Air Quality is Ok");
    }

  Serial.print("Begin Sleep");

  ESP.deepSleep(time_between * 1000);
  delay(100);
}
