/*
*  gcode_line.c
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
#include "gcode_line.h"

void
gcode_line_init (GCODE_INIT_PARAMETERS)
{
  gcode_line_t *line;

  *block = (gcode_block_t *) malloc (sizeof (gcode_block_t));
  gcode_internal_init (*block, parent, gcode, GCODE_TYPE_LINE, 0);

  (*block)->free = gcode_line_free;
  (*block)->make = gcode_line_make;
  (*block)->save = gcode_line_save;
  (*block)->load = gcode_line_load;
  (*block)->ends = gcode_line_ends;
  (*block)->draw = gcode_line_draw;
  (*block)->eval = gcode_line_eval;
  (*block)->length = gcode_line_length;
  (*block)->duplicate = gcode_line_duplicate;
  (*block)->scale = gcode_line_scale;
  (*block)->aabb = gcode_line_aabb;
  (*block)->pdata = malloc (sizeof (gcode_line_t));

  strcpy ((*block)->comment, "Line");
  strcpy ((*block)->status, "OK");
  GCODE_INIT((*block));
  GCODE_CLEAR((*block));

  /* defaults */
  line = (gcode_line_t *)(*block)->pdata;
  line->p0[0] = 0.0;
  line->p0[1] = 0.0;
  line->p1[0] = GCODE_UNITS ((*block)->gcode, 1.0);
  line->p1[1] = 0.0;
}


void
gcode_line_free (gcode_block_t **block)
{
  free ((*block)->code);
  free ((*block)->pdata);
  free (*block);
  *block = NULL;
}


void
gcode_line_make (gcode_block_t *block)
{
  char string[256];
  gcode_vec2d_t p0, p1, normal;

  GCODE_CLEAR(block);
  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  sprintf (string, "LINE: %s", block->comment);
  GCODE_COMMENT (block, string);

  gcode_line_with_offset (block, p0, p1, normal);

  gsprintf (string, block->gcode->decimal, "G01 X%z Y%z\n", p0[0], p0[1]);
  GCODE_APPEND(block, string);

  if (fabs (block->offset->z[0] - block->offset->z[1]) < GCODE_PRECISION)
  {
    gsprintf (string, block->gcode->decimal, "G01 X%z Y%z\n", p1[0], p1[1]);
  }
  else
  {
    gsprintf (string, block->gcode->decimal, "G01 X%z Y%z Z%z\n", p1[0], p1[1], block->offset->z[1]);
  }
  GCODE_APPEND(block, string);

  /* Update block->offset->endmill_pos */
  block->offset->endmill_pos[0] = p1[0];
  block->offset->endmill_pos[1] = p1[1];
}


void
gcode_line_save (gcode_block_t *block, FILE *fh)
{
  gcode_line_t *line;
  uint32_t size;
  uint8_t data;

  line = (gcode_line_t *) block->pdata;

  data = GCODE_DATA_LINE_POS;
  size = 4 * sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (line->p0, sizeof (gfloat_t), 2, fh);
  fwrite (line->p1, sizeof (gfloat_t), 2, fh);
}


void
gcode_line_load (gcode_block_t *block, FILE *fh)
{
  gcode_line_t *line;
  uint32_t bsize, dsize, start;
  uint8_t data;

  line = (gcode_line_t *) block->pdata;

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

      case GCODE_DATA_LINE_POS:
        fread (line->p0, sizeof (gfloat_t), 2, fh);
        fread (line->p1, sizeof (gfloat_t), 2, fh);
        break;

      default:
        fseek (fh, dsize, SEEK_CUR);
        break;
    }
  }
}


