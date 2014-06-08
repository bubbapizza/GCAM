/*
*  gcode_internal.h
*  Source code file for G-Code generation, simulation, and visualization
*  library. This software is Copyright (C) 2006 by Justin Shumaker
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _GCODE_INTERNAL_H
#define _GCODE_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "gcode_math.h"

#define GCODE_USE_OPENGL		1

#if GCODE_USE_OPENGL
  #include <GL/gl.h>

  static const gfloat_t GCODE_OPENGL_COLOR[2][3] = { {0.9, 0.82, 0.72}, {1.0, 0.5, 0.2} };
  static const gfloat_t GCODE_OPENGL_POINT_COLOR[3] = {1.0, 1.0, 0.0};
#endif


#define	GCODE_FILE_HEADER		0x4743414d
#define	GCODE_VERSION			0x20100727

#define	GCODE_DATA			0xff
#define	GCODE_DATA_NAME			0x01
#define	GCODE_DATA_UNITS		0x02
#define	GCODE_DATA_MATERIAL_TYPE	0x03
#define	GCODE_DATA_MATERIAL_SIZE	0x04
#define	GCODE_DATA_ZTRAVERSE		0x05
#define	GCODE_DATA_NOTES		0x06
#define GCODE_DATA_MATERIAL_ORIGIN	0x07


/*
* Machine Specific Data/Options/Features/Settings etc.
*/
#define GCODE_DATA_MACHINE				0xfe
#define GCODE_DATA_MACHINE_NAME				0x01
#define GCODE_DATA_MACHINE_OPTIONS			0x02

#define GCODE_MACHINE_OPTION_SPINDLE_CONTROL		0x01
#define GCODE_MACHINE_OPTION_AUTOMATIC_TOOL_CHANGE	0x02
#define GCODE_MACHINE_OPTION_HOME_SWITCHES		0x04
#define GCODE_MACHINE_OPTION_COOLANT			0x08

/*
* GCODE_DATA_BLOCK_XXX flags are complimentary to the specialized DATA_BLOCK ids
* found in the header of each block that begin with the id 0x00.  For this reason
* we count backwards from 0xff for these special #defines.
*/
#define GCODE_DATA_BLOCK_FLAGS		0xfe
#define	GCODE_DATA_BLOCK_COMMENT	0xff

#define	GCODE_UNITS_INCH		0x00
#define	GCODE_UNITS_MILLIMETER		0x01

#define	GCODE_MATERIAL_ALUMINUM		0x00
#define	GCODE_MATERIAL_FOAM		0x01
#define	GCODE_MATERIAL_PLASTIC		0x02
#define	GCODE_MATERIAL_STEEL		0x03
#define	GCODE_MATERIAL_WOOD		0x04

#define GCODE_GET			0x0
#define	GCODE_SET			0x1
#define GCODE_GET_WITH_OFFSET		0x2
#define	GCODE_GET_NORMAL		0x3
#define GCODE_GET_TANGENT		0x4

#define	GCODE_DRIVER_EMC		0x00
#define GCODE_DRIVER_TURBOCNC		0x01
#define GCODE_DRIVER_HAAS		0x02

#define GCODE_FLAGS_LOCK		0x01
#define	GCODE_FLAGS_SUPPRESS		0x02

#define GCODE_INIT_PARAMETERS		gcode_t *gcode, gcode_block_t **block, gcode_block_t *parent

enum
{
	GCODE_TYPE_BEGIN,
	GCODE_TYPE_END,
	GCODE_TYPE_TEMPLATE,
	GCODE_TYPE_TOOL,
	GCODE_TYPE_CODE,
	GCODE_TYPE_EXTRUSION,
	GCODE_TYPE_SKETCH,
	GCODE_TYPE_LINE,
	GCODE_TYPE_ARC,
	GCODE_TYPE_BEZIER,
	GCODE_TYPE_IMAGE,
	GCODE_TYPE_BOLT_HOLES,
	GCODE_TYPE_DRILL_HOLES,
	GCODE_TYPE_POINT,
	GCODE_TYPE_STL,
	GCODE_TYPE_NUM
};

