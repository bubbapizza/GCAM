/*
*  gcode_arc.c
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
#include "gcode_arc.h"

#define TESS	50

void
gcode_arc_init (GCODE_INIT_PARAMETERS)
{
  gcode_arc_t *arc;

  *block = (gcode_block_t *) malloc (sizeof (gcode_block_t));

  gcode_internal_init (*block, parent, gcode, GCODE_TYPE_ARC, 0);

  (*block)->free = gcode_arc_free;
  (*block)->make = gcode_arc_make;
  (*block)->save = gcode_arc_save;
  (*block)->load = gcode_arc_load;
  (*block)->ends = gcode_arc_ends;
  (*block)->draw = gcode_arc_draw;
  (*block)->eval = gcode_arc_eval;
  (*block)->length = gcode_arc_length;
  (*block)->duplicate = gcode_arc_duplicate;
  (*block)->scale = gcode_arc_scale;
  (*block)->aabb = gcode_arc_aabb;
  (*block)->pdata = malloc (sizeof (gcode_arc_t));

  strcpy ((*block)->comment, "Arc");
  strcpy ((*block)->status, "OK");
  GCODE_INIT((*block));
  GCODE_CLEAR((*block));

  /* defaults */
  arc = (gcode_arc_t *)(*block)->pdata;
  arc->pos[0] = 0.0;
  arc->pos[1] = 0.0;
  arc->radius = GCODE_UNITS ((*block)->gcode, 0.5);
  arc->start_angle = 180.0;
  arc->sweep = -90.0;
  arc->interface = GCODE_ARC_INTERFACE_SWEEP;
}


void
gcode_arc_free (gcode_block_t **block)
{
  free ((*block)->code);
  free ((*block)->pdata);
  free (*block);
  *block = NULL;
}


void
gcode_arc_make (gcode_block_t *block)
{
  gcode_arc_t *arc;
  gcode_vec2d_t p0, origin, center;
  gfloat_t arc_radius_offset, start_angle;
  char string[256];

  GCODE_CLEAR(block);
  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  arc = (gcode_arc_t *) block->pdata;

  sprintf (string, "ARC: %s", block->comment);
  GCODE_COMMENT (block, string);

  gcode_arc_with_offset (block, origin, center, p0, &arc_radius_offset, &start_angle);

  /*
  * Do not proceed if arc_radius is <= GCODE_PRECISION
  */
  if (arc_radius_offset <= GCODE_PRECISION)
    return;

  gsprintf (string, block->gcode->decimal, "G01 X%z Y%z\n", origin[0], origin[1]);
  GCODE_APPEND(block, string);

  if (arc->sweep < 0.0)
  {
    /* Clockwise */
    if (fabs (block->offset->z[0] - block->offset->z[1]) < GCODE_PRECISION)
    {
      gsprintf (string, block->gcode->decimal, "G02 X%z Y%z I%z J%z\n", p0[0], p0[1], center[0]-origin[0], center[1]-origin[1]);
    }
    else
    {
      gsprintf (string, block->gcode->decimal, "G02 X%z Y%z Z%z I%z J%z\n", p0[0], p0[1], block->offset->z[1], center[0]-origin[0], center[1]-origin[1]);
    }
    GCODE_APPEND(block, string);
  }
  else
  {
    /* Counter-Clockwise */
    if (fabs (block->offset->z[0] - block->offset->z[1]) < GCODE_PRECISION)
    {
      gsprintf (string, block->gcode->decimal, "G03 X%z Y%z I%z J%z\n", p0[0], p0[1], center[0]-origin[0], center[1]-origin[1]);
    }
    else
    {
      gsprintf (string, block->gcode->decimal, "G03 X%z Y%z Z%z I%z J%z\n", p0[0], p0[1], block->offset->z[1], center[0]-origin[0], center[1]-origin[1]);
    }
    GCODE_APPEND(block, string);
  }

  /* Update block->offset->endmill_pos */
  block->offset->endmill_pos[0] = p0[0];
  block->offset->endmill_pos[1] = p0[1];
}


