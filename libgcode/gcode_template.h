/*
*  gcode_template.h
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
#ifndef _GCODE_TEMPLATE_H
#define _GCODE_TEMPLATE_H

#include "gcode_internal.h"

#define GCODE_DATA_TEMPLATE_NUM		0x00
#define GCODE_DATA_TEMPLATE_POSITION	0x01
#define GCODE_DATA_TEMPLATE_ROTATION	0x02

typedef struct gcode_template_s
{
  gcode_block_t *list;
  gcode_vec2d_t position;
  gfloat_t rotation;
  gcode_offset_t offset;
} gcode_template_t;

void gcode_template_init (GCODE_INIT_PARAMETERS);
void gcode_template_free (gcode_block_t **block);
void gcode_template_make (gcode_block_t *block);
void gcode_template_save (gcode_block_t *block, FILE *fh);
void gcode_template_load (gcode_block_t *block, FILE *fh);
void gcode_template_draw (gcode_block_t *block, gcode_block_t *selected);
void gcode_template_duplicate (gcode_block_t *block, gcode_block_t **duplicate);
void gcode_template_scale (gcode_block_t *block, gfloat_t scale);

#endif
