/*
*  gcode_drill_holes.c
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
#include "gcode_drill_holes.h"
#include "gcode_point.h"
#include "gcode_tool.h"
#include "gcode.h"

typedef struct hole_sort_s
{
  gcode_vec2d_t p;
  int used;
} hole_sort_t;

void
gcode_drill_holes_init (GCODE_INIT_PARAMETERS)
{
  gcode_drill_holes_t *drill_holes;

  *block = (gcode_block_t *) malloc (sizeof (gcode_block_t));
  gcode_internal_init (*block, parent, gcode, GCODE_TYPE_DRILL_HOLES, 0);

  (*block)->free = gcode_drill_holes_free;
  (*block)->make = gcode_drill_holes_make;
  (*block)->save = gcode_drill_holes_save;
  (*block)->load = gcode_drill_holes_load;
  (*block)->draw = gcode_drill_holes_draw;
  (*block)->duplicate = gcode_drill_holes_duplicate;
  (*block)->scale = gcode_drill_holes_scale;
  (*block)->pdata = malloc (sizeof (gcode_drill_holes_t));

  strcpy ((*block)->comment, "Drill Holes");
  strcpy ((*block)->status, "OK");
  GCODE_INIT((*block));
  GCODE_CLEAR((*block));

  /* defaults */
  drill_holes = (gcode_drill_holes_t *)(*block)->pdata;
  drill_holes->list = NULL;
  drill_holes->depth = -gcode->material_size[2];
  drill_holes->increment = 0.0;
  drill_holes->optimal_path = 1;

  drill_holes->offset.origin[0] = 0.0;
  drill_holes->offset.origin[1] = 0.0;
  drill_holes->offset.side = -1.0;
  drill_holes->offset.tool = 0.0;
  drill_holes->offset.eval = 0.0;
  drill_holes->offset.rotation = 0.0;
}


void
gcode_drill_holes_free (gcode_block_t **block)
{
  free ((*block)->code);
  free ((*block)->pdata);
  free (*block);
  *block = NULL;
}


void
gcode_drill_holes_make (gcode_block_t *block)
{
  gcode_drill_holes_t *drill_holes;
  gcode_point_t *point;
  gcode_block_t *iter_block;
  gcode_tool_t *tool;
  gcode_vec2d_t xform_pt, vec;
  hole_sort_t *hole_sort_array;
  gfloat_t nearest_dist, dist;
  int hole_num, nearest_ind, i, j, last_nearest_ind = 0;
  char string[256];

  GCODE_CLEAR(block);
  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  drill_holes = (gcode_drill_holes_t *) block->pdata;
  tool = gcode_tool_find (block);
  if (tool == NULL)
    return;

  /* Update origin */
  drill_holes->offset.origin[0] = 0.0;
  drill_holes->offset.origin[1] = 0.0;
  if (block->offset)
  {
    drill_holes->offset.origin[0] += block->offset->origin[0];
    drill_holes->offset.origin[1] += block->offset->origin[1];
    drill_holes->offset.rotation = block->offset->rotation;
  }

  /* Count total number of drill holes */
  hole_num = 0;
  iter_block = drill_holes->list;
  while (iter_block)
  {
    hole_num++;
    iter_block = iter_block->next;
  }

  hole_sort_array = (hole_sort_t *) malloc (hole_num * sizeof (hole_sort_t));

  /* Populate array with all drill hole data */
  i = 0;
  iter_block = drill_holes->list;
  while (iter_block)
  {
    hole_sort_array[i].p[0] = ((gcode_point_t *) iter_block->pdata)->p[0];
    hole_sort_array[i].p[1] = ((gcode_point_t *) iter_block->pdata)->p[1];
    hole_sort_array[i].used = 0;
    i++;
    iter_block = iter_block->next;
  }


  sprintf (string, "DRILL HOLES: %s", block->comment);
  GCODE_COMMENT (block, string);

  /* Pecking Cycle G83 Start, Let the first X Y get placed on this line hence no '\n' */
  if (drill_holes->increment <= GCODE_PRECISION)
  {
    gsprintf (string, block->gcode->decimal, "G83 Z%z F%.3f R%z ", drill_holes->depth, 0.1 * tool->feed, block->gcode->ztraverse);
  }
  else
  {
    gsprintf (string, block->gcode->decimal, "G83 Z%z F%.3f R%z Q%z ", drill_holes->depth, 0.1 * tool->feed, block->gcode->ztraverse, drill_holes->increment);
  }
  GCODE_APPEND(block, string);


  if (drill_holes->optimal_path)
  {
    /* Choose the first point in the list as the initial point */
    hole_sort_array[0].used = 1;

    GCODE_MATH_ROTATE(xform_pt, hole_sort_array[0].p, drill_holes->offset.rotation);
    xform_pt[0] += drill_holes->offset.origin[0];
    xform_pt[1] += drill_holes->offset.origin[1];
    gsprintf (string, block->gcode->decimal, "X%z Y%z\n", xform_pt[0], xform_pt[1]);
    GCODE_APPEND(block, string);

    for (j = 1; j < hole_num; j++)
    {
      nearest_ind = 0;
      /* Cycle through hole indices not already evaluated */
      for (i = 0; i < hole_num; i++)
      {
        /* Hole is not already used */
        if (hole_sort_array[i].used == 0)
        {
          GCODE_MATH_VEC2D_SUB (vec, hole_sort_array[last_nearest_ind].p, hole_sort_array[i].p);
          GCODE_MATH_VEC2D_MAG (dist, vec);

          if (dist < nearest_dist || !nearest_ind)
          {
            nearest_dist = dist;
            nearest_ind = i;
          }
        }
      }
      hole_sort_array[nearest_ind].used = 1;
      last_nearest_ind = nearest_ind;

      GCODE_MATH_ROTATE(xform_pt, hole_sort_array[nearest_ind].p, drill_holes->offset.rotation);
      xform_pt[0] += drill_holes->offset.origin[0];
      xform_pt[1] += drill_holes->offset.origin[1];
      gsprintf (string, block->gcode->decimal, "X%z Y%z\n", xform_pt[0], xform_pt[1]);
      GCODE_APPEND(block, string);
    }
  }
  else
  {
    iter_block = drill_holes->list;
    while (iter_block)
    {
      point = (gcode_point_t *) iter_block->pdata;

      GCODE_MATH_ROTATE(xform_pt, point->p, drill_holes->offset.rotation);
      xform_pt[0] += drill_holes->offset.origin[0];
      xform_pt[1] += drill_holes->offset.origin[1];
      gsprintf (string, block->gcode->decimal, "X%z Y%z\n", xform_pt[0], xform_pt[1]);
      GCODE_APPEND(block, string);

      iter_block = iter_block->next;
    }
  }

  /* Can Cycle G80 End */
  sprintf (string, "G80\n");
  GCODE_APPEND(block, string);
  sprintf (string, "F%.3f ", tool->feed);
  GCODE_APPEND(block, string);
  sprintf (string, "normal feed rate");
  GCODE_COMMENT (block, string);

  free (hole_sort_array);
}


