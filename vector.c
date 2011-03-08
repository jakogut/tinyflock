#include "config.h"
#include <stdlib.h>

#include "vector.h"

vector* create_vector(float x, float y, float z)
{
	vector* v = malloc(sizeof(vector));

	v->x = x;
	v->y = y;
	v->z = z;

	return v;
}

void randomize_vector(vector* v, float min, float max)
{
	float range = max - min;

	v->x = (rand() / (float)RAND_MAX) * range;
	v->x += min;

	v->y = (rand() / (float)RAND_MAX) * range;
	v->y += min;

	v->z = (rand() / (float)RAND_MAX) * range;
	v->z += min;
}

void copy_vector(vector* dest, vector* src)
{
	dest->x = src->x;
	dest->y = src->y;
	dest->z = src->z;
}

void destroy_vector(vector* v)
{
	free(v);
}

void init_vector(vector* v, float x, float y, float z)
{
	v->x = x;
	v->y = y;
	v->z = z;
}

void init_vector_scalar(vector* v, float init)
{
	v->x = init;
	v->y = init;
	v->z = init;
}

void vector_add(vector* a, vector* b)
{
	a->x += b->x;
	a->y += b->y;
	a->z += b->z;
}

void vector_sub(vector* a, vector* b)
{
	a->x -= b->x;
	a->y -= b->y;
	a->z -= b->z;
}

void vector_mul(vector* a, vector* b)
{
	a->x *= b->x;
	a->y *= b->y;
	a->z *= b->z;
}

void vector_mul_scalar(vector* a, float b)
{
	a->x *= b;
	a->y *= b;
	a->z *= b;
}

void vector_div(vector* a, vector* b)
{
	a->x /= b->x;
	a->y /= b->y;
	a->z /= b->z;
}

void vector_div_scalar(vector* v, float divisor)
{
	v->x /= divisor;
	v->y /= divisor;
	v->z /= divisor;
}

float vector_distance(vector* a, vector* b)
{
	register float xd = b->x - a->x;
	register float yd = b->y - a->y;
	register float zd = b->z - a->z;

	return sqrt(xd * xd + yd * yd + zd * zd);
}

float vector_distance_nosqrt(vector* a, vector* b)
{
	register float xd = b->x - a->x;
	register float yd = b->y - a->y;
	register float zd = b->z - a->z;

	return (xd * xd + yd * yd + zd * zd);

}
float vector_magnitude(vector* v)
{
	return sqrt((v->x * v->x) + (v->y * v->y) + (v->z * v->z));
}

void vector_normalize(vector* v)
{
	float magnitude = vector_magnitude(v);

	if(magnitude > 0)
	{
		v->x /= magnitude;
		v->y /= magnitude;
		v->z /= magnitude;
	}
}

void vector_clamp(vector* v, vector* min, vector* max)
{
	if(v->x < min->x) v->x = min->x;
	if(v->y < min->y) v->y = min->y;
	if(v->z < min->z) v->z = min->z;

	if(v->x > max->x) v->x = max->x;
	if(v->y > max->y) v->y = max->y;
	if(v->z > max->z) v->z = max->z;
}

void vector_clamp_scalar(vector* v, float min, float max)
{
	if(v->x < min) v->x = min;
	if(v->y < min) v->y = min;
	if(v->z < min) v->z = min;

	if(v->x > max) v->x = max;
	if(v->y > max) v->y = max;
	if(v->z > max) v->z = max;
}
