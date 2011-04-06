#include <math.h>

#include "boid.h"

void init_boid(boid* b, float loc_x, float loc_y, float vel_x, float vel_y)
{
	init_vector(&b->location, loc_x, loc_y, 0);
	init_vector(&b->velocity, vel_x, vel_y, 0);
	init_vector(&b->acceleration, 0, 0, 0);
}
