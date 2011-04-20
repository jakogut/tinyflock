#include <math.h>

#include "boid.h"

void init_boid(boid* b, float loc_x, float loc_y, float vel_x, float vel_y)
{
	vector_init(&b->location, loc_x, loc_y, 0);
	vector_init(&b->velocity, vel_x, vel_y, 0);
	vector_init(&b->acceleration, 0, 0, 0);
}
