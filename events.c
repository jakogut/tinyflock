#include <GLFW/glfw3.h>

#include "VLIQ/vliq.h"

#include "events.h"
#include "flock.h"

extern int run;
extern vec2_t cursor_pos;
extern int cursor_interaction;
extern flock* flock_ptr;
extern configuration* config;

void callback_cursormov(GLFWwindow* window, double x, double y)
{
        cursor_pos[0] = x;
        cursor_pos[1] = y;
}

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

void callback_keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
        if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) run = 0;
	else if(key == GLFW_KEY_R && action == GLFW_PRESS) flock_randomize_acceleration(flock_ptr, config);
}

extern void init_gl(int width, int height);
void callback_windowresize(GLFWwindow* window, int width, int height)
{
	config->video.screen_width  = width;
	config->video.screen_height = height;
	init_gl(width, height);
}

void callback_wclose(GLFWwindow* window)
{
	run = 0;
}
