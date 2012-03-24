#include "render.h"

void flock_render(boid* flock, configuration* config, SDL_Surface* screen)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int i;
	for(i = 0; i < config->flock.size; i++)
	{
		glColor3f(0.0f, 0.0f, 0.0f);

		glBegin(GL_LINES);
			glVertex3f(flock[i].location.x, flock[i].location.y, 0.0f);
			glVertex3f((flock[i].location.x - flock[i].velocity.x), (flock[i].location.y - flock[i].velocity.y), 0.0f);
		glEnd();

		glLoadIdentity();
	}

	SDL_GL_SwapBuffers();
}
