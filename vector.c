#include "vector.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

vec3_t* vec3_create()
{
	vec3_t* v = calloc(1, sizeof(vec3_t));

	return v;
}


void vec2_destroy(vec2_t* v)
{
	free(v);
}

void vec3_destroy(vec3_t* v)
{
	free(v);
}

void vector_copy(vec3_t* dest, vec3_t* src)
{
	memcpy(dest->xyz, src->xyz, sizeof(vec_t) * 3);
}

void vector_zero(vec3_t* v)
{
	memset(v->xyz, 0, sizeof(vec_t) * 3);
}

void vector_add(vec3_t* a, vec3_t* b)
{
	a->scalars.x += b->scalars.x;
	a->scalars.y += b->scalars.y;
	a->scalars.z += b->scalars.z;
}

void vector_sub(vec3_t* a, vec3_t* b)
{
	a->scalars.x -= b->scalars.x;
	a->scalars.y -= b->scalars.y;
	a->scalars.z -= b->scalars.z;
}

void vector_sub_scalar(vec3_t* a, float b)
{
	a->scalars.x -= b;
	a->scalars.y -= b;
	a->scalars.z -= b;
}

void vector_mul(vec3_t* a, vec3_t* b)
{
	a->scalars.x *= b->scalars.x;
	a->scalars.y *= b->scalars.y;
	a->scalars.z *= b->scalars.z;
}

void vector_mul_scalar(vec3_t* a, float b)
{
	a->scalars.x *= b;
	a->scalars.y *= b;
	a->scalars.z *= b;
}

void vector_div(vec3_t* a, vec3_t* b)
{
	a->scalars.x /= b->scalars.x;
	a->scalars.y /= b->scalars.y;
	a->scalars.z /= b->scalars.z;
}

void vector_div_scalar(vec3_t* v, float divisor)
{
	v->scalars.x /= divisor;
	v->scalars.y /= divisor;
	v->scalars.z /= divisor;
}

float vector_distance(vec3_t* a, vec3_t* b)
{
	register float xd = b->scalars.x - a->scalars.x;
	register float yd = b->scalars.y - a->scalars.y;
	register float zd = b->scalars.z - a->scalars.z;

	return sqrtf(xd * xd + yd * yd + zd * zd);
}

float vector_distance_nosqrt(vec3_t* a, vec3_t* b)
{
	float xd = b->scalars.x - a->scalars.x;
	float yd = b->scalars.y - a->scalars.y;
	float zd = b->scalars.z - a->scalars.z;

	return (xd * xd + yd * yd + zd * zd);

}

float vector_magnitude(vec3_t* v)
{
	return sqrtf((v->scalars.x * v->scalars.x) + (v->scalars.y * v->scalars.y) + (v->scalars.z * v->scalars.z));
}

void vector_normalize(vec3_t* v)
{
	float magnitude = vector_magnitude(v), z = 0;

	if(magnitude > 0)
	{
		z = powf(magnitude, 0.5f);
		vector_div_scalar(v, z);
	}
}
