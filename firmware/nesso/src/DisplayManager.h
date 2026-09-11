#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <Arduino_Nesso_N1.h>

class DisplayManager
{
public:

    void begin();

    void update(
        bool wifiConnected,
        bool mqttConnected,
        const IPAddress& ip,
        int batteryPercent
    );

private:

    NessoDisplay _display;
    NessoTouch _touch;

    bool _lastWifiConnected = false;
    bool _lastMqttConnected = false;
    int _lastBatteryPercent = -1; 
    bool _hasRenderedStatus = false;

    bool _screenOn = true;
    unsigned long _lastInteractionAt = 0;

    void drawStatus(
        bool wifiConnected,
        bool mqttConnected,
        const IPAddress& ip,
        int batteryPercent
    );

    void drawRobotIcon();

    void wake();
    void sleep();
};