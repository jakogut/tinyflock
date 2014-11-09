#ifndef EVENTS_H_
#define EVENTS_H_

void callback_cursormov(GLFWwindow* window, double x, double y);

void callback_mousebtn(GLFWwindow* window, int button, int action, int mods);

void callback_keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);

void callback_windowresize(GLFWwindow* window, int width, int height);

void callback_wclose(GLFWwindow* window);

#endif
