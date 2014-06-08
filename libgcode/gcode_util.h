/*
*  gcode_util.h
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
#ifndef _GCODE_UTIL_H
#define _GCODE_UTIL_H

#include "gcode_internal.h"

/* Scales default values such that they are relatively the similiar meaning but clean rounded values. */
/* #define GCODE_UNITS(_gcode, _num) (_gcode->units == GCODE_UNITS_MILLIMETER ? _num * 25.0 : _num) */

int gcode_util_qsort_compare_asc (const void *a, const void *b);
void gcode_util_remove_spaces (char *string);
void gcode_util_remove_comment (char *string);
void gcode_util_remove_duplicate_scalars (gfloat_t *array, uint32_t *num);
int gcode_util_intersect (gcode_block_t *block_a, gcode_block_t *block_b, gcode_vec2d_t ip_array[2], int *ip_num);
void gcode_util_duplicate_list (gcode_block_t *start_block, gcode_block_t *end_block, gcode_block_t **duplicate_list);
void gcode_util_push_offset (gcode_block_t *list);
void gcode_util_fillet (gcode_block_t *line1, gcode_block_t *line2, gcode_block_t *fillet_arc, gfloat_t radius);
void gcode_util_order_list (gcode_block_t *list);

#endif
