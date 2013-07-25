#include "render.h"

#include <GLFW/glfw3.h>

#include <stdio.h>

void flock_render(GLFWwindow* window, flock* f, configuration* config)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glColor3f(0.0f, 0.0f, 0.0f);

	for(int i = 0; i < config->flock.size; i++)
	{

		glBegin(GL_LINES);
			glVertex3f(f->location[i][0], f->location[i][1], 0.0f);
			glVertex3f((f->location[i][0] - f->velocity[i][0]), (f->location[i][1] - f->velocity[i][1]), 0.0f);
		glEnd();
	}

	glfwSwapBuffers(window);
}
