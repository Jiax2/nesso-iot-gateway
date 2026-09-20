#include <Wire.h>

#include "DisplayManager.h"
#include "RobotIcon.h"
#include "config.h"


namespace
{
    constexpr int SCREEN_WIDTH = 240;
    constexpr int SCREEN_HEIGHT = 135;

    constexpr int ROBOT_X = 198;
    constexpr int ROBOT_Y = 0;

    constexpr int TEMP_X = 4;
    constexpr int TEMP_Y = 42;
    constexpr int TEMP_WIDTH = 112;
    constexpr int TEMP_HEIGHT = 48;

    constexpr int TIME_X = 124;
    constexpr int TIME_Y = 42;
    constexpr int TIME_WIDTH = 112;
    constexpr int TIME_HEIGHT = 48;

    constexpr int ACTION_Y = 96;
    constexpr int ACTION_HEIGHT = 36;

    constexpr int FULL_ACTION_X = 4;
    constexpr int FULL_ACTION_WIDTH = 232;

    constexpr int LEFT_ACTION_X = 4;
    constexpr int LEFT_ACTION_WIDTH = 112;

    constexpr int RIGHT_ACTION_X = 124;
    constexpr int RIGHT_ACTION_WIDTH = 112;

    constexpr int SELECT_Y = 43;
    constexpr int SELECT_HEIGHT = 50;

    constexpr int SELECT_LEFT_X = 4;
    constexpr int SELECT_LEFT_WIDTH = 112;

    constexpr int SELECT_RIGHT_X = 124;
    constexpr int SELECT_RIGHT_WIDTH = 112;

    constexpr int BACK_X = 4;
    constexpr int BACK_Y = 99;
    constexpr int BACK_WIDTH = 232;
    constexpr int BACK_HEIGHT = 33;

    constexpr int CAT_MIN_X = 118;
    constexpr int CAT_MAX_X = 171;

    constexpr int CAT_Y = 8;

    constexpr int CAT_CLEAR_X = 114;
    constexpr int CAT_CLEAR_Y = 4;
    constexpr int CAT_CLEAR_WIDTH = 82;
    constexpr int CAT_CLEAR_HEIGHT = 28;

    constexpr unsigned long CAT_FRAME_MS = 120;
    constexpr unsigned long BUTTON_POLL_MS = 25;
}


void DisplayManager::begin()
{
    Wire.begin();

    _display.begin();

    _display.setRotation(
        1
    );

    if (!_touch.begin())
    {
        Serial.println(
            "[Display] Touch not found"
        );
    }

    _screenOn =
        true;

    _lastInteractionAt =
        millis();

    _lastCatFrameAt =
        millis();

    _lastButtonPollAt =
        millis();

    _lastKey1Pressed =
        digitalRead(KEY1) == LOW;

    _lastKey2Pressed =
        digitalRead(KEY2) == LOW;
    _needsRedraw =
        true;

    _display.fillScreen(
        TFT_BLACK
    );
}


