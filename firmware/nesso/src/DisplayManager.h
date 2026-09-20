#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <Arduino_Nesso_N1.h>


enum class AirFryerUiAction
{
    None,
    Start,
    Stop,
    Pause,
    Resume
};


enum class AirFryerScreen
{
    Main,
    Temperature,
    Time
};


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

    void setAirFryerState(
        int status,
        int fault,
        int temperature,
        int targetTime,
        int leftTime
    );

    void setAirFryerOnline(
        bool online
    );

    AirFryerUiAction consumeAirFryerAction();

    int selectedTemperature() const;

    int selectedTime() const;


private:

    NessoDisplay _display;
    NessoTouch _touch;

    bool _lastWifiConnected = false;
    bool _lastMqttConnected = false;

    int _lastBatteryPercent = -1;

    bool _airFryerOnline = false;

    int _airFryerStatus = -1;
    int _airFryerFault = 0;

    int _airFryerTemperature = 180;
    int _airFryerTargetTime = 15;
    int _airFryerLeftTime = 0;

    int _selectedTemperature = 180;
    int _selectedTime = 15;

    bool _selectionDirty = false;

    AirFryerUiAction _pendingAction =
        AirFryerUiAction::None;

    AirFryerScreen _screen =
        AirFryerScreen::Main;

    bool _screenOn = true;
    bool _touchWasPressed = false;
    bool _needsRedraw = true;

    bool _lastKey1Pressed = false;
    bool _lastKey2Pressed = false;

    unsigned long _lastInteractionAt = 0;
    unsigned long _lastButtonPollAt = 0;

    int _catX = 118;
    int _catDirection = 1;

    bool _catFrame = false;

    unsigned long _lastCatFrameAt = 0;


    void drawScreen();

    void drawMainScreen();

    void drawTemperatureScreen();

    void drawTimeScreen();

    void drawButton(
        int x,
        int y,
        int width,
        int height,
        const char* label,
        uint16_t color
    );

    void drawRobotIcon();

    void drawCat(
        int x,
        int y,
        bool facingRight,
        bool frame
    );

    void updateCatAnimation();

    void handleTouch(
        int16_t x,
        int16_t y
    );

    void handleMainTouch(
        int16_t x,
        int16_t y
    );

    void handleTemperatureTouch(
        int16_t x,
        int16_t y
    );

    void handleTimeTouch(
        int16_t x,
        int16_t y
    );

    void handleButtons();

    void handleKey1();

    void handleKey2();

    bool isRunningStatus() const;

    bool inside(
        int16_t x,
        int16_t y,
        int areaX,
        int areaY,
        int areaWidth,
        int areaHeight
    );

    void wake();

    void sleep();
};