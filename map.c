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
#include "map.h"
#include "object.h"
#include "octree.h"
#include "my_math.h"

extern void *read_png(const char *, unsigned int *, unsigned int *, int *);

/* return highest of four floats */
static float
highest(float f1, float f2, float f3, float f4)
{
	float retval;

	retval = f1;
	if(f2 > retval)
		retval = f2;
	if(f3 > retval)
		retval = f3;
	if(f4 > retval)
		retval = f4;

	return retval;
}

/* return lowest of four floats */
static float
lowest(float f1, float f2, float f3, float f4)
{
	float retval;

	retval = f1;
	if(f2 < retval)
		retval = f2;
	if(f3 < retval)
		retval = f3;
	if(f4 < retval)
		retval = f4;

	return retval;
}

unsigned char
get_pixel(unsigned char *data, unsigned int x, unsigned int y, unsigned int w)
{
	return data[(w * y * 3) + (x * 3)];
}

/*
 * create an octree, load the heightmap, load all
 * map quads into object structures and place them
 * in the appropriate leaf nodes of the octree
 */
struct map *
load_map(const char *filename)
{
	unsigned int i, j, k;
	unsigned char *data;
	unsigned int width, height;
	int type;
	unsigned int tilesize = 8;
	float xydiv = 1.0f;
	float zdiv = 9.0f;
	static struct map map_structure;
	struct vertex vertices[4];
	struct object *o;
	struct plane_object *p;
	struct octree_node *on;

	/* data is a pointer to the raw rgb data of the heightmap */
	data = (unsigned char *)read_png(filename, &width, &height, &type);
	if(!data) {
		fprintf(stderr, "Error: Couldn't load heightmap %s\n", filename);
		return NULL;
	}

	map_structure.octree = new_octree_branch(NULL, -((float)width / xydiv), (float)width / xydiv, -((float)height / xydiv), (float)height / xydiv, -255.0f, 255.0f);
	if(!map_structure.octree) {
		fprintf(stderr, "Error: Couldn't create octree\n");
		free(data);
		return NULL;
	}

	snprintf(map_structure.skypic, 256, "data/sky.png");

	/*
	 * create map quads from heightmap; the z value of each
	 * point is taken from the pixel corresponding to the
	 * current position on the heightmap using the get_pixel
	 * function
	 */
	k = 0;
	for(i = 0; i < height - tilesize; i+= tilesize) {
		for(j = 0; j < width - tilesize; j += tilesize) {
			vertices[k].texcoord[0] = 0.25f * (j/tilesize % 4);
			vertices[k].texcoord[1] = 0.25f * (i/tilesize % 4);
			vertices[k].point[0] = (float)j / xydiv - (float)(width / 2) / xydiv;
			vertices[k].point[1] = (float)(i + tilesize) / xydiv - (float)(height / 2) / xydiv;
			vertices[k].point[2] = (float)get_pixel(data, j, i + tilesize, width) / zdiv;
			k++;

			vertices[k].texcoord[0] = 0.25f * ((j/tilesize % 4) + 1.0f);
			vertices[k].texcoord[1] = 0.25f * (i/tilesize % 4);
			vertices[k].point[0] = (float)(j + tilesize) / xydiv - (float)(width / 2) / xydiv;
			vertices[k].point[1] = (float)(i + tilesize) / xydiv - (float)(height / 2) / xydiv;
			vertices[k].point[2] = (float)get_pixel(data, j + tilesize, i + tilesize, width) / zdiv;
			k++;

			vertices[k].texcoord[0] = 0.25f * ((j/tilesize % 4) + 1.0f);
			vertices[k].texcoord[1] = 0.25f * ((i/tilesize % 4) + 1.0f);
			vertices[k].point[0] = (float)(j + tilesize) / xydiv - (float)(width / 2) / xydiv;
			vertices[k].point[1] = (float)i / xydiv - (float)(height / 2) / xydiv;
			vertices[k].point[2] = (float)get_pixel(data, j + tilesize, i, width) / zdiv;
			k++;

			vertices[k].texcoord[0] = 0.25f * (j/tilesize % 4);
			vertices[k].texcoord[1] = 0.25f * ((i/tilesize % 4) + 1.0f);
			vertices[k].point[0] = (float)j / xydiv - (float)(width / 2) / xydiv;
			vertices[k].point[1] = (float)i / xydiv - (float)(height / 2) / xydiv;
			vertices[k].point[2] = (float)get_pixel(data, j, i, width) / zdiv;
			k = 0;

			o = create_object(OBJ_PLANE);
			if(!o) {
				free(data);
				free_octree_branch(map_structure.octree);
				return NULL;
			}

			o->render_separately = 0;
			o->vertices = malloc(sizeof(struct vertex) * 4);
			if(!o->vertices) {
				fprintf(stderr, "Error: Couldn't allocate memory for heightmap vertices\n");	
				free(data);
				free_octree_branch(map_structure.octree);
				return NULL;
			}
			o->num_vertices = 4;
			memcpy(o->vertices, vertices, sizeof(struct vertex) * 4);

			p = (struct plane_object *)(o->aux);
			setup_plane(p->plane, o->vertices[0].point, o->vertices[1].point, o->vertices[2].point);
			p->minx = lowest(o->vertices[0].point[0], o->vertices[1].point[0],
			                 o->vertices[2].point[0], o->vertices[3].point[0]);
			p->maxx = highest(o->vertices[0].point[0], o->vertices[1].point[0],
			                 o->vertices[2].point[0], o->vertices[3].point[0]);
			p->miny = lowest(o->vertices[0].point[1], o->vertices[1].point[1],
			                 o->vertices[2].point[1], o->vertices[3].point[1]);
			p->maxy = highest(o->vertices[0].point[1], o->vertices[1].point[1],
			                 o->vertices[2].point[1], o->vertices[3].point[1]);
			p->minz = lowest(o->vertices[0].point[2], o->vertices[1].point[2],
			                 o->vertices[2].point[2], o->vertices[3].point[2]);
			p->maxz = highest(o->vertices[0].point[2], o->vertices[1].point[2],
			                 o->vertices[2].point[2], o->vertices[3].point[2]);

			on = get_octree_leaf_from_point(map_structure.octree, o->vertices[0].point);
			add_object_to_octree_node(on, o);
		}
	}

	free(data);
	fprintf(stderr, "%s loaded\n", filename);
	return &map_structure;
}
