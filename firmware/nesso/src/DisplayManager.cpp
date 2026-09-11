#include <Wire.h>
#include "DisplayManager.h"
#include "RobotIcon.h"
#include "config.h"


void DisplayManager::begin()
{
    Wire.begin();

    _display.begin();
    _display.setRotation(1);

    if (!_touch.begin())
    {
        Serial.println("[Display] Touch not found");
    }

    _screenOn = true;
    _lastInteractionAt = millis();

    _display.fillScreen(TFT_BLACK);

    _display.setTextColor(TFT_CYAN);
    _display.setTextSize(2);

    _display.setCursor(10, 10);
    _display.print("NESSITO");

    drawRobotIcon();

    _display.setTextColor(TFT_WHITE);
    _display.setTextSize(1);

    _display.setCursor(10, 45);
    _display.print("Starting...");
}


void DisplayManager::update(
    bool wifiConnected,
    bool mqttConnected,
    const IPAddress& ip,
    int batteryPercent
)
{
    unsigned long now = millis();

    // touch
    if (_touch.isTouched())
    {
        _lastInteractionAt = now;

        if (!_screenOn)
        {
            wake();

            drawStatus(
                wifiConnected,
                mqttConnected,
                ip,
                batteryPercent
            );
        }
    }

    // screen timeout
    if (
        _screenOn
        &&
        now - _lastInteractionAt >=
            Config::DISPLAY_TIMEOUT_MS
    )
    {
        sleep();
    }

    // update status
    if (
        !_hasRenderedStatus
        ||
        wifiConnected != _lastWifiConnected
        ||
        mqttConnected != _lastMqttConnected
        ||
        batteryPercent != _lastBatteryPercent
    )
    {
        _lastWifiConnected = wifiConnected;
        _lastMqttConnected = mqttConnected;
        _hasRenderedStatus = true;
        _lastBatteryPercent = batteryPercent; 

        if (_screenOn)
        {
            drawStatus(
                wifiConnected,
                mqttConnected,
                ip,
                batteryPercent
            );
        }
    }
}


void DisplayManager::wake()
{
    digitalWrite(
        LCD_BACKLIGHT,
        HIGH
    );

    _screenOn = true;

    Serial.println(
        "[Display] Wake"
    );
}


void DisplayManager::sleep()
{
    digitalWrite(
        LCD_BACKLIGHT,
        LOW
    );

    _screenOn = false;

    Serial.println(
        "[Display] Sleep"
    );
}


void DisplayManager::drawStatus(
    bool wifiConnected,
    bool mqttConnected,
    const IPAddress& ip, 
    int batteryPercent
)
{
    _display.fillScreen(TFT_BLACK);

    // title
    _display.setTextColor(TFT_CYAN);
    _display.setTextSize(2);

    _display.setCursor(10, 10);
    _display.print("NESSITO");

    // robot
    drawRobotIcon();

    // wifi
    _display.setTextSize(1);
    _display.setTextColor(TFT_WHITE);

    _display.setCursor(10, 45);
    _display.print("WiFi:");

    _display.setCursor(70, 45);

    if (wifiConnected)
    {
        _display.setTextColor(TFT_GREEN);
        _display.print("CONNECTED");
    }
    else
    {
        _display.setTextColor(TFT_YELLOW);
        _display.print("CONNECTING");
    }

    // mqtt
    _display.setTextColor(TFT_WHITE);

    _display.setCursor(10, 70);
    _display.print("MQTT:");

    _display.setCursor(70, 70);

    if (mqttConnected)
    {
        _display.setTextColor(TFT_GREEN);
        _display.print("CONNECTED");
    }
    else
    {
        _display.setTextColor(TFT_YELLOW);
        _display.print("WAITING");
    }

    // ip
    if (wifiConnected)
    {
        _display.setTextColor(TFT_WHITE);

        _display.setCursor(10, 100);
        _display.print("IP:");

        _display.setTextColor(TFT_CYAN);

        _display.setCursor(40, 100);
        _display.print(ip.toString());
    }

    //battery
    // battery
    _display.setTextColor(TFT_WHITE);

    _display.setCursor(180, 110);
    _display.print("BAT:");

    _display.setTextColor(TFT_GREEN);

    _display.print(batteryPercent);
    _display.print("%");
}


void DisplayManager::drawRobotIcon()
{
    constexpr int x = 190;
    constexpr int y = 3;

    constexpr uint16_t color = 0x424B;

    _display.drawXBitmap(
        x,
        y,
        ROBOT_ICON_BITS,
        ROBOT_ICON_WIDTH,
        ROBOT_ICON_HEIGHT,
        color
    );
}