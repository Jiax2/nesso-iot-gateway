import json
import os

import paho.mqtt.client as mqtt
from dotenv import load_dotenv

from xiaomi_airfryer import XiaomiAirFryer


load_dotenv()


MQTT_BROKER = os.getenv("MQTT_BROKER", "localhost")
MQTT_PORT = int(os.getenv("MQTT_PORT", "1883"))

MQTT_CLIENT_ID = "xiaomi-gateway"

DEVICE_ID = "airfryer-01"

BASE_TOPIC = f"nesso-iot/v1/devices/{DEVICE_ID}"

TOPIC_COMMAND = f"{BASE_TOPIC}/command"
TOPIC_COMMAND_RESULT = f"{BASE_TOPIC}/command-result"
TOPIC_STATE = f"{BASE_TOPIC}/state"
TOPIC_AVAILABILITY = f"{BASE_TOPIC}/availability"


airfryer = XiaomiAirFryer()


def publish_json(client, topic, payload, retain=False):
    message = json.dumps(payload)

    client.publish(
        topic,
        message,
        qos=1,
        retain=retain
    )


def publish_result(
    client,
    request_id,
    command,
    status,
    error=None
):
    payload = {
        "request_id": request_id,
        "command": command,
        "status": status
    }

    if error is not None:
        payload["error"] = error

    publish_json(
        client,
        TOPIC_COMMAND_RESULT,
        payload
    )


def publish_state(client):
    try:
        state = airfryer.get_state()

        payload = {
            "device_id": DEVICE_ID,
            **state
        }

        publish_json(
            client,
            TOPIC_STATE,
            payload,
            retain=True
        )

    except Exception as error:
        print(f"State error: {error}")


def handle_start(client, payload):
    request_id = payload.get("request_id")

    temperature_c = payload.get("temperature_c")
    duration_min = payload.get("duration_min")

    if temperature_c is None:
        publish_result(
            client,
            request_id,
            "start",
            "error",
            "temperature_c is required"
        )
        return

    if duration_min is None:
        publish_result(
            client,
            request_id,
            "start",
            "error",
            "duration_min is required"
        )
        return

    if not 40 <= temperature_c <= 200:
        publish_result(
            client,
            request_id,
            "start",
            "error",
            "temperature_c must be between 40 and 200"
        )
        return

    if not 1 <= duration_min <= 1440:
        publish_result(
            client,
            request_id,
            "start",
            "error",
            "duration_min must be between 1 and 1440"
        )
        return

    try:
        print(
            f"Starting air fryer: "
            f"{temperature_c} C, "
            f"{duration_min} min"
        )

        airfryer.set_temperature(
            temperature_c
        )

        airfryer.set_time(
            duration_min
        )

        airfryer.start()

        publish_result(
            client,
            request_id,
            "start",
            "ok"
        )

        publish_state(client)

    except Exception as error:
        print(f"Start error: {error}")

        publish_result(
            client,
            request_id,
            "start",
            "error",
            str(error)
        )


def handle_stop(client, payload):
    request_id = payload.get("request_id")

    try:
        print("Stopping air fryer")

        airfryer.stop()

        publish_result(
            client,
            request_id,
            "stop",
            "ok"
        )

        publish_state(client)

    except Exception as error:
        print(f"Stop error: {error}")

        publish_result(
            client,
            request_id,
            "stop",
            "error",
            str(error)
        )


def handle_pause(client, payload):
    request_id = payload.get("request_id")

    try:
        print("Pausing air fryer")

        airfryer.pause()

        publish_result(
            client,
            request_id,
            "pause",
            "ok"
        )

        publish_state(client)

    except Exception as error:
        print(f"Pause error: {error}")

        publish_result(
            client,
            request_id,
            "pause",
            "error",
            str(error)
        )


def handle_resume(client, payload):
    request_id = payload.get("request_id")

    try:
        print("Resuming air fryer")

        airfryer.resume()

        publish_result(
            client,
            request_id,
            "resume",
            "ok"
        )

        publish_state(client)

    except Exception as error:
        print(f"Resume error: {error}")

        publish_result(
            client,
            request_id,
            "resume",
            "error",
            str(error)
        )


def handle_get_state(client, payload):
    request_id = payload.get("request_id")

    try:
        publish_state(client)

        publish_result(
            client,
            request_id,
            "get_state",
            "ok"
        )

    except Exception as error:
        publish_result(
            client,
            request_id,
            "get_state",
            "error",
            str(error)
        )


def handle_command(client, payload):
    command = payload.get("command")

    if command == "start":
        handle_start(client, payload)
        return

    if command == "stop":
        handle_stop(client, payload)
        return

    if command == "pause":
        handle_pause(client, payload)
        return

    if command == "resume":
        handle_resume(client, payload)
        return

    if command == "get_state":
        handle_get_state(client, payload)
        return

    publish_result(
        client,
        payload.get("request_id"),
        command,
        "error",
        "unknown command"
    )


def on_connect(
    client,
    userdata,
    flags,
    reason_code,
    properties
):
    if reason_code != 0:
        print(
            f"MQTT connection failed: "
            f"{reason_code}"
        )
        return

    print("Connected to MQTT broker")

    client.subscribe(
        TOPIC_COMMAND,
        qos=1
    )

    client.publish(
        TOPIC_AVAILABILITY,
        "online",
        qos=1,
        retain=True
    )

    publish_state(client)

    print(
        f"Subscribed to {TOPIC_COMMAND}"
    )


def on_message(
    client,
    userdata,
    message
):
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

    except Exception as error:
        print(
            f"Command error: {error}"
        )


def main():
    client = mqtt.Client(
        mqtt.CallbackAPIVersion.VERSION2,
        client_id=MQTT_CLIENT_ID
    )

    client.on_connect = on_connect
    client.on_message = on_message

    client.will_set(
        TOPIC_AVAILABILITY,
        "offline",
        qos=1,
        retain=True
    )

    print(
        f"Connecting to MQTT broker "
        f"{MQTT_BROKER}:{MQTT_PORT}"
    )

    try:
        client.connect(
            MQTT_BROKER,
            MQTT_PORT,
            keepalive=60
        )

        client.loop_forever()

    except KeyboardInterrupt:
        print()
        print("Stopping gateway")

        client.publish(
            TOPIC_AVAILABILITY,
            "offline",
            qos=1,
            retain=True
        )

        client.disconnect()


if __name__ == "__main__":
    main()