int
gcode_line_ends (gcode_block_t *block, gfloat_t p0[2], gfloat_t p1[2], uint8_t mode)
{
  gcode_line_t *line;
  gcode_vec2d_t normal;

  line = (gcode_line_t *) block->pdata;

  if (mode == GCODE_GET)
  {
    p0[0] = line->p0[0];
    p0[1] = line->p0[1];

    p1[0] = line->p1[0];
    p1[1] = line->p1[1];
  }
  else if (mode == GCODE_SET)
  {
    line->p0[0] = p0[0];
    line->p0[1] = p0[1];

    line->p1[0] = p1[0];
    line->p1[1] = p1[1];
  }
  else if (mode == GCODE_GET_WITH_OFFSET)
  {
    gcode_line_with_offset (block, p0, p1, normal);
  }
  else if (mode == GCODE_GET_NORMAL)
  {
    gcode_line_with_offset (block, p0, p1, normal);

    p0[0] = normal[0];
    p0[1] = normal[1];
    p1[0] = normal[0];
    p1[1] = normal[1];
  }
  else if (mode == GCODE_GET_TANGENT)
  {
    p0[0] = line->p0[0] - line->p1[0];
    p0[1] = line->p0[1] - line->p1[1];
    GCODE_MATH_VEC2D_UNITIZE (p0);

    p1[0] = line->p1[0] - line->p0[0];
    p1[1] = line->p1[1] - line->p0[1];
    GCODE_MATH_VEC2D_UNITIZE (p1);
  }

  return (0);
}


void
gcode_line_draw (gcode_block_t *block, gcode_block_t *selected)
{
#if GCODE_USE_OPENGL
  gcode_vec2d_t p0, p1, normal;
  gcode_line_t *line;
  gfloat_t coef;
  uint32_t sind;

  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  line = (gcode_line_t *) block->pdata;

  gcode_line_with_offset (block, p0, p1, normal);

  coef = 1.0;
  sind = 0;
  if (selected)
    if (block == selected || block->parent == selected)
      sind = 1;

  if (block == selected)
    coef = 0.5;

//  sind = (block == selected || !block) ? 1 : 0;

  glLoadName ((GLuint) block->name); /* Set Name to block pointer value */
  glLineWidth (1);

  glBegin (GL_LINES);
    glColor3f (coef*GCODE_OPENGL_COLOR[sind][0], coef*GCODE_OPENGL_COLOR[sind][1], coef*GCODE_OPENGL_COLOR[sind][2]);
    glVertex3f (p0[0], p0[1], block->offset->z[0]);

    glColor3f (GCODE_OPENGL_COLOR[sind][0], GCODE_OPENGL_COLOR[sind][1], GCODE_OPENGL_COLOR[sind][2]);
    glVertex3f (p1[0], p1[1], block->offset->z[1]);
  glEnd ();

  if (block == selected)
  {
    glPointSize (4);
    glColor3f (GCODE_OPENGL_POINT_COLOR[0], GCODE_OPENGL_POINT_COLOR[1], GCODE_OPENGL_POINT_COLOR[2]);
    glBegin (GL_POINTS);
      glVertex3f (p0[0], p0[1], block->offset->z[0]);
      glVertex3f (p1[0], p1[1], block->offset->z[1]);
    glEnd ();
  }
#endif
}


int
gcode_line_eval (gcode_block_t *block, gfloat_t y, gfloat_t *x_array, int *xind)
{
  gcode_line_t *line;
  gcode_vec2d_t p0, p1, normal;
  gfloat_t dx, dy;

  line = (gcode_line_t *) block->pdata;

  gcode_line_with_offset (block, p0, p1, normal);

  dx = p0[0] - p1[0];
  dy = p0[1] - p1[1];

  /* y is outside the bounds of the line segment. */
  if ((y-GCODE_PRECISION > p0[1] && y-GCODE_PRECISION > p1[1]) || (y+GCODE_PRECISION < p0[1] && y+GCODE_PRECISION < p1[1]))
    return (1);

  if (fabs (dx) < GCODE_PRECISION)
  {
    /* Line is vertical, assign x-value from either of the points */
    x_array[(*xind)++] = p0[0];
  }
  else if (fabs (dy) < GCODE_PRECISION)
  {
    /* Line is horizontal, assign both of the x-value points */
    x_array[(*xind)++] = p0[0];
    x_array[(*xind)++] = p1[0];
  }
  else
  {
    /*
    * y = mx + b
    * x = (y - b)
    * x = p0[0] + (y - p0[1]) / (dy / dx)
    */

    x_array[(*xind)++] = p0[0] + (y - p0[1]) / (dy / dx);
  }

  return (0);
}


