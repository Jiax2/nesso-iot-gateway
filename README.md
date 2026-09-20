# Nesso IoT Gateway

IoT project built around the **Arduino Nesso N1**, combining embedded firmware, MQTT communication and local control of a Xiaomi Air Fryer.

The Nesso communicates with an **EMQX MQTT broker**, while a Python gateway translates MQTT commands into local Xiaomi MIoT commands.

No Xiaomi Cloud connection is required during normal operation.

![Nesso IoT Gateway demo](docs/demo.gif)

---

## Architecture

```text
Arduino Nesso N1
       |
       | Wi-Fi / MQTT
       v
      EMQX
       |
       | MQTT
       v
 Python Gateway
       |
       | Local MIoT
       v
 Xiaomi Air Fryer
```

The communication is bidirectional, allowing the Nesso to send commands and receive the current state of the air fryer.

---

## Features

- Modular C++ firmware
- ESP32-C6 based Nesso N1
- Wi-Fi connectivity
- MQTT publish / subscribe
- Automatic MQTT reconnection
- JSON telemetry
- Device availability tracking
- Bidirectional MQTT commands
- Battery monitoring
- Touchscreen interface
- Physical button controls
- Temperature and cooking time selection
- Start, stop, pause and resume controls
- Real-time air fryer state
- Remaining cooking time display
- Animated UI while cooking
- Automatic display timeout
- Python MQTT gateway
- Local Xiaomi MIoT control
- EMQX broker running with Docker
- PlatformIO build environment

---

## MQTT Architecture

### Nesso

```text
nesso-iot/v1/devices/nessito/
├── availability
├── telemetry
├── command
└── command-result
```

### Air Fryer

```text
nesso-iot/v1/devices/airfryer-01/
├── availability
├── state
├── command
└── command-result
```

Example command:

```json
{
  "request_id": "nesso-3",
  "command": "start",
  "temperature_c": 180,
  "duration_min": 15
}
```

Example state:

```json
{
  "device_id": "airfryer-01",
  "status": 9,
  "fault": 0,
  "target_temperature_c": 180,
  "target_time_min": 15,
  "left_time_min": 14
}
```

---

## Telemetry

The Nesso periodically publishes device telemetry:

```json
{
  "device_id": "nessito",
  "firmware_version": "0.1.0",
  "uptime_s": 120,
  "wifi_rssi_dbm": -38,
  "free_heap_bytes": 309344,
  "battery_percent": 75
}
```

---

## User Interface

The Nesso provides a local interface for controlling the air fryer.

From the main screen the user can:

- Select temperature
- Select cooking time
- Start cooking
- Pause or resume cooking
- Stop cooking
- View the remaining time

Large touch areas are used for temperature and time selection, while the Nesso physical buttons can also be used for reliable interaction.

While the air fryer is running, an animated cat is displayed together with the current temperature and remaining cooking time.

---

## Project Structure

```text
nesso-iot-gateway/
├── firmware/
│   └── nesso/
│       ├── platformio.ini
│       └── src/
│           ├── main.cpp
│           ├── config.h
│           ├── WifiManager.cpp
│           ├── WifiManager.h
│           ├── MqttService.cpp
│           ├── MqttService.h
│           ├── DisplayManager.cpp
│           ├── DisplayManager.h
│           └── RobotIcon.h
│
├── gateway/
│   ├── main.py
│   ├── config.py
│   ├── xiaomi_airfryer.py
│   ├── requirements.txt
│   └── .env.example
│
├── infrastructure/
├── docs/
├── scripts/
├── compose.yaml
└── README.md
```

---

## Development

### Start EMQX

From the project root:

```bash
docker compose up -d
```

Check the broker:

```bash
docker compose ps
```

---

### Run the Python Gateway

```bash
cd gateway
source .venv/bin/activate
python main.py
```

The gateway requires the air fryer IP and local Xiaomi token through environment variables.

Example:

```env
MQTT_BROKER=localhost
MQTT_PORT=1883

XIAOMI_AIRFRYER_IP=192.168.1.X
XIAOMI_AIRFRYER_TOKEN=
```

Secrets are not stored in the repository.

---

### Build Firmware

```bash
cd firmware/nesso
pio run
```

### Upload Firmware

```bash
pio run -t upload
```

### Serial Monitor

```bash
pio device monitor
```

---

## Communication Flow

Starting the air fryer from the Nesso follows this path:

```text
Nesso UI
   |
   v
MQTT command
   |
   v
EMQX
   |
   v
Python Gateway
   |
   v
Xiaomi MIoT
   |
   v
Air Fryer
```

The resulting state is sent back through the same architecture:

```text
Air Fryer
   |
   v
Python Gateway
   |
   v
EMQX
   |
   v
Nesso
   |
   v
Display
```

---

## Tech Stack

### Firmware

- C++
- ESP32-C6
- Arduino Nesso N1
- Arduino framework
- PlatformIO
- ArduinoJson
- ArduinoMqttClient

### Gateway

- Python
- paho-mqtt
- python-miio
- python-dotenv

### Communication

- MQTT
- JSON
- Xiaomi MIoT

### Infrastructure

- EMQX
- Docker
- Docker Compose
