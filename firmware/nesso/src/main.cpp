#include <Arduino.h>
#include <ArduinoJson.h>
#include <Arduino_Nesso_N1.h>

#include "config.h"
#include "secrets.h"

#include "WifiManager.h"
#include "MqttService.h"
#include "DisplayManager.h"


WifiManager wifiManager(
    Secrets::WIFI_SSID,
    Secrets::WIFI_PASSWORD
);

MqttService mqttService;

DisplayManager displayManager;

NessoBattery battery;


constexpr unsigned long
AIRFRYER_POLL_INTERVAL_MS =
    5000;


int batteryPercent = 0;

unsigned long lastBatteryUpdate = 0;
unsigned long lastTelemetryAt = 0;
unsigned long lastAirFryerStateRequestAt = 0;

bool mqttWasConnected = false;

unsigned long airFryerRequestId = 0;

String serialInput = "";


void publishTelemetry();

void handleMqttMessages();

void handleSerialCommands();

void handleDisplayActions();

void processSerialCommand(
    const String& input
);

void handleNessoCommand(
    const String& payload
);

void handleAirFryerState(
    const String& payload
);

void handleAirFryerCommandResult(
    const String& payload
);

void requestAirFryerState();

void sendAirFryerStart(
    int temperature,
    int duration
);

void sendAirFryerCommand(
    const char* command
);


void setup()
{
    Serial.begin(
        115200
    );

    delay(
        1000
    );

    serialInput.reserve(
        64
    );

    Serial.println();

    Serial.println(
        "=============================="
    );

    Serial.println(
        "      Nesso IoT Client"
    );

    Serial.println(
        "=============================="
    );

    Serial.print(
        "Firmware: "
    );

    Serial.println(
        Config::FIRMWARE_VERSION
    );

    // display
    displayManager.begin();

    // battery
    battery.begin();

    battery.enableCharge();

    batteryPercent =
        battery.getChargeLevel();

    // wifi
    wifiManager.begin();

    // mqtt
    mqttService.begin();
}


void loop()
{
    unsigned long now =
        millis();

    // wifi
    wifiManager.loop();

    // mqtt
    mqttService.loop(
        wifiManager.connected()
    );

    bool mqttConnected =
        mqttService.connected();

    if (
        mqttConnected
        &&
        !mqttWasConnected
    )
    {
        Serial.println(
            "[MQTT] Connection ready"
        );

        requestAirFryerState();
    }

    mqttWasConnected =
        mqttConnected;

    // mqtt messages
    handleMqttMessages();

    // poll air fryer
    if (
        mqttConnected
        &&
        now - lastAirFryerStateRequestAt
            >= AIRFRYER_POLL_INTERVAL_MS
    )
    {
        requestAirFryerState();
    }

    // battery
    if (
        now - lastBatteryUpdate
            >= 10000
    )
    {
        lastBatteryUpdate =
            now;

        batteryPercent =
            battery.getChargeLevel();
    }

    // display
    displayManager.update(
        wifiManager.connected(),
        mqttConnected,
        wifiManager.localIP(),
        batteryPercent
    );

    // display actions
    handleDisplayActions();

    // serial
    handleSerialCommands();

    // telemetry
    if (
        wifiManager.connected()
        &&
        mqttConnected
        &&
        now - lastTelemetryAt
            >= Config::TELEMETRY_INTERVAL_MS
    )
    {
        lastTelemetryAt =
            now;

        publishTelemetry();
    }

    delay(
        5
    );
}


void handleDisplayActions()
{
    AirFryerUiAction action =
        displayManager.consumeAirFryerAction();

    switch (action)
    {
        case AirFryerUiAction::Start:
        {
            int temperature =
                displayManager.selectedTemperature();

            int duration =
                displayManager.selectedTime();

            Serial.print(
                "[Display] Start "
            );

            Serial.print(
                temperature
            );

            Serial.print(
                " C, "
            );

            Serial.print(
                duration
            );

            Serial.println(
                " min"
            );

            sendAirFryerStart(
                temperature,
                duration
            );

            break;
        }

        case AirFryerUiAction::Stop:
        {
            Serial.println(
                "[Display] Stop"
            );

            sendAirFryerCommand(
                "stop"
            );

            break;
        }

        case AirFryerUiAction::Pause:
        {
            Serial.println(
                "[Display] Pause"
            );

            sendAirFryerCommand(
                "pause"
            );

            break;
        }

        case AirFryerUiAction::Resume:
        {
            Serial.println(
                "[Display] Resume"
            );

            sendAirFryerCommand(
                "resume"
            );

            break;
        }

        case AirFryerUiAction::None:
        default:
        {
            break;
        }
    }
}


