#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL_image.h>

#include <GL/gl.h>
#include <GL/glu.h>


#include "flock.h"
#include "boid.h"

#include "configuration.h"

// Returns true if key pressed, false otherwise
int key_pressed(SDL_Event* event, int key)
{
	return (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE);
}

// Handle input and events. Return 0 to quit the program, or 1 to keep running.
inline int handle_events(SDL_Event* event, vector* cursor_pos, int* cursor_interaction)
{
	while(SDL_PollEvent(event))
	{
		int x, y;

		switch(event->type)
		{
			case SDL_KEYDOWN:
				if(key_pressed(event, SDLK_ESCAPE)) return 0;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(event->button.button == 1) *cursor_interaction = 1;
				else if(event->button.button == 3) *cursor_interaction = 2;
				break;
			case SDL_MOUSEBUTTONUP:
				*cursor_interaction = 0;
				break;
			case SDL_MOUSEMOTION:
				SDL_GetMouseState(&x, &y);
				cursor_pos->x = x, cursor_pos->y = y;
				break;
			case SDL_QUIT:
				return 0;
				break;
		}
	}

	return 1;
}

void init_gl(int width, int height)
{
	glViewport(0, 0, width, height);
	glClearColor(255.0f, 255.0f, 255.0f, 0.0f);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
}

int print_help()
{
	printf("\n(C) 2011 by Joseph A. Kogut (joseph.kogut@gmail.com)\n"
		"This software is distributed under the MIT license,\n"
		"with no warranty, express or implied. Run this software\n"
		"at your own risk.\n\n"

		"-h | --help\t\tPrint this help message.\n\n"

		"Video configuration\n"
		"------------------------------------------------------------\n"
		"-r | --renderer [software | gl]\n"
		"\t\tSpecify the method used to render the demo\n"

		"--height [number]\n"
		"\tSpecify screen height in pixels.\n\n"

		"--width [number]\n"
		"\tSpecify screen width in pixels.\n\n"

		"--depth [number]\n"
		"\tSpecify screen depth in bits.\n\n"

		"--fps [number]\n"
		"\tLimit the framerate to the number specified\n\n"

		"--draw-anchor\n"
		"\tDisplay a visual anchor to prevent motion sickness\n\n"

		"Input Configuration\n"
		"------------------------------------------------------------\n"
		"-ir | --influence-radius [pixels]\n\tSpecify the maximum distance from the cursor that"
		"\n\twill influence boids.\n\n"

		"Flock configuration\n"
		"------------------------------------------------------------\n"
		"-fc | --flock-count\n\tSpecify the number of boids to create.\n\n"
		"-fs | --flock-separation\n\tSpecify a minimum distance to keep from neighbors.\n\n"
		"-fv | --flock-velocity\n\tSpecify a maximum velocity a boid can travel.\n\n"
		"-fn | --flock-neighborhood\n\tSpecify the size of the neighborhood a boid can see.\n\n"

		"Misc.\n"
		"------------------------------------------------------------\n"
		"-t | --num-threads\n\tSpecify the number of worker threads used to\n"
		"\tcalculate boid movement.\n"
	);

	return 0;
}

