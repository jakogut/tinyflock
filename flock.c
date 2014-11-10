#include "flock.h"

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <GLFW/glfw3.h>

#include "floatfann.h"

#define TPS_BUFFER_SIZE 5

flock* flock_create(configuration* config)
{
	flock* f = calloc(1, sizeof(flock));

	f->location = calloc(config->flock.size, sizeof(vec2_t));
	f->acceleration = calloc(config->flock.size, sizeof(vec2_t));
	f->velocity = calloc(config->flock.size, sizeof(vec2_t));

	for(int i = 0; i < config->flock.size; i++)
	{
		f->location[i][0] = rand_range(0.0f, config->video.screen_width);
		f->location[i][1] = rand_range(0.0f, config->video.screen_height);

		flock_randomize_acceleration(f, config);
	}

	return f;
}

void flock_destroy(flock* f)
{
	free(f->location);
	free(f->acceleration);
	free(f->velocity);

	free(f);
}

void flock_randomize_acceleration(flock* f, configuration* config)
{
	for(int i = 0; i < config->flock.size; i++)
	{
		f->velocity[i][0] = rand_range((0.0f - config->flock.max_velocity), config->flock.max_velocity);
		f->velocity[i][1] = rand_range((0.0f - config->flock.max_velocity), config->flock.max_velocity);
	}
}

void* flock_update_worker_thread(void* arg)
{
	flock_update_worker_args* args = (flock_update_worker_args*)arg;

	int flock_size = args->config->flock.size;

	long tps_buffer[TPS_BUFFER_SIZE];
	struct timespec curr_time, new_time;
	clock_gettime(CLOCK_MONOTONIC, &curr_time);

	struct fann* ann_flock = fann_create_from_file("flock.net");
	float* ann_input = NULL;
	fann_type* ann_output = NULL;

	while(*args->run)
	{

		clock_gettime(CLOCK_MONOTONIC, &new_time);
		long tick_time_nsec = (new_time.tv_nsec - curr_time.tv_nsec) + (1000000000 * (new_time.tv_sec - curr_time.tv_sec));
		curr_time = new_time;

		long ticks_per_second = 1000000000 / tick_time_nsec;

		for(int i = TPS_BUFFER_SIZE - 1; i != 0; i--) tps_buffer[i] = tps_buffer[i - 1];
		tps_buffer[0] = ticks_per_second;

		long tps_avg = 0;
		for(int i = 0; i < TPS_BUFFER_SIZE; i++) tps_avg += tps_buffer[i];
		tps_avg /= TPS_BUFFER_SIZE;
		args->ticks[args->thread_id] = tps_avg;

		ann_input = calloc(2 * flock_size, sizeof(float));

		for(int i = 0; i < flock_size; i++)
		{
			ann_input[i * 2]     = (args->f->location[i][0] - (args->f->location[i][0] / 2)) / args->f->location[i][0];
			ann_input[i * 2 + 1] = (args->f->location[i][1] - (args->f->location[i][1] / 2)) / args->f->location[i][1];
		}

		ann_output = fann_run(ann_flock, ann_input);

		for(int i = 0; i < flock_size; i++)
		{
			args->f->velocity[i][0] = ann_output[i * 2] * args->config->flock.max_velocity;
			args->f->velocity[i][1] = ann_output[i * 2 + 1] * args->config->flock.max_velocity;
		}

		for(int i = 0; i < flock_size; i++)
		{
			vec2_add(args->f->location[i], args->f->velocity[i]);

			// Wrap coordinates
			args->f->location[i][0] -= args->config->video.screen_width * (args->f->location[i][0] > args->config->video.screen_width);
			args->f->location[i][0] += args->config->video.screen_width * (args->f->location[i][0] < 0);

			args->f->location[i][1] -= args->config->video.screen_height * (args->f->location[i][1] > args->config->video.screen_height);
			args->f->location[i][1] += args->config->video.screen_height * (args->f->location[i][1] < 0);
		}

		++(*args->ticks);
	}

	return NULL;
}

float rand_range(float min, float max)
{
	float range = max - min;

	float num = (rand() / (float)RAND_MAX) * range;
	num += min;

	return num;
}