void DisplayManager::update(
    bool wifiConnected,
    bool mqttConnected,
    const IPAddress& ip,
    int batteryPercent
)
{
    (void)ip;

    unsigned long now =
        millis();

    if (
        wifiConnected != _lastWifiConnected
        ||
        mqttConnected != _lastMqttConnected
        ||
        batteryPercent != _lastBatteryPercent
    )
    {
        _lastWifiConnected =
            wifiConnected;

        _lastMqttConnected =
            mqttConnected;

        _lastBatteryPercent =
            batteryPercent;

        _needsRedraw =
            true;
    }

    handleButtons();

    bool touched =
        _touch.isTouched();

    if (touched)
    {
        _lastInteractionAt =
            now;

        if (!_screenOn)
        {
            wake();

            _touchWasPressed =
                true;
        }
        else if (!_touchWasPressed)
        {
            int16_t x = 0;
            int16_t y = 0;

            if (
                _touch.read(
                    x,
                    y
                )
            )
            {
                _display.convertRawXY(
                    &x,
                    &y
                );

                Serial.print(
                    "[Display] Touch: "
                );

                Serial.print(
                    x
                );

                Serial.print(
                    ","
                );

                Serial.println(
                    y
                );

                handleTouch(
                    x,
                    y
                );
            }

            _touchWasPressed =
                true;
        }
    }
    else
    {
        _touchWasPressed =
            false;
    }

    if (
        _screenOn
        &&
        now - _lastInteractionAt
            >= Config::DISPLAY_TIMEOUT_MS
    )
    {
        sleep();
    }

    if (
        _screenOn
        &&
        _needsRedraw
    )
    {
        drawScreen();

        _needsRedraw =
            false;
    }

    if (
        _screenOn
        &&
        _screen == AirFryerScreen::Main
        &&
        isRunningStatus()
        &&
        now - _lastCatFrameAt
            >= CAT_FRAME_MS
    )
    {
        _lastCatFrameAt =
            now;

        updateCatAnimation();
    }
}


void DisplayManager::setAirFryerState(
    int status,
    int fault,
    int temperature,
    int targetTime,
    int leftTime
)
{
    bool firstState =
        _airFryerStatus < 0;

    bool enteringIdle =
        status == 1
        &&
        _airFryerStatus != 1;

    bool wasRunning =
        isRunningStatus();

    _airFryerStatus =
        status;

    _airFryerFault =
        fault;

    _airFryerTemperature =
        temperature;

    _airFryerTargetTime =
        targetTime;

    _airFryerLeftTime =
        leftTime;

    _airFryerOnline =
        true;

    if (
        firstState
        ||
        enteringIdle
        ||
        status != 1
        ||
        !_selectionDirty
    )
    {
        if (
            temperature >= 40
            &&
            temperature <= 200
        )
        {
            _selectedTemperature =
                temperature;
        }

        if (
            targetTime >= 1
            &&
            targetTime <= 1440
        )
        {
            _selectedTime =
                targetTime;
        }

        _selectionDirty =
            false;
    }

    if (
        status != 1
        &&
        _screen != AirFryerScreen::Main
    )
    {
        _screen =
            AirFryerScreen::Main;
    }

    if (
        !wasRunning
        &&
        isRunningStatus()
    )
    {
        _catX =
            CAT_MIN_X;

        _catDirection =
            1;

        _catFrame =
            false;

        _lastCatFrameAt =
            millis();
    }

    _needsRedraw =
        true;
}


void DisplayManager::setAirFryerOnline(
    bool online
)
{
    if (
        _airFryerOnline == online
    )
    {
        return;
    }

    _airFryerOnline =
        online;

    if (!online)
    {
        _screen =
            AirFryerScreen::Main;
    }

    _needsRedraw =
        true;
}


AirFryerUiAction
DisplayManager::consumeAirFryerAction()
{
    AirFryerUiAction action =
        _pendingAction;

    _pendingAction =
        AirFryerUiAction::None;

    return action;
}


int DisplayManager::selectedTemperature() const
{
    return _selectedTemperature;
}


int DisplayManager::selectedTime() const
{
    return _selectedTime;
}


bool DisplayManager::isRunningStatus() const
{
    return
        _airFryerStatus == 4
        ||
        _airFryerStatus == 9;
}


void DisplayManager::handleButtons()
{
    unsigned long now =
        millis();

    if (
        now - _lastButtonPollAt
            < BUTTON_POLL_MS
    )
    {
        return;
    }

    _lastButtonPollAt =
        now;

    bool key1Pressed =
        digitalRead(KEY1) == LOW;

    bool key2Pressed =
        digitalRead(KEY2) == LOW;

    if (
        key1Pressed
        &&
        !_lastKey1Pressed
    )
    {
        _lastInteractionAt =
            now;

        if (!_screenOn)
        {
            wake();
        }
        else
        {
            handleKey1();
        }
    }

    if (
        key2Pressed
        &&
        !_lastKey2Pressed
    )
    {
        _lastInteractionAt =
            now;

        if (!_screenOn)
        {
            wake();
        }
        else
        {
            handleKey2();
        }
    }

    _lastKey1Pressed =
        key1Pressed;

    _lastKey2Pressed =
        key2Pressed;
}


