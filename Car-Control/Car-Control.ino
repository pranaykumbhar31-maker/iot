
#include <WiFi.h>
#include <WebServer.h>

// ESP32 creates its own Wi-Fi network
const char* apName = "ESP32_CAR";
const char* apPassword = "car12345";

// L298N motor driver pins
#define ENA 25
#define IN1 26
#define IN2 27

#define ENB 23
#define IN3 18
#define IN4 19

// HC-SR04 ultrasonic sensor pins
#define TRIG 5
#define ECHO 4

const float OBSTACLE_DISTANCE = 20.0;

WebServer server(80);

String currentCommand = "stop";
float distanceCM = -1;
bool obstacleDetected = true;

unsigned long lastDistanceCheck = 0;
unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 1000;

void stopCar() {
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  currentCommand = "stop";
}

float readDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  unsigned long duration = pulseIn(ECHO, HIGH, 25000);

  if (duration == 0) return -1;

  return duration * 0.0343 / 2.0;
}

void updateObstacle() {
  if (millis() - lastDistanceCheck >= 100) {
    lastDistanceCheck = millis();
    distanceCM = readDistance();

    // Fail safe: unknown distance blocks forward movement.
    obstacleDetected =
      (distanceCM < 0 || distanceCM <= OBSTACLE_DISTANCE);

    if (obstacleDetected && currentCommand == "forward") {
      stopCar();
    }
  }
}

void drive(String direction) {
  if (direction == "forward" && obstacleDetected) {
    stopCar();
    return;
  }

  if (direction == "forward") {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else if (direction == "backward") {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  } else if (direction == "left") {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else if (direction == "right") {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  } else {
    stopCar();
    return;
  }

  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);

  currentCommand = direction;
  lastCommandTime = millis();
}

void handleRoot() {
  String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 Car Control</title>
<style>
body {
  background: #101827;
  color: white;
  font-family: Arial;
  text-align: center;
  padding: 16px;
}
button {
  min-width: 90px;
  height: 70px;
  margin: 5px;
  border: 0;
  border-radius: 12px;
  color: white;
  background: #2563eb;
  font-size: 17px;
  font-weight: bold;
}
.stop { background: #dc2626; }
</style>
</head>
<body>
<h1>ESP32 Smart Car</h1>
<p id="status">Ready</p>
<p id="distance">Checking sensor...</p>

<div>
<button onclick="move('forward')">Forward</button>
</div>
<div>
<button onclick="move('left')">Left</button>
<button class="stop" onclick="move('stop')">STOP</button>
<button onclick="move('right')">Right</button>
</div>
<div>
<button onclick="move('backward')">Backward</button>
</div>

<script>
async function move(dir) {
  try {
    const response = await fetch('/move?dir=' + dir);
    const message = await response.text();
    document.getElementById('status').textContent = message;
  } catch (e) {
    document.getElementById('status').textContent =
      'Connection lost';
  }
}

async function refreshStatus() {
  try {
    const response = await fetch('/status');
    const data = await response.json();
    document.getElementById('distance').textContent =
      'Distance: ' + data.distance +
      ' | Obstacle: ' + data.obstacle;
  } catch (e) {}
}

setInterval(refreshStatus, 500);
window.addEventListener('pagehide', () => {
  navigator.sendBeacon('/stop');
});
</script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", page);
}

void handleMove() {
  if (!server.hasArg("dir")) {
    server.send(400, "text/plain", "Missing direction");
    return;
  }

  String direction = server.arg("dir");

  if (direction != "forward" &&
      direction != "backward" &&
      direction != "left" &&
      direction != "right" &&
      direction != "stop") {
    server.send(400, "text/plain", "Invalid command");
    return;
  }

  if (direction == "stop") {
    stopCar();
    server.send(200, "text/plain", "Car stopped");
    return;
  }

  drive(direction);

  if (direction == "forward" && obstacleDetected) {
    server.send(200, "text/plain",
                "Forward blocked: obstacle or no sensor echo");
  } else {
    server.send(200, "text/plain", "Command: " + direction);
  }
}

void handleStatus() {
  String json = "{\"distance\":\"";

  if (distanceCM < 0) {
    json += "No echo";
  } else {
    json += String(distanceCM, 1) + " cm";
  }

  json += "\",\"obstacle\":\"";
  json += obstacleDetected ? "YES" : "NO";
  json += "\"}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  stopCar();

  WiFi.mode(WIFI_AP);

  if (!WiFi.softAP(apName, apPassword)) {
    Serial.println("Failed to start Wi-Fi hotspot");
    return;
  }

  Serial.println("ESP32 car Wi-Fi started");
  Serial.print("Wi-Fi name: ");
  Serial.println(apName);
  Serial.print("Wi-Fi password: ");
  Serial.println(apPassword);
  Serial.print("Open on mobile: http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/move", handleMove);
  server.on("/status", handleStatus);

  server.on("/stop", HTTP_GET, []() {
    stopCar();
    server.send(200, "text/plain", "Stopped");
  });

  server.on("/stop", HTTP_POST, []() {
    stopCar();
    server.send(200, "text/plain", "Stopped");
  });

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
  updateObstacle();

  // Stop if the browser stops refreshing movement commands.
  if (currentCommand != "stop" &&
      millis() - lastCommandTime > COMMAND_TIMEOUT) {
    stopCar();
  }
}