void requestAirFryerState()
{
    JsonDocument doc;

    airFryerRequestId++;

    String requestId =
        "nesso-"
        + String(
            airFryerRequestId
        );

    doc["request_id"] =
        requestId;

    doc["command"] =
        "get_state";

    String payload;

    serializeJson(
        doc,
        payload
    );

    Serial.print(
        "[AirFryer] Request: "
    );

    Serial.println(
        payload
    );

    if (
        mqttService.publishAirFryerCommand(
            payload
        )
    )
    {
        lastAirFryerStateRequestAt =
            millis();

        Serial.println(
            "[AirFryer] State requested"
        );
    }
    else
    {
        Serial.println(
            "[AirFryer] Request failed"
        );
    }
}


void sendAirFryerStart(
    int temperature,
    int duration
)
{
    if (
        temperature < 40
        ||
        temperature > 200
    )
    {
        Serial.println(
            "[AirFryer] Invalid temperature"
        );

        return;
    }

    if (
        duration < 1
        ||
        duration > 1440
    )
    {
        Serial.println(
            "[AirFryer] Invalid duration"
        );

        return;
    }

    JsonDocument doc;

    airFryerRequestId++;

    String requestId =
        "nesso-"
        + String(
            airFryerRequestId
        );

    doc["request_id"] =
        requestId;

    doc["command"] =
        "start";

    doc["temperature_c"] =
        temperature;

    doc["duration_min"] =
        duration;

    String payload;

    serializeJson(
        doc,
        payload
    );

    Serial.print(
        "[AirFryer] Sending: "
    );

    Serial.println(
        payload
    );

    if (
        !mqttService.publishAirFryerCommand(
            payload
        )
    )
    {
        Serial.println(
            "[AirFryer] Publish failed"
        );
    }
}


void sendAirFryerCommand(
    const char* command
)
{
    JsonDocument doc;

    airFryerRequestId++;

    String requestId =
        "nesso-"
        + String(
            airFryerRequestId
        );

    doc["request_id"] =
        requestId;

    doc["command"] =
        command;

    String payload;

    serializeJson(
        doc,
        payload
    );

    Serial.print(
        "[AirFryer] Sending: "
    );

    Serial.println(
        payload
    );

    if (
        !mqttService.publishAirFryerCommand(
            payload
        )
    )
    {
        Serial.println(
            "[AirFryer] Publish failed"
        );
    }
}


void handleSerialCommands()
{
    while (
        Serial.available() > 0
    )
    {
        char character =
            (char)Serial.read();

        if (
            character == '\n'
            ||
            character == '\r'
        )
        {
            if (
                serialInput.length() == 0
            )
            {
                continue;
            }

            serialInput.trim();

            Serial.print(
                "[Serial] Command: "
            );

            Serial.println(
                serialInput
            );

            processSerialCommand(
                serialInput
            );

            serialInput =
                "";

            continue;
        }

        if (
            serialInput.length() < 63
        )
        {
            serialInput +=
                character;
        }
    }
}


void processSerialCommand(
    const String& input
)
{
    if (
        input == "state"
    )
    {
        requestAirFryerState();

        return;
    }

    if (
        input == "stop"
    )
    {
        sendAirFryerCommand(
            "stop"
        );

        return;
    }

    if (
        input == "pause"
    )
    {
        sendAirFryerCommand(
            "pause"
        );

        return;
    }

    if (
        input == "resume"
    )
    {
        sendAirFryerCommand(
            "resume"
        );

        return;
    }

    int temperature = 0;
    int duration = 0;

    if (
        sscanf(
            input.c_str(),
            "start %d %d",
            &temperature,
            &duration
        ) == 2
    )
    {
        sendAirFryerStart(
            temperature,
            duration
        );

        return;
    }

    Serial.println(
        "[Serial] Unknown command"
    );

    Serial.println(
        "state"
    );

    Serial.println(
        "start <temperature> <minutes>"
    );

    Serial.println(
        "stop"
    );

    Serial.println(
        "pause"
    );

    Serial.println(
        "resume"
    );
}


void handleMqttMessages()
{
    String topic;
    String payload;

    if (
        !mqttService.readMessage(
            topic,
            payload
        )
    )
    {
        return;
    }

    if (
        topic
        == Config::TOPIC_COMMAND
    )
    {
        handleNessoCommand(
            payload
        );

        return;
    }

    if (
        topic
        == Config::AIRFRYER_TOPIC_STATE
    )
    {
        handleAirFryerState(
            payload
        );

        return;
    }

    if (
        topic
        == Config::AIRFRYER_TOPIC_COMMAND_RESULT
    )
    {
        handleAirFryerCommandResult(
            payload
        );

        return;
    }

    if (
        topic
        == Config::AIRFRYER_TOPIC_AVAILABILITY
    )
    {
        bool online =
            payload == "online";

        displayManager.setAirFryerOnline(
            online
        );

        Serial.print(
            "[AirFryer] Availability: "
        );

        Serial.println(
            payload
        );

        return;
    }
}


