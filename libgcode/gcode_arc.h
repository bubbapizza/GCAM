/*
*  gcode_arc.h
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
#ifndef _GCODE_ARC_H
#define _GCODE_ARC_H

#include "gcode_internal.h"

#define	GCODE_DATA_ARC_POS		0x00
#define	GCODE_DATA_ARC_RADIUS		0x01
#define	GCODE_DATA_ARC_START		0x02
#define	GCODE_DATA_ARC_SWEEP		0x03
#define GCODE_DATA_ARC_INTERFACE	0x04

#define GCODE_ARC_INTERFACE_SWEEP	0x0
#define	GCODE_ARC_INTERFACE_RADIUS	0x1
#define GCODE_ARC_INTERFACE_CENTER	0x2

typedef struct gcode_arc_s
{
  gfloat_t pos[2];
  gfloat_t radius;
  gfloat_t start_angle;
  gfloat_t sweep;
  uint8_t interface;
} gcode_arc_t;

void gcode_arc_init (GCODE_INIT_PARAMETERS);
void gcode_arc_free (gcode_block_t **block);
void gcode_arc_make (gcode_block_t *block);
void gcode_arc_save (gcode_block_t *block, FILE *fh);
void gcode_arc_load (gcode_block_t *block, FILE *fh);
int gcode_arc_ends (gcode_block_t *block, gfloat_t p0[2], gfloat_t p1[2], uint8_t mode);
void gcode_arc_draw (gcode_block_t *block, gcode_block_t *selected);
int gcode_arc_eval (gcode_block_t *block, gfloat_t y, gfloat_t *x_array, int *xind);
gfloat_t gcode_arc_length (gcode_block_t *block);
void gcode_arc_duplicate (gcode_block_t *block, gcode_block_t **duplicate);
void gcode_arc_scale (gcode_block_t *block, gfloat_t scale);
void gcode_arc_aabb (gcode_block_t *block, gcode_vec2d_t min, gcode_vec2d_t max);
void gcode_arc_with_offset (gcode_block_t *block, gcode_vec2d_t origin, gcode_vec2d_t center, gcode_vec2d_t p0, gfloat_t *arc_radius_offset, gfloat_t *start_angle);
void gcode_arc_flip_direction (gcode_block_t *block);

#endif