void DisplayManager::handleKey1()
{
    Serial.println(
        "[Display] KEY1"
    );

    if (
        _screen == AirFryerScreen::Temperature
    )
    {
        _selectedTemperature -=
            5;

        if (
            _selectedTemperature < 40
        )
        {
            _selectedTemperature =
                40;
        }

        _selectionDirty =
            true;

        _needsRedraw =
            true;

        return;
    }

    if (
        _screen == AirFryerScreen::Time
    )
    {
        _selectedTime--;

        if (
            _selectedTime < 1
        )
        {
            _selectedTime =
                1;
        }

        _selectionDirty =
            true;

        _needsRedraw =
            true;

        return;
    }

    if (!_airFryerOnline)
    {
        return;
    }

    if (
        _airFryerStatus == 1
    )
    {
        _pendingAction =
            AirFryerUiAction::Start;

        return;
    }

    if (
        isRunningStatus()
    )
    {
        _pendingAction =
            AirFryerUiAction::Pause;

        return;
    }

    _pendingAction =
        AirFryerUiAction::Resume;
}


void DisplayManager::handleKey2()
{
    Serial.println(
        "[Display] KEY2"
    );

    if (
        _screen == AirFryerScreen::Temperature
    )
    {
        _selectedTemperature +=
            5;

        if (
            _selectedTemperature > 200
        )
        {
            _selectedTemperature =
                200;
        }

        _selectionDirty =
            true;

        _needsRedraw =
            true;

        return;
    }

    if (
        _screen == AirFryerScreen::Time
    )
    {
        _selectedTime++;

        if (
            _selectedTime > 1440
        )
        {
            _selectedTime =
                1440;
        }

        _selectionDirty =
            true;

        _needsRedraw =
            true;

        return;
    }

    if (
        !_airFryerOnline
        ||
        _airFryerStatus == 1
    )
    {
        return;
    }

    _pendingAction =
        AirFryerUiAction::Stop;
}


void DisplayManager::handleTouch(
    int16_t x,
    int16_t y
)
{
    switch (_screen)
    {
        case AirFryerScreen::Temperature:
        {
            handleTemperatureTouch(
                x,
                y
            );

            break;
        }

        case AirFryerScreen::Time:
        {
            handleTimeTouch(
                x,
                y
            );

            break;
        }

        case AirFryerScreen::Main:
        default:
        {
            handleMainTouch(
                x,
                y
            );

            break;
        }
    }
}


void DisplayManager::handleMainTouch(
    int16_t x,
    int16_t y
)
{
    if (
        _airFryerStatus == 1
    )
    {
        if (
            inside(
                x,
                y,
                TEMP_X,
                TEMP_Y,
                TEMP_WIDTH,
                TEMP_HEIGHT
            )
        )
        {
            _screen =
                AirFryerScreen::Temperature;

            _needsRedraw =
                true;

            return;
        }

        if (
            inside(
                x,
                y,
                TIME_X,
                TIME_Y,
                TIME_WIDTH,
                TIME_HEIGHT
            )
        )
        {
            _screen =
                AirFryerScreen::Time;

            _needsRedraw =
                true;

            return;
        }

        if (
            _airFryerOnline
            &&
            inside(
                x,
                y,
                FULL_ACTION_X,
                ACTION_Y,
                FULL_ACTION_WIDTH,
                ACTION_HEIGHT
            )
        )
        {
            _pendingAction =
                AirFryerUiAction::Start;

            return;
        }

        return;
    }

    if (!_airFryerOnline)
    {
        return;
    }

    if (
        inside(
            x,
            y,
            LEFT_ACTION_X,
            ACTION_Y,
            LEFT_ACTION_WIDTH,
            ACTION_HEIGHT
        )
    )
    {
        if (
            isRunningStatus()
        )
        {
            _pendingAction =
                AirFryerUiAction::Pause;
        }
        else
        {
            _pendingAction =
                AirFryerUiAction::Resume;
        }

        return;
    }

    if (
        inside(
            x,
            y,
            RIGHT_ACTION_X,
            ACTION_Y,
            RIGHT_ACTION_WIDTH,
            ACTION_HEIGHT
        )
    )
    {
        _pendingAction =
            AirFryerUiAction::Stop;
    }
}