void
gcode_arc_save (gcode_block_t *block, FILE *fh)
{
  gcode_arc_t *arc;
  uint32_t size;
  uint8_t data;

  arc = (gcode_arc_t *) block->pdata;

  data = GCODE_DATA_ARC_POS;
  size = 2 * sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&arc->pos[0], sizeof (gfloat_t), 1, fh);
  fwrite (&arc->pos[1], sizeof (gfloat_t), 1, fh);

  data = GCODE_DATA_ARC_RADIUS;
  size = sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&arc->radius, size, 1, fh);

  data = GCODE_DATA_ARC_START;
  size = sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&arc->start_angle, size, 1, fh);

  data = GCODE_DATA_ARC_SWEEP;
  size = sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&arc->sweep, size, 1, fh);

  data = GCODE_DATA_ARC_INTERFACE;
  size = sizeof (uint8_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&arc->interface, size, 1, fh);
}


void
gcode_arc_load (gcode_block_t *block, FILE *fh)
{
  gcode_arc_t *arc;
  uint32_t bsize, dsize, start;
  uint8_t data;
      
  arc = (gcode_arc_t *) block->pdata;
   
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

      case GCODE_DATA_ARC_POS:
        fread (arc->pos, sizeof (gfloat_t), 2, fh);
        break;

      case GCODE_DATA_ARC_RADIUS:
        fread (&arc->radius, dsize, 1, fh);
        break;

      case GCODE_DATA_ARC_START:
        fread (&arc->start_angle, dsize, 1, fh);
        break;

      case GCODE_DATA_ARC_SWEEP:
        fread (&arc->sweep, dsize, 1, fh);
        break;

      case GCODE_DATA_ARC_INTERFACE:
        fread (&arc->interface, dsize, 1, fh);
        break;

      default:
        fseek (fh, dsize, SEEK_CUR);
        break;
    }
  }
}


int
gcode_arc_ends (gcode_block_t *block, gfloat_t p0[2], gfloat_t p1[2], uint8_t mode)
{
  gcode_arc_t *arc;
  gcode_vec2d_t start_pos;

  arc = (gcode_arc_t *) block->pdata;

  start_pos[0] = arc->radius * cos (arc->start_angle * GCODE_DEG2RAD);
  start_pos[1] = arc->radius * sin (arc->start_angle * GCODE_DEG2RAD);

  if (mode == GCODE_GET)
  {
    p0[0] = arc->pos[0];
    p0[1] = arc->pos[1];

    p1[0] = arc->pos[0] - start_pos[0] + arc->radius * cos (arc->start_angle * GCODE_DEG2RAD + arc->sweep * GCODE_DEG2RAD);
    p1[1] = arc->pos[1] - start_pos[1] + arc->radius * sin (arc->start_angle * GCODE_DEG2RAD + arc->sweep * GCODE_DEG2RAD);

    return (0);
  }
  else if (mode == GCODE_SET)
  {
    arc->pos[0] = p0[0];
    arc->pos[1] = p0[1];
    return (0);
  }
  else if (mode == GCODE_GET_WITH_OFFSET)
  {
    gcode_vec2d_t origin, center;
    gfloat_t arc_radius_offset, start_angle;

    gcode_arc_with_offset (block, origin, center, p0, &arc_radius_offset, &start_angle);

    p1[0] = p0[0];
    p1[1] = p0[1];

    p0[0] = origin[0];
    p0[1] = origin[1];

    return (0);
  }
  else if (mode == GCODE_GET_NORMAL)
  {
    gfloat_t xform_angle, flip;

    xform_angle = arc->start_angle + block->offset->rotation;
    flip = block->offset->side * (arc->sweep < 0.0 ? -1.0 : 1.0);

    p0[0] = flip * cos (xform_angle * GCODE_DEG2RAD);
    p0[1] = flip * sin (xform_angle * GCODE_DEG2RAD);

    p1[0] = flip * cos ((xform_angle+arc->sweep) * GCODE_DEG2RAD);
    p1[1] = flip * sin ((xform_angle+arc->sweep) * GCODE_DEG2RAD);

    return (0);
  }
  else if (mode == GCODE_GET_TANGENT)
  {
    gfloat_t angle;

    angle = arc->start_angle - 90.0;
    if (angle < 0.0)
      angle += 360.0;

    p0[0] = cos (GCODE_DEG2RAD * angle);
    p0[1] = sin (GCODE_DEG2RAD * angle);
    if (arc->sweep > 0.0)
      GCODE_MATH_VEC2D_SCALE (p0, -1.0);

    angle = arc->start_angle + arc->sweep - 90.0;
    if (angle < 0.0)
      angle += 360.0;
    if (angle > 360.0)
      angle -= 360.0;

    p1[0] = cos (GCODE_DEG2RAD * angle);
    p1[1] = sin (GCODE_DEG2RAD * angle);
    if (arc->sweep > 0.0)
      GCODE_MATH_VEC2D_SCALE (p1, -1.0);

    return (0);
  }

  return (1);
}


