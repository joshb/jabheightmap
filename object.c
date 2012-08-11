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

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include "object.h"
#include "octree.h"
#include "my_math.h"

static struct object *objects = NULL;
static unsigned int num_objects = 0;

/* allocate memory for object and return pointer to it */
struct object *
create_object(int type)
{
	struct object *tmp;
	void *aux;

	switch(type) {
		default:
			fprintf(stderr, "Error: Invalid object type\n");
			return NULL;
			break;
		case OBJ_DEFAULT:
			fprintf(stderr, "making default object\n");
			aux = NULL;
			break;
		case OBJ_PLANE:
			aux = malloc(sizeof(struct plane_object));
			break;
	}
	if(type != OBJ_DEFAULT && !aux) {
		fprintf(stderr, "Error: Couldn't allocate memory for type-specific object data\n");
		return NULL;
	}

	tmp = realloc(objects, sizeof(struct object) * (num_objects + 1));
	if(!tmp) {
		fprintf(stderr, "Error: Couldn't allocate memory for object\n");
		free(aux);
		return NULL;
	}

	objects = tmp;
	objects[num_objects].type = type;
	objects[num_objects].aux = aux;
	objects[num_objects].render_separately = 1;
	objects[num_objects].gl_primitive = GL_QUADS;
	objects[num_objects].vertices = NULL;
	objects[num_objects].num_vertices = 0;

	return &objects[num_objects++];
}

void
free_all_objects()
{
	int i;
	struct object *o;

	for(i = 0; i < num_objects; i++) {
		o = &objects[i];
		if(o->aux)
			free(o->aux);
		if(o->vertices)
			free(o->vertices);
	}

	free(objects);
	num_objects = 0;
}

/* return object number from pointer */
int
get_object_num(struct object *o)
{
	int i;

	for(i = 0; i < num_objects; i++) {
		if(&objects[i] == o)
			return i;
	}

	return -1;
}

void
draw_object(int n)
{
	int i;
	struct object *o;

	if(n >= num_objects) {
		fprintf(stderr, "Error: Bad object number %d\n", n);
		return;
	}

	o = &objects[n];
	if(!o)
		return;

	if(o->render_separately)
		glBegin(o->gl_primitive);
	for(i = 0; i < o->num_vertices; i++) {
		glTexCoord2f(o->vertices[i].texcoord[0], o->vertices[i].texcoord[1]);
		glVertex3f(o->vertices[i].point[0], o->vertices[i].point[1], o->vertices[i].point[2]);
	}
	if(o->render_separately)
		glEnd();
}

static int
plane_object_collision(struct object *o1, struct object *o2)
{
	struct plane_object *p;
	float *v;
	float d;

	if(!o1 || !o2)
		return 0;

	v = o1->position;
	p = o2->aux;

	if(v[0] < p->minx || v[0] > p->maxx ||
	   v[1] < p->miny || v[1] > p->maxy)
		return 0;

	d = plane_equation(p->plane, v);
	if(d <= 0.0f) {
		o1->position[0] += -d * p->plane[0];
		o1->position[1] += -d * p->plane[1];
		o1->position[2] += (-d * p->plane[2]) / 2.0f;
		return 1;
	}

	return 0;
}

/* check if specified object collides with any others */
int
object_collision(struct object *o)
{
	int i;

	for(i = 0; i < num_objects; i++) {
		switch(objects[i].type) {
			case OBJ_PLANE:
				if(plane_object_collision(o, &objects[i]))
					return 1;
				break;
		}
	}

	return 0;
}
