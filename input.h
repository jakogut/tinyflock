#ifndef INPUT_H_
#define INPUT_H_

vec2_t cursor_pos;
void callback_mousemov(int x, int y)
{
        cursor_pos[0] = x;
        cursor_pos[1] = y;
}

int cursor_interaction;
void callback_mousebtn(int button, int status)
{
        if(status == GLFW_RELEASE)
        {
                cursor_interaction = 0;
                return;
        }
        else if(status == GLFW_PRESS)
        {
                switch(button)
                {
                        case GLFW_MOUSE_BUTTON_LEFT:
                                cursor_interaction = 1;
                        break;
                        case GLFW_MOUSE_BUTTON_RIGHT:
                                cursor_interaction = 2;
                        break;
                        default:
                                cursor_interaction = 0;
                        break;
                };
        }
}

// Dirty fucking hack, fix later
int run = 1;
void callback_keyboard(int button, int status)
{
        if(button == GLFW_KEY_ESC) run = 0;
}

#endif