void
gcode_arc_draw (gcode_block_t *block, gcode_block_t *selected)
{
#if GCODE_USE_OPENGL
  gcode_arc_t *arc;
  uint32_t i, sind;
  gcode_vec2d_t start_pos, xform_pos;
  gfloat_t arc_radius_offset, flip, xform_angle, coef, t;

  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  arc = (gcode_arc_t *) block->pdata;

  /* Transform */
  GCODE_MATH_ROTATE(xform_pos, arc->pos, block->offset->rotation);
  xform_angle = arc->start_angle + block->offset->rotation;

  /* Prevent negative radii */
  flip = block->offset->side * (arc->sweep < 0.0 ? -1.0 : 1.0);
  arc_radius_offset = arc->radius + flip * (block->offset->tool + block->offset->eval);
  if (arc_radius_offset < 0.0)
    arc_radius_offset = 0.0;

  /* Do not display this arc if it's got a 0 radius and it's not selected */
  if (block != selected && arc_radius_offset == 0.0)
    return;

/*  tess = (uint32_t) (fabs(arc->sweep / 360.0) * (gfloat_t) 32); */

  start_pos[0] = arc->radius * cos (xform_angle * GCODE_DEG2RAD);
  start_pos[1] = arc->radius * sin (xform_angle * GCODE_DEG2RAD);

  /* Note: xform_pos - start_pos = arc center */

  sind = 0;
  if (selected)
    if (block == selected || block->parent == selected)
      sind = 1;

  if (block->parent->type == GCODE_TYPE_SKETCH)
    glLoadName ((GLuint) block->name); /* Set Name to block pointer value only if part of a sketch */

  glLineWidth (1);
  glBegin (GL_LINE_STRIP);
  for (i = 0; i < TESS; i++)
  {
    t = (gfloat_t) i / (gfloat_t) (TESS-1);

    if (block == selected && block->parent->type == GCODE_TYPE_SKETCH) /* Because this could be used in bolt holes */
      coef = t;
    else
      coef = 1.0;

    glColor3f (0.5*(1.0 + coef)*GCODE_OPENGL_COLOR[sind][0], 0.5*(1.0 + coef)*GCODE_OPENGL_COLOR[sind][1], 0.5*(1.0 + coef)*GCODE_OPENGL_COLOR[sind][2]);
    glVertex3f (block->offset->origin[0] + xform_pos[0] - start_pos[0] + arc_radius_offset * cos (t * GCODE_2PI * arc->sweep / 360.0 + xform_angle * GCODE_DEG2RAD),
                block->offset->origin[1] + xform_pos[1] - start_pos[1] + arc_radius_offset * sin (t * GCODE_2PI * arc->sweep / 360.0 + xform_angle * GCODE_DEG2RAD),
                block->offset->z[0]*(1.0-t) + block->offset->z[1]*t);
  }
  glEnd ();

  if (block == selected && block->parent->type == GCODE_TYPE_SKETCH) /* No pts if used by bolt holes */
  {
    glPointSize (4);
    glColor3f (GCODE_OPENGL_POINT_COLOR[0], GCODE_OPENGL_POINT_COLOR[1], GCODE_OPENGL_POINT_COLOR[2]);
    glBegin (GL_POINTS);
      glVertex3f (block->offset->origin[0] + xform_pos[0] - start_pos[0] + arc_radius_offset * cos (xform_angle * GCODE_DEG2RAD),
                  block->offset->origin[1] + xform_pos[1] - start_pos[1] + arc_radius_offset * sin (xform_angle * GCODE_DEG2RAD),
                  block->offset->z[0]);
      glVertex3f (block->offset->origin[0] + xform_pos[0] - start_pos[0] + arc_radius_offset * cos ((xform_angle+arc->sweep) * GCODE_DEG2RAD),
                  block->offset->origin[1] + xform_pos[1] - start_pos[1] + arc_radius_offset * sin ((xform_angle+arc->sweep) * GCODE_DEG2RAD),
                  block->offset->z[1]);
  }

  glEnd ();
#endif
}


