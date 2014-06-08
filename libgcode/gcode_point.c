/*
*  gcode_point.c
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
#include "gcode_point.h"


void
gcode_point_init (GCODE_INIT_PARAMETERS)
{
  gcode_point_t *point;

  *block = (gcode_block_t *) malloc (sizeof (gcode_block_t));
  gcode_internal_init (*block, parent, gcode, GCODE_TYPE_POINT, 0);

  (*block)->free = gcode_point_free;
  (*block)->save = gcode_point_save;
  (*block)->load = gcode_point_load;
  (*block)->draw = gcode_point_draw;
  (*block)->duplicate = gcode_point_duplicate;
  (*block)->scale = gcode_point_scale;
  (*block)->pdata = malloc (sizeof (gcode_point_t));

  strcpy ((*block)->comment, "Point");
  strcpy ((*block)->status, "OK");
  GCODE_INIT((*block));
  GCODE_CLEAR((*block));

  /* defaults */
  point = (gcode_point_t *)(*block)->pdata;
  point->p[0] = 0.0;
  point->p[1] = 0.0;
}


void
gcode_point_free (gcode_block_t **block)
{
  free ((*block)->code);
  free ((*block)->pdata);
  free (*block);
  *block = NULL;
}


void
gcode_point_save (gcode_block_t *block, FILE *fh)
{
  gcode_point_t *point;
  uint32_t size;
  uint8_t data;

  point = (gcode_point_t *) block->pdata;

  data = GCODE_DATA_POINT_POS;
  size = 2 * sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (point->p, sizeof (gfloat_t), 2, fh);
}


void
gcode_point_load (gcode_block_t *block, FILE *fh)
{
  gcode_point_t *point;
  uint32_t bsize, dsize, start;
  uint8_t data;

  point = (gcode_point_t *) block->pdata;

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

      case GCODE_DATA_POINT_POS:
        fread (point->p, sizeof (gfloat_t), 2, fh);
        break;

      default:
        fseek (fh, dsize, SEEK_CUR);
        break;
    }
  }
}


void
gcode_point_draw (gcode_block_t *block, gcode_block_t *selected)
{
#if GCODE_USE_OPENGL
  gcode_point_t *point;
  gcode_vec2d_t xform_pt;

  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  point = (gcode_point_t *) block->pdata;

  GCODE_MATH_ROTATE(xform_pt, point->p, block->offset->rotation);

  glPointSize (4);
  glColor3f (GCODE_OPENGL_POINT_COLOR[0], GCODE_OPENGL_POINT_COLOR[1], GCODE_OPENGL_POINT_COLOR[2]);
  glBegin (GL_POINTS);
    glVertex3f (block->offset->origin[0] + xform_pt[0], block->offset->origin[1] + xform_pt[1], 0.0);
  glEnd ();
#endif
}


void
gcode_point_duplicate (gcode_block_t *block, gcode_block_t **duplicate)
{
  gcode_point_t *point, *duplicate_point;

  point = (gcode_point_t *) block->pdata;

  gcode_point_init (block->gcode, duplicate, block->parent);
  (*duplicate)->name = block->name;

  strcpy ((*duplicate)->comment, block->comment);
  (*duplicate)->parent = block->parent;
  (*duplicate)->offset = block->offset;
  
  duplicate_point = (gcode_point_t *) (*duplicate)->pdata;
  
  duplicate_point->p[0] = point->p[0];
  duplicate_point->p[1] = point->p[1];
}


void
gcode_point_scale (gcode_block_t *block, gfloat_t scale)
{
  gcode_point_t *point;

  point = (gcode_point_t *) block->pdata;

  point->p[0] *= scale;
  point->p[1] *= scale;
}