void
gcode_drill_holes_save (gcode_block_t *block, FILE *fh)
{
  gcode_drill_holes_t *drill_holes;
  gcode_block_t *iter_block;
  uint32_t size, marker, num;
  uint8_t data;

  drill_holes = (gcode_drill_holes_t *) block->pdata;

  num = 0;
  iter_block = drill_holes->list;
  while (iter_block)
  {
    num++;
    iter_block = iter_block->next;
  }

  data = GCODE_DATA_DRILL_HOLES_NUM;
  size = sizeof (uint32_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&num, sizeof (uint32_t), 1, fh);

  iter_block = drill_holes->list;
  num = 0;
  while (iter_block)
  {
    /* Write block type */
    fwrite (&iter_block->type, sizeof (uint8_t), 1, fh);
    marker = ftell (fh);
    size = 0;
    fwrite (&size, sizeof (uint32_t), 1, fh);

    /* Write comment */
    data = GCODE_DATA_BLOCK_COMMENT;
    size = strlen (iter_block->comment) + 1;
    fwrite (&data, sizeof (uint8_t), 1, fh);
    fwrite (&size, sizeof (uint32_t), 1, fh);
    fwrite (iter_block->comment, sizeof (char), size, fh);

    iter_block->save (iter_block, fh);

    size = ftell (fh) - marker - sizeof (uint32_t);
    fseek (fh, marker, SEEK_SET);
    fwrite (&size, sizeof (uint32_t), 1, fh);
    fseek (fh, marker + size + sizeof (uint32_t), SEEK_SET);

    iter_block = iter_block->next;
  }

  data = GCODE_DATA_DRILL_HOLES_DEPTH;
  size = sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&drill_holes->depth, sizeof (gfloat_t), 1, fh);

  data = GCODE_DATA_DRILL_HOLES_INCREMENT;
  size = sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&drill_holes->increment, sizeof (gfloat_t), 1, fh);
}