void handleNessoCommand(
    const String& payload
)
{
    JsonDocument command;

    DeserializationError error =
        deserializeJson(
            command,
            payload
        );

    if (error)
    {
        Serial.print(
            "[Command] Invalid JSON: "
        );

        Serial.println(
            error.c_str()
        );

        return;
    }

    const char* requestId =
        command["request_id"] | "";

    const char* action =
        command["action"] | "";

    Serial.print(
        "[Command] Action: "
    );

    Serial.println(
        action
    );

    JsonDocument result;

    result["request_id"] =
        requestId;

    result["action"] =
        action;

    // ping
    if (
        strcmp(
            action,
            "ping"
        ) == 0
    )
    {
        result["success"] =
            true;

        result["response"] =
            "pong";
    }
    else
    {
        result["success"] =
            false;

        JsonObject errorObject =
            result["error"]
                .to<JsonObject>();

        errorObject["code"] =
            "UNKNOWN_ACTION";
    }

    String response;

    serializeJson(
        result,
        response
    );

    mqttService.publishCommandResult(
        response
    );

    Serial.print(
        "[Command] Result: "
    );

    Serial.println(
        response
    );
}


void handleAirFryerState(
    const String& payload
)
{
    JsonDocument doc;

    DeserializationError error =
        deserializeJson(
            doc,
            payload
        );

    if (error)
    {
        Serial.print(
            "[AirFryer] Invalid state JSON: "
        );

        Serial.println(
            error.c_str()
        );

        return;
    }

    int status =
        doc["status"] | -1;

    int fault =
        doc["fault"] | -1;

    int temperature =
        doc["target_temperature_c"] | 0;

    int targetTime =
        doc["target_time_min"] | 0;

    int leftTime =
        doc["left_time_min"] | 0;

    displayManager.setAirFryerState(
        status,
        fault,
        temperature,
        targetTime,
        leftTime
    );

    Serial.println(
        "[AirFryer] State"
    );

    Serial.print(
        "[AirFryer] Status: "
    );

    Serial.println(
        status
    );

    Serial.print(
        "[AirFryer] Fault: "
    );

    Serial.println(
        fault
    );

    Serial.print(
        "[AirFryer] Temperature: "
    );

    Serial.print(
        temperature
    );

    Serial.println(
        " C"
    );

    Serial.print(
        "[AirFryer] Target time: "
    );

    Serial.print(
        targetTime
    );

    Serial.println(
        " min"
    );

    Serial.print(
        "[AirFryer] Left time: "
    );

    Serial.print(
        leftTime
    );

    Serial.println(
        " min"
    );
}


void handleAirFryerCommandResult(
    const String& payload
)
{
    JsonDocument doc;

    DeserializationError error =
        deserializeJson(
            doc,
            payload
        );

    if (error)
    {
        Serial.print(
            "[AirFryer] Invalid result JSON: "
        );

        Serial.println(
            error.c_str()
        );

        return;
    }

    const char* requestId =
        doc["request_id"] | "";

    const char* command =
        doc["command"] | "unknown";

    const char* status =
        doc["status"] | "unknown";

    Serial.print(
        "[AirFryer] Request "
    );

    Serial.print(
        requestId
    );

    Serial.print(
        " - "
    );

    Serial.print(
        command
    );

    Serial.print(
        ": "
    );

    Serial.println(
        status
    );

    if (
        doc["error"].is<const char*>()
    )
    {
        Serial.print(
            "[AirFryer] Error: "
        );

        Serial.println(
            doc["error"]
                .as<const char*>()
        );
    }
}


void publishTelemetry()
{
    JsonDocument doc;

    doc["device_id"] =
        Config::DEVICE_ID;

    doc["firmware_version"] =
        Config::FIRMWARE_VERSION;

    doc["uptime_s"] =
        millis() / 1000UL;

    doc["wifi_rssi_dbm"] =
        wifiManager.rssi();

    doc["free_heap_bytes"] =
        ESP.getFreeHeap();

    doc["battery_percent"] =
        batteryPercent;

    String payload;

    serializeJson(
        doc,
        payload
    );

    Serial.print(
        "[MQTT] Telemetry: "
    );

    Serial.println(
        payload
    );

    if (
        !mqttService.publishTelemetry(
            payload
        )
    )
    {
        Serial.println(
            "[MQTT] Publish failed"
        );
    }
}