static const char *GCODE_TYPE_STRING[GCODE_TYPE_NUM] = 
{
	"BEGIN",
	"END",
	"TEMPLATE",
	"TOOL",
	"CODE",
	"EXTRUSION",
	"SKETCH",
	"LINE",
	"ARC",
	"BEZIER",
	"IMAGE",
	"BOLT HOLES",
	"DRILL HOLES",
	"POINT",
	"STL"
};


struct gcode_block_s;
typedef void gcode_free_t (struct gcode_block_s **block);
typedef void gcode_make_t (struct gcode_block_s *block);
typedef void gcode_save_t (struct gcode_block_s *block, FILE *fh);
typedef void gcode_load_t (struct gcode_block_s *block, FILE *fh);
typedef int gcode_ends_t (struct gcode_block_s *block, gfloat_t p0[2], gfloat_t p1[2], uint8_t mode);
typedef void gcode_draw_t (struct gcode_block_s *block, struct gcode_block_s *selected);
typedef int gcode_eval_t (struct gcode_block_s *block, gfloat_t y, gfloat_t *x_array, int *xind);
typedef gfloat_t gcode_length_t (struct gcode_block_s *block);
typedef void gcode_duplicate_t (struct gcode_block_s *block, struct gcode_block_s **duplicate);
typedef void gcode_scale_t (struct gcode_block_s *block, gfloat_t scale);
typedef void gcode_aabb_t (struct gcode_block_s *block, gcode_vec2d_t min, gcode_vec2d_t max);

typedef void gcode_progress_callback_t (void *gui, gfloat_t progress);
typedef void gcode_message_callback_t (void *gui, char *message);

typedef struct gcode_offset_s
{
  gfloat_t side;
  gfloat_t tool;
  gfloat_t eval;
  gcode_vec2d_t origin;
  gfloat_t rotation;
  gcode_vec2d_t endmill_pos;
  gcode_vec2d_t z; /* z values for helical paths. */
} gcode_offset_t;


typedef struct gcode_block_s
{
  struct gcode_s *gcode;

  uint8_t type;
  uint8_t flags; /* flags include: lock, supress */
  char comment[64];
  char status[64];

  uint32_t name; /* This is used primarily for opengl picking, so that this blocks rendered lines link back to something in the treeview */
  struct gcode_block_s *parent;
  struct gcode_block_s **parent_list;

  gcode_offset_t *offset;

  void *pdata;

  int code_alloc;
  int code_len;
  char *code;

  gcode_free_t *free;
  gcode_make_t *make;
  gcode_save_t *save;
  gcode_load_t *load;
  gcode_ends_t *ends;
  gcode_draw_t *draw;
  gcode_eval_t *eval;
  gcode_length_t *length;
  gcode_duplicate_t *duplicate;
  gcode_scale_t *scale;
  gcode_aabb_t *aabb;

  struct gcode_block_s *prev;
  struct gcode_block_s *next;
} gcode_block_t;


typedef struct gcode_s
{
  gcode_block_t *list;
  char name[32];
  char notes[512];
  uint8_t units;
  uint8_t material_type;
  gfloat_t material_size[3];
  gfloat_t material_origin[3];
  gfloat_t ztraverse;

  void *gui;
  gcode_progress_callback_t *progress_callback;
  gcode_message_callback_t *message_callback;

  uint16_t voxel_res;
  uint16_t voxel_num[3];
  uint8_t *voxel_map;

  uint8_t driver;

  char machine_name[32];
  uint8_t machine_options;

  uint32_t decimal;	/* number of decimal places to print */

  uint32_t project_number; /* For Haas Machines only */
} gcode_t;


void gcode_internal_init (gcode_block_t *block, gcode_block_t *parent, gcode_t *gcode, uint8_t type, uint8_t flags);
void format_z (char *format, char **format2, unsigned int num);

