/*
*  gcode_sketch.c
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
#include "gcode_sketch.h"
#include "gcode_extrusion.h"
#include "gcode_tool.h"
#include "gcode_pocket.h"
#include "gcode_util.h"
#include "gcode_arc.h"
#include "gcode_line.h"
#include "gcode.h"


#define SKETCH_TOL		0.0001

void
gcode_sketch_init (GCODE_INIT_PARAMETERS)
{
  gcode_sketch_t *sketch;

  *block = (gcode_block_t *) malloc (sizeof (gcode_block_t));
  gcode_internal_init (*block, parent, gcode, GCODE_TYPE_SKETCH, 0);

  (*block)->free = gcode_sketch_free;
  (*block)->make = gcode_sketch_make;
  (*block)->save = gcode_sketch_save;
  (*block)->load = gcode_sketch_load;
  (*block)->draw = gcode_sketch_draw;
  (*block)->duplicate = gcode_sketch_duplicate;
  (*block)->scale = gcode_sketch_scale;
  (*block)->aabb = gcode_sketch_aabb;
  (*block)->pdata = malloc (sizeof (gcode_sketch_t));

  strcpy ((*block)->comment, "Sketch");
  strcpy ((*block)->status, "OK");
  GCODE_INIT((*block));
  GCODE_CLEAR((*block));

  /* defaults */
  sketch = (gcode_sketch_t *) (*block)->pdata;

  /* Create Extrusion block */
  gcode_extrusion_init (gcode, &sketch->extrusion, *block);

  /* Initialize Sketch List */
  sketch->list = NULL;

  sketch->taper_offset[0] = 0.0;
  sketch->taper_offset[1] = 0.0;
  sketch->pocket = 0;
  sketch->zero_pass = 0;
  sketch->helical = 0;

  sketch->offset.origin[0] = 0.0;
  sketch->offset.origin[1] = 0.0;
  sketch->offset.side = 0.0;
  sketch->offset.tool = 0.0;
  sketch->offset.eval = 0.0;
  sketch->offset.rotation = 0.0;

  sketch->offset.endmill_pos[0] = 0.0;
  sketch->offset.endmill_pos[1] = 0.0;

  (*block)->offset = &sketch->offset;
}


void
gcode_sketch_free (gcode_block_t **block)
{
  gcode_sketch_t *sketch;
  gcode_block_t *tmp, *child_block;

  sketch = (gcode_sketch_t *) (*block)->pdata;
      
  /* Free the extrusion list */
  sketch->extrusion->free (&sketch->extrusion);

  /* Walk the list and free */
  child_block = sketch->list;
  while (child_block)
  {   
    tmp = child_block;
    child_block = child_block->next;
    tmp->free (&tmp);
  }

  free ((*block)->code);
  free ((*block)->pdata);
  free (*block);
  *block = NULL;
}


/*
* Return 1 if right side of line is inside,
* Return 0 if right side of line is outside.
*/
static int
gcode_sketch_inside (gcode_block_t *start_block, gcode_block_t *end_block)
{
  int xind, i;
  gfloat_t x_array[2], first_x;
  gcode_block_t *index1_block, *first_block;
  gcode_vec2d_t p0, p1;

  /* Set first_x to max */
  first_x = start_block->gcode->material_size[0];
  first_block = NULL;


  index1_block = start_block;
  start_block->ends (start_block, p0, p1, GCODE_GET_WITH_OFFSET);

  index1_block = start_block;
  while (index1_block != end_block->next)
  {
    xind = 0;
    if (!index1_block->eval (index1_block, p0[1], x_array, &xind))
    {
      for (i = 0; i < xind; i++)
        if (x_array[i] < first_x)
        {
          first_x = x_array[i];
          first_block = index1_block;
        }
    }
    index1_block = index1_block->next;
  }


  if (!first_block)
  {
    /*
    * This indicates that the sketch is not closed, eventually the sketch should only
    * call this function if the sketch is closed.
    */
    return (0);
  }

  /* Determine the side */
  switch (first_block->type)
  {
    case GCODE_TYPE_LINE:
      {
        gcode_line_t *line;

        /* Check the End Points orientation wrt "y" */
        line = (gcode_line_t *) first_block->pdata;
        first_block->ends (first_block, p0, p1, GCODE_GET_WITH_OFFSET);
        /* skip if this line has a 0 slope. */
        if (fabs (p0[1] - p1[1]) > GCODE_PRECISION)
        {
          if (p0[1] < p1[1])
            return (1);
          return (0);
        }
      }
      break;

    case GCODE_TYPE_ARC:
      {
        gcode_arc_t *arc;
        gcode_vec2d_t origin, center, p0;
        gfloat_t arc_radius_offset, start_angle;

        /* Check the sweep angle */
        arc = (gcode_arc_t *) first_block->pdata;
        gcode_arc_with_offset (first_block, origin, center, p0, &arc_radius_offset, &start_angle);

        if (first_x < center[0])
        {
          return (arc->sweep < 0.0 ? 1 : 0);
        }
        else
        {
          return (arc->sweep < 0.0 ? 0 : 1);
        }
      }
      break;
  }

  return (0);
}


