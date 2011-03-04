#include "boid.h"

boid* create_boid(SDL_Surface* image, int loc_x, int loc_y, int vel_x, int vel_y)
{
	boid* b = malloc(sizeof *b);

	b->sprite = image;

	b->location = create_vector(loc_x, loc_y, 0);
	b->velocity = create_vector(vel_x, vel_y, 0);
	b->acceleration = create_vector(0, 0, 0);

	return b;
}

void destroy_boid(boid* b)
{
	destroy_vector(b->location);
	destroy_vector(b->velocity);
	destroy_vector(b->acceleration);

	free(b);
}

// Avoid crowding flockmates
vector* flock_separate (boid** flock, boid* b)
{
	vector* v = create_vector(0, 0, 0);
	int neighborhood_population = 0;

	int i;
	for(i = 0; i < NUM_BOIDS; i++)
	{
		if(flock[i] != b)
		{
			float distance = vector_distance(flock[i]->location, b->location);

			if((distance < NEIGHBORHOOD_RADIUS) && (distance < MIN_BOID_SEPARATION))
			{
				vector* loc = copy_vector(b->location);

				vector_sub(loc, flock[i]->location);
				vector_normalize(loc);
				vector_div_scalar(loc, distance);

				vector_add(v, loc);

				destroy_vector(loc);

				neighborhood_population++;
			}
		}
	}

	if(neighborhood_population > 0)	vector_div_scalar(v, neighborhood_population);
	else vector_init(v, 0);

	if(vector_magnitude(v) > 0)
	{
		vector_normalize(v);
		vector_mul_scalar(v, MAX_BOID_VELOCITY);
		vector_sub(v, b->velocity);
		vector_mul_scalar(v, MAX_BOID_STEERING_FORCE);
	}

	return v;
}

// Head in the same average direction as flockmates
vector* flock_align(boid** flock, boid* b)
{
	vector* v = create_vector(0, 0, 0);
	int neighborhood_population = 0;

	int i;
	for(i = 0; i < NUM_BOIDS; i++)
	{
		if(flock[i] != b)
		{
			float distance = vector_distance(flock[i]->location, b->location);

			if(distance < NEIGHBORHOOD_RADIUS)
			{
				vector_add(v, flock[i]->velocity);
				neighborhood_population++;
			}
		}
	}

	if(neighborhood_population > 0)	vector_div_scalar(v, neighborhood_population);
	else vector_init(v, 0);

	if(vector_magnitude(v) > 0)
	{
		vector_normalize(v);
		vector_mul_scalar(v, MAX_BOID_VELOCITY);
		vector_sub(v, b->velocity);
		vector_mul_scalar(v, MAX_BOID_STEERING_FORCE);
	}

	return v;
}

// Head towards the average position of flockmates
vector* flock_cohere(boid** flock, boid* b)
{
	vector* v = create_vector(0, 0, 0);
	int neighborhood_population = 0;

	int i;
	for(i = 0; i < NUM_BOIDS; i++)
	{
		if(flock[i] != b)
		{
			float distance = vector_distance(flock[i]->location, b->location);

			if(distance < NEIGHBORHOOD_RADIUS)
			{
				vector_add(v, flock[i]->location);
				neighborhood_population++;
			}
		}
	}

	if(neighborhood_population > 0)	vector_div_scalar(v, neighborhood_population);
	else vector_init(v, 0);

	if(vector_magnitude(v) > 0)
	{
		vector_normalize(v);
		vector_mul_scalar(v, MAX_BOID_VELOCITY);
		vector_sub(v, b->velocity);
		vector_mul_scalar(v, MAX_BOID_STEERING_FORCE);
	}

	return v;
}

void flock_limit_velocity(boid** flock, int num_boids, float max_velocity)
{
	int i;
	for(i = 0; i < num_boids; i++)
		vector_clamp_scalar(flock[i]->velocity, (0 - max_velocity), max_velocity);
}