int
gcode_arc_eval (gcode_block_t *block, gfloat_t y, gfloat_t *x_array, int *xind)
{
  gcode_arc_t *arc;
  gfloat_t angle1, angle2, start_angle, end_angle, arc_radius_offset, xform_start_angle;
  gcode_vec2d_t origin, center, p0;
  int fail;

  arc = (gcode_arc_t *) block->pdata;

  /* Transform */
  gcode_arc_with_offset (block, origin, center, p0, &arc_radius_offset, &xform_start_angle);
  if (arc_radius_offset < GCODE_PRECISION)
    return (1);

  /* Check whether y is outside of the circle boundaries */
  if (fabs (center[1] - y) > arc_radius_offset)
    return (1);

  /* y is now in unit circle coordinates */
  y = (y - center[1]) / arc_radius_offset;

  /* Take the arcsin to get the angles */
  angle1 = GCODE_RAD2DEG * asin (y);
  angle2 = angle1 + 2.0 * (90.0 - angle1);
  if (angle1 < 0.0)
    angle1 += 360.0;

  /*
  * Notes:
  * angle2 can never be negative.
  * angle1 represents quadrants 1 and 4. (+X)
  * angle2 represents quadrants 2 and 3. (-X)
  */

  /* Set the start and end angle in a counter clockwise format */
  if (arc->sweep < 0.0)
  {
    start_angle = xform_start_angle + arc->sweep;
    end_angle = xform_start_angle;
  }
  else
  {
    start_angle = xform_start_angle;
    end_angle = xform_start_angle + arc->sweep;
  }

  /* Only the case if arc->sweep < 0 */
  if (start_angle < 0.0)
  {
    start_angle += 360.0;
    end_angle += 360.0;
  }

  /* Precision adjustment */
  if (fabs (angle1 - start_angle) < GCODE_PRECISION)
    angle1 = start_angle;

#if 0
  if (angle1+360.0 >= start_angle && angle1+360.0 <= end_angle)
    angle1 += 360.0;

  if (angle2+360.0 >= start_angle && angle2+360.0 <= end_angle)
    angle1 += 360.0;
#endif

  if (angle1 < start_angle - GCODE_PRECISION)
    angle1 += 360.0;

  if (angle2 < start_angle - GCODE_PRECISION)
    angle2 += 360.0;

  fail = 1;
  if (angle1 >= start_angle && angle1 <= end_angle)
  {
    x_array[(*xind)++] = center[0] + arc_radius_offset * cos (GCODE_DEG2RAD * angle1);
    fail = 0;
  }

  if (fabs (angle1 - angle2) > GCODE_PRECISION && angle2 >= (start_angle - GCODE_PRECISION) && angle2 <= (end_angle + GCODE_PRECISION))
  {
    x_array[(*xind)++] = center[0] + arc_radius_offset * cos (GCODE_DEG2RAD * angle2);
    fail = 0;
  }

  return (fail);
}


gfloat_t
gcode_arc_length (gcode_block_t *block)
{
  gcode_arc_t *arc;
  gfloat_t length;

  arc = (gcode_arc_t *) block->pdata;

  length = fabs (arc->radius * GCODE_2PI * arc->sweep / 360.0);

  return (length);
}


void
gcode_arc_duplicate (gcode_block_t *block, gcode_block_t **duplicate)
{
  gcode_arc_t *arc, *duplicate_arc;

  arc = (gcode_arc_t *) block->pdata;

  gcode_arc_init (block->gcode, duplicate, block->parent);
  (*duplicate)->name = block->name;

  strcpy ((*duplicate)->comment, block->comment);
  (*duplicate)->parent = block->parent;
  (*duplicate)->offset = block->offset;

  duplicate_arc = (gcode_arc_t *) (*duplicate)->pdata;

  duplicate_arc->pos[0] = arc->pos[0];
  duplicate_arc->pos[1] = arc->pos[1];

  duplicate_arc->radius = arc->radius;
  duplicate_arc->start_angle = arc->start_angle;
  duplicate_arc->sweep = arc->sweep;
  duplicate_arc->interface = arc->interface;
}


void
gcode_arc_scale (gcode_block_t *block, gfloat_t scale)
{
  gcode_arc_t *arc;

  arc = (gcode_arc_t *) block->pdata;

  arc->pos[0] *= scale;
  arc->pos[1] *= scale;
  arc->radius *= scale;
}


