#include "MqttService.h"
#include "config.h"


MqttService::MqttService()
    : _mqttClient(_networkClient),
      _lastReconnectAttempt(0),
      _messageAvailable(false)
{
}


void MqttService::begin()
{
    Serial.println("[MQTT] Initializing...");

    _mqttClient.setId(
        Config::MQTT_CLIENT_ID
    );

    _mqttClient.setKeepAliveInterval(
        60000
    );

    _mqttClient.setConnectionTimeout(
        5000
    );
}


void MqttService::loop(
    bool networkAvailable
)
{
    if (!networkAvailable)
    {
        return;
    }

    if (_mqttClient.connected())
    {
        handleIncomingMessage();

        return;
    }

    unsigned long now = millis();

    if (
        _lastReconnectAttempt == 0
        ||
        now - _lastReconnectAttempt
            >= Config::MQTT_RETRY_MS
    )
    {
        _lastReconnectAttempt = now;

        connect();
    }
}


void MqttService::connect()
{
    Serial.print("[MQTT] Connecting to ");
    Serial.print(Config::MQTT_BROKER);
    Serial.print(":");
    Serial.println(Config::MQTT_PORT);

    if (
        !_mqttClient.connect(
            Config::MQTT_BROKER,
            Config::MQTT_PORT
        )
    )
    {
        Serial.print(
            "[MQTT] Connection failed. Error: "
        );

        Serial.println(
            _mqttClient.connectError()
        );

        return;
    }

    Serial.println("[MQTT] Connected");

    publish(
        Config::TOPIC_AVAILABILITY,
        "online",
        true,
        1
    );

    // Nesso commands
    if (
        _mqttClient.subscribe(
            Config::TOPIC_COMMAND,
            1
        )
    )
    {
        Serial.print(
            "[MQTT] Subscribed to "
        );

        Serial.println(
            Config::TOPIC_COMMAND
        );
    }
    else
    {
        Serial.println(
            "[MQTT] Subscribe failed"
        );
    }

    // Air fryer state
    if (
        _mqttClient.subscribe(
            Config::AIRFRYER_TOPIC_STATE,
            1
        )
    )
    {
        Serial.print(
            "[MQTT] Subscribed to "
        );

        Serial.println(
            Config::AIRFRYER_TOPIC_STATE
        );
    }
    else
    {
        Serial.println(
            "[MQTT] Air fryer state subscribe failed"
        );
    }

    // Air fryer result
    if (
        _mqttClient.subscribe(
            Config::AIRFRYER_TOPIC_COMMAND_RESULT,
            1
        )
    )
    {
        Serial.print(
            "[MQTT] Subscribed to "
        );

        Serial.println(
            Config::AIRFRYER_TOPIC_COMMAND_RESULT
        );
    }
    else
    {
        Serial.println(
            "[MQTT] Air fryer result subscribe failed"
        );
    }

    // Air fryer availability
    if (
        _mqttClient.subscribe(
            Config::AIRFRYER_TOPIC_AVAILABILITY,
            1
        )
    )
    {
        Serial.print(
            "[MQTT] Subscribed to "
        );

        Serial.println(
            Config::AIRFRYER_TOPIC_AVAILABILITY
        );
    }
    else
    {
        Serial.println(
            "[MQTT] Air fryer availability subscribe failed"
        );
    }
}


void MqttService::handleIncomingMessage()
{
    int messageSize =
        _mqttClient.parseMessage();

    if (!messageSize)
    {
        return;
    }

    _messageTopic =
        _mqttClient.messageTopic();

    _messagePayload = "";

    while (_mqttClient.available())
    {
        _messagePayload +=
            (char)_mqttClient.read();
    }

    _messageAvailable = true;

    Serial.print(
        "[MQTT] Message received: "
    );

    Serial.println(
        _messageTopic
    );

    Serial.print(
        "[MQTT] Payload: "
    );

    Serial.println(
        _messagePayload
    );
}


bool MqttService::readMessage(
    String& topic,
    String& payload
)
{
    if (!_messageAvailable)
    {
        return false;
    }

    topic =
        _messageTopic;

    payload =
        _messagePayload;

    _messageAvailable = false;

    return true;
}


bool MqttService::publishTelemetry(
    const String& payload
)
{
    return publish(
        Config::TOPIC_TELEMETRY,
        payload,
        false,
        0
    );
}


bool MqttService::publishCommandResult(
    const String& payload
)
{
    return publish(
        Config::TOPIC_COMMAND_RESULT,
        payload,
        false,
        1
    );
}


bool MqttService::publishAirFryerCommand(
    const String& payload
)
{
    return publish(
        Config::AIRFRYER_TOPIC_COMMAND,
        payload,
        false,
        1
    );
}


bool MqttService::publish(
    const char* topic,
    const String& payload,
    bool retained,
    uint8_t qos
)
{
    if (!_mqttClient.connected())
    {
        return false;
    }

    bool success =
        _mqttClient.beginMessage(
            topic,
            payload.length(),
            retained,
            qos
        );

    if (!success)
    {
        return false;
    }

    _mqttClient.print(payload);

    return _mqttClient.endMessage();
}


bool MqttService::connected()
{
    return _mqttClient.connected();
}