static void
transition_arc (gcode_block_t *start_block, gcode_block_t *child_block)
{
  gcode_block_t *next_block, *arc_block;
  gcode_vec2d_t e0, e1, t, n0, n1, v0;
  gfloat_t inv_mag, dist, alpha;
  gcode_arc_t *arc;

  /*
  * Determine if a transition arc needs to be inserted between this block and the next
  * to provide C1 continuity between blocks.  These blocks have already been evaluated
  * meaning there is no offset, it's already been applied.
  */
  next_block = child_block->next ? child_block->next : start_block;

  /* If the ends are NOT joined then a transition arc must be created. */
  child_block->ends (child_block, v0, e0, GCODE_GET);
  next_block->ends (next_block, e1, t, GCODE_GET);
  dist = sqrt ((e0[0]-e1[0])*(e0[0]-e1[0]) + (e0[1]-e1[1])*(e0[1]-e1[1]));

  if (dist < SKETCH_TOL)
    return;

  /* These ends are not connected, compute an arc to join them. */
  child_block->ends (child_block, t, n0, GCODE_GET_NORMAL);
  next_block->ends (next_block, n1, t, GCODE_GET_NORMAL);

  gcode_arc_init (start_block->gcode, &arc_block, NULL);
  arc = (gcode_arc_t *) arc_block->pdata;

  arc->pos[0] = e0[0];
  arc->pos[1] = e0[1];

  arc->sweep = GCODE_RAD2DEG * acos (n0[0]*n1[0] + n0[1]*n1[1]); /* Dot product between two normals */
  /*
  * Calculating the radius took about an hour or two to figure out.
  *    .      The sides 'R' are equal and the base 'D' is the distance.  Bisect the triangle
  *   /|\     to form a right triangle then all 3 angles are known.  Using pythagorean theorem
  * R/ | \R   C = sqrt(A^2 + B^2) you solve for the bisector length which is 'opposite'.
  * /__D_A\   Since 'adjacent' is known employ tan(alpha) = opposite/adjacent.  Therefore the
  *     D/2   length of opposite = D/2 * tan(A).  R = sqrt ((D/2)^2 + [D/2 * tan(alpha)]^2)
  */

  alpha = tan (GCODE_DEG2RAD * (90.0 - 0.5 * arc->sweep));
  arc->radius = sqrt (0.25*dist*dist * (1+alpha*alpha));

  /* Start Angle */
  if (child_block->type == GCODE_TYPE_LINE)
  {
    gfloat_t side;
    gcode_vec2d_t normal;

    v0[0] = e0[0] - v0[0];
    v0[1] = e0[1] - v0[1];
    inv_mag = 1.0 / sqrt (v0[0]*v0[0] + v0[1]*v0[1]);
    v0[0] *= inv_mag;
    v0[1] *= inv_mag;
    arc->start_angle = GCODE_RAD2DEG * (v0[1] < 0.0 ? (GCODE_2PI) - acos (v0[0]) : acos (v0[0])) - 90.0;

    /*
    * The normal from ends() is always outward and of no use here.
    * calculate the unbiased normal.
    */
    normal[0] = -v0[1];
    normal[1] = v0[0];
    side = normal[0]*(e1[0]-e0[0]) + normal[1]*(e1[1]-e0[1]);

    if (side < 0.0)
    {
      arc->sweep *= -1.0;
      arc->start_angle += 180.0;
    }
  }
  else if (child_block->type == GCODE_TYPE_ARC)
  {
    gcode_arc_t *child_arc;
    gcode_vec2d_t origin, center, p0;
    gfloat_t arc_radius_offset, start_angle;

    gcode_arc_with_offset (child_block, origin, center, p0, &arc_radius_offset, &start_angle);
    dist = sqrt ((center[0]-e1[0])*(center[0]-e1[0]) + (center[1]-e1[1])*(center[1]-e1[1]));

    child_arc = (gcode_arc_t *) child_block->pdata;
    arc->start_angle = child_arc->start_angle + child_arc->sweep;

    if (dist < child_arc->radius)
    {
      if (child_arc->sweep < 0.0)
        arc->sweep *= -1.0;
    }
    else
    {
      arc->start_angle += 180.0;
      if (child_arc->sweep > 0.0)
        arc->sweep *= -1.0;
    }
  }

  if (arc->start_angle < -GCODE_PRECISION)
    arc->start_angle += 360.0;
  if (arc->start_angle > 360.0 + GCODE_PRECISION)
    arc->start_angle -= 360.0;

  /* Set offset pointer and generate the g-code. */
  arc_block->offset = child_block->offset;
  arc_block->parent = child_block->parent; /* set this blocks parent */
  gcode_list_insert (&child_block, arc_block);
}


static void
gcode_sketch_path_length (gcode_block_t *block, gfloat_t *length, int *num)
{
  *length = 0;
  *num = 0;
  while (block)
  {
    (*length) += block->length (block);
    (*num)++;
    block = block->next;
  }
}


