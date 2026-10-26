# ESP32 Smart Projects

A collection of two ESP32 embedded projects demonstrating wireless communication, sensor integration, and motor actuation:

1. **Wi-Fi Controlled Smart Car** — An ESP32-powered RC car hosted on its own standalone Access Point (SoftAP) with a browser-based directional control interface and ultrasonic obstacle detection.
2. **Temperature & Humidity Monitor** — A cold chain environmental monitoring station utilizing a DHT11 sensor and buzzer alarm, with real-time web dashboard reporting over Wi-Fi.

---

## Repository Structure

```text
ESP32-Smart-Projects/
├── Car-Control/
│   └── Car-Control.ino        # ESP32 Access Point car controller sketch
├── Temperature-Monitor/
│   └── Temperature-Monitor.ino # Cold chain temperature & humidity monitoring sketch
├── .gitignore                 # Git ignore rules for Arduino & IDE artifacts
└── README.md                  # Project documentation and setup guide
```

---

## Project 1: Wi-Fi Controlled Smart Car

### Overview & Features
* **Standalone Access Point (SoftAP):** The ESP32 broadcasts its own Wi-Fi network (`ESP32_CAR`). No router or internet connection is required.
* **Web-Based Control Panel:** Serves an interactive mobile-friendly web interface with Directional controls (Forward, Backward, Left, Right, Stop).
* **Ultrasonic Obstacle Detection:** Measures distance via an HC-SR04 sensor. Forward movement is automatically stopped if an obstacle is within 20 cm or if no echo is detected.
* **Command Watchdog Failsafe:** Automatically stops motors if the web client loses connection or fails to send keep-alive commands within 1000 ms.

### Hardware Components
* ESP32 Development Board (NodeMCU ESP-WROOM-32 or equivalent)
* L298N Dual H-Bridge Motor Driver module
* 4x DC Geared Motors with Wheels
* HC-SR04 Ultrasonic Distance Sensor
* Robot Car Chassis
* Dedicated Motor Battery Pack (e.g., 2x 18650 Li-ion cells or 7.4V–12V pack)
* ESP32 Power Supply (USB power bank or 5V step-down regulator)
* Jumper Wires & Resistors (for logic level divider)

### Pinout & Wiring Table

| Peripheral | Component Pin | ESP32 GPIO | Notes |
| :--- | :--- | :--- | :--- |
| **L298N Motor Driver** | `ENA` | `GPIO 25` | Left motor enable / PWM |
| | `IN1` | `GPIO 26` | Left motor input 1 |
| | `IN2` | `GPIO 27` | Left motor input 2 |
| | `ENB` | `GPIO 23` | Right motor enable / PWM |
| | `IN3` | `GPIO 18` | Right motor input 1 |
| | `IN4` | `GPIO 19` | Right motor input 2 |
| | `GND` | `GND` | **Must share common ground with ESP32** |
| | `+12V` | Motor Battery (+) | Power input for motors |
| **HC-SR04 Ultrasonic** | `VCC` | `5V` | Sensor supply |
| | `TRIG` | `GPIO 5` | Trigger output |
| | `ECHO` | `GPIO 4` | **Use voltage divider** (5V to 3.3V logic) |
| | `GND` | `GND` | Ground connection |

> **IMPORTANT ELECTRICAL SAFETY NOTE:**  
> The HC-SR04 operates at 5V and outputs a 5V logic signal on the `ECHO` pin. Because ESP32 GPIO pins are rated for **3.3V logic**, connecting a 5V echo pin directly can damage the ESP32. Use a simple two-resistor voltage divider (e.g., 1kΩ and 2kΩ) or a logic level shifter between HC-SR04 `ECHO` and ESP32 `GPIO 4`.

### Setup & Flashing Instructions
1. Open Arduino IDE and select your ESP32 board (e.g., **ESP32 Dev Module**).
2. Open `Car-Control/Car-Control.ino`.
3. Check and confirm the Wi-Fi AP settings:
   ```cpp
   const char* apName = "ESP32_CAR";
   const char* apPassword = "car12345";
   ```
4. Connect the ESP32 via USB and upload the sketch.
5. Elevate the car so wheels spin freely in the air during initial testing.
6. Connect your smartphone/laptop to the Wi-Fi network **`ESP32_CAR`** with password **`car12345`**.
7. Open a browser and navigate to:
   ```text
   http://192.168.4.1
   ```