void
gcode_arc_aabb (gcode_block_t *block, gcode_vec2d_t min, gcode_vec2d_t max)
{
  gcode_arc_t *arc;
  gcode_vec2d_t origin, center, end_pos;
  gfloat_t arc_radius_offset, start_angle;

  arc = (gcode_arc_t *) block->pdata;

  gcode_arc_with_offset (block, origin, center, end_pos, &arc_radius_offset, &start_angle);

  /* Use start and end points to check for min and max */
  min[0] = origin[0];
  min[1] = origin[1];
  max[0] = origin[0];
  max[1] = origin[1];

  if (end_pos[0] < min[0])
    min[0] = end_pos[0];

  if (end_pos[0] > max[0])
    max[0] = end_pos[0];

  if (end_pos[1] < min[1])
    min[1] = end_pos[1];

  if (end_pos[1] > max[1])
    max[1] = end_pos[1];


  /* Test if arc intersects X or Y axis with respect to arc center */
  if (!gcode_math_angle_within_arc (start_angle, arc->sweep, 0.0))
    max[0] = center[0] + arc_radius_offset;

  if (!gcode_math_angle_within_arc (start_angle, arc->sweep, 90.0))
    max[1] = center[1] + arc_radius_offset;

  if (!gcode_math_angle_within_arc (start_angle, arc->sweep, 180.0))
    min[0] = center[0] - arc_radius_offset;

  if (!gcode_math_angle_within_arc (start_angle, arc->sweep, 270.0))
    min[1] = center[1] - arc_radius_offset;
}


void
gcode_arc_with_offset (gcode_block_t *block, gcode_vec2d_t origin, gcode_vec2d_t center, gcode_vec2d_t end_pos, gfloat_t *arc_radius_offset, gfloat_t *start_angle)
{
  gcode_arc_t *arc;
  gcode_vec2d_t start_pos, xform_pos;
  gfloat_t flip;

  arc = (gcode_arc_t *) block->pdata;

  /* Transform */
  GCODE_MATH_ROTATE(xform_pos, arc->pos, block->offset->rotation);
  *start_angle = arc->start_angle + block->offset->rotation;

  /* Prevent negative radii */
  flip = block->offset->side * (arc->sweep < 0.0 ? -1.0 : 1.0);
/* printf ("arc_radius_offset: %f + %f * (%f + %f)\n", arc->radius, flip, block->offset->tool, block->offset->eval); */
  *arc_radius_offset = arc->radius + flip * (block->offset->tool + block->offset->eval);
  if (*arc_radius_offset < 0.0)
    *arc_radius_offset = 0.0;

  start_pos[0] = arc->radius * cos (*start_angle * GCODE_DEG2RAD);
  start_pos[1] = arc->radius * sin (*start_angle * GCODE_DEG2RAD);

  /* Note: xform_pos - start_pos = arc center */
  end_pos[0] = block->offset->origin[0] + xform_pos[0] - start_pos[0] + *arc_radius_offset * cos (*start_angle * GCODE_DEG2RAD + arc->sweep * GCODE_DEG2RAD);
  end_pos[1] = block->offset->origin[1] + xform_pos[1] - start_pos[1] + *arc_radius_offset * sin (*start_angle * GCODE_DEG2RAD + arc->sweep * GCODE_DEG2RAD);

  origin[0] = block->offset->origin[0] + xform_pos[0] - start_pos[0] + *arc_radius_offset * cos (*start_angle * GCODE_DEG2RAD);
  origin[1] = block->offset->origin[1] + xform_pos[1] - start_pos[1] + *arc_radius_offset * sin (*start_angle * GCODE_DEG2RAD);

  center[0] = origin[0] - *arc_radius_offset * cos (*start_angle * GCODE_DEG2RAD);
  center[1] = origin[1] - *arc_radius_offset * sin (*start_angle * GCODE_DEG2RAD);
}


void
gcode_arc_flip_direction (gcode_block_t *block)
{
  gcode_arc_t *arc;

  arc = (gcode_arc_t *) block->pdata;

  arc->pos[0] = arc->pos[0] - (arc->radius * cos (arc->start_angle * GCODE_DEG2RAD)) + arc->radius * cos (arc->start_angle * GCODE_DEG2RAD + arc->sweep * GCODE_DEG2RAD);
  arc->pos[1] = arc->pos[1] - (arc->radius * sin (arc->start_angle * GCODE_DEG2RAD)) + arc->radius * sin (arc->start_angle * GCODE_DEG2RAD + arc->sweep * GCODE_DEG2RAD);
  arc->start_angle = fmod (arc->start_angle+arc->sweep, 360.0);
  arc->sweep *= -1.0;
}
