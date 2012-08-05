#include "render.h"

void flock_render(flock* f, configuration* config, SDL_Surface* screen)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glColor3f(0.0f, 0.0f, 0.0f);

	for(int i = 0; i < config->flock.size; i++)
	{

		glBegin(GL_LINES);
			glVertex3f(f->location[i].x, f->location[i].y, 0.0f);
			glVertex3f((f->location[i].x - f->velocity[i].x), (f->location[i].y - f->velocity[i].y), 0.0f);
		glEnd();
	}

	SDL_GL_SwapBuffers();
}
