/*
*  gcode.h
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
#ifndef _GCODE_H
#define _GCODE_H

#include "gcode_internal.h"

void	gcode_list_insert (gcode_block_t **list, gcode_block_t *block);
void	gcode_list_splice (gcode_block_t **list, gcode_block_t *block);
void	gcode_list_remove (gcode_block_t *block);
void	gcode_list_move_prev (gcode_block_t *block);
void	gcode_list_move_next (gcode_block_t *block);
void	gcode_list_make (gcode_t *gcode);
void	gcode_list_free (gcode_block_t **list);

void	gcode_init (gcode_t *gcode);
void	gcode_prep (gcode_t *gcode);
void	gcode_free (gcode_t *gcode);

int	gcode_save (gcode_t *gcode, const char *filename);
int	gcode_load (gcode_t *gcode, const char *filename);
int	gcode_export (gcode_t *gcode, const char *filename);

void	gcode_render_final (gcode_t *gcode, gfloat_t *time_elapsed);

#include "gcode_begin.h"
#include "gcode_end.h"
#include "gcode_template.h"
#include "gcode_tool.h"
#include "gcode_code.h"
#include "gcode_extrusion.h"
#include "gcode_sketch.h"
#include "gcode_line.h"
#include "gcode_arc.h"
#include "gcode_bolt_holes.h"
#include "gcode_drill_holes.h"
#include "gcode_point.h"
#include "gcode_gerber.h"
#include "gcode_excellon.h"
#include "gcode_util.h"
#include "gcode_svg.h"
#include "gcode_image.h"
#include "gcode_stl.h"

#endif