void
gcode_sketch_make (gcode_block_t *block)
{
  gcode_sketch_t *sketch;
  gcode_extrusion_t *extrusion;
  gcode_tool_t *tool;
  gcode_block_t *child_block, *start_block, *index_block;
  gcode_vec2d_t p0, p1, e0, e1, t;
  gfloat_t z, last_z, inside, tool_rad, block_length, path_length, accum_length, length_coef;
  int closed, taper_exists, path_num;
  char string[256];

  GCODE_CLEAR(block);
  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  sketch = (gcode_sketch_t *) block->pdata;
  if (!sketch->list)
    return;

  sprintf (block->status, "OK");

  extrusion = (gcode_extrusion_t *) sketch->extrusion->pdata;

  tool = gcode_tool_find (block);
  if (tool == NULL)
    return;
  tool_rad = tool->diam * 0.5;

  GCODE_APPEND (block, "\n");
  sprintf (string, "SKETCH: %s", block->comment);
  GCODE_COMMENT (block, string);
  GCODE_APPEND (block, "\n");

  /*
  * WARNING: Code is being duplicated into the sketch.  This is wasteful,
  * but the amount of waste memory as of right now is negligible.
  */


  /*
  * Evaluate the Extrusion curve to provide an offset and depth to
  * each of the child block make functions.
  */
  sketch->extrusion->ends (sketch->extrusion, p0, p1, GCODE_GET);
  taper_exists = gcode_extrusion_taper_exists (sketch->extrusion);
  /* Swap ends if necessary so p0 is above p1 */
  if (p0[1] < p1[1])
  {
    z = p0[1];
    p0[1] = p1[1];
    p1[1] = z;
  }


/*  child_block = sketch->list; */


  for (index_block = sketch->list; index_block; index_block = index_block->next)
  {
    /*
    * find a continuous string of blocks for the purpose of
    * milling a set of continuous blocks at the same depth.
    */
    start_block = index_block;

    if (index_block->next)
    {
      if (index_block->ends && index_block->next->ends)
      {
        index_block->ends (index_block, t, e0, GCODE_GET);
        index_block->next->ends (index_block->next, e1, t, GCODE_GET);

        /* while the blocks are connected end to end */
        while (index_block->next && (fabs (e0[0]-e1[0]) + fabs (e0[1]-e1[1]) < SKETCH_TOL))
        {
          index_block = index_block->next;
          index_block->ends (index_block, t, e0, GCODE_GET);
          if (index_block->next)
            index_block->next->ends (index_block->next, e1, t, GCODE_GET);
        }
      }
    }

    start_block->ends (start_block, e0, t, GCODE_GET);
    index_block->ends (index_block, t, e1, GCODE_GET);
    closed = (fabs (e0[0]-e1[0]) + fabs (e0[1]-e1[1])) < SKETCH_TOL ? 1 : 0;


    /*
    * Determine if the right side of the first line is inside or outside the sketch.
    * Use this to tell whether the offset by the extrusion sketch should be right
    * or left of the primitive.
    */

    /*
    * temporarily set side to 0 so that when the blocks are evaluated the offset is not applied
    * because at this point any truncation/filleting has not occured and self intersections may
    * result creating bogus results.
    */
    if (closed)
    {
      sketch->offset.side = 0.0;
      inside = gcode_sketch_inside (start_block, index_block);
      sketch->offset.side = inside ? -1.0 : 1.0;
    }

    sketch->offset.tool = tool_rad;
    if (fabs (p0[0] - p1[0]) < GCODE_PRECISION && extrusion->cut_side == GCODE_EXTRUSION_ALONG)
    {
      sketch->offset.tool = 0.0;
    }
    else
    {
      if (extrusion->cut_side == GCODE_EXTRUSION_INSIDE)
        sketch->offset.side *= -1.0;
    }


    /*
    * Retract - should already be retracted, but here for safety reasons.
    */
    GCODE_RETRACT(block, block->gcode->ztraverse);

    /*
    * Mill down to the depth specified by the extrusion.
    * Set the first pass to 0 if zero_pass or helical is specified.
    */
    last_z = block->gcode->material_origin[2];
    z = sketch->zero_pass | sketch->helical ? p0[1] : p0[1]-extrusion->resolution;

    if (z-GCODE_PRECISION > p1[1] && (z - extrusion->resolution) < p1[1])
      z = p1[1];


    while (z >= p1[1])
    {
      gcode_block_t *evaluated_offset_list;


      /* Update origin based on taper offset */
      sketch->offset.origin[0] = sketch->taper_offset[0] * (p0[1] - z) / (p0[1] - p1[1]);
      sketch->offset.origin[1] = sketch->taper_offset[1] * (p0[1] - z) / (p0[1] - p1[1]);
      if (block->offset)
      {
        sketch->offset.origin[0] += block->offset->origin[0];
        sketch->offset.origin[1] += block->offset->origin[1];
        sketch->offset.rotation = block->offset->rotation;
      }

      gcode_extrusion_evaluate_offset (sketch->extrusion, z, &sketch->offset.eval);

      /* For now, generate duplicate list regardless if it will be used or not */
      gcode_util_duplicate_list (start_block, index_block->next, &evaluated_offset_list);
      gcode_util_push_offset (evaluated_offset_list);

      /*
      * POCKETING:
      *   Implies that sketch section is closed.
      *   Perform inside pocketing if explicitly set or automatically do:
      *   - Outside pocket if extrusion is outward
      *   - Inside pocket if extrusion is inward (same as choosing GCODE_SKETCH_POCKET) right now
      */
      if ((fabs (p1[0] - p0[0]) > GCODE_PRECISION || sketch->pocket) && closed)
      {
        if (extrusion->cut_side == GCODE_EXTRUSION_INSIDE)
        {
          gcode_pocket_t pocket;

          /*
          * Inward Taper:
          *  Pocketing applied automatically.
          */
          gcode_pocket_init (&pocket, tool_rad);
          gcode_pocket_prep (&pocket, evaluated_offset_list, NULL);
          gcode_pocket_make (&pocket, block, z, last_z, tool);
          gcode_pocket_free (&pocket);
        }
        else if (extrusion->cut_side == GCODE_EXTRUSION_OUTSIDE)
        {
          gcode_pocket_t inside_pocket, outside_pocket;
          gcode_block_t *evaluated_outside_offset_list;
          gcode_vec2d_t ep0, ep1;

          /*
          * Outward Taper:
          *  Pocketing applied automatically to the difference between
          *  the outter (last z value offset) and inner (current z value offset).
          */

          gcode_pocket_init (&inside_pocket, tool_rad);
          gcode_pocket_init (&outside_pocket, tool_rad);

          gcode_pocket_prep (&inside_pocket, evaluated_offset_list, NULL);

          /* Set extrusion to max */
          sketch->extrusion->ends (sketch->extrusion, ep0, ep1, GCODE_GET);
          gcode_extrusion_evaluate_offset (sketch->extrusion, ep1[1], &sketch->offset.eval);

          /* Update origin based on taper offset */
          sketch->offset.origin[0] = sketch->taper_offset[0];
          sketch->offset.origin[1] = sketch->taper_offset[1];
          if (block->offset)
          {
            sketch->offset.origin[0] = block->offset->origin[0];
            sketch->offset.origin[1] = block->offset->origin[1];
            sketch->offset.rotation = block->offset->rotation;
          }

          /* Generate the evaluated outside offset list */
          gcode_util_duplicate_list (start_block, index_block->next, &evaluated_outside_offset_list);
          gcode_util_push_offset (evaluated_outside_offset_list);

          gcode_pocket_prep (&outside_pocket, evaluated_outside_offset_list, NULL);

          /* Return extrusion to existing state */
/*          gcode_extrusion_evaluate_offset (sketch->extrusion, z, &sketch->offset.eval); */

          gcode_pocket_subtract (&outside_pocket, &inside_pocket);
          gcode_pocket_make (&outside_pocket, block, z, last_z, tool);

          gcode_pocket_free (&inside_pocket);
          gcode_pocket_free (&outside_pocket);

          /* Free the offset that was created, this is kind of ugly having 2 lines to free right now. */
          free (evaluated_outside_offset_list->offset);
          gcode_list_free (&evaluated_outside_offset_list);
        }
      }


      if (sketch->helical == 0 || z == last_z) /* If it's not a helical path or it's the first pass */
      {
        /* If the sketch is not closed then raise to Traverse(Z) */
        if (!closed && z != p0[1])
          GCODE_RETRACT(block, block->gcode->ztraverse);

        /* Move into position before plunging */
        if (closed)
        {
          evaluated_offset_list->ends (evaluated_offset_list, e0, e1, GCODE_GET_WITH_OFFSET);
        }
        else
        {
          start_block->ends (start_block, e0, e1, GCODE_GET_WITH_OFFSET);
        }
        gsprintf (string, block->gcode->decimal, "G00 X%z Y%z ", e0[0], e0[1]);
        GCODE_APPEND(block, string);
        GCODE_COMMENT (block, "move to start");

        if (fabs (last_z - z) > GCODE_PRECISION || last_z > block->gcode->material_origin[2])
          GCODE_PLUNGE_RAPID(block, last_z);
        GCODE_PLUNGE(block, z, tool);
      }


      /*
      * Generate G-Code for each block primitive (Pocketing is complete) ...
      */
/*
gcode_sketch_path_length (evaluated_offset_list, &path_length, &path_num);
printf ("path_length1: %f from %d segments\n", path_length, path_num);
*/
      /* Create all of the transition arcs */
      if (closed)
      {
        child_block = evaluated_offset_list;
        do
        {
          transition_arc (evaluated_offset_list, child_block);
          child_block = child_block->next;
        } while (child_block);
      }

      gcode_sketch_path_length (evaluated_offset_list, &path_length, &path_num);
/*
printf ("path_length2: %f from %d segments\n", path_length, path_num);
*/

      /* Generate the G-Code */
      child_block = evaluated_offset_list;
      accum_length = 0.0;
      do
      {
        if (closed && sketch->helical && !taper_exists && z-GCODE_PRECISION > p1[1])
        {
          child_block->offset->z[0] = z;

          block_length = child_block->length (child_block);

          length_coef = accum_length / path_length;

          if (z - extrusion->resolution + GCODE_PRECISION >= p1[1])
          {
            child_block->offset->z[0] = z * (1.0 - length_coef) + (z - extrusion->resolution) * length_coef;
            accum_length += block_length;

            length_coef = accum_length / path_length;
            child_block->offset->z[1] = z * (1.0 - length_coef) + (z - extrusion->resolution) * length_coef;
          }
          else
          {
            child_block->offset->z[0] = z * (1.0 - length_coef) + p1[1] * length_coef;
            accum_length += block_length;

            length_coef = accum_length / path_length;
            child_block->offset->z[1] = z * (1.0 - length_coef) + p1[1] * length_coef;
          }
        }
        else
        {
          child_block->offset->z[0] = z;
          child_block->offset->z[1] = z;
        }

        child_block->make (child_block);
        GCODE_APPEND(block, child_block->code);

        child_block = child_block->next;
      } while (child_block);


      /* Free the offset that was created, this is kind of ugly having 2 lines to free right now. */
      free (evaluated_offset_list->offset);
      gcode_list_free (&evaluated_offset_list);

      last_z = z;
      if (z-GCODE_PRECISION > p1[1] && (z - extrusion->resolution) < p1[1])
      {
        /* set equal to last layer. */
        z = p1[1];
      }
      else
      {
        /* decrement to next deepest layer or terminate. */
        z -= extrusion->resolution;
      }
    }
  }

  sketch->offset.side = 0.0;
  sketch->offset.tool = 0.0;
  sketch->offset.eval = 0.0;
}


