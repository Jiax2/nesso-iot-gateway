#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoMqttClient.h>


class MqttService
{
public:
    MqttService();

    void begin();

    void loop(
        bool networkAvailable
    );

    bool connected();

    bool readMessage(
        String& topic,
        String& payload
    );

    bool publishTelemetry(
        const String& payload
    );

    bool publishCommandResult(
        const String& payload
    );

    bool publishAirFryerCommand(
        const String& payload
    );

private:
    WiFiClient _networkClient;

    MqttClient _mqttClient;

    unsigned long _lastReconnectAttempt;

    bool _messageAvailable;

    String _messageTopic;

    String _messagePayload;


    void connect();

    void handleIncomingMessage();

    bool publish(
        const char* topic,
        const String& payload,
        bool retained,
        uint8_t qos
    );
};