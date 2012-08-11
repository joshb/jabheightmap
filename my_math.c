/*
 * Copyright (C) 2003 Josh A. Beam
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <math.h>
#include <GL/gl.h>
#include "my_math.h"

static float view[4][4];

/* multiply 4x4 matrix */
void
mult_matrix_4x4(float out[16], float m1[16], float m2[16])
{
	out[0] = m1[0] * m2[0] + m1[1] * m2[4] + m1[2] * m2[8] + m1[3] * m2[12];
	out[1] = m1[0] * m2[1] + m1[1] * m2[5] + m1[2] * m2[9] + m1[3] * m2[13];
	out[2] = m1[0] * m2[2] + m1[1] * m2[6] + m1[2] * m2[10] + m1[3] * m2[14];
	out[3] = m1[0] * m2[3] + m1[1] * m2[7] + m1[2] * m2[11] + m1[3] * m2[15];

	out[4] = m1[4] * m2[0] + m1[5] * m2[4] + m1[6] * m2[8] + m1[7] * m2[12];
	out[5] = m1[4] * m2[1] + m1[5] * m2[5] + m1[6] * m2[9] + m1[7] * m2[13];
	out[6] = m1[4] * m2[2] + m1[5] * m2[6] + m1[6] * m2[10] + m1[7] * m2[14];
	out[7] = m1[4] * m2[3] + m1[5] * m2[7] + m1[6] * m2[11] + m1[7] * m2[15];

	out[8] = m1[8] * m2[0] + m1[9] * m2[4] + m1[10] * m2[8] + m1[11] * m2[12];
	out[9] = m1[8] * m2[1] + m1[9] * m2[5] + m1[10] * m2[9] + m1[11] * m2[13];
	out[10] = m1[8] * m2[2] + m1[9] * m2[6] + m1[10] * m2[10] + m1[11] * m2[14];
	out[11] = m1[8] * m2[3] + m1[9] * m2[7] + m1[10] * m2[11] + m1[11] * m2[15];

	out[12] = m1[12] * m2[0] + m1[13] * m2[4] + m1[14] * m2[8] + m1[15] * m2[12];
	out[13] = m1[12] * m2[1] + m1[13] * m2[5] + m1[14] * m2[9] + m1[15] * m2[13];
	out[14] = m1[12] * m2[2] + m1[13] * m2[6] + m1[14] * m2[10] + m1[15] * m2[14];
	out[15] = m1[12] * m2[3] + m1[13] * m2[7] + m1[14] * m2[11] + m1[15] * m2[15];
}

/* update the view frustum; called once every rendering cycle */
void
update_view_frustum()
{
	float mm[16], pm[16], product[16];
	float scale;

	glGetFloatv(GL_MODELVIEW_MATRIX, mm);
	glGetFloatv(GL_PROJECTION_MATRIX, pm);

	mult_matrix_4x4(product, mm, pm);

	view[0][0] = product[3] - product[0];
	view[0][1] = product[7] - product[4];
	view[0][2] = product[11] - product[8];
	view[0][3] = product[15] - product[12];
	scale  = 1.0f / sqrtf(SQUARE(view[0][0]) + SQUARE(view[0][1]) + SQUARE(view[0][2]));
	view[0][0] *= scale;
	view[0][1] *= scale;
	view[0][2] *= scale;
	view[0][3] *= scale;

	view[1][0] = product[3] + product[0];
	view[1][1] = product[7] + product[4];
	view[1][2] = product[11] + product[8];
	view[1][3] = product[15] + product[12];
	scale = 1.0f / sqrtf(SQUARE(view[1][0]) + SQUARE(view[1][1]) + SQUARE(view[1][2]));
	view[1][0] *= scale;
	view[1][1] *= scale;
	view[1][2] *= scale;
	view[1][3] *= scale;

	view[2][0] = product[3] - product[1];
	view[2][1] = product[7] - product[5];
	view[2][2] = product[11] - product[9];
	view[2][3] = product[15] - product[13];
	scale = 1.0f / sqrtf(SQUARE(view[1][0]) + SQUARE(view[1][1]) + SQUARE(view[1][2]));
	view[2][0] *= scale;
	view[2][1] *= scale;
	view[2][2] *= scale;
	view[2][3] *= scale;

	view[3][0] = product[3] + product[1];
	view[3][1] = product[7] + product[5];
	view[3][2] = product[11] + product[9];
	view[3][3] = product[15] + product[13];
	scale = 1.0f / sqrtf(SQUARE(view[1][0]) + SQUARE(view[1][1]) + SQUARE(view[1][2]));
	view[3][0] *= scale;
	view[3][1] *= scale;
	view[3][2] *= scale;
	view[3][3] *= scale;

	view[4][0] = product[3] - product[2];
	view[4][1] = product[7] - product[6];
	view[4][2] = product[11] - product[10];
	view[4][3] = product[15] - product[14];
	scale = 1.0f / sqrtf(SQUARE(view[1][0]) + SQUARE(view[1][1]) + SQUARE(view[1][2]));
	view[4][0] *= scale;
	view[4][1] *= scale;
	view[4][2] *= scale;
	view[4][3] *= scale;

	view[5][0] = product[3] + product[2];
	view[5][1] = product[7] + product[6];
	view[5][2] = product[11] + product[10];
	view[5][3] = product[15] + product[14];
	scale = 1.0f / sqrtf(SQUARE(view[1][0]) + SQUARE(view[1][1]) + SQUARE(view[1][2]));
	view[5][0] *= scale;
	view[5][1] *= scale;
	view[5][2] *= scale;
	view[5][3] *= scale;
}

/* normalize vector v */
void
normalize(float v[3])
{
	float length;

	length = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= length;
	v[1] /= length;
	v[2] /= length;
}

/* store the cross product of vectors v1 and v2 in out */
void
cross_product(float out[3], float v1[3], float v2[3])
{
	out[0] = v1[1] * v2[2] - v1[2] * v2[1];
	out[1] = v1[2] * v2[0] - v1[0] * v2[2];
	out[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

/* use three vectors to create a plane to fill in the plane equation */
void
setup_plane(float p[4], float v1[3], float v2[3], float v3[3])
{
	float normal[3];
	float d1[3], d2[3];

	d1[0] = v2[0] - v1[0];
	d1[1] = v2[1] - v1[1];
	d1[2] = v2[2] - v1[2];

	d2[0] = v3[0] - v1[0];
	d2[1] = v3[1] - v1[1];
	d2[2] = v3[2] - v1[2];

	cross_product(normal, d2, d1);
	normalize(normal);

	p[0] = normal[0];
	p[1] = normal[1];
	p[2] = normal[2];
	p[3] = -(normal[0] * v1[0] + normal[1] * v1[1] + normal[2] * v1[2]);
}

/* plane equation; returns v's distance from plane p */
float
plane_equation(float p[4], float v[3])
{
	return (v[0] * p[0] + v[1] * p[1] + v[2] * p[2] + p[3]);
}

/* return 1 if v is inside the view frustum */
int
is_point_in_viewport(float v[3], float r)
{
	int i;

	for(i = 0; i < 6; i++) {
		if(plane_equation(view[i], v) <= -r)
			return 0;
	}

	return 1;
}
