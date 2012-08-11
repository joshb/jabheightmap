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
#include "my_math.h"
#include "octree.h"

#define MAX_LEAF_SIZE 10.0f /* leaf node width/height/depth will <= this */

/*
 * recursively create octree nodes until we have
 * leaf nodes with a size <= MAX_LEAF_SIZE
 * max[xyz] and min[xyz] are the boundaries
 */
struct octree_node *
new_octree_branch(struct octree_node *parent, float minx, float maxx,
                  float miny, float maxy, float minz, float maxz)
{
	int i;
	struct octree_node *branch;
	float midx, midy, midz;

	if(maxx - minx <= MAX_LEAF_SIZE)
		return NULL;
	if(maxy - miny <= MAX_LEAF_SIZE)
		return NULL;
	if(maxz - minz <= MAX_LEAF_SIZE)
		return NULL;

	midx = minx + ((maxx - minx) / 2);
	midy = miny + ((maxy - miny) / 2);
	midz = minz + ((maxz - minz) / 2);

	branch = malloc(sizeof(struct octree_node));
	if(!branch) {
		fprintf(stderr, "Error: Couldn't allocate memory for octree node\n");
		return NULL;
	}

	branch->subnodes[0] = new_octree_branch(branch, minx, midx, miny, midy, midz, maxz);
	branch->subnodes[1] = new_octree_branch(branch, midx, maxx, miny, midy, midz, maxz);
	branch->subnodes[2] = new_octree_branch(branch, midx, maxx, midy, maxy, midz, maxz);
	branch->subnodes[3] = new_octree_branch(branch, minx, midx, midy, maxy, midz, maxz);

	branch->subnodes[4] = new_octree_branch(branch, minx, midx, miny, midy, minz, midz);
	branch->subnodes[5] = new_octree_branch(branch, midx, maxx, miny, midy, minz, midz);
	branch->subnodes[6] = new_octree_branch(branch, midx, maxx, midy, maxy, minz, midz);
	branch->subnodes[7] = new_octree_branch(branch, minx, midx, midy, maxy, minz, midz);

	branch->parent = parent;
	branch->minx = minx; branch->maxx = maxx;
	branch->miny = miny; branch->maxy = maxy;
	branch->minz = minz; branch->maxz = maxz;
	branch->num_objects = 0;
	for(i = 0; i < MAX_OCTREE_NODE_OBJECTS; i++)
		branch->objects[i] = 0;

	return branch;
}

void
free_octree_branch(struct octree_node *o)
{
	int i;

	if(!o)
		return;

	for(i = 0; i < 8; i++)
		free_octree_branch(o->subnodes[i]);

	free(o);
}

/* get the leaf node that point p falls within */
struct octree_node *
get_octree_leaf_from_point(struct octree_node *root, float p[3])
{
	int i;

	if(!root->subnodes[0])
		return root;

	for(i = 0; i < 8; i++) {
		if(p[0] >= root->subnodes[i]->minx && p[0] < root->subnodes[i]->maxx &&
		   p[1] >= root->subnodes[i]->miny && p[1] < root->subnodes[i]->maxy &&
		   p[2] >= root->subnodes[i]->minz && p[2] < root->subnodes[i]->maxz)
			return get_octree_leaf_from_point(root->subnodes[i], p);
	}

	return root;
}

/* add an object to a node */
void
add_object_to_octree_node(struct octree_node *on, struct object *o)
{
	int i;

	if(!on || !o)
		return;

	if(on->num_objects >= MAX_OCTREE_NODE_OBJECTS) {
		fprintf(stderr, "Error: octree_node already has max number of objects\n");
		return;
	}

	i = get_object_num(o);
	if(i == -1) {
		fprintf(stderr, "Error: Couldn't get number for object at %p\n", o);
		return;
	}
	on->objects[on->num_objects] = i;
	on->num_objects++;
}

/*
 * recursively draw objects in a branch; if the node
 * we're testing is outside of the view frustum, don't
 * draw its objects or process the child nodes
 */
void
draw_octree_branch_objects(struct octree_node *branch)
{
	int i;
	float v[8][3];
	float mid;

	if(!branch)
		return;

	mid = (branch->maxx - branch->minx) * 0.5f;
	v[0][0] = branch->minx + mid;
	v[0][1] = branch->miny + mid;
	v[0][2] = branch->minz + mid;

	if(!is_point_in_viewport(v[0], mid * 4.0f))
		return;

	if(!branch->parent)
		glBegin(GL_QUADS);

	for(i = 0; i < branch->num_objects; i++)
		draw_object(branch->objects[i]);

	for(i = 0; i < 8; i++)
		draw_octree_branch_objects(branch->subnodes[i]);

	if(!branch->parent)
		glEnd();
}
