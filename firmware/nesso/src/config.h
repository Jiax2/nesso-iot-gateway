#pragma once

#include <Arduino.h>

namespace Config {

  // device
  constexpr char DEVICE_ID[] = "nessito";
  constexpr char FIRMWARE_VERSION[] = "0.1.0";

  // mqtt broker
  constexpr char MQTT_BROKER[] = "192.168.1.36";

  constexpr uint16_t MQTT_PORT = 1883;
  constexpr char MQTT_CLIENT_ID[] = "nessito";

  // topics
  constexpr char TOPIC_AVAILABILITY[] =
    "nesso-iot/v1/devices/nessito/availability";

  constexpr char TOPIC_TELEMETRY[] =
    "nesso-iot/v1/devices/nessito/telemetry";

  constexpr char TOPIC_COMMAND[] =
    "nesso-iot/v1/devices/nessito/command";

  constexpr char TOPIC_COMMAND_RESULT[] =
    "nesso-iot/v1/devices/nessito/command-result";

  //airfryer
  constexpr char AIRFRYER_DEVICE_ID[] = "airfryer-01";

  constexpr char AIRFRYER_TOPIC_COMMAND[] =
    "nesso-iot/v1/devices/airfryer-01/command";

  constexpr char AIRFRYER_TOPIC_COMMAND_RESULT[] =
    "nesso-iot/v1/devices/airfryer-01/command-result";

  constexpr char AIRFRYER_TOPIC_STATE[] =
    "nesso-iot/v1/devices/airfryer-01/state";

  constexpr char AIRFRYER_TOPIC_AVAILABILITY[] =
    "nesso-iot/v1/devices/airfryer-01/availability";

  // timings
  constexpr unsigned long WIFI_RETRY_MS = 8000;
  constexpr unsigned long MQTT_RETRY_MS = 6000;
  constexpr unsigned long TELEMETRY_INTERVAL_MS = 60000;
  constexpr unsigned long DISPLAY_TIMEOUT_MS = 15000;

}