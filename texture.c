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
#include <png.h>
#include "texture.h"

void *read_png(const char *, unsigned int *, unsigned int *, int *);

static struct texture *textures = NULL;
static unsigned int num_textures = 0;
static unsigned int curr_gl_num = 0;

static struct texture *
create_texture_structure()
{
	struct texture *tmp;

	tmp = realloc(textures, sizeof(struct texture) * (num_textures + 1));
	if(!tmp) {
		fprintf(stderr, "Error: Couldn't allocate memory for texture\n");
		return NULL;
	}

	textures = tmp;

	return &textures[num_textures++];
}

void
free_all_textures()
{
	free(textures);
	num_textures = 0;
}

struct texture *
get_texture_with_name(const char *filename)
{
	int i;

	for(i = 0; i < num_textures; i++) {
		if(strcmp(textures[i].name, filename) == 0)
			return &textures[i];
	}

	return NULL;
}

struct texture *
load_texture_from_png(const char *filename)
{
	struct texture *newtexture;
	unsigned int width, height;
	int type;
	unsigned char *data;

	if((newtexture = get_texture_with_name(filename)))
		return newtexture;

	data = (unsigned char *)read_png(filename, &width, &height, &type);
	if(!data) {
		fprintf(stderr, "Error: Couldn't load texture %s\n", filename);
		return NULL;
	}

	newtexture = create_texture_structure();
	if(!newtexture)
		return NULL;

	snprintf(newtexture->name, 256, "%s", filename);
	newtexture->gl_num = curr_gl_num++;
	newtexture->width = width;
	newtexture->height = height;

	glBindTexture(GL_TEXTURE_2D, newtexture->gl_num);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	free(data);

	return newtexture;
}

void *
read_png(const char *filename, unsigned int *widthp, unsigned int *heightp,
         int *typep)
{
	int i;
	void *data;
	FILE *fp;
	unsigned char header[9];
	int width, height;
	int bit_depth, color_type, interlace_method, compression_method, filter_method;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytepp rows;

	if(!filename || !widthp || !heightp)
		return NULL;

	fp = fopen(filename, "rb");
	if(!fp)
		return NULL;
	fread(header, 1, 8, fp);
	if(png_sig_cmp(header, 0, 8) != 0) {
		fclose(fp);
		return NULL;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr) {
		fclose(fp);
		return NULL;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr) {
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		fclose(fp);
		return NULL;
	}

	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose(fp);
		return NULL;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_ALPHA | PNG_TRANSFORM_PACKING, NULL);
	png_get_IHDR(png_ptr, info_ptr, (png_uint_32 *)&width, (png_uint_32 *)&height, &bit_depth, &color_type, &interlace_method, &compression_method, &filter_method);

	rows = (void *)png_get_rows(png_ptr, info_ptr);
	fclose(fp);

	data = malloc(width * 3 * height);
	if(!data)
		return NULL;

	for(i = 0; i < height; i++)
		memcpy((unsigned char *)data + (width * 3 * i), rows[i], width * 3);

	*widthp = width;
	*heightp = height;
	*typep = color_type;

	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	return data;
}