void
gcode_sketch_save (gcode_block_t *block, FILE *fh)
{
  gcode_sketch_t *sketch;
  gcode_block_t *child_block;
  uint32_t size, num, marker;
  uint8_t data;

  sketch = (gcode_sketch_t *) block->pdata;

  /* SAVE EXTRUSION DATA */
  data = GCODE_DATA_SKETCH_EXTRUSION;
  size = 0;
  fwrite (&data, sizeof (uint8_t), 1, fh);

  /* Write block type */
  marker = ftell (fh);
  size = 0;
  fwrite (&size, sizeof (uint32_t), 1, fh);

  /* Write comment */
  data = GCODE_DATA_BLOCK_COMMENT;
  size = strlen (sketch->extrusion->comment) + 1;
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (sketch->extrusion->comment, sizeof (char), size, fh);

  sketch->extrusion->save (sketch->extrusion, fh);

  size = ftell (fh) - marker - sizeof (uint32_t);
  fseek (fh, marker, SEEK_SET);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fseek (fh, marker + size + sizeof (uint32_t), SEEK_SET);
  /***********************/

  child_block = sketch->list;
  num = 0;
  while (child_block)
  {
    num++;
    child_block = child_block->next;
  }

  data = GCODE_DATA_SKETCH_NUM;
  size = sizeof (uint32_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&num, size, 1, fh);

  child_block = sketch->list;
  num = 0;
  while (child_block)
  {
    /* Write block type */
    fwrite (&child_block->type, sizeof (uint8_t), 1, fh);
    marker = ftell (fh);
    size = 0;
    fwrite (&size, sizeof (uint32_t), 1, fh);

    /* Write comment */
    data = GCODE_DATA_BLOCK_COMMENT;
    size = strlen (child_block->comment) + 1;
    fwrite (&data, sizeof (uint8_t), 1, fh);
    fwrite (&size, sizeof (uint32_t), 1, fh);
    fwrite (child_block->comment, sizeof (char), size, fh);

    /* Write flags */
    data = GCODE_DATA_BLOCK_FLAGS;
    size = 1;
    fwrite (&data, sizeof (uint8_t), 1, fh);
    fwrite (&size, sizeof (uint32_t), 1, fh);
    fwrite (&child_block->flags, 1, 1, fh);

    child_block->save (child_block, fh);

    size = ftell (fh) - marker - sizeof (uint32_t);
    fseek (fh, marker, SEEK_SET);
    fwrite (&size, sizeof (uint32_t), 1, fh);
    fseek (fh, marker + size + sizeof (uint32_t), SEEK_SET);

    child_block = child_block->next;
  }

  data = GCODE_DATA_SKETCH_TAPER_OFFSET;
  size = 2 * sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (sketch->taper_offset, size, 1, fh);

  data = GCODE_DATA_SKETCH_POCKET;
  size = sizeof (uint8_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&sketch->pocket, size, 1, fh);

  data = GCODE_DATA_SKETCH_ZERO_PASS;
  size = sizeof (uint8_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&sketch->zero_pass, size, 1, fh);

  data = GCODE_DATA_SKETCH_HELICAL;
  size = sizeof (uint8_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&sketch->helical, size, 1, fh);
}


