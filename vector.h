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


#define vec3_copy(dest, src) { dest.xyz[0] = src.xyz[0]; dest.xyz[1] = src.xyz[1]; dest.xyz[2] = src.xyz[2]; }
#define vec3_zero(v) { memset(v.xyz, 0, sizeof(vec_t) * 3); }

#define vec3_magnitude_squared(v) ((v.xyz[0] * v.xyz[0]) + (v.xyz[1] * v.xyz[1]) + (v.xyz[2] * v.xyz[2]))
#define vec3_magnitude(v) (sqrtf(vec3_magnitude_squared(v)))

#define vec3_add(a, b) { a.xyz[0] += b.xyz[0]; a.xyz[1] += b.xyz[1]; a.xyz[2] += b.xyz[2]; }
#define vec3_sub(a, b) { a.xyz[0] -= b.xyz[0]; a.xyz[1] -= b.xyz[1]; a.xyz[2] -= b.xyz[2]; }
#define vec3_sub_scalar(a, b) { a.xyz[0] -= b; a.xyz[1] -= b; a.xyz[2] -= b; }
#define vec3_mul(a, b) { a.xyz[0] *= b.xyz[0]; a.xyz[1] *= b.xyz[1]; a.xyz[2] *= b.xyz[2]; }
#define vec3_mul_scalar(a, b) { a.xyz[0] *= b; a.xyz[1] *= b; a.xyz[2] *= b; }
#define vec3_div(a, b) { a.xyz[0] /= b.xyz[0]; a.xyz[1] /= b.xyz[1]; a.xyz[2] /= b.xyz[2]; }
#define vec3_div_scalar(a, b) { a.xyz[0] /= b; a.xyz[1] /= b; a.xyz[2] /= b; }

inline vec_t vec3_distance_squared(vec3_t a, vec3_t b)
{
	register vec_t 	xd = b.scalars.x - a.scalars.x,
			yd = b.scalars.y - a.scalars.y,
			zd = b.scalars.z - a.scalars.z;

	return ((xd * xd) + (yd * yd) + (zd * zd));
}

inline vec_t vec3_distance(vec3_t a, vec3_t b)
{
	return sqrtf(vec3_distance_squared(a, b));
}

inline void vec3_normalize(vec3_t* v)
{
        vec_t magnitude = vec3_magnitude((*v)), z = 0;

        if(magnitude > 0)
        {
                z = powf(magnitude, 0.5f);
                vec3_div_scalar((*v), z);
        }
}

#endif
