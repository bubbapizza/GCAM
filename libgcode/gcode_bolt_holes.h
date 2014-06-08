/*
*  gcode_bolt_holes.h
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
#ifndef _GCODE_BOLT_HOLES_H
#define _GCODE_BOLT_HOLES_H

#include "gcode_internal.h"

#define GCODE_DATA_BOLT_HOLES_EXTRUSION		0x00
#define GCODE_DATA_BOLT_HOLES_POS		0x01
#define GCODE_DATA_BOLT_HOLES_HOLE_DIAMETER	0x02
#define GCODE_DATA_BOLT_HOLES_OFFSET_DISTANCE	0x03
#define	GCODE_DATA_BOLT_HOLES_TYPE		0x04
#define GCODE_DATA_BOLT_HOLES_NUM		0x05
#define GCODE_DATA_BOLT_HOLES_OFFSET_ANGLE	0x06
#define	GCODE_DATA_BOLT_HOLES_POCKET		0x07

#define GCODE_BOLT_HOLES_TYPE_RADIAL		0x0
#define	GCODE_BOLT_HOLES_TYPE_MATRIX		0x1

typedef struct gcode_bolt_holes_s
{
  gcode_vec2d_t pos;
  int num[2];		/* num[0] used in radial, num[0] AND num[1] used in matrix */
  uint8_t type;
  gfloat_t hole_diameter;
  gfloat_t offset_distance;
  gfloat_t offset_angle;
  uint8_t pocket;
  gcode_block_t *extrusion;
  gcode_block_t *arc_list;
  gcode_offset_t offset;
} gcode_bolt_holes_t;

void gcode_bolt_holes_init (GCODE_INIT_PARAMETERS);
void gcode_bolt_holes_free (gcode_block_t **block);
void gcode_bolt_holes_make (gcode_block_t *block);
void gcode_bolt_holes_save (gcode_block_t *block, FILE *fh);
void gcode_bolt_holes_load (gcode_block_t *block, FILE *fh);
void gcode_bolt_holes_draw (gcode_block_t *block, gcode_block_t *selected);
void gcode_bolt_holes_duplicate (gcode_block_t *block, gcode_block_t **duplicate);
void gcode_bolt_holes_scale (gcode_block_t *block, gfloat_t scale);
void gcode_bolt_holes_rebuild (gcode_block_t *block);


#endif