void
gcode_sketch_load (gcode_block_t *block, FILE *fh)
{
  gcode_sketch_t *sketch;
  gcode_block_t *child_block, *last_block;
  uint32_t bsize, dsize, start, num, i;
  uint8_t data, type;

  sketch = (gcode_sketch_t *) block->pdata;

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

      case GCODE_DATA_SKETCH_EXTRUSION:
        /* Rewind 4 bytes because the extrusion wants to read in its block size too. */
        fseek (fh, -4, SEEK_CUR);
        gcode_extrusion_load (sketch->extrusion, fh);
        break;
  
      case GCODE_DATA_SKETCH_NUM:
        fread (&num, sizeof (uint32_t), 1, fh);
        for (i = 0; i < num; i++)
        {
          /* Read Data */
          fread (&type, sizeof (uint8_t), 1, fh);

          switch (type)
          {
            case GCODE_TYPE_ARC:
              gcode_arc_init (block->gcode, &child_block, block);
              child_block->offset = &sketch->offset;
              break;

            case GCODE_TYPE_LINE:
              gcode_line_init (block->gcode, &child_block, block);
              child_block->offset = &sketch->offset;
              break;

            default:
              break;
          }

          /* Add to the end of the list */
          if (sketch->list)
          {
            gcode_list_insert (&last_block, child_block);
           }
          else
          {
            gcode_list_insert (&sketch->list, child_block);
          }
          last_block = child_block;

          child_block->parent_list = &sketch->list;
          child_block->load (child_block, fh);
        }
        break;

      case GCODE_DATA_SKETCH_TAPER_OFFSET:
        fread (&sketch->taper_offset, dsize, 1, fh);
        break;
  
      case GCODE_DATA_SKETCH_POCKET:
        fread (&sketch->pocket, dsize, 1, fh);
        break;

      case GCODE_DATA_SKETCH_ZERO_PASS:
        fread (&sketch->zero_pass, dsize, 1, fh);
        break;

      case GCODE_DATA_SKETCH_HELICAL:
        fread (&sketch->helical, dsize, 1, fh);
        break;

      default:
        fseek (fh, dsize, SEEK_CUR);
        break;
    }
  }
}


