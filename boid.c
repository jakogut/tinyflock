#include "config.h"
#include "boid.h"

void init_boid(boid* b, SDL_Surface* image, int loc_x, int loc_y, int vel_x, int vel_y)
{
	b->sprite = image;

	init_vector(&b->location, loc_x, loc_y, 0);
	init_vector(&b->velocity, vel_x, vel_y, 0);
	init_vector(&b->acceleration, 0, 0, 0);
}

void  flock_influence(vector* v, boid* flock, boid* b)
{
	init_vector(v, 0, 0, 0);

	int neighborhood_population = 0;

	int i;
	for(i = 0; i < NUM_BOIDS; i++)
	{
		if(&flock[i] != b)
		{
			float distance = vector_distance_nosqrt(&flock[i].location, &b->location);

			if(distance < NEIGHBORHOOD_RADIUS_SQUARED)
			{
				vector_add(v, &flock[i].velocity);
				vector_add(v, &flock[i].location);

				if(distance < MIN_BOID_SEPARATION_SQUARED)
				{
					vector loc;
					copy_vector(&loc, &b->location);

					vector_sub(&loc, &flock[i].location);
					vector_normalize(&loc);
					vector_div_scalar(&loc, distance);

					vector_add(v, &loc);
				}
			}

			neighborhood_population++;
		}
	}

	if(neighborhood_population > 0)	vector_div_scalar(v, neighborhood_population);
	else init_vector_scalar(v, 0);

	if(vector_magnitude(v) > 0)
	{
		vector_normalize(v);
		vector_mul_scalar(v, MAX_BOID_VELOCITY);
		vector_sub(v, &b->velocity);
		vector_mul_scalar(v, MAX_BOID_STEERING_FORCE);
	}
}

void flock_limit_velocity(boid* flock, int num_boids, float max_velocity)
{
	int i;
	for(i = 0; i < num_boids; i++)
		vector_clamp_scalar(&flock[i].velocity, (0 - max_velocity), max_velocity);
}