---

## Project 2: Temperature & Humidity Monitor

### Overview & Features
* **Live Environmental Monitoring:** Reads real-time ambient temperature and relative humidity using a DHT11 digital sensor every 2 seconds.
* **Local Web Dashboard:** Hosts an auto-refreshing HTTP status page displaying temperature, humidity, and operational status.
* **Audible Alarm (Buzzer):** Automatically sounds an active buzzer whenever the temperature exceeds the configured threshold (`TEMP_LIMIT = 35.0 °C`).
* **Sensor Health Check:** Detects sensor read failures and alerts the user on both the web UI and Serial Monitor.

### Hardware Components
* ESP32 Development Board
* DHT11 Temperature and Humidity Sensor (or module with built-in pull-up resistor)
* 5V or 3.3V Active Buzzer
* 5V/3.3V Power Source (USB cable or power bank)
* Breadboard and Jumper Wires

### Pinout & Wiring Table

| Peripheral | Component Pin | ESP32 GPIO | Notes |
| :--- | :--- | :--- | :--- |
| **DHT11 Sensor** | `VCC` | `3.3V` or `5V` | Refer to sensor module rating |
| | `DATA` | `DHTPIN` | Configure pin in code (e.g., `GPIO 4`) |
| | `GND` | `GND` | Ground connection |
| **Active Buzzer** | `VCC / (+)` | `GPIO 15` | Digital output pin |
| | `GND / (-)` | `GND` | Ground connection |

> **CONFIGURATION REQUIRED BEFORE COMPILING:**  
> In `Temperature-Monitor/Temperature-Monitor.ino`, ensure `DHTPIN` has a GPIO assigned (for example `#define DHTPIN 4` if connected to GPIO 4).  
> In addition, set your Wi-Fi network credentials:
> ```cpp
> const char* ssid = "YOUR_WIFI_SSID";
> const char* password = "YOUR_WIFI_PASSWORD";
> ```

### Setup & Flashing Instructions
1. Install required libraries in Arduino IDE (Library Manager):
   * **DHT sensor library** by Adafruit
   * **Adafruit Unified Sensor**
2. Open `Temperature-Monitor/Temperature-Monitor.ino`.
3. Set your Wi-Fi SSID and Password.
4. Set the `DHTPIN` to your connected GPIO.
5. Upload the sketch to your ESP32.
6. Open the **Serial Monitor** at **115200 baud** to view connection progress and the assigned local IP address.
7. Open the printed IP address (e.g., `http://192.168.1.xxx`) on any device connected to the same local Wi-Fi network.

---

## Security Best Practices for Git & GitHub

* **Wi-Fi Credentials & Secrets:** Never push real Wi-Fi network names, passwords, or authentication keys to public Git repositories.
* **Using Placeholders:** Replace personal credentials with placeholders such as `"YOUR_WIFI_SSID"` and `"YOUR_WIFI_PASSWORD"` before staging commits.
* **Separating Secrets:** For advanced projects, consider loading secrets from a gitignored `secrets.h` header file.

---

## Troubleshooting

### ESP32 COM Port Not Found
* Ensure you are using a USB data cable (many micro-USB cables are charge-only).
* Install the appropriate USB-to-UART bridge VCP driver (Silicon Labs CP210x or WCH CH340).

### Car Motors Not Spinning
* Confirm the motor driver has its own dedicated battery pack; the ESP32 cannot supply enough current to drive DC motors directly.
* Ensure a common ground connection exists between the ESP32 `GND` and the L298N `GND`.
* Check whether the L298N onboard 5V regulator jumper is installed or removed based on your motor voltage.

### Temperature Sensor Reads "Failed"
* Ensure the 10kΩ pull-up resistor between DHT11 `DATA` and `VCC` is present (modules usually have this onboard).
* Confirm that `DHTPIN` matches the physical GPIO wired to the sensor data line.

---

## Verification & Hardware Disclaimer

Pin assignments, timing thresholds, and logic reflect the code as written in the Arduino sketches. Always verify pin capabilities, current draw limitations, and logic voltage levels on your specific ESP32 development board and motor modules before connecting external power supplies.
