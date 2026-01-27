#include <cstdint>

struct keyboard_state
{

    bool azerty = true;
    char key_up = 'Z';
    char key_down = 'S';
    char key_left = 'Q';
    char key_right = 'D';

    bool keys[256] = {0};

    bool is_shift_down() const { return keys[0x10] || keys[0xA0] || keys[0xA1]; }

    bool is_ctrl_down() const { return keys[0x11] || keys[0xA2] || keys[0xA3]; }

    bool is_alt_down() const { return keys[0x12] || keys[0xA4] || keys[0xA5]; }
    bool is_escape_down() const { return keys[0x1B]; }

    bool is_key_down(int vk_code) const { return keys[vk_code]; }

    void set_key_state(int vk_code, bool down) { keys[vk_code] = down; }
};


enum MouseButtonMasks
{
    MouseButton_Left = 0x1,
    MouseButton_Right = 0x2,
    MouseButton_Middle = 0x4,
    MouseButton_X1 = 0x8,
    MouseButton_X2 = 0x10
};

struct mouse_state
{

    int x = 0;
    int y = 0;

    int last_x = 0;
    int last_y = 0;

    int wheel_delta = 0;


    uint8_t state = 0;


    void SetMouseStateWindows(uint32_t button_mask)
    {
        state = 0;
        if (button_mask & 0x0001)
            state |= MouseButton_Left;
        if (button_mask & 0x0002)
            state |= MouseButton_Right;
        if (button_mask & 0x0010)
            state |= MouseButton_Middle;
        if (button_mask & 0x0020)
            state |= MouseButton_X1;
        if (button_mask & 0x0040)
            state |= MouseButton_X2;
    }

    bool is_button_down(uint8_t button_mask) const { return (state & button_mask) != 0; }

    bool is_left_down() const { return is_button_down(MouseButton_Left); }

    bool is_right_down() const { return is_button_down(MouseButton_Right); }

    bool is_middle_down() const { return is_button_down(MouseButton_Middle); }
    bool is_x1_down() const { return is_button_down(MouseButton_X1); }

    bool is_x2_down() const { return is_button_down(MouseButton_X2); }

};