void DisplayManager::handleTemperatureTouch(
    int16_t x,
    int16_t y
)
{
    if (
        inside(
            x,
            y,
            SELECT_LEFT_X,
            SELECT_Y,
            SELECT_LEFT_WIDTH,
            SELECT_HEIGHT
        )
    )
    {
        _selectedTemperature -=
            5;

        if (
            _selectedTemperature < 40
        )
        {
            _selectedTemperature =
                40;
        }

        _selectionDirty =
            true;

        _needsRedraw =
            true;

        return;
    }

    if (
        inside(
            x,
            y,
            SELECT_RIGHT_X,
            SELECT_Y,
            SELECT_RIGHT_WIDTH,
            SELECT_HEIGHT
        )
    )
    {
        _selectedTemperature +=
            5;

        if (
            _selectedTemperature > 200
        )
        {
            _selectedTemperature =
                200;
        }

        _selectionDirty =
            true;

        _needsRedraw =
            true;

        return;
    }

    if (
        inside(
            x,
            y,
            BACK_X,
            BACK_Y,
            BACK_WIDTH,
            BACK_HEIGHT
        )
    )
    {
        _screen =
            AirFryerScreen::Main;

        _needsRedraw =
            true;
    }
}


void DisplayManager::handleTimeTouch(
    int16_t x,
    int16_t y
)
{
    if (
        inside(
            x,
            y,
            SELECT_LEFT_X,
            SELECT_Y,
            SELECT_LEFT_WIDTH,
            SELECT_HEIGHT
        )
    )
    {
        _selectedTime--;

        if (
            _selectedTime < 1
        )
        {
            _selectedTime =
                1;
        }

        _selectionDirty =
            true;

        _needsRedraw =
            true;

        return;
    }

    if (
        inside(
            x,
            y,
            SELECT_RIGHT_X,
            SELECT_Y,
            SELECT_RIGHT_WIDTH,
            SELECT_HEIGHT
        )
    )
    {
        _selectedTime++;

        if (
            _selectedTime > 1440
        )
        {
            _selectedTime =
                1440;
        }

        _selectionDirty =
            true;

        _needsRedraw =
            true;

        return;
    }

    if (
        inside(
            x,
            y,
            BACK_X,
            BACK_Y,
            BACK_WIDTH,
            BACK_HEIGHT
        )
    )
    {
        _screen =
            AirFryerScreen::Main;

        _needsRedraw =
            true;
    }
}


void DisplayManager::drawScreen()
{
    switch (_screen)
    {
        case AirFryerScreen::Temperature:
        {
            drawTemperatureScreen();

            break;
        }

        case AirFryerScreen::Time:
        {
            drawTimeScreen();

            break;
        }

        case AirFryerScreen::Main:
        default:
        {
            drawMainScreen();

            break;
        }
    }
}