void
gcode_drill_holes_load (gcode_block_t *block, FILE *fh)
{
  gcode_drill_holes_t *drill_holes;
  gcode_block_t *child_block, *last_block;
  uint32_t bsize, dsize, start, num, i;
  uint8_t data, type;

  drill_holes = (gcode_drill_holes_t *) block->pdata;

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
        fread (&block->flags, sizeof (uint8_t) , dsize, fh);
        break;

      case GCODE_DATA_DRILL_HOLES_NUM:
        fread (&num, sizeof (uint32_t), 1, fh);
        for (i = 0; i < num; i++)
        {
          /* Read Data */
          fread (&type, sizeof (uint8_t), 1, fh);
  
          gcode_point_init (block->gcode, &child_block, block);
 
          child_block->parent_list = &drill_holes->list;
          child_block->load (child_block, fh);
          child_block->offset = &drill_holes->offset;

          /* Add to the end of the list */
          if (drill_holes->list)
          {
            gcode_list_insert (&last_block, child_block);
           }
          else
          {
            gcode_list_insert (&drill_holes->list, child_block);
          }
          last_block = child_block;
        }
        break;

      case GCODE_DATA_DRILL_HOLES_DEPTH:
        fread (&drill_holes->depth, dsize, 1, fh);
        break;

      case GCODE_DATA_DRILL_HOLES_INCREMENT:
        fread (&drill_holes->increment, dsize, 1, fh);
        break;

      case GCODE_DATA_DRILL_HOLES_OPTIMAL_PATH:
        fread (&drill_holes->optimal_path, dsize, 1, fh);
        break;

      default:
        fseek (fh, dsize, SEEK_CUR);
        break;
    }
  }
}


void
gcode_drill_holes_draw (gcode_block_t *block, gcode_block_t *selected)
{
#if GCODE_USE_OPENGL
  gcode_drill_holes_t *drill_holes;
  gcode_point_t *point;
  gcode_block_t *iter_block;
  gcode_tool_t *tool;
  gfloat_t coef, tool_rad;
  uint32_t sind, i, tess;
  gcode_vec2d_t xform_pt;

  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  drill_holes = (gcode_drill_holes_t *) block->pdata;

  tool = gcode_tool_find (block);
  if (tool == NULL)
    return;

  tool_rad = 0.5 * tool->diam;

  drill_holes->offset.origin[0] = 0.0;
  drill_holes->offset.origin[1] = 0.0;
  if (block->offset)
  {
    drill_holes->offset.origin[0] += block->offset->origin[0];
    drill_holes->offset.origin[1] += block->offset->origin[1];
    drill_holes->offset.rotation = block->offset->rotation;
  }

  tess = 32;

  iter_block = drill_holes->list;
  while (iter_block)
  {
    point = (gcode_point_t *) iter_block->pdata;

    /* Draw the Point */
    iter_block->draw (iter_block, NULL);

    GCODE_MATH_ROTATE(xform_pt, point->p, drill_holes->offset.rotation);
    xform_pt[0] += drill_holes->offset.origin[0];
    xform_pt[1] += drill_holes->offset.origin[1];

    sind = 0;
    if (block == selected || iter_block == selected)
      sind = 1;

    glLoadName ((GLuint) iter_block->name); /* Set Name to block pointer value */
    glLineWidth (2);
    glBegin (GL_LINE_STRIP);
    for (i = 0; i < tess; i++)
    {
      coef = (gfloat_t) i / (gfloat_t) (tess-1);
      glColor3f (GCODE_OPENGL_COLOR[sind][0], GCODE_OPENGL_COLOR[sind][1], GCODE_OPENGL_COLOR[sind][2]);
      glVertex3f (xform_pt[0] + tool_rad * cos (coef * GCODE_2PI), xform_pt[1] + tool_rad * sin (coef * GCODE_2PI), 0.0);
    }
    glEnd ();

    glBegin (GL_LINE_STRIP);
    for (i = 0; i < tess; i++)
    {
      coef = (gfloat_t) i / (gfloat_t) (tess-1);
      glColor3f (GCODE_OPENGL_COLOR[sind][0], GCODE_OPENGL_COLOR[sind][1], GCODE_OPENGL_COLOR[sind][2]);
      glVertex3f (xform_pt[0] + tool_rad * cos (coef * GCODE_2PI), xform_pt[1] + tool_rad * sin (coef * GCODE_2PI), drill_holes->depth);
    }
    glEnd ();

    glBegin (GL_LINES);
      glVertex3f (xform_pt[0] + tool_rad * cos (0.25 * GCODE_2PI), xform_pt[1] + tool_rad * sin (0.25 * GCODE_2PI), 0.0);
      glVertex3f (xform_pt[0] + tool_rad * cos (0.25 * GCODE_2PI), xform_pt[1] + tool_rad * sin (0.25 * GCODE_2PI), drill_holes->depth);

      glVertex3f (xform_pt[0] + tool_rad * cos (0.50 * GCODE_2PI), xform_pt[1] + tool_rad * sin (0.50 * GCODE_2PI), 0.0);
      glVertex3f (xform_pt[0] + tool_rad * cos (0.50 * GCODE_2PI), xform_pt[1] + tool_rad * sin (0.50 * GCODE_2PI), drill_holes->depth);

      glVertex3f (xform_pt[0] + tool_rad * cos (0.75 * GCODE_2PI), xform_pt[1] + tool_rad * sin (0.75 * GCODE_2PI), 0.0);
      glVertex3f (xform_pt[0] + tool_rad * cos (0.75 * GCODE_2PI), xform_pt[1] + tool_rad * sin (0.75 * GCODE_2PI), drill_holes->depth);

      glVertex3f (xform_pt[0] + tool_rad * cos (1.00 * GCODE_2PI), xform_pt[1] + tool_rad * sin (1.00 * GCODE_2PI), 0.0);
      glVertex3f (xform_pt[0] + tool_rad * cos (1.00 * GCODE_2PI), xform_pt[1] + tool_rad * sin (1.00 * GCODE_2PI), drill_holes->depth);
    glEnd ();

    iter_block = iter_block->next;
  }
#endif
}


