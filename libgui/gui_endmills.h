/*
*  gui_endmills.h
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
#ifndef _GUI_ENDMILLS_H
#define _GUI_ENDMILLS_H

#include "gcode.h"

typedef struct gui_endmill_s
{
  uint8_t number;
  gfloat_t diameter;
  uint8_t unit;
  char description[64];
} gui_endmill_t;


typedef struct gui_endmill_list_s
{
  gui_endmill_t *endmill;
  uint8_t num;
} gui_endmill_list_t;


void gui_endmills_init (gui_endmill_list_t *endmill_list);
void gui_endmills_free (gui_endmill_list_t *endmill_list);
int gui_endmills_read (gui_endmill_list_t *endmill_list, gcode_t *gcode);

#endif
