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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include "object.h"
#include "world.h"

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480

extern void check_input(Display *, Window);

static int done = 0;

void
quit_app()
{
	done = 1;
	world_cleanup();
}

/* makes a raw rgba screenshot */
void
screenshot()
{
	unsigned char *pixels;
	FILE *fp;

	pixels = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * 4);
	if(!pixels)
		return;

	fp = fopen("screen.raw", "w");
	if(!fp) {
		free(pixels);
		return;
	}

	glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	fwrite(pixels, (WINDOW_WIDTH * 4), WINDOW_HEIGHT, fp);
	fclose(fp);

	free(pixels);
}

int
main(int argc, char *argv[])
{
	char *dpyname;
	Display *dpy;
	XVisualInfo *xvisinfo;
	GLXContext context;
	GLXDrawable drawable;
	Window root;
	Window window;
	int attriblist[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 16, None };

	if(!(dpyname = getenv("DISPLAY")))
		dpyname = ":0.0";

	dpy = XOpenDisplay(dpyname);
	if(!dpy) {
		fprintf(stderr, "Error: Couldn't open X display\n");
		return 1;
	}

	xvisinfo = glXChooseVisual(dpy, DefaultScreen(dpy), attriblist);
	if(!xvisinfo) {
		fprintf(stderr, "Error: Couldn't get GLX visual\n");
		return 1;
	}

	root = RootWindow(dpy, xvisinfo->screen);

	context = glXCreateContext(dpy, xvisinfo, 0, GL_TRUE);

	window = XCreateWindow(dpy, root, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, CopyFromParent, 0, NULL, 0, NULL);
	XSelectInput(dpy, window, KeyPressMask | KeyReleaseMask | PointerMotionMask);
	XMapWindow(dpy, window);

	glXMakeCurrent(dpy, window, context);
	drawable = glXGetCurrentDrawable();

	printf("GL vendor: %s\nGL renderer: %s\nGL version %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 350.0f);
	glMatrixMode(GL_MODELVIEW);

	init_world();
	while(!done) {
		draw_world(dpy, drawable);
		while(XPending(dpy))
			check_input(dpy, window);
	}

	XUnmapWindow(dpy, window);
	glXDestroyContext(dpy, context);
	XCloseDisplay(dpy);

	return 0;
}
