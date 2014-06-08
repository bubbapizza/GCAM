/*
*  gui_machines.h
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
#ifndef _GUI_MACHINES_H
#define _GUI_MACHINES_H

#include "gcode.h"

typedef struct gui_machine_s
{
  char name[64];
  gfloat_t travel[3];
  gfloat_t maxipm[3];
  unsigned char options;
} gui_machine_t;


typedef struct gui_machine_list_s
{
  gui_machine_t *machine;
  int num;
} gui_machine_list_t;


void gui_machines_init (gui_machine_list_t *machine_list);
void gui_machines_free (gui_machine_list_t *machine_list);
int gui_machines_read (gui_machine_list_t *machine_list);

#endif
