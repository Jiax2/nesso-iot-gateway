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

int batteryPercent = 0;
unsigned long lastBatteryUpdate = 0;


unsigned long lastTelemetryAt = 0;


void publishTelemetry();
void handleMqttMessages();


void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("==============================");
    Serial.println("      Nesso IoT Client");
    Serial.println("==============================");

    Serial.print("Firmware: ");
    Serial.println(Config::FIRMWARE_VERSION);

    // display
    displayManager.begin();

    //battery
    battery.begin();
    battery.enableCharge();
    batteryPercent = battery.getChargeLevel();

    // wifi
    wifiManager.begin();

    // mqtt
    mqttService.begin();
}


void loop()
{
    unsigned long now = millis(); 
    // wifi
    wifiManager.loop();

    // mqtt
    mqttService.loop(
        wifiManager.connected()
    );

    //battery
    if(now - lastBatteryUpdate >= 10000)
    {
        lastBatteryUpdate = now; 
        batteryPercent = battery.getChargeLevel(); 
    }

    // display
    displayManager.update(
        wifiManager.connected(),
        mqttService.connected(),
        wifiManager.localIP(),
        batteryPercent
    );

    // mqtt commands
    handleMqttMessages();

    // telemetry
    if (
        wifiManager.connected()
        &&
        mqttService.connected()
        &&
        now - lastTelemetryAt
            >= Config::TELEMETRY_INTERVAL_MS
    )
    {
        lastTelemetryAt = now;

        publishTelemetry();
    }

    delay(5);
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
        topic != Config::TOPIC_COMMAND
    )
    {
        return;
    }

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

    Serial.print("[Command] Action: ");
    Serial.println(action);

    JsonDocument result;

    result["request_id"] = requestId;
    result["action"] = action;

    // ping
    if (
        strcmp(action, "ping") == 0
    )
    {
        result["success"] = true;
        result["response"] = "pong";
    }
    else
    {
        result["success"] = false;

        JsonObject errorObject =
            result["error"].to<JsonObject>();

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

    Serial.print("[Command] Result: ");
    Serial.println(response);
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

    String payload;

    serializeJson(
        doc,
        payload
    );

    Serial.print("[MQTT] Telemetry: ");
    Serial.println(payload);

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