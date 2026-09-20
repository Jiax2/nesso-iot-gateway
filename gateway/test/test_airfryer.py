from xiaomi_airfryer import XiaomiAirFryer


airfryer = XiaomiAirFryer()

print("Current state:")
print(airfryer.get_state())

print()
print("Setting temperature:")
print(airfryer.set_temperature(180))

print()
print("Setting time:")
print(airfryer.set_time(5))

print()
print("Updated state:")
print(airfryer.get_state())