gfloat_t
gcode_line_length (gcode_block_t *block)
{
  gcode_line_t *line;
  gcode_vec2d_t vec;
  gfloat_t length;

  line = (gcode_line_t *) block->pdata;

  GCODE_MATH_VEC2D_SUB (vec, line->p0, line->p1);
  GCODE_MATH_VEC2D_MAG (length, vec);

  return (length);
}


void
gcode_line_duplicate (gcode_block_t *block, gcode_block_t **duplicate)
{
  gcode_line_t *line, *duplicate_line;

  line = (gcode_line_t *) block->pdata;

  gcode_line_init (block->gcode, duplicate, block->parent);
  (*duplicate)->name = block->name;

  strcpy ((*duplicate)->comment, block->comment);
  (*duplicate)->parent = block->parent;
  (*duplicate)->offset = block->offset;
  
  duplicate_line = (gcode_line_t *) (*duplicate)->pdata;
  
  duplicate_line->p0[0] = line->p0[0];
  duplicate_line->p0[1] = line->p0[1];
  duplicate_line->p1[0] = line->p1[0];
  duplicate_line->p1[1] = line->p1[1];
}


void
gcode_line_scale (gcode_block_t *block, gfloat_t scale)
{
  gcode_line_t *line;

  line = (gcode_line_t *) block->pdata;

  line->p0[0] *= scale;
  line->p0[1] *= scale;
  line->p1[0] *= scale;
  line->p1[1] *= scale;
}


void
gcode_line_aabb (gcode_block_t *block, gcode_vec2d_t min, gcode_vec2d_t max)
{
  gcode_line_t *line;
  gcode_vec2d_t p0, p1, normal;

  line = (gcode_line_t *) block->pdata;

  gcode_line_with_offset (block, p0, p1, normal);

  min[0] = p0[0];
  min[1] = p0[1];
  max[0] = p0[0];
  max[1] = p0[1];

  if (p1[0] < min[0])
    min[0] = p1[0];

  if (p1[0] > max[0])
    max[0] = p1[0];

  if (p1[1] < min[1])
    min[1] = p1[1];

  if (p1[1] > max[1])
    max[1] = p1[1];
}


void
gcode_line_with_offset (gcode_block_t *block, gcode_vec2d_t p0, gcode_vec2d_t p1, gcode_vec2d_t normal)
{
  gcode_line_t *line;
  gcode_vec2d_t xform_p0, xform_p1, normal_offset;
  gfloat_t inv_mag, line_dx, line_dy;

  line = (gcode_line_t *) block->pdata;

  /* Transform */
  GCODE_MATH_ROTATE(xform_p0, line->p0, block->offset->rotation);
  GCODE_MATH_ROTATE(xform_p1, line->p1, block->offset->rotation);

  /*
  * Calculate the normal vector wrt this line.
  * Multiply the normal vector by the offset to get
  * the offset line that will be cut.
  */

  /* Get the normal vector to the first line */
  line_dx = xform_p0[0] - xform_p1[0];
  line_dy = xform_p0[1] - xform_p1[1];

  inv_mag = 1.0 / sqrt (line_dx*line_dx + line_dy*line_dy);

  normal[0] = -line_dy;
  normal[1] = line_dx;

  inv_mag = 1.0 / sqrt (normal[0]*normal[0] + normal[1]*normal[1]);
  normal[0] *= inv_mag * block->offset->side;
  normal[1] *= inv_mag * block->offset->side;

  normal_offset[0] = normal[0] * (block->offset->eval + block->offset->tool);
  normal_offset[1] = normal[1] * (block->offset->eval + block->offset->tool);

  p0[0] = block->offset->origin[0] + xform_p0[0] + normal_offset[0];
  p0[1] = block->offset->origin[1] + xform_p0[1] + normal_offset[1];

  p1[0] = block->offset->origin[0] + xform_p1[0] + normal_offset[0];
  p1[1] = block->offset->origin[1] + xform_p1[1] + normal_offset[1];
}


void
gcode_line_flip_direction (gcode_block_t *block)
{
  gcode_line_t *line;
  gcode_vec2d_t swap;

  line = (gcode_line_t *) block->pdata;

  swap[0] = line->p0[0];
  swap[1] = line->p0[1];

  line->p0[0] = line->p1[0];
  line->p0[1] = line->p1[1];

  line->p1[0] = swap[0];
  line->p1[1] = swap[1];
}
