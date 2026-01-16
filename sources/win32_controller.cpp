#include <Xinput.h>


class Win32ControllerInput
{

private:
    XINPUT_GAMEPAD *pad;

public:
    Win32ControllerInput(XINPUT_GAMEPAD &gamepad)
    {
        pad = &gamepad; }

    bool up() const { return (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0; }
    bool down() const { return (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0; }
    bool left() const { return (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0; }
    bool right() const { return (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0; }

    bool start() const { return (pad->wButtons & XINPUT_GAMEPAD_START) != 0; }
    bool back() const { return (pad->wButtons & XINPUT_GAMEPAD_BACK) != 0; }
    bool leftThumb() const { return (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0; }
    bool rightThumb() const { return (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0; }

    bool leftShoulder() const { return (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0; }
    bool rightShoulder() const { return (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0; }

    bool aButton() const { return (pad->wButtons & XINPUT_GAMEPAD_A) != 0; }
    bool bButton() const { return (pad->wButtons & XINPUT_GAMEPAD_B) != 0; }
    bool xButton() const { return (pad->wButtons & XINPUT_GAMEPAD_X) != 0; }
    bool yButton() const { return (pad->wButtons & XINPUT_GAMEPAD_Y) != 0; }
};