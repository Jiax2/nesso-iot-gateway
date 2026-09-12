import json

import paho.mqtt.client as mqtt

import config
from airfryer import Airfryer

airfryer = Airfryer()

def publish_json(client, topic, payload, retain=False): 
    message = json.dumps(payload)
    client.publish(topic, message, qos=1, retain=retain)

def publish_state(client): 
    state = airfryer.get_state()
    payload = {
        "device_id": config.DEVICE_ID,
        **state
    }

    publish_json(client, config.TOPIC_STATE, payload, retain=True)

def publish_result(client, request_id, command, status, error=None): 
    payload = {
        "device_id": request_id,
        "command": command,
        "status": status,
    }
    if error is not None:
        payload["error"] = error

    publish_json(client, config.TOPIC_RESULT, payload)

def handle_command(client, payload): 
    request_id = payload.get("request_id")
    command = payload.get("command")

    if command == "start":
        temperature_c = payload.get("temperature_c")
        duration_min = payload.get("duration_min")

        if temperature_c is None or duration_min is None:
            publish_result(
                client,
                request_id,
                command,
                "error",
                "temperature_c and duration_min are required"
            )
            return

        airfryer.start(
            temperature_c,
            duration_min
        )

        publish_result(
            client,
            request_id,
            command,
            "ok"
        )

        publish_state(client)
        return

    if command == "stop":
        airfryer.stop()

        publish_result(
            client,
            request_id,
            command,
            "ok"
        )

        publish_state(client)
        return

    if command == "get_state":
        publish_result(
            client,
            request_id,
            command,
            "ok"
        )

        publish_state(client)
        return

    publish_result(
        client,
        request_id,
        command,
        "error",
        "unknown command"
    )

def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code != 0:
        print(f"MQTT connection failed: {reason_code}")
        return

    print("Connected to MQTT broker")

    client.subscribe(
        config.TOPIC_COMMAND,
        qos=1
    )

    client.publish(
        config.TOPIC_AVAILABILITY,
        "online",
        qos=1,
        retain=True
    )

    publish_state(client)

    print(f"Subscribed to {config.TOPIC_COMMAND}")

def on_message(client, userdata, message):
    try:
        payload = json.loads(
            message.payload.decode("utf-8")
        )

        print(
            f"Command received: {payload}"
        )

        handle_command(
            client,
            payload
        )

    except json.JSONDecodeError:
        print("Invalid JSON received")

        publish_result(
            client,
            None,
            None,
            "error",
            "invalid JSON"
        )

def main():
    client = mqtt.Client(
        mqtt.CallbackAPIVersion.VERSION2,
        client_id=config.MQTT_CLIENT_ID
    )

    client.on_connect = on_connect
    client.on_message = on_message

    client.will_set(
        config.TOPIC_AVAILABILITY,
        "offline",
        qos=1,
        retain=True
    )

    print(
        f"Connecting to MQTT broker "
        f"{config.MQTT_BROKER}:{config.MQTT_PORT}"
    )

    try:
        client.connect(
            config.MQTT_BROKER,
            config.MQTT_PORT,
            keepalive=60
        )

        client.loop_forever()

    except KeyboardInterrupt:
        print("\nStopping gateway")

        client.publish(
            config.TOPIC_AVAILABILITY,
            "offline",
            qos=1,
            retain=True
        )

        client.disconnect()


if __name__ == "__main__":
    main()