void DisplayManager::drawMainScreen()
{
    _display.fillScreen(
        TFT_BLACK
    );

    // title
    _display.setTextColor(
        TFT_CYAN
    );

    _display.setTextSize(
        2
    );

    _display.setCursor(
        4,
        4
    );

    _display.print(
        "AIR FRYER"
    );

    if (!isRunningStatus()){
        drawRobotIcon();
    }

    // battery
    _display.setTextColor(
        TFT_WHITE
    );

    _display.setTextSize(
        1
    );

    _display.setCursor(
        164,
        31
    );

    _display.print(
        _lastBatteryPercent
    );

    _display.print(
        "%"
    );

    if (!_airFryerOnline)
    {
        _display.setTextColor(
            TFT_YELLOW
        );

        _display.setTextSize(
            2
        );

        _display.setCursor(
            43,
            58
        );

        _display.print(
            "OFFLINE"
        );

        return;
    }

    if (
        _airFryerFault != 0
    )
    {
        _display.setTextColor(
            TFT_RED
        );

        _display.setTextSize(
            2
        );

        _display.setCursor(
            75,
            60
        );

        _display.print(
            "FAULT"
        );

        return;
    }

    if (
        _airFryerStatus == 1
    )
    {
        _display.drawRect(
            TEMP_X,
            TEMP_Y,
            TEMP_WIDTH,
            TEMP_HEIGHT,
            TFT_CYAN
        );

        _display.drawRect(
            TIME_X,
            TIME_Y,
            TIME_WIDTH,
            TIME_HEIGHT,
            TFT_CYAN
        );

        _display.setTextSize(
            1
        );

        _display.setTextColor(
            TFT_WHITE
        );

        _display.setCursor(
            26,
            48
        );

        _display.print(
            "TEMPERATURE"
        );

        _display.setCursor(
            167,
            48
        );

        _display.print(
            "TIME"
        );

        _display.setTextSize(
            2
        );

        _display.setTextColor(
            TFT_CYAN
        );

        _display.setCursor(
            31,
            67
        );

        _display.print(
            _selectedTemperature
        );

        _display.print(
            " C"
        );

        _display.setCursor(
            151,
            67
        );

        _display.print(
            _selectedTime
        );

        _display.print(
            " min"
        );

        drawButton(
            FULL_ACTION_X,
            ACTION_Y,
            FULL_ACTION_WIDTH,
            ACTION_HEIGHT,
            "START  K1",
            TFT_GREEN
        );

        return;
    }

    if (
        isRunningStatus()
    )
    {
        drawCat(
            _catX,
            CAT_Y,
            _catDirection > 0,
            _catFrame
        );

        _display.setTextColor(
            TFT_WHITE
        );

        _display.setTextSize(
            1
        );

        _display.setCursor(
            31,
            47
        );

        _display.print(
            "TEMP"
        );

        _display.setCursor(
            150,
            47
        );

        _display.print(
            "TIME LEFT"
        );

        _display.setTextColor(
            TFT_CYAN
        );

        _display.setTextSize(
            2
        );

        _display.setCursor(
            24,
            66
        );

        _display.print(
            _airFryerTemperature
        );

        _display.print(
            " C"
        );

        _display.setCursor(
            157,
            66
        );

        _display.print(
            _airFryerLeftTime
        );

        _display.print(
            " min"
        );

        drawButton(
            LEFT_ACTION_X,
            ACTION_Y,
            LEFT_ACTION_WIDTH,
            ACTION_HEIGHT,
            "K1 PAUSE",
            TFT_YELLOW
        );

        drawButton(
            RIGHT_ACTION_X,
            ACTION_Y,
            RIGHT_ACTION_WIDTH,
            ACTION_HEIGHT,
            "K2 STOP",
            TFT_RED
        );

        return;
    }

    // paused
    _display.setTextColor(
        TFT_YELLOW
    );

    _display.setTextSize(
        1
    );

    _display.setCursor(
        95,
        39
    );

    _display.print(
        "PAUSED"
    );

    _display.setTextColor(
        TFT_WHITE
    );

    _display.setCursor(
        31,
        53
    );

    _display.print(
        "TEMP"
    );

    _display.setCursor(
        150,
        53
    );

    _display.print(
        "TIME LEFT"
    );

    _display.setTextColor(
        TFT_CYAN
    );

    _display.setTextSize(
        2
    );

    _display.setCursor(
        24,
        69
    );

    _display.print(
        _airFryerTemperature
    );

    _display.print(
        " C"
    );

    _display.setCursor(
        157,
        69
    );

    _display.print(
        _airFryerLeftTime
    );

    _display.print(
        " min"
    );

    drawButton(
        LEFT_ACTION_X,
        ACTION_Y,
        LEFT_ACTION_WIDTH,
        ACTION_HEIGHT,
        "K1 RESUME",
        TFT_GREEN
    );

    drawButton(
        RIGHT_ACTION_X,
        ACTION_Y,
        RIGHT_ACTION_WIDTH,
        ACTION_HEIGHT,
        "K2 STOP",
        TFT_RED
    );
}


