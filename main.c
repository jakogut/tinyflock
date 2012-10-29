#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <GL/glfw.h>

#include "flock.h"
#include "render.h"

#include "input.h"
#include "configuration.h"

#define FRACTIONAL_INFLUENCE 0.5

void init_gl(int width, int height)
{
	glViewport(0, 0, width, height);
	glClearColor(255.0f, 255.0f, 255.0f, 0.0f);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);

	glLineWidth(0.2f);

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

	config.video.screen_width = SCREEN_WIDTH;
	config.video.screen_height = SCREEN_HEIGHT;
	config.video.screen_depth = SCREEN_DEPTH;
	config.video.frames_per_second = FPS;

	config.input.influence_radius = INFLUENCE_RADIUS;

	config.flock.size = NUM_BOIDS;
	config.flock.max_velocity = MAX_BOID_VELOCITY;
	config.flock.min_separation = MIN_BOID_SEPARATION;
	config.flock.max_steering_force = MAX_BOID_STEERING_FORCE;
	config.flock.neighborhood_radius = NEIGHBORHOOD_RADIUS;

	// Parse arguments
	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			return print_help();
		else if(strcmp(argv[i], "--width") == 0)
			config.video.screen_width = atoi(argv[++i]);
		else if(strcmp(argv[i], "--height") == 0)
			config.video.screen_height = atoi(argv[++i]);
		else if(strcmp(argv[i], "--depth") == 0)
			config.video.screen_depth = atoi(argv[++i]);
		else if(strcmp(argv[i], "--fps") == 0)
			config.video.frames_per_second = atoi(argv[++i]);
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

	glfwInit();
	int window = glfwOpenWindow(config.video.screen_width, config.video.screen_height, 0, 0, 0, 0, 0, 0, GLFW_WINDOW);
	if(!window) printf("Unable to set video mode.\n");

	init_gl(config.video.screen_width, config.video.screen_height);

	glfwSetMousePosCallback(callback_mousemov);
	glfwSetMouseButtonCallback(callback_mousebtn);
	glfwSetKeyCallback(callback_keyboard);

	vec3_zero(cursor_pos);
	cursor_interaction = 0;

	// Create our flock
	flock* f = flock_create(&config);

	int* ticks = calloc(sizeof(int), config.num_threads);
	int frame_count = 0;

	// DISPATCH //
	extern int fractional_flock_size;
	extern int* flock_sample;

	GLFWthread* workers = calloc(sizeof(GLFWthread), config.num_threads);
        flock_update_worker_args* worker_args = calloc(sizeof(flock_update_worker_args), config.num_threads);

        fractional_flock_size = config.flock.size * FRACTIONAL_INFLUENCE;
        flock_sample = calloc(sizeof(int), fractional_flock_size);

        for(int i = 0; i < fractional_flock_size; i++)
                flock_sample[i] = rand() % config.flock.size;

        for(int i = 0; i < config.num_threads; i++)
                worker_args[i] = (flock_update_worker_args){&run, i, &ticks[i], f, &config, &cursor_pos, &cursor_interaction};

        for(int i = 0; i < config.num_threads; i++)
                workers[i] = glfwCreateThread(flock_update_worker_thread, (void*)&worker_args[i]);
	//////////////


	// If the frame limit is not greater than 0, don't delay between frames at all.
	double delay = (1.0f / config.video.frames_per_second);

	glfwSetTime(0);

	while(run)
	{
		flock_render(f, &config);
		++frame_count;

		glfwSleep(delay);
	}

	uint32_t sec_elapsed = glfwGetTime();

	int update_count = 0;
	for(int i = 0; i < config.num_threads; i++) update_count += ticks[i];

	if(sec_elapsed > 0)
		printf("Average Frames Per Second: %i, Average Ticks Per Second: %i\n", (frame_count / sec_elapsed), ((update_count / config.num_threads) / sec_elapsed));

        for(int i = 0; i < config.num_threads; i++)
                glfwWaitThread(workers[i], GLFW_WAIT);

        free(flock_sample);
	free(ticks);
        free(worker_args);
        free(workers);
	flock_destroy(f);

	glfwTerminate();
	return 0;
}
