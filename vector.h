#ifndef VECTOR_H_
#define VECTOR_H_

#include <math.h>

typedef float vec_t;

typedef union
{
	vec_t xy[2];
	struct { vec_t x, y; } scalars;

} vec2_t;

typedef union
{
	vec_t xyz[3];
	struct { vec_t x, y, z; } scalars;

} vec3_t;

vec3_t* vec3_create();

void vec2_destroy(vec2_t* v);
void vec3_destroy(vec3_t* v);

void vector_copy(vec3_t* dest, vec3_t* src);

void vector_zero(vec3_t* v);

/* The performance of these vector instructions could be improved on x86 using hand-coded SSE intrinsics,
though the compiler may do that itself. */
void vector_add(vec3_t* a, vec3_t* b);

void vector_sub(vec3_t* a, vec3_t* b);

void vector_sub_scalar(vec3_t* a, float b);

void vector_mul(vec3_t* a, vec3_t* b);

void vector_mul_scalar(vec3_t* a, float b);

void vector_div(vec3_t* a, vec3_t* b);

void vector_div_scalar(vec3_t* v, float divisor);

float vector_distance(vec3_t* a, vec3_t* b);

float vector_distance_nosqrt(vec3_t* a, vec3_t* b);

float vector_magnitude(vec3_t* v);

void vector_normalize(vec3_t* v);

#endif