void DisplayManager::drawTemperatureScreen()
{
    _display.fillScreen(
        TFT_BLACK
    );

    drawRobotIcon();

    _display.setTextColor(
        TFT_WHITE
    );

    _display.setTextSize(
        1
    );

    _display.setCursor(
        75,
        5
    );

    _display.print(
        "TEMPERATURE"
    );

    _display.setTextColor(
        TFT_CYAN
    );

    _display.setTextSize(
        3
    );

    _display.setCursor(
        62,
        20
    );

    _display.print(
        _selectedTemperature
    );

    _display.print(
        " C"
    );

    drawButton(
        SELECT_LEFT_X,
        SELECT_Y,
        SELECT_LEFT_WIDTH,
        SELECT_HEIGHT,
        "K1  -5",
        TFT_YELLOW
    );

    drawButton(
        SELECT_RIGHT_X,
        SELECT_Y,
        SELECT_RIGHT_WIDTH,
        SELECT_HEIGHT,
        "K2  +5",
        TFT_GREEN
    );

    drawButton(
        BACK_X,
        BACK_Y,
        BACK_WIDTH,
        BACK_HEIGHT,
        "BACK",
        TFT_WHITE
    );
}


void DisplayManager::drawTimeScreen()
{
    _display.fillScreen(
        TFT_BLACK
    );

    drawRobotIcon();

    _display.setTextColor(
        TFT_WHITE
    );

    _display.setTextSize(
        1
    );

    _display.setCursor(
        103,
        5
    );

    _display.print(
        "TIME"
    );

    _display.setTextColor(
        TFT_CYAN
    );

    _display.setTextSize(
        3
    );

    if (
        _selectedTime < 100
    )
    {
        _display.setCursor(
            68,
            20
        );
    }
    else
    {
        _display.setCursor(
            50,
            20
        );
    }

    _display.print(
        _selectedTime
    );

    _display.print(
        " min"
    );

    drawButton(
        SELECT_LEFT_X,
        SELECT_Y,
        SELECT_LEFT_WIDTH,
        SELECT_HEIGHT,
        "K1  -1",
        TFT_YELLOW
    );

    drawButton(
        SELECT_RIGHT_X,
        SELECT_Y,
        SELECT_RIGHT_WIDTH,
        SELECT_HEIGHT,
        "K2  +1",
        TFT_GREEN
    );

    drawButton(
        BACK_X,
        BACK_Y,
        BACK_WIDTH,
        BACK_HEIGHT,
        "BACK",
        TFT_WHITE
    );
}


void DisplayManager::drawRobotIcon()
{
    _display.drawXBitmap(
        ROBOT_X,
        ROBOT_Y,
        ROBOT_ICON_BITS,
        ROBOT_ICON_WIDTH,
        ROBOT_ICON_HEIGHT,
        TFT_CYAN
    );
}