void
gcode_sketch_draw (gcode_block_t *block, gcode_block_t *selected)
{
#if GCODE_USE_OPENGL
  gcode_sketch_t *sketch;
  gcode_extrusion_t *extrusion;
  gcode_tool_t *tool;
  gcode_block_t *child_block, *start_block, *index_block;
  gcode_vec2d_t p0, p1, e0, e1, t;
  gfloat_t z, inside, block_length, accum_length, path_length, length_coef;
  int closed, top_only, taper_exists, path_num;


  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  sketch = (gcode_sketch_t *) block->pdata;
  if (!sketch->list)
    return;

  extrusion = (gcode_extrusion_t *) sketch->extrusion->pdata;
  taper_exists = gcode_extrusion_taper_exists (sketch->extrusion);

  tool = gcode_tool_find (block);
  if (tool == NULL)
    return;

  /* Whether to just display the top or with extrusion. */
  top_only = 0;
  if (selected)
    if (selected->parent == block)
      top_only = 1;

  /*
  * Evaluate the Extrusion curve to provide an offset and depth to
  * each of the child block make functions.
  */
  sketch->extrusion->ends (sketch->extrusion, p0, p1, GCODE_GET);
  /* Swap ends if necessary so p0 is above p1 */
  if (p0[1] < p1[1])
  {
    z = p0[1];
    p0[1] = p1[1];
    p1[1] = z;
  }


  for (index_block = sketch->list; index_block; index_block = index_block->next)
  {
    /*
    * find a continuous string of continuous blocks for the purpose of
    * milling continuous blocks at the same depth at the same time.
    */

    start_block = index_block;

    if (index_block->next)
    {
      if (index_block->ends && index_block->next->ends)
      {
        index_block->ends (index_block, t, e0, GCODE_GET);
        index_block->next->ends (index_block->next, e1, t, GCODE_GET);

        while (index_block->next && (fabs (e0[0]-e1[0]) + fabs (e0[1]-e1[1]) < SKETCH_TOL))
        {
          index_block = index_block->next;
          index_block->ends (index_block, t, e0, GCODE_GET);
          if (index_block->next)
            index_block->next->ends (index_block->next, e1, t, GCODE_GET);
        }
      }
    }

    start_block->ends (start_block, e0, t, GCODE_GET);
    index_block->ends (index_block, t, e1, GCODE_GET);
    closed = (fabs (e0[0]-e1[0]) + fabs (e0[1]-e1[1])) < SKETCH_TOL ? 1 : 0;

    /*
    * Determine if the right side of the first line is inside or outside the sketch.
    * Use this to tell whether the offset by the extrusion sketch should be right
    * or left of the primitive.
    */

    /*
    * temporarily set side to 0 so that when the blocks are evaluated the offset is not applied
    * because at this point any truncation/filleting has not occured and self intersections may
    * result creating bogus results.
    */
    if (closed)
    {
      sketch->offset.side = 0.0;
      inside = gcode_sketch_inside (start_block, index_block);
      sketch->offset.side = inside ? -1.0 : 1.0;
    }
    if (fabs (p0[0] - p1[0]) < GCODE_PRECISION && extrusion->cut_side == GCODE_EXTRUSION_ALONG)
    {
      sketch->offset.tool = 0.0;
    }
    else
    {
      if (extrusion->cut_side == GCODE_EXTRUSION_INSIDE)
        sketch->offset.side *= -1.0;
    }

    /*
    * Mill down to the depth specified by the extrusion.
    * Set the first pass to 0 if zero_pass or helical is specified.
    */
    z = sketch->zero_pass | sketch->helical ? p0[1] : p0[1]-extrusion->resolution;

    if (z-GCODE_PRECISION > p1[1] && (z - extrusion->resolution) < p1[1])
      z = p1[1];

    while (z >= p1[1])
    {
      gcode_block_t *evaluated_offset_list;

      /* Update origin based on taper offset */
      sketch->offset.origin[0] = sketch->taper_offset[0] * (p0[1] - z) / (p0[1] - p1[1]);
      sketch->offset.origin[1] = sketch->taper_offset[1] * (p0[1] - z) / (p0[1] - p1[1]);
      if (block->offset)
      {
        sketch->offset.origin[0] += block->offset->origin[0];
        sketch->offset.origin[1] += block->offset->origin[1];
        sketch->offset.rotation = block->offset->rotation;
      }

      gcode_extrusion_evaluate_offset (sketch->extrusion, z, &sketch->offset.eval);

/*      if (fabs (e0[0] - e1[0]) < GCODE_PRECISION && fabs (e0[1] - e1[1]) < GCODE_PRECISION && (block == selected || !selected)) */

      if (top_only)
      {
        child_block = start_block;
        child_block->offset->z[0] = z;
        child_block->offset->z[1] = z;
        do
        {
          child_block->draw (child_block, selected);
          child_block = child_block->next;
        } while (child_block && child_block != index_block->next);

        /* Only display top level to make editing easier */
        z = p1[1] - extrusion->resolution;
      }
      else
      {
        /*
        * Duplicate List, apply offsets, set eval and tool to 0 (zero offsets), make/draw, free
        */
        gcode_util_duplicate_list (start_block, index_block->next, &evaluated_offset_list);
        gcode_util_push_offset (evaluated_offset_list);

        /* Draw the blocks */
        child_block = closed ? evaluated_offset_list : start_block;

        /* Create all of the transition arcs */
        if (closed)
        {
          child_block = evaluated_offset_list;
          do
          {
            transition_arc (evaluated_offset_list, child_block);
            child_block = child_block->next;
          } while (child_block);
        }

        gcode_sketch_path_length (evaluated_offset_list, &path_length, &path_num);
/*
printf ("path_length_draw: %f from %d segments\n", path_length, path_num);
*/
        /* Generate the G-Code */
        child_block = evaluated_offset_list;
        accum_length = 0.0;
        do
        {
          if (closed && sketch->helical && !taper_exists && z-GCODE_PRECISION > p1[1])
          {
            block_length = child_block->length (child_block);

            length_coef = accum_length / path_length;

            if (z - extrusion->resolution + GCODE_PRECISION >= p1[1])
            {
              child_block->offset->z[0] = z * (1.0 - length_coef) + (z - extrusion->resolution) * length_coef;
              accum_length += block_length;

              length_coef = accum_length / path_length;
              child_block->offset->z[1] = z * (1.0 - length_coef) + (z - extrusion->resolution) * length_coef;
            }
            else
            {
              child_block->offset->z[0] = z * (1.0 - length_coef) + p1[1] * length_coef;
              accum_length += block_length;

              length_coef = accum_length / path_length;
              child_block->offset->z[1] = z * (1.0 - length_coef) + p1[1] * length_coef;
            }
          }
          else
          {
            child_block->offset->z[0] = z;
            child_block->offset->z[1] = z;
          }
          child_block->draw (child_block, selected);

          child_block = child_block->next;
        } while (child_block);


        /* Free the offset that was created, this is kind of ugly having 2 lines to free right now. */
        free (evaluated_offset_list->offset);
        gcode_list_free (&evaluated_offset_list);
      }

      if (z-GCODE_PRECISION > p1[1] && (z - extrusion->resolution) < p1[1])
      {
        /* set equal to last layer. */
        z = p1[1];
      }
      else
      {
        /* decrement to next deepest layer or terminate. */
        z -= extrusion->resolution;
      }
    }
  }

  sketch->offset.side = 0.0;
  sketch->offset.tool = 0.0;
  sketch->offset.eval = 0.0;
#endif
}


