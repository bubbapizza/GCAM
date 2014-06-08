/*
*  gcode_begin.c
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
#include "gcode_begin.h"
#include <time.h>

void
gcode_begin_init (GCODE_INIT_PARAMETERS)
{
  gcode_begin_t *begin;

  *block = (gcode_block_t *) malloc (sizeof (gcode_block_t));
  gcode_internal_init (*block, parent, gcode, GCODE_TYPE_BEGIN, GCODE_FLAGS_LOCK);

  (*block)->free = gcode_begin_free;
  (*block)->make = gcode_begin_make;
  (*block)->save = gcode_begin_save;
  (*block)->load = gcode_begin_load;
  (*block)->pdata = malloc (sizeof (gcode_begin_t));

  strcpy ((*block)->comment, "Initialize Mill");
  strcpy ((*block)->status, "OK");
  GCODE_INIT((*block));
  GCODE_CLEAR((*block));

  begin = (gcode_begin_t *) (*block)->pdata;
  begin->coordinate_system = GCODE_BEGIN_COORDINATE_SYSTEM_NONE;
}


void
gcode_begin_free (gcode_block_t **block)
{
  free (*block);
  *block = NULL;
}


void
gcode_begin_make (gcode_block_t *block)
{
  gcode_begin_t *begin;
  char string[256], date_string[32];
  time_t timer;

  begin = (gcode_begin_t *) block->pdata;

  GCODE_CLEAR(block);

  if (block->gcode->driver == GCODE_DRIVER_HAAS)
  {
    GCODE_APPEND (block, "%\n");
    sprintf (string, "O%.5d\n", block->gcode->project_number);
    GCODE_APPEND (block, string);
  }

  sprintf (string, "Project: %s", block->gcode->name);
  GCODE_COMMENT (block, string);
  timer = time (NULL);
  strcpy (date_string, asctime (localtime (&timer)));
  date_string[strlen (date_string) - 1] = 0;
  sprintf (string, "Created: %s", date_string);
  GCODE_COMMENT (block, string);
  sprintf (string, "Material Dimensions: X=%.3f Y=%.3f Z=%.3f", block->gcode->material_size[0], block->gcode->material_size[1], block->gcode->material_size[2]);
  GCODE_COMMENT (block, string);
  sprintf (string, "Notes: %s", block->gcode->notes);
  GCODE_COMMENT (block, string);
  sprintf (string, "GCAM:ORIGIN:%f:%f:%f", block->gcode->material_origin[0], block->gcode->material_origin[1], block->gcode->material_origin[2]);
  GCODE_COMMENT (block, string);

  GCODE_APPEND (block, "\n");

  sprintf (string, "%s", block->comment);
  GCODE_COMMENT (block, string);
  GCODE_APPEND (block, "\n");

  if (begin->coordinate_system == GCODE_BEGIN_COORDINATE_SYSTEM_NONE)
  {
    GCODE_COMMENT (block, "Machine Coordinates");
  }
  else
  {
    sprintf (string, "G%d ", 53+begin->coordinate_system);
    GCODE_APPEND (block, string);

    sprintf (string, "Workspace %d", begin->coordinate_system);
    GCODE_COMMENT (block, string);
  }

  if (block->gcode->units == GCODE_UNITS_INCH)
  {
    GCODE_APPEND (block, "G20 ");
    GCODE_COMMENT (block, "Units are inches");
  }
  else
  {
    GCODE_APPEND (block, "G21 ");
    GCODE_COMMENT (block, "Units are millimeters");
  }

  GCODE_APPEND (block, "G90 ");
  GCODE_COMMENT (block, "Absolute Positioning");
}


void
gcode_begin_save (gcode_block_t *block, FILE *fh)
{
  gcode_begin_t *begin;
  uint32_t size;
  uint8_t data;
  
  begin = (gcode_begin_t *) block->pdata;
  
  data = GCODE_DATA_BEGIN_COORDINATE_SYSTEM;
  size = sizeof (uint8_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&begin->coordinate_system, size, 1, fh);
}


void
gcode_begin_load (gcode_block_t *block, FILE *fh)
{
  gcode_begin_t *begin;
  uint32_t bsize, dsize, start;
  uint8_t data;

  begin = (gcode_begin_t *) block->pdata;

  fread (&bsize, sizeof (uint32_t), 1, fh);

  start = ftell (fh);
  while (ftell (fh) - start < bsize)
  {
    fread (&data, sizeof (uint8_t), 1, fh);
    fread (&dsize, sizeof (uint32_t), 1, fh);

    switch (data)
    {
      case GCODE_DATA_BLOCK_COMMENT:
        fread (block->comment, sizeof (char), dsize, fh);
        break;

      case GCODE_DATA_BLOCK_FLAGS:
        fread (&block->flags, sizeof (uint8_t), dsize, fh);
        break;

      case GCODE_DATA_BEGIN_COORDINATE_SYSTEM:
        fread (&begin->coordinate_system, dsize, 1, fh);
        break;

      default:
        fseek (fh, dsize, SEEK_CUR);
        break;
    }
  }
}
