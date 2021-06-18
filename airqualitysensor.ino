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

  Serial.print("Retrieving time…\n");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP

  botSetup();

  if (airSensor.begin() == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    bot.sendMessage(CHATID, "Air sensor not detected. Please check wiring.\n", "");
    while (1);
  }

  Serial.print("Setup end\n");
}

void loop()
{
  Serial.print("Begin Loop\n");
  if ((airSensor.dataAvailable())) {
    if (airSensor.getCO2() > 1000) {
      Serial.print("CO2 too high\n");
      String message = translateActualValue + String(airSensor.getCO2()) + "ppm.\n" + translateOpen;
      bot.sendMessage(CHATID, message, "");
    } else {
      Serial.print("Air Quality is Ok\n");
    }

    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("User wants something!\n");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Serial.print("Begin Sleep\n");
    WiFi.disconnect();
    ESP.deepSleep(time_between * 1000);
    delay(100);
  }
}

void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    if (text == "/getStatus")
    {
      String message = "CO2: " + String(airSensor.getCO2()) + "ppm.\n" + "Temperature: " + airSensor.getTemperature() + "C\n" + "Humidity: " + airSensor.getHumidity() + "%";
      bot.sendMessage(CHATID, message, "");
    }
  }
}

void botSetup()
{
  const String commands = F("["
                            "{\"command\":\"start\", \"description\":\"Hello, from now on I will always inform you when it is time to open the window\"},"
                            "{\"command\":\"getStatus\",\"description\":\"Responds with the sensor data the next time it wakes up\"}" // no comma on last command
                            "]");
  bot.setMyCommands(commands);
}
