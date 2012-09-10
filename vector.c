#include "vector.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

void vector_div_scalar(vec3_t* v, vec_t divisor)
{
	v->scalars.x /= divisor;
	v->scalars.y /= divisor;
	v->scalars.z /= divisor;
}

vec_t vector_magnitude(vec3_t* v)
{
	return sqrtf((v->scalars.x * v->scalars.x) + (v->scalars.y * v->scalars.y) + (v->scalars.z * v->scalars.z));
}

void vector_normalize(vec3_t* v)
{
	vec_t magnitude = vector_magnitude(v), z = 0;

	if(magnitude > 0)
	{
		z = powf(magnitude, 0.5f);
		vector_div_scalar(v, z);
	}
}

void vec3_normalize(vec3_t v)
{
	vec_t magnitude = vec3_magnitude(v), z = 0;

	if(magnitude > 0)
	{
		z = powf(magnitude, 0.5f);
		vec3_div_scalar(v, z);
	}
}

