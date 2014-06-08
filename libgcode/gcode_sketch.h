/*
*  gcode_sketch.h
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
#ifndef _GCODE_SKETCH_H
#define _GCODE_SKETCH_H

#include "gcode_internal.h"

#define GCODE_DATA_SKETCH_EXTRUSION	0x00
#define GCODE_DATA_SKETCH_NUM		0x01
#define	GCODE_DATA_SKETCH_TAPER_OFFSET	0x04
#define GCODE_DATA_SKETCH_POCKET	0x05
#define	GCODE_DATA_SKETCH_ZERO_PASS	0x06
#define GCODE_DATA_SKETCH_HELICAL	0x07

typedef struct gcode_sketch_s
{
  gcode_block_t *extrusion;
  gcode_block_t *list;
  gcode_vec2d_t taper_offset;
  uint8_t pocket;
  uint8_t zero_pass;
  uint8_t helical;
  gcode_offset_t offset;
} gcode_sketch_t;

void gcode_sketch_init (GCODE_INIT_PARAMETERS);
void gcode_sketch_free (gcode_block_t **block);
void gcode_sketch_make (gcode_block_t *block);
void gcode_sketch_save (gcode_block_t *block, FILE *fh);
void gcode_sketch_load (gcode_block_t *block, FILE *fh);
void gcode_sketch_draw (gcode_block_t *block, gcode_block_t *selected);
void gcode_sketch_duplicate (gcode_block_t *block, gcode_block_t **duplicate);
void gcode_sketch_scale (gcode_block_t *block, gfloat_t scale);
void gcode_sketch_aabb (gcode_block_t *block, gcode_vec2d_t min, gcode_vec2d_t max);
void gcode_sketch_pattern (gcode_block_t *block, int iterations, gfloat_t translate_x, gfloat_t translate_y, gfloat_t rotate_about_x, gfloat_t rotate_about_y, gfloat_t rotation);
int gcode_sketch_is_closed (gcode_block_t *block);
gcode_block_t* gcode_sketch_prev_connected (gcode_block_t *block);
gcode_block_t* gcode_sketch_next_connected (gcode_block_t *block);

#endif
