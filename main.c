#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <GLFW/glfw3.h>

#include "flock.h"
#include "render.h"

#include "events.h"
#include "configuration.h"

#define WINDOW_TITLE "tinyflock"

#define FRACTIONAL_INFLUENCE 0.5

#define FPS_BUFFER_SIZE 5

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

int parse_arguments(int argc, char** argv, configuration* config)
{
	config->num_threads = NUM_THREADS;

	config->video.screen_width = SCREEN_WIDTH;
	config->video.screen_height = SCREEN_HEIGHT;
	config->video.screen_depth = SCREEN_DEPTH;
	config->video.frames_per_second = FPS;

	config->input.influence_radius = INFLUENCE_RADIUS;

	config->flock.size = NUM_BOIDS;
	config->flock.max_velocity = MAX_BOID_VELOCITY;
	config->flock.min_separation = MIN_BOID_SEPARATION;
	config->flock.max_steering_force = MAX_BOID_STEERING_FORCE;
	config->flock.neighborhood_radius = NEIGHBORHOOD_RADIUS;

	// Parse arguments
	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			return print_help();
		else if(strcmp(argv[i], "--width") == 0)
			config->video.screen_width = atoi(argv[++i]);
		else if(strcmp(argv[i], "--height") == 0)
			config->video.screen_height = atoi(argv[++i]);
		else if(strcmp(argv[i], "--depth") == 0)
			config->video.screen_depth = atoi(argv[++i]);
		else if(strcmp(argv[i], "--fps") == 0)
			config->video.frames_per_second = atoi(argv[++i]);
		else if(strcmp(argv[i], "-ir") == 0 || strcmp(argv[i], "--influence-radius") == 0)
			config->input.influence_radius = atoi(argv[++i]);
		else if(strcmp(argv[i], "-fc") == 0 || strcmp(argv[i], "--flock-count") == 0)
			config->flock.size = atoi(argv[++i]);
		else if(strcmp(argv[i], "-fs") == 0 || strcmp(argv[i], "--flock-separation") == 0)
			config->flock.min_separation = atoi(argv[++i]);
		else if(strcmp(argv[i], "-fv") == 0 || strcmp(argv[i], "--flock-velocity") == 0)
			config->flock.max_velocity = atoi(argv[++i]);
		else if(strcmp(argv[i], "-fn") == 0 || strcmp(argv[i], "--flock-neighborhood") == 0)
			config->flock.neighborhood_radius = atoi(argv[++i]);
		else if(strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--num-threads") == 0)
			config->num_threads = atoi(argv[++i]);
	}

	return 1;
}

/* Here we take the time taken to render the newest frame and average it with the last N frames to get our FPS */
void print_frametime(long frame_time_nsec)
{
	long frames_per_second = 1000000000 / frame_time_nsec;

	static long fps_buffer[FPS_BUFFER_SIZE];
	for(int i = FPS_BUFFER_SIZE - 1; i != 0; i--) fps_buffer[i] = fps_buffer[i - 1];
	fps_buffer[0] = frames_per_second;

	long fps_avg = 0;
	for(int i = 0; i < FPS_BUFFER_SIZE; i++) fps_avg += fps_buffer[i];
	fps_avg /= FPS_BUFFER_SIZE;

	printf("\rFPS: %ld\t", fps_avg);
	fflush(stdout);
}

int main(int argc, char** argv)
{
	// Create a configuration object, and set the values to the defaults
	configuration config;
	if(!parse_arguments(argc, argv, &config)) return 0;

	srand(time(NULL));

	glfwInit();

	GLFWwindow* window = glfwCreateWindow(config.video.screen_width, config.video.screen_height, WINDOW_TITLE, NULL, NULL);
	if(!window) printf("Unable to set video mode.\n");

	// Register callbacks
	callback_config_ptr = &config;
	glfwSetCursorPosCallback(window, callback_cursormov);
	glfwSetMouseButtonCallback(window, callback_mousebtn);
	glfwSetKeyCallback(window, callback_keyboard);
	glfwSetWindowSizeCallback(window, callback_windowresize);
	glfwSetWindowCloseCallback(window, callback_wclose);

	vec2_zero(cursor_pos);
	cursor_interaction = 0;

	glfwMakeContextCurrent(window);

	init_gl(config.video.screen_width, config.video.screen_height);
	printf("Using OpenGL Version: %i.%i\n", glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR),
						glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR));

	// Create our flock
	flock* f = flock_create(&config);

	int* ticks = calloc(sizeof(int), config.num_threads);

// DISPATCH //
	extern int fractional_flock_size;
	extern int* flock_sample;

        fractional_flock_size = config.flock.size * FRACTIONAL_INFLUENCE;
        flock_sample = calloc(sizeof(int), fractional_flock_size);

        for(int i = 0; i < fractional_flock_size; i++)
                flock_sample[i] = rand() % config.flock.size;

	pthread_t* workers = calloc(sizeof(pthread_t), config.num_threads);
        flock_update_worker_args* worker_args = calloc(sizeof(flock_update_worker_args), config.num_threads);

        for(int i = 0; i < config.num_threads; i++)
                worker_args[i] = (flock_update_worker_args){&run, i, &ticks[i], f, &config, &cursor_pos, &cursor_interaction};

        for(int i = 0; i < config.num_threads; i++)
		pthread_create(&workers[i], NULL, flock_update_worker_thread, (void*)&worker_args[i]);
//////////////

	struct timespec curr_time, new_time;
	long frame_time_nsec;

        clock_gettime(CLOCK_MONOTONIC, &curr_time);

	while(run && !glfwWindowShouldClose(window))
	{
		flock_render(window, f, &config);
		clock_gettime(CLOCK_MONOTONIC, &new_time);

		frame_time_nsec = (new_time.tv_nsec - curr_time.tv_nsec) + (1000000000 * (new_time.tv_sec - curr_time.tv_sec));
		curr_time = new_time;

		print_frametime(frame_time_nsec);

		glfwPollEvents();
	}

	int update_count = 0;
	for(int i = 0; i < config.num_threads; i++) update_count += ticks[i];

        for(int i = 0; i < config.num_threads; i++)
                pthread_join(workers[i], NULL);

	glfwDestroyWindow(window);

        free(flock_sample);
	free(ticks);
        free(worker_args);
        free(workers);
	flock_destroy(f);

	glfwTerminate();
	return 0;
}