#define gsprintf(_string, _num, _format, args...) { \
	char *_format2 = NULL; \
	format_z (_format, &_format2, _num); \
	sprintf (_string, _format2 , ##args); \
	free (_format2); }

/* Scales default values such that they are relatively the similiar meaning but clean rounded values. */
#define GCODE_UNITS(_gcode, _num) (_gcode->units == GCODE_UNITS_MILLIMETER ? _num * 25.0 : _num)


#define GCODE_INIT(_block) { \
	_block->code = NULL; \
	}

#define GCODE_CLEAR(_block) { \
	_block->code_len = 1; \
	_block->code_alloc = 1; \
	_block->code = (char *) realloc (_block->code, _block->code_alloc); \
	_block->code[0] = 0; \
	}

/*
* increase by 64kB + string length + 1 if request to append greater than current buffer size.
* includes optimized manual strcat code to vastly improve performance for large strings.
*/
#define GCODE_APPEND(_block, _str) { \
	int _slen = strlen (_str)+1, _i; \
	if (_block->code_len + _slen > _block->code_alloc) \
	{ \
	  _block->code_alloc +=  (1<<16) + _slen; \
	  _block->code = (char *) realloc (_block->code, _block->code_alloc); \
	} \
	for (_i = 0; _i < _slen; _i++) \
	  _block->code[_block->code_len+_i-1] = _str[_i]; \
	_block->code_len += _slen-1; \
	}
/*	strcat (_block->code, _str); */

#define GCODE_COMMENT(_block, _str) { \
	switch (_block->gcode->driver) \
	{ \
	  case GCODE_DRIVER_EMC: \
	    GCODE_APPEND (_block, "("); \
	    GCODE_APPEND (_block, _str); \
	    GCODE_APPEND (_block, ")\n"); \
	    break; \
	  case GCODE_DRIVER_TURBOCNC: \
	    GCODE_APPEND (_block, "; "); \
	    GCODE_APPEND (_block, _str); \
	    GCODE_APPEND (_block, "\n"); \
	    break; \
	  case GCODE_DRIVER_HAAS: \
	    GCODE_APPEND (_block, "("); \
	    GCODE_APPEND (_block, _str); \
	    GCODE_APPEND (_block, ")\n"); \
	    break; \
	} }

#define GCODE_PLUNGE(_block, _depth, _tool) { \
	char _string[256]; \
	gsprintf (_string, _block->gcode->decimal, "G01 Z%z F%.3f ", (_block->gcode->material_origin[2] + _depth), _tool->feed * _tool->plunge_ratio); \
	GCODE_APPEND(_block, _string); \
	GCODE_COMMENT(_block, "plunge"); \
	sprintf (_string, "F%.3f ", _tool->feed); \
	GCODE_APPEND(_block, _string); \
	GCODE_COMMENT(_block, "normal feed rate"); }

#define GCODE_PLUNGE_RAPID(_block, _depth) { \
	char _string[256]; \
	gsprintf (_string, _block->gcode->decimal, "G00 Z%z ", (_block->gcode->material_origin[2] + _depth)); \
	GCODE_APPEND(_block, _string); \
	GCODE_COMMENT(_block, "rapid plunge"); }

#define GCODE_RETRACT(_block, _depth) { \
	char _string[256]; \
	gsprintf (_string, _block->gcode->decimal, "G00 Z%z ", (_block->gcode->material_origin[2] + _depth)); \
	GCODE_APPEND(_block, _string); \
	GCODE_COMMENT(_block, "retract"); }

#define GCODE_WRITE_DATA(_fh, _desc, _size, _ptr) { \
	uint8_t _d = _desc; \
	uint32_t _s = _size; \
	fwrite (&_d, sizeof (uint8_t), 1, _fh); \
	fwrite (&_s, sizeof (uint32_t), 1, _fh); \
	fwrite (_ptr, _size, 1, _fh); }

#endif
