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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include "object.h"
#include "world.h"

extern void quit_app();
extern void screenshot();

static int wireframe = 0;

static void
key_input(Window window, XKeyEvent *e)
{
	switch(e->type) {
		default:
			break;
		case KeyPress:
			switch(XKeycodeToKeysym(e->display, e->keycode, 0)) {
				default:
					break;
				case XK_Escape:
					quit_app();
					break;
				case XK_s:
					screenshot();
					break;
				case XK_w:
					wireframe = wireframe ? 0 : 1;
					if(wireframe)
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					else
						glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					break;
			}
			break;
	}

	world_key_input(window, e);
}

/* run event-type-specific input functions */
void
check_input(Display *dpy, Window window)
{
	XEvent event;

	XNextEvent(dpy, &event);
	switch(event.type) {
		default:
			break;
		case KeyPress:
		case KeyRelease:
			key_input(window, &(event.xkey));
			break;
		case MotionNotify:
			world_mouse_input(window, &(event.xmotion));
			break;
	}
}
