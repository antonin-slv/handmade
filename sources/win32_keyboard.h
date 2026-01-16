#include <Windows.h>

struct keyboard_state
{

    bool azerty = true;
    char key_up = 'Z';
    char key_down = 'S';
    char key_left = 'Q';
    char key_right = 'D';

    bool keys[256] = {0};

    bool is_shift_down() const { return keys[VK_SHIFT] || keys[VK_LSHIFT] || keys[VK_RSHIFT]; }

    bool is_ctrl_down() const { return keys[VK_CONTROL] || keys[VK_LCONTROL] || keys[VK_RCONTROL]; }

    bool is_alt_down() const { return keys[VK_MENU] || keys[VK_LMENU] || keys[VK_RMENU]; }

    bool is_escape_down() const { return keys[VK_ESCAPE]; }

    bool is_key_down(int vk_code) const { return keys[vk_code]; }

    void set_key_state(int vk_code, bool down) { keys[vk_code] = down; }
};

struct mouse_state
{

    int x = 0;
    int y = 0;

    uint32_t state = 0; // bitfield for buttons

    bool is_button_down(uint32_t button_mask) const { return (state & button_mask) != 0; }

    bool is_left_down() const { return is_button_down(MK_LBUTTON); }

    bool is_right_down() const { return is_button_down(MK_RBUTTON); }

    bool is_middle_down() const { return is_button_down(MK_MBUTTON); }

    bool is_x1_down() const { return is_button_down(MK_XBUTTON1); }

    bool is_x2_down() const { return is_button_down(MK_XBUTTON2); }

    void set_mouse(uint32_t button_mask) { state = button_mask; }
};