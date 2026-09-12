MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_CLIENT_ID = "xiaomi-gateway"

DEVICE_ID = "airfryer-01"

BASE_TOPIC = f"nesso-iot/v1/devices/{DEVICE_ID}"

TOPIC_COMMAND = f"{BASE_TOPIC}/command"
TOPIC_COMMAND_RESULT = f"{BASE_TOPIC}/command-result"
TOPIC_STATE = f"{BASE_TOPIC}/state"
TOPIC_AVAILABILITY = f"{BASE_TOPIC}/availability"