// IoT Based Plant Watering System
// NodeMCU ESP8266 + soil moisture sensor + DHT11 + relay-controlled water pump, monitored through Blynk.
// Recovered from the CSE-460 IoT Laboratory project report (Jahangirnagar University, May 2024).
//
// Before uploading: replace the placeholders below with your own Blynk auth token and Wi-Fi details.
// Never commit real credentials to a public repository.

#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "IoT Plant Watering System"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

#define sensorPin A0   // soil moisture sensor (analog)
#define waterPump D3   // relay controlling the pump
#define DHTTYPE DHT11
#define DHTPIN D4      // DHT11 temperature and humidity sensor

DHT dht(DHTPIN, DHTTYPE);

bool Relay = false;
bool lastState = false;

char auth[] = "YOUR_BLYNK_AUTH_TOKEN";
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(waterPump, OUTPUT);
  digitalWrite(waterPump, HIGH);   // relay is active-LOW, so HIGH keeps the pump off
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
}

// Read temperature and humidity and send them to the Blynk app
void sendSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("%\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println("°C");
  Blynk.virtualWrite(V4, t);
  Blynk.virtualWrite(V3, h);
}

void loop() {
  Blynk.run();
  sendSensor();
  soilMoistureSensor();
}

// Pump on/off button in the Blynk app (virtual pin V1)
BLYNK_WRITE(V1) {
  Relay = param.asInt();
  if (Relay) {
    digitalWrite(waterPump, LOW);   // pump on
  } else {
    digitalWrite(waterPump, HIGH);  // pump off
  }
}

// Read soil moisture, convert it to a percentage, and send alerts
void soilMoistureSensor() {
  int value = analogRead(sensorPin);
  value = map(value, 0, 1024, 0, 100);
  value = (value - 100) * -1;       // higher value = wetter soil

  Blynk.virtualWrite(V0, value);

  if (value < 25 && !lastState) {
    Blynk.logEvent("Plant needs watering!");
    lastState = true;
  } else if (value >= 25) {
    lastState = false;
  }

  if (value > 40) {
    Blynk.logEvent("Do not water the plant!");
  }
}
