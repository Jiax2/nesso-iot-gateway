# Nesso IoT Gateway

![ESP32-C6](https://img.shields.io/badge/ESP32--C6-firmware-blue) ![MQTT](https://img.shields.io/badge/protocol-MQTT-purple) ![PlatformIO](https://img.shields.io/badge/build-PlatformIO-orange)

IoT project built around the **Arduino Nesso N1**, focused on embedded firmware, MQTT communication and device-to-server integration.

The project currently connects the Nesso N1 to an **EMQX MQTT broker running in Docker**, with bidirectional messaging, telemetry and an on-device status interface.

---

## Architecture

```text
Nesso N1
   |
   | Wi-Fi / MQTT
   v
 EMQX
   |
   v
Gateway Service
   |
   v
IoT Device
```

---

## Features

- Modular C++ firmware
- Wi-Fi connectivity
- MQTT publish / subscribe
- Automatic MQTT reconnection
- JSON telemetry
- Device availability tracking
- Bidirectional MQTT commands
- Battery monitoring
- Touchscreen status interface
- Automatic display timeout
- PlatformIO build environment
- EMQX broker with Docker

---

## Telemetry

The Nesso currently publishes:

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

## MQTT Topics

```text
nesso-iot/v1/devices/nessito/
├── availability
├── telemetry
├── command
└── command-result
```

Future device services will use their own MQTT namespace.

---

## Project Structure

```text
nesso-iot-gateway/
├── firmware/
│   └── nesso/
│       ├── platformio.ini
│       └── src/
│
├── gateway/
├── infrastructure/
├── docs/
├── scripts/
├── compose.yaml
└── README.md
```

---

## Development

### Build firmware

```bash
cd firmware/nesso
pio run
```

### Upload firmware

```bash
pio run -t upload
```

### Serial monitor

```bash
pio device monitor
```

### Start EMQX

```bash
docker compose up -d
```

---

## Roadmap

- [X] Nesso N1 firmware structure
- [X] Wi-Fi connectivity
- [X] EMQX MQTT broker
- [X] MQTT telemetry
- [X] Bidirectional MQTT communication
- [X] Touchscreen status UI
- [X] Battery monitoring
- [X] PlatformIO migration
- [X] Python MQTT gateway
- [ ] External IoT device integration
- [ ] MQTT authentication and ACLs
- [ ] TLS communication
- [ ] Persistent telemetry storage
- [ ] Monitoring dashboard
- [ ] OTA firmware updates

---

## Tech Stack

### Firmware

- C++
- ESP32-C6
- PlatformIO
- Arduino framework

### Communication

- MQTT
- ArduinoMqttClient
- JSON

### Infrastructure

- EMQX
- Docker
- Docker Compose

### Gateway

- Python planned

---

## Status

Work in progress.

The current stage focuses on the communication layer between the **Nesso N1 and the MQTT infrastructure**.

The next step is implementing the Python gateway.