void DisplayManager::drawCat(
    int x,
    int y,
    bool facingRight,
    bool frame
)
{
    uint16_t color =
        TFT_CYAN;

    // body
    _display.fillRect(
        x + 6,
        y + 6,
        11,
        6,
        color
    );

    if (facingRight)
    {
        // head
        _display.fillRect(
            x + 16,
            y + 3,
            7,
            8,
            color
        );

        // ears
        _display.fillRect(
            x + 17,
            y,
            2,
            4,
            color
        );

        _display.fillRect(
            x + 21,
            y,
            2,
            4,
            color
        );

        // eye
        _display.drawPixel(
            x + 21,
            y + 6,
            TFT_BLACK
        );

        // tail
        _display.drawLine(
            x + 6,
            y + 7,
            x + 2,
            y + 4,
            color
        );

        _display.drawPixel(
            x + 1,
            y + 3,
            color
        );
    }
    else
    {
        // head
        _display.fillRect(
            x,
            y + 3,
            7,
            8,
            color
        );

        // ears
        _display.fillRect(
            x,
            y,
            2,
            4,
            color
        );

        _display.fillRect(
            x + 4,
            y,
            2,
            4,
            color
        );

        // eye
        _display.drawPixel(
            x + 2,
            y + 6,
            TFT_BLACK
        );

        // tail
        _display.drawLine(
            x + 17,
            y + 7,
            x + 21,
            y + 4,
            color
        );

        _display.drawPixel(
            x + 22,
            y + 3,
            color
        );
    }

    // legs
    if (frame)
    {
        _display.fillRect(
            x + 8,
            y + 12,
            2,
            4,
            color
        );

        _display.fillRect(
            x + 14,
            y + 12,
            2,
            2,
            color
        );
    }
    else
    {
        _display.fillRect(
            x + 8,
            y + 12,
            2,
            2,
            color
        );

        _display.fillRect(
            x + 14,
            y + 12,
            2,
            4,
            color
        );
    }
}


void DisplayManager::updateCatAnimation()
{
    _display.fillRect(
        CAT_CLEAR_X,
        CAT_CLEAR_Y,
        CAT_CLEAR_WIDTH,
        CAT_CLEAR_HEIGHT,
        TFT_BLACK
    );

    _catX +=
        2 * _catDirection;

    if (
        _catX >= CAT_MAX_X
    )
    {
        _catX =
            CAT_MAX_X;

        _catDirection =
            -1;
    }
    else if (
        _catX <= CAT_MIN_X
    )
    {
        _catX =
            CAT_MIN_X;

        _catDirection =
            1;
    }

    _catFrame =
        !_catFrame;

    drawCat(
        _catX,
        CAT_Y,
        _catDirection > 0,
        _catFrame
    );
}


void DisplayManager::drawButton(
    int x,
    int y,
    int width,
    int height,
    const char* label,
    uint16_t color
)
{
    _display.drawRect(
        x,
        y,
        width,
        height,
        color
    );

    _display.drawRect(
        x + 1,
        y + 1,
        width - 2,
        height - 2,
        color
    );

    _display.setTextSize(
        1
    );

    _display.setTextColor(
        color
    );

    int textWidth =
        strlen(label) * 6;

    int textX =
        x
        + (
            width - textWidth
        ) / 2;

    int textY =
        y
        + (
            height - 8
        ) / 2;

    _display.setCursor(
        textX,
        textY
    );

    _display.print(
        label
    );
}


bool DisplayManager::inside(
    int16_t x,
    int16_t y,
    int areaX,
    int areaY,
    int areaWidth,
    int areaHeight
)
{
    return
        x >= areaX
        &&
        x < areaX + areaWidth
        &&
        y >= areaY
        &&
        y < areaY + areaHeight;
}


void DisplayManager::wake()
{
    digitalWrite(
        LCD_BACKLIGHT,
        HIGH
    );

    _screenOn =
        true;

    _needsRedraw =
        true;

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

    _screenOn =
        false;

    Serial.println(
        "[Display] Sleep"
    );
}