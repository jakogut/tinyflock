#ifndef EVENTS_H_
#define EVENTS_H_

vec2_t cursor_pos;
void callback_cursormov(GLFWwindow* window, double x, double y)
{
        cursor_pos[0] = x;
        cursor_pos[1] = y;
}

int cursor_interaction;
void callback_mousebtn(GLFWwindow* window, int button, int action, int mods)
{
        if(action == GLFW_RELEASE)
        {
                cursor_interaction = 0;
                return;
        }
        else if(action == GLFW_PRESS)
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
void callback_keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
        if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) run = 0;
}

void callback_wclose(GLFWwindow* window)
{
	run = 0;
}

#endif
