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
#include <string.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include "texture.h"
#include "object.h"
#include "octree.h"
#include "map.h"
#include "my_math.h"
#include "world.h"

static char terrainpic[] = "data/terrain.png";
static struct camera *cam = NULL;
static struct octree_node *octree = NULL;

void
init_world()
{
	struct map *m;

	cam = malloc(sizeof(struct camera));
	if(!cam) {
		fprintf(stderr, "Error: Couldn't allocate memory for camera\n");
		exit(1);
	}

	bzero(cam, sizeof(struct camera));
	cam->direction[1] = -1.0f;

	m = load_map("data/map.png");
	if(!m) {
		fprintf(stderr, "Error: Couldn't load map\n");
		exit(1);
	}

	glEnable(GL_TEXTURE_2D);
	load_texture_from_png(terrainpic);

	octree = m->octree;
#if 0
	skypic = m->skypic;
#endif
}

void
world_cleanup()
{
	free_octree_branch(octree);
	free_all_objects();
	free_all_textures();
	free(cam);
}

static void
world_rotate_camera(int x, int y)
{
	float olddir[3];
	float tmp;

	olddir[0] = cam->direction[0];
	olddir[1] = cam->direction[1];
	olddir[2] = cam->direction[2];

	tmp = (float)y;
	cam->rotation[0] += tmp;

	tmp = (float)x;
	cam->direction[0] = olddir[0] * cosf(DEG2RAD(tmp)) + olddir[1] * sinf(DEG2RAD(tmp));
	cam->direction[1] = olddir[1] * cosf(DEG2RAD(tmp)) - olddir[0] * sinf(DEG2RAD(tmp));
	cam->rotation[2] += tmp;
}

void
world_mouse_input(Window window, XMotionEvent *e)
{
	static int old_x = -1, old_y = -1;
	int x_rel, y_rel;

	if(e->type != MotionNotify)
		return;

	if(old_x == -1 || old_y == -1) {
		old_x = e->x;
		old_y = e->y;
		return;
	}

	x_rel = e->x - old_x;
	y_rel = e->y - old_y;

	if(e->x != 320 || e->y != 240) {
		world_rotate_camera(x_rel, y_rel);
		XWarpPointer(e->display, window, window, e->x, e->y, 640, 480, 320, 240);
	}

	old_x = e->x;
	old_y = e->y;
}

void
world_key_input(Window window, XKeyEvent *e)
{
	if(!(e->type & KeyPressMask))
		return;

	switch(XKeycodeToKeysym(e->display, e->keycode, 0)) {
		default:
			break;
		case XK_Up:
			cam->obj.position[0] -= cam->direction[0] * 1.0f;
			cam->obj.position[1] -= cam->direction[1] * 1.0f;
			break;
		case XK_Down:
			cam->obj.position[0] += cam->direction[0] * 1.0f;
			cam->obj.position[1] += cam->direction[1] * 1.0f;
			break;
		case XK_Left:
			cam->obj.position[0] += cam->direction[1] * 1.0f;
			cam->obj.position[1] -= cam->direction[0] * 1.0f;
			break;
		case XK_Right:
			cam->obj.position[0] -= cam->direction[1] * 1.0f;
			cam->obj.position[1] += cam->direction[0] * 1.0f;
			break;
	}
}

#if 0 /* making use of this function is left as an exercise for the reader */
static void
draw_skybox()
{
	static struct texture *t = NULL;
	static float separation = (300.0f / 2.0f);
	float topcolor[3] = { 0.0f, 0.0f, 0.0f };
	float high = 0.0f;
	float low = 1.5f;

	if(!t) {
		t = get_texture_with_name(skypic);
		if(!t) {
			fprintf(stderr, "Error: Sky texture not loaded; exiting...\n");
			exit(1);
		}
	}

	glBindTexture(GL_TEXTURE_2D, t->gl_num);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
		glColor3fv(topcolor);
		glTexCoord2f(0.25f, high);
		glVertex3f(-separation, -separation, separation);
		glTexCoord2f(0.0f, high);
		glVertex3f(separation, -separation, separation);
		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, low);
		glVertex3f(separation, -separation, -separation);
		glTexCoord2f(0.25f, low);
		glVertex3f(-separation, -separation, -separation);

		glColor3fv(topcolor);
		glTexCoord2f(0.5f, high);
		glVertex3f(-separation, separation, separation);
		glTexCoord2f(0.75f, high);
		glVertex3f(separation, separation, separation);
		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.75f, low);
		glVertex3f(separation, separation, -separation);
		glTexCoord2f(0.5f, low);
		glVertex3f(-separation, separation, -separation);

		glColor3fv(topcolor);
		glTexCoord2f(0.25f, high);
		glVertex3f(-separation, -separation, separation);
		glTexCoord2f(0.5f, high);
		glVertex3f(-separation, separation, separation);
		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.5f, low);
		glVertex3f(-separation, separation, -separation);
		glTexCoord2f(0.25f, low);
		glVertex3f(-separation, -separation, -separation);

		glColor3fv(topcolor);
		glTexCoord2f(1.0f, high);
		glVertex3f(separation, -separation, separation);
		glTexCoord2f(0.75f, high);
		glVertex3f(separation, separation, separation);
		glColor3f(1.0f, 1.0f, 1.0f);
		glTexCoord2f(0.75f, low);
		glVertex3f(separation, separation, -separation);
		glTexCoord2f(1.0f, low);
		glVertex3f(separation, -separation, -separation);
	glEnd();
}
#endif

void
draw_world(Display *dpy, GLXDrawable drawable)
{
	static float fogcolor[3] = { 0.25f, 0.25f, 0.3f };
	static struct texture *t = NULL;

	if(!t) {
		t = get_texture_with_name(terrainpic);
		if(!t) {
			fprintf(stderr, "Error: terrain texture not loaded; exiting...\n");
			exit(1);
		}
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	glRotatef(cam->rotation[0] - 90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(cam->rotation[1], 0.0f, 1.0f, 0.0f);
	glRotatef(cam->rotation[2], 0.0f, 0.0f, 1.0f);
	glTranslatef(-cam->obj.position[0], -cam->obj.position[1], -cam->obj.position[2] - 1.5f);
	update_view_frustum();

#if 0
	glDisable(GL_FOG);
	draw_skybox();
#endif

	glEnable(GL_FOG);
	glFogfv(GL_FOG_COLOR, fogcolor);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_DENSITY, 0.1f);
	glFogf(GL_FOG_START, 0.5f);
	glFogf(GL_FOG_END, 200.0f);

	glBindTexture(GL_TEXTURE_2D, t->gl_num);
	glColor4f(0.9f, 0.9f, 0.9f, 1.0f);
	draw_octree_branch_objects(octree);

	glFlush();
	glXSwapBuffers(dpy, drawable);

	cam->obj.position[2] -= 1.0f;
	while(object_collision(&(cam->obj)))
		cam->obj.position[2] += 1.0f;
}
