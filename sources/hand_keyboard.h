#ifndef HAND_KEYBOARD_H
#define HAND_KEYBOARD_H

#include <cstdint>

enum standard_keys
{
    Key_Up = 0x26,
    Key_Down = 0x28,
    Key_Left = 0x25,
    Key_Right = 0x27,
    Key_Escape = 0x1B,
    Key_Space = 0x20,
    Key_Enter = 0x0D,
    Key_Shift = 0x10,
    Key_Ctrl = 0x11,
    Key_Alt = 0x12,
};

struct button_state
{
    bool started_down = false;
    bool ended_down = false;
    uint8_t half_transition_count = 0;
};

struct keyboard_state
{

    bool azerty = true;
    char key_up = 'Z';
    char key_down = 'S';
    char key_left = 'Q';
    char key_right = 'D';

    button_state keys[256];

    bool is_shift_down() const { return keys[Key_Shift].ended_down; }

    bool is_ctrl_down() const { return keys[Key_Ctrl].ended_down ; }

    bool is_alt_down() const { return keys[Key_Alt].ended_down; }
    bool is_escape_down() const { return keys[Key_Escape].ended_down; }

    bool is_key_down(int vk_code) const { return keys[vk_code].ended_down; }
    bool was_key_pressed(int vk_code) const { return keys[vk_code].ended_down && (keys[vk_code].half_transition_count > 0); }
    bool was_key_released(int vk_code) const { return !keys[vk_code].ended_down && (keys[vk_code].half_transition_count > 0); }

    void reset_on_end_frame()
    {
        for (int i = 0; i < 256; ++i)
        {
            keys[i].half_transition_count = 0;
            keys[i].started_down = keys[i].ended_down;
        }
    }
};

enum MouseButtonIndex : uint8_t
{
    MouseButton_Left = 0,
    MouseButton_Right = 1,
    MouseButton_Middle = 2,
    MouseButton_X1 = 3,
    MouseButton_X2 = 4
};

struct mouse_state
{

    int x = 0;
    int y = 0;

    int last_x = 0;
    int last_y = 0;

    int wheel_delta = 0;
    union
    {
        button_state buttons[5];
        struct
        {
            button_state left;
            button_state right;
            button_state middle;
            button_state x1;
            button_state x2;
        };
    };

    mouse_state()
    {
        for (int i = 0; i < 5; ++i)
        {
            buttons[i] = button_state();
        }
    }
    
    void onButtonAction(uint8_t buttonIndex, bool isPressed)
    {
        button_state &button = buttons[buttonIndex];
        button.half_transition_count += 1;
        button.ended_down = isPressed;
    }

    void resetOnEndFrame()
    {
        wheel_delta = 0;
        last_x = x;
        last_y = y;
        for (int i = 0; i < 5; ++i)
        {
            buttons[i].half_transition_count = 0;
            buttons[i].started_down = buttons[i].ended_down;
        }
    }
};

struct unified_input
{
    keyboard_state Keyboard;
    mouse_state Mouse;

    void resetOnEndFrame()
    {
        Keyboard.reset_on_end_frame();
        Mouse.resetOnEndFrame();
    }
};

#endif // HAND_KEYBOARD_H