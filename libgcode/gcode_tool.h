/*
*  gcode_tool.h
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
#ifndef _GCODE_TOOL_H
#define _GCODE_TOOL_H

#include "gcode_internal.h"

#define	GCODE_DATA_TOOL_DIAM			0x00
#define	GCODE_DATA_TOOL_LEN			0x01
#define	GCODE_DATA_TOOL_PROMPT			0x02
#define	GCODE_DATA_TOOL_LABEL			0x03
#define GCODE_DATA_TOOL_FEED			0x04
#define GCODE_DATA_TOOL_CHANGE			0x05
#define GCODE_DATA_TOOL_NUMBER			0x06
#define GCODE_DATA_TOOL_PLUNGE_RATIO		0x07
#define GCODE_DATA_TOOL_SPINDLE_RPM		0x08
#define GCODE_DATA_TOOL_COOLANT			0x09

typedef struct gcode_tool_s
{
  gfloat_t diam;
  gfloat_t len;
  uint8_t prompt;
  char label[32];
  gfloat_t hinc;
  gfloat_t vinc;
  gfloat_t feed;
  gfloat_t change[3];
  uint8_t number;
  gfloat_t plunge_ratio;
  uint32_t spindle_rpm;
  uint8_t coolant;
} gcode_tool_t;

void gcode_tool_init (GCODE_INIT_PARAMETERS);
void gcode_tool_free (gcode_block_t **block);
void gcode_tool_make (gcode_block_t *block);
void gcode_tool_save (gcode_block_t *block, FILE *fh);
void gcode_tool_load (gcode_block_t *block, FILE *fh);
void gcode_tool_duplicate (gcode_block_t *block, gcode_block_t **duplicate);
void gcode_tool_calc (gcode_block_t *block);
gcode_tool_t* gcode_tool_find (gcode_block_t *block);

#endif
