
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

#define DHTPIN 
#define DHTTYPE DHT11
#define BUZZER 15

// Wi-Fi credentials placeholder - replace with your network details
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const float TEMP_LIMIT = 35.0;

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

float temperature = NAN;
float humidity = NAN;
bool sensorOK = false;

unsigned long lastRead = 0;
const unsigned long READ_INTERVAL = 2000;

// Read sensor and control buzzer
void updateSensor() {
  if (millis() - lastRead >= READ_INTERVAL || lastRead == 0) {
    lastRead = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(t) || isnan(h)) {
      sensorOK = false;
      digitalWrite(BUZZER, LOW);
      Serial.println("DHT11 reading failed!");
      return;
    }

    temperature = t;
    humidity = h;
    sensorOK = true;

    digitalWrite(BUZZER, temperature > TEMP_LIMIT ? HIGH : LOW);

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    if (temperature > TEMP_LIMIT) {
      Serial.println("WARNING: HIGH TEMPERATURE!");
    }
  }
}

// Mobile webpage
void handleRoot() {
  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="refresh" content="2">
  <title>Cold Chain Monitor</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #101827;
      color: white;
      text-align: center;
      padding: 20px;
    }
    .card {
      background: #1f2b3d;
      padding: 25px;
      margin: 18px auto;
      border-radius: 15px;
      max-width: 400px;
    }
    .value {
      font-size: 34px;
      font-weight: bold;
      margin: 12px;
    }
    .status {
      padding: 12px;
      border-radius: 8px;
      font-weight: bold;
    }
    .normal { background: #176b45; }
    .alert { background: #a12626; }
    .error { background: #70551b; }
  </style>
</head>
<body>
  <h1>Cold Chain Monitor</h1>
  <p>ESP32 Live Temperature & Humidity</p>
)rawliteral";

  if (!sensorOK) {
    page += "<div class='card'><h2>Sensor Error</h2>";
    page += "<p>Check DHT11 wiring and refresh.</p></div>";
  } else {
    page += "<div class='card'><h2>Temperature</h2><div class='value'>";
    page += String(temperature, 1) + " &deg;C</div></div>";

    page += "<div class='card'><h2>Humidity</h2><div class='value'>";
    page += String(humidity, 1) + " %</div></div>";

    if (temperature > TEMP_LIMIT) {
      page += "<div class='status alert'>HIGH TEMPERATURE - BUZZER ALERT</div>";
    } else {
      page += "<div class='status normal'>Temperature Normal</div>";
    }
  }

  page += "<p>Alarm threshold: 35 &deg;C</p>";
  page += "</body></html>";

  server.send(200, "text/html", page);
}

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  dht.begin();
  delay(2000);

  Serial.println("\nStarting Cold Chain Monitor...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi Connected!");
  Serial.print("Open this address on your mobile: http://");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();

  Serial.println("Web server started!");
}

void loop() {
  server.handleClient();
  updateSensor();
}
