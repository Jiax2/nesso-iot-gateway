import os
from pathlib import Path

from dotenv import load_dotenv
from miio.integrations.genericmiot.genericmiot import GenericMiot


ENV_PATH = Path(__file__).resolve().parent / ".env"

load_dotenv(ENV_PATH)


class XiaomiAirFryer:
    MODEL = "careli.fryer.maf07"

    def __init__(self):
        ip = os.getenv("XIAOMI_AIRFRYER_IP")
        token = os.getenv("XIAOMI_AIRFRYER_TOKEN")

        if not ip or not token:
            raise RuntimeError("Missing Xiaomi IP or token")

        self.device = GenericMiot(
            ip=ip,
            token=token,
            model=self.MODEL
        )

    def get_state(self):
        status = self.device.status()
        data = status.data

        return {
            "status": data.get("air-fryer:status"),
            "fault": data.get("air-fryer:fault"),
            "target_temperature_c": data.get(
                "air-fryer:target-temperature"
            ),
            "target_time_min": data.get(
                "air-fryer:target-time"
            ),
            "left_time_min": data.get(
                "air-fryer:left-time"
            )
        }

    def set_temperature(self, temperature_c):
        return self.device.set_property_by(
            2,
            4,
            temperature_c
        )

    def set_time(self, duration_min):
        return self.device.set_property_by(
            2,
            3,
            duration_min
        )

    def start(self):
        return self.device.call_action_by(
            2,
            1
        )

    def stop(self):
        return self.device.call_action_by(
            2,
            2
        )

    def pause(self):
        return self.device.call_action_by(
            2,
            3
        )

    def resume(self):
        return self.device.call_action_by(
            3,
            2
        )