void
gcode_drill_holes_duplicate (gcode_block_t *block, gcode_block_t **duplicate)
{
  gcode_drill_holes_t *drill_holes, *duplicate_drill_holes;
  gcode_block_t *last_block, *child_block, *new_block;

  drill_holes = (gcode_drill_holes_t *) block->pdata;

  gcode_drill_holes_init (block->gcode, duplicate, block->parent);
  (*duplicate)->name = block->name;

  strcpy ((*duplicate)->comment, block->comment);
  (*duplicate)->parent = block->parent;
  (*duplicate)->offset = block->offset;
  
  duplicate_drill_holes = (gcode_drill_holes_t *) (*duplicate)->pdata;
  duplicate_drill_holes->depth = drill_holes->depth;
  duplicate_drill_holes->increment = drill_holes->increment;
  duplicate_drill_holes->optimal_path = drill_holes->optimal_path;

  duplicate_drill_holes->list = NULL;

  child_block = drill_holes->list;
  while (child_block)
  {
    child_block->duplicate (child_block, &new_block);
    new_block->parent = *duplicate;
    new_block->offset = &duplicate_drill_holes->offset;
    if (!duplicate_drill_holes->list)
    {
      gcode_list_insert (&duplicate_drill_holes->list, new_block);
    }
    else
    {
      gcode_list_insert (&last_block, new_block);
    }
  
    last_block = new_block;
    child_block = child_block->next;
  }
}


void
gcode_drill_holes_scale (gcode_block_t *block, gfloat_t scale)
{
  gcode_drill_holes_t *drill_holes;
  gcode_block_t *index_block;

  drill_holes = (gcode_drill_holes_t *) block->pdata;

  index_block = drill_holes->list;
  while (index_block)
  {
    index_block->scale (index_block, scale);
    index_block = index_block->next;
  }
}


void
gcode_drill_holes_pattern (gcode_block_t *block, int iterations, gfloat_t translate_x, gfloat_t translate_y, gfloat_t rotate_about_x, gfloat_t rotate_about_y, gfloat_t rotation)
{
  gcode_drill_holes_t *drill_holes;
  gcode_block_t *child_block, *pattern_block, **pattern_list, *last_block;
  gfloat_t inc_rotation, inc_translate_x, inc_translate_y;
  int i;

  drill_holes = (gcode_drill_holes_t *) block->pdata;

  pattern_list = (gcode_block_t **) malloc (sizeof (gcode_block_t **));
  *pattern_list = NULL;

  for (i = 0; i < iterations; i++)
  {
    inc_rotation = ((float) i) * rotation;
    inc_translate_x = ((float) i) * translate_x;
    inc_translate_y = ((float) i) * translate_y;

    child_block = drill_holes->list;
    while (child_block)
    {
      gcode_point_t *point, *pattern_point;
      gcode_vec2d_t xform_pt, pt;

      point = (gcode_point_t *) child_block->pdata;

      gcode_point_init (block->gcode, &pattern_block, *pattern_list);
      pattern_point = (gcode_point_t *) pattern_block->pdata;

      /* Rotation and Translate*/
      pt[0] = point->p[0] - rotate_about_x;
      pt[1] = point->p[1] - rotate_about_y;
      GCODE_MATH_ROTATE (xform_pt, pt, inc_rotation);
      pattern_point->p[0] = xform_pt[0] + rotate_about_x + inc_translate_x;
      pattern_point->p[1] = xform_pt[1] + rotate_about_y + inc_translate_y;

      strcpy (pattern_block->comment, child_block->comment);
      pattern_block->offset = child_block->offset;
      pattern_block->parent = child_block->parent;

      if (!*pattern_list)
      {
        gcode_list_insert (pattern_list, pattern_block);
      }
      else
      {
        gcode_list_insert (&last_block, pattern_block);
      }
      last_block = pattern_block;

      child_block = child_block->next;
    }
  }

  gcode_list_free (&drill_holes->list);
  drill_holes->list = *pattern_list;
}
