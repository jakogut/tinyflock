#ifndef INPUT_H_
#define INPUT_H_

vec3_t cursor_pos;
void callback_mousemov(int x, int y)
{
        cursor_pos.scalars.x = x;
        cursor_pos.scalars.y = y;
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

int run;
void callback_keyboard(int button, int status)
{
        if(button == GLFW_KEY_ESC) run = 0;
}

#endif