void
gcode_sketch_duplicate (gcode_block_t *block, gcode_block_t **duplicate)
{
  gcode_sketch_t *sketch, *duplicate_sketch;
  gcode_block_t *child_block, *new_block, *last_block;

  sketch = (gcode_sketch_t *) block->pdata;

  gcode_sketch_init (block->gcode, duplicate, block->parent);
  (*duplicate)->name = block->name;

  strcpy ((*duplicate)->comment, block->comment);
  (*duplicate)->parent = block->parent;

  duplicate_sketch = (gcode_sketch_t *) (*duplicate)->pdata;
  
  duplicate_sketch->taper_offset[0] = sketch->taper_offset[0];
  duplicate_sketch->taper_offset[1] = sketch->taper_offset[1];
  duplicate_sketch->offset = sketch->offset;
  duplicate_sketch->pocket = sketch->pocket;
  duplicate_sketch->zero_pass = sketch->zero_pass;

  sketch->extrusion->duplicate (sketch->extrusion, &duplicate_sketch->extrusion);
  duplicate_sketch->list = NULL;

  child_block = sketch->list;
  while (child_block)
  {
    child_block->duplicate (child_block, &new_block);
    new_block->parent = *duplicate;
    new_block->offset = &duplicate_sketch->offset;
    if (!duplicate_sketch->list)
    {
      gcode_list_insert (&duplicate_sketch->list, new_block);
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
gcode_sketch_scale (gcode_block_t *block, gfloat_t scale)
{
  gcode_sketch_t *sketch;
  gcode_block_t *index_block;

  sketch = (gcode_sketch_t *) block->pdata;
  sketch->extrusion->scale (sketch->extrusion, scale);
  index_block = sketch->list;
  while (index_block)
  {
    index_block->scale (index_block, scale);
    index_block = index_block->next;
  }
}


void
gcode_sketch_aabb (gcode_block_t *block, gcode_vec2d_t min, gcode_vec2d_t max)
{
  gcode_sketch_t *sketch;
  gcode_block_t *index_block;
  gcode_vec2d_t tmin, tmax;

  sketch = (gcode_sketch_t *) block->pdata;

  for (index_block = sketch->list; index_block; index_block = index_block->next)
  {
    index_block->aabb (index_block, tmin, tmax); /* asserted that the blocks are only arcs and lines */

    if (index_block == sketch->list)
    {
      min[0] = tmin[0];
      min[1] = tmin[1];
      max[0] = tmax[0];
      max[1] = tmax[1];
    }

    if (tmin[0] < min[0])
      min[0] = tmin[0];

    if (tmin[0] > max[0])
      max[0] = tmin[0];

    if (tmax[1] < min[1])
      min[1] = tmax[1];

    if (tmax[1] > max[1])
      max[1] = tmax[1];
  }
}


void
gcode_sketch_pattern (gcode_block_t *block, int iterations, gfloat_t translate_x, gfloat_t translate_y, gfloat_t rotate_about_x, gfloat_t rotate_about_y, gfloat_t rotation)
{
  gcode_sketch_t *sketch;
  gcode_block_t *child_block, *pattern_block, **pattern_list, *last_block;
  gfloat_t inc_rotation, inc_translate_x, inc_translate_y;
  int i;

  sketch = (gcode_sketch_t *) block->pdata;
  pattern_list = (gcode_block_t **) malloc (sizeof (gcode_block_t **));
  *pattern_list = NULL;

  for (i = 0; i < iterations; i++)
  {
    inc_rotation = ((float) i) * rotation;
    inc_translate_x = ((float) i) * translate_x;
    inc_translate_y = ((float) i) * translate_y;

    child_block = sketch->list;
    while (child_block)
    {
      switch (child_block->type)
      {
        case GCODE_TYPE_ARC:
          {
            gcode_arc_t *arc, *pattern_arc;
            gcode_vec2d_t xform_pt, pt;

            arc = (gcode_arc_t *) child_block->pdata;

            gcode_arc_init (block->gcode, &pattern_block, *pattern_list);
            pattern_arc = (gcode_arc_t *) pattern_block->pdata;

            /* Rotation and Translate */
            pt[0] = arc->pos[0] - rotate_about_x;
            pt[1] = arc->pos[1] - rotate_about_y;
            GCODE_MATH_ROTATE (xform_pt, pt, inc_rotation);
            pattern_arc->pos[0] = xform_pt[0] + rotate_about_x + inc_translate_x;
            pattern_arc->pos[1] = xform_pt[1] + rotate_about_y + inc_translate_y;
            pattern_arc->sweep = arc->sweep;
            pattern_arc->radius = arc->radius;
            pattern_arc->start_angle = arc->start_angle + inc_rotation;
          }
          break;

        case GCODE_TYPE_LINE:
          {
            gcode_line_t *line, *pattern_line;
            gcode_vec2d_t xform_pt, pt;

            line = (gcode_line_t *) child_block->pdata;

            gcode_line_init (block->gcode, &pattern_block, *pattern_list);
            pattern_line = (gcode_line_t *) pattern_block->pdata;

            /* Rotate and Translate */
            pt[0] = line->p0[0] - rotate_about_x;
            pt[1] = line->p0[1] - rotate_about_y;
            GCODE_MATH_ROTATE (xform_pt, pt, inc_rotation);
            pattern_line->p0[0] = xform_pt[0] + rotate_about_x + inc_translate_x;
            pattern_line->p0[1] = xform_pt[1] + rotate_about_y + inc_translate_y;

            pt[0] = line->p1[0] - rotate_about_x;
            pt[1] = line->p1[1] - rotate_about_y;
            GCODE_MATH_ROTATE (xform_pt, pt, inc_rotation);
            pattern_line->p1[0] = xform_pt[0] + rotate_about_x + inc_translate_x;
            pattern_line->p1[1] = xform_pt[1] + rotate_about_y + inc_translate_y;
          }
          break;
      }

      strcpy (pattern_block->comment, child_block->comment);
      pattern_block->offset = child_block->offset;
      pattern_block->parent = child_block->parent;
      pattern_block->next = NULL;
      pattern_block->prev = NULL;

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

  gcode_list_free (&sketch->list);
  sketch->list = *pattern_list;
}


int
gcode_sketch_is_closed (gcode_block_t *block)
{
  gcode_sketch_t *sketch;
  gcode_block_t *start_block, *index_block;
  gcode_vec2d_t e0, e1, t;
  int closed = 1;

  sketch = (gcode_sketch_t *) block->pdata;

  for (index_block = sketch->list; index_block; index_block = index_block->next)
  {
    /*
    * find a continuous string of blocks for the purpose of
    * milling a set of continuous blocks at the same depth.
    */
    start_block = index_block;

    if (index_block->next)
    {
      if (index_block->ends && index_block->next->ends)
      {
        index_block->ends (index_block, t, e0, GCODE_GET);
        index_block->next->ends (index_block->next, e1, t, GCODE_GET);

        while (index_block->next && (fabs (e0[0]-e1[0]) + fabs (e0[1]-e1[1]) < SKETCH_TOL))
        {
          index_block = index_block->next;
          index_block->ends (index_block, t, e0, GCODE_GET);
          if (index_block->next)
            index_block->next->ends (index_block->next, e1, t, GCODE_GET);
        }
      }
    }

    start_block->ends (start_block, e0, t, GCODE_GET);
    index_block->ends (index_block, t, e1, GCODE_GET);
    closed &= (fabs (e0[0]-e1[0]) + fabs (e0[1]-e1[1])) < SKETCH_TOL ? 1 : 0;
  }

  return (closed);
}


gcode_block_t*
gcode_sketch_prev_connected (gcode_block_t *block)
{
  gcode_block_t *index_block;
  gcode_vec2d_t t, e0, e1;
  gfloat_t dist;

  /*
  * Find prev connected block.
  */
  index_block = *block->parent_list;
  while (index_block)
  {
    if (index_block != block)
    {
      /* Check if ends are the same pt. */
      block->ends (block, e0, t, GCODE_GET);

      index_block->ends (index_block, t, e1, GCODE_GET);

      dist = sqrt ((e0[0]-e1[0])*(e0[0]-e1[0]) + (e0[1]-e1[1])*(e0[1]-e1[1]));
      if (dist <= SKETCH_TOL)
        return (index_block);
    }

    index_block = index_block->next;
  }

  return (NULL);
}


gcode_block_t*
gcode_sketch_next_connected (gcode_block_t *block)
{
  gcode_block_t *index_block;
  gcode_vec2d_t t, e0, e1;
  gfloat_t dist;

  /*
  * Find next connected block.
  */

  index_block = *block->parent_list;

  while (index_block)
  {
    if (index_block != block)
    {
      /* Check if ends are the same pt. */
      block->ends (block, t, e0, GCODE_GET);
      index_block->ends (index_block, e1, t, GCODE_GET);

      dist = sqrt ((e0[0]-e1[0])*(e0[0]-e1[0]) + (e0[1]-e1[1])*(e0[1]-e1[1]));
      if (dist <= SKETCH_TOL)
        return (index_block);
    }

    index_block = index_block->next;
  }

  return (NULL);
}
