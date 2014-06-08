/*
*  gcode_end.c
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
#include "gcode_end.h"

void
gcode_end_init (GCODE_INIT_PARAMETERS)
{
  gcode_end_t *end;

  *block = (gcode_block_t *) malloc (sizeof (gcode_block_t));
  gcode_internal_init (*block, parent, gcode, GCODE_TYPE_END, GCODE_FLAGS_LOCK);

  (*block)->free = gcode_end_free;
  (*block)->make = gcode_end_make;
  (*block)->save = gcode_end_save;
  (*block)->load = gcode_end_load;
  (*block)->scale = gcode_end_scale;
  (*block)->pdata = malloc (sizeof (gcode_end_t));;

  strcpy ((*block)->comment, "Shutdown Mill");
  strcpy ((*block)->status, "OK");
  GCODE_INIT((*block));
  GCODE_CLEAR((*block));

  end = (gcode_end_t *) (*block)->pdata;
  end->pos[0] = gcode->material_origin[0];
  end->pos[1] = gcode->material_origin[1];
  end->pos[2] = gcode->material_origin[2] + GCODE_UNITS ((*block)->gcode, 1.0);

  end->home = (gcode->machine_options & GCODE_MACHINE_OPTION_HOME_SWITCHES) == 0 ? 0 : 1;
}


void
gcode_end_free (gcode_block_t **block)
{
  free (*block);
  *block = NULL;
}


void
gcode_end_make (gcode_block_t *block)
{
  char string[256];
  gcode_end_t *end;

  end = (gcode_end_t *) block->pdata;

  GCODE_CLEAR(block);

  GCODE_APPEND (block, "\n");
  sprintf (string, "%s", block->comment);
  GCODE_COMMENT (block, string);
  GCODE_APPEND (block, "\n");

  if (block->gcode->machine_options & GCODE_MACHINE_OPTION_HOME_SWITCHES)
  {
    gsprintf (string, block->gcode->decimal, "G28 Z%z\n", block->gcode->ztraverse);
    GCODE_APPEND (block, string);
  }
  else
  {
    gsprintf (string, block->gcode->decimal, "G00 Z%z\n", end->pos[2]);
    GCODE_APPEND (block, string);
    gsprintf (string, block->gcode->decimal, "G00 X%z Y%z\n", end->pos[0], end->pos[1]);
    GCODE_APPEND (block, string);
  }

  sprintf (string, "M30\n");
  GCODE_APPEND(block, string);

  if (block->gcode->driver == GCODE_DRIVER_HAAS)
    GCODE_APPEND (block, "%\n");
}


void
gcode_end_save (gcode_block_t *block, FILE *fh)
{
  gcode_end_t *end;
  uint32_t size;
  uint8_t data;

  end = (gcode_end_t *) block->pdata;

  data = GCODE_DATA_END_RETRACT_POS;
  size = sizeof (gcode_vec3d_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (end->pos, size, 1, fh);

  data = GCODE_DATA_END_HOME_ALL_AXES;
  size = sizeof (uint8_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&end->home, size, 1, fh);
}


void
gcode_end_load (gcode_block_t *block, FILE *fh)
{
  gcode_end_t *end;
  uint32_t bsize, dsize, start;
  uint8_t data;

  end = (gcode_end_t *) block->pdata;

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

      case GCODE_DATA_END_RETRACT_POS:
        fread (end->pos, sizeof (gcode_vec3d_t), 1, fh);
        break;

      case GCODE_DATA_END_HOME_ALL_AXES:
        fread (&end->home, sizeof (uint8_t), 1, fh);
        break;

      default:
        fseek (fh, dsize, SEEK_CUR);
        break;
    }
  }
}


void
gcode_end_scale (gcode_block_t *block, gfloat_t scale)
{
  gcode_end_t *end;


  end = (gcode_end_t *) block->pdata;

  GCODE_MATH_VEC3D_MUL_SCALAR (end->pos, end->pos, scale);
}
