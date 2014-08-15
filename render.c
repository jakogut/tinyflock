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
		vec2_t normalized_velocity = {f->velocity[i][0], f->velocity[i][1]};

		vec2_normalize(&normalized_velocity);
		vec2_mul_scalar(normalized_velocity, 15);

		glBegin(GL_LINES);
			glVertex2f(f->location[i][0], f->location[i][1]);
			glVertex2f((f->location[i][0] - normalized_velocity[0]), (f->location[i][1] - normalized_velocity[1]));
		glEnd();
	}

	glfwSwapBuffers(window);
}