int main(int argc, char** argv)
{
	// Create a configuration object, and set the values to the defaults
	configuration config;

	config.num_threads = NUM_THREADS;

	config.video.renderer = RENDERER_GL;
	config.video.screen_width = SCREEN_WIDTH;
	config.video.screen_height = SCREEN_HEIGHT;
	config.video.screen_depth = SCREEN_DEPTH;
	config.video.frames_per_second = FPS;
	config.video.draw_anchor = DRAW_ANCHOR;

	config.input.influence_radius = INFLUENCE_RADIUS;

	config.flock.size = NUM_BOIDS;
	config.flock.max_velocity = MAX_BOID_VELOCITY;
	config.flock.min_separation = MIN_BOID_SEPARATION;
	config.flock.max_steering_force = MAX_BOID_STEERING_FORCE;
	config.flock.neighborhood_radius = NEIGHBORHOOD_RADIUS;

	// Parse arguments
	int i;
	for(i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			return print_help();
		else if((strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--renderer") == 0) && strcmp(argv[++i], "software") == 0)
			config.video.renderer = RENDERER_SOFTWARE;
		else if(strcmp(argv[i], "--width") == 0)
			config.video.screen_width = atoi(argv[++i]);
		else if(strcmp(argv[i], "--height") == 0)
			config.video.screen_height = atoi(argv[++i]);
		else if(strcmp(argv[i], "--depth") == 0)
			config.video.screen_depth = atoi(argv[++i]);
		else if(strcmp(argv[i], "--fps") == 0)
			config.video.frames_per_second = atoi(argv[++i]);
		else if(strcmp(argv[i], "--draw-anchor") == 0)
			config.video.draw_anchor = 1 && ++i;
		else if(strcmp(argv[i], "-ir") == 0 || strcmp(argv[i], "--influence-radius") == 0)
			config.input.influence_radius = atoi(argv[++i]);
		else if(strcmp(argv[i], "-fc") == 0 || strcmp(argv[i], "--flock-count") == 0)
			config.flock.size = atoi(argv[++i]);
		else if(strcmp(argv[i], "-fs") == 0 || strcmp(argv[i], "--flock-separation") == 0)
			config.flock.min_separation = atoi(argv[++i]);
		else if(strcmp(argv[i], "-fv") == 0 || strcmp(argv[i], "--flock-velocity") == 0)
			config.flock.max_velocity = atoi(argv[++i]);
		else if(strcmp(argv[i], "-fn") == 0 || strcmp(argv[i], "--flock-neighborhood") == 0)
			config.flock.neighborhood_radius = atoi(argv[++i]);
		else if(strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--num-threads") == 0)
			config.num_threads = atoi(argv[++i]);
	}

	srand(time(NULL));

	// Init SDL and create our screen
	if(SDL_Init(SDL_INIT_VIDEO) < 0) printf("Unable to initialize SDL. %s\n", SDL_GetError());
	SDL_Event event;

	int flags = 0;
	if(config.video.renderer == RENDERER_SOFTWARE) flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
	else if(config.video.renderer == RENDERER_GL) flags = SDL_OPENGL;

	// Attempt to set the video mode
	SDL_Surface* screen = SDL_SetVideoMode(config.video.screen_width, config.video.screen_height,
					       config.video.screen_depth, flags);

	if(!screen) printf("Unable to set video mode.\n");

	// Set the window caption
	SDL_WM_SetCaption("tinyflock", NULL);

	// Load and format our images
	SDL_Surface* temp = IMG_Load("boid.png");
	config.boid_sprite = SDL_DisplayFormatAlpha(temp);
	SDL_FreeSurface(temp);

	temp = IMG_Load("anchor.png");
	config.anchor_sprite = SDL_DisplayFormatAlpha(temp);
	SDL_FreeSurface(temp);

	// Create our flock
	boid* flock = create_flock(&config);

	// Set the render function
	render_func_t render;

	switch(config.video.renderer)
	{
		case RENDERER_SOFTWARE:
			render = &flock_render_software;
			break;
		case RENDERER_GL:
			init_gl(config.video.screen_width, config.video.screen_height);
			render = &flock_render_gl;
			break;
	};



	vector cursor_pos;
	vector_init_scalar(&cursor_pos, 0);

	int cursor_interaction = 0;

	int run = 1;

	// If the frame limit is not greater than 0, don't delay between frames at all.
	if(config.video.frames_per_second > 0)
	{
		float delay = (1000 / config.video.frames_per_second);

		while(run)
		{
			run = handle_events(&event, &cursor_pos, &cursor_interaction);

			flock_update(flock, &config, &cursor_pos, &cursor_interaction);
			render(flock, &config, screen);

			SDL_Delay(delay);
		}
	}
	else
	{
		while(run)
		{
			run = handle_events(&event, &cursor_pos, &cursor_interaction);

			flock_update(flock, &config, &cursor_pos, &cursor_interaction);
			render(flock, &config, screen);
		}
	}

	destroy_flock(flock);

	SDL_FreeSurface(config.boid_sprite);
	SDL_Quit();

	return 0;
}
