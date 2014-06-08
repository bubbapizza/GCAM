/*
*  gcode_bolt_holes.c
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
#include "gcode_bolt_holes.h"
#include "gcode_extrusion.h"
#include "gcode_tool.h"
#include "gcode_arc.h"
#include "gcode_pocket.h"
#include "gcode_util.h"
#include "gcode.h"

void
gcode_bolt_holes_init (GCODE_INIT_PARAMETERS)
{
  gcode_bolt_holes_t *bolt_holes;

  *block = (gcode_block_t *) malloc (sizeof (gcode_block_t));
  gcode_internal_init (*block, parent, gcode, GCODE_TYPE_BOLT_HOLES, 0);

  (*block)->free = gcode_bolt_holes_free;
  (*block)->make = gcode_bolt_holes_make;
  (*block)->save = gcode_bolt_holes_save;
  (*block)->load = gcode_bolt_holes_load;
  (*block)->draw = gcode_bolt_holes_draw;
  (*block)->duplicate = gcode_bolt_holes_duplicate;
  (*block)->scale = gcode_bolt_holes_scale;
  (*block)->pdata = malloc (sizeof (gcode_bolt_holes_t));

  strcpy ((*block)->comment, "Bolt Holes");
  strcpy ((*block)->status, "OK");
  GCODE_INIT((*block));
  GCODE_CLEAR((*block));

  /* defaults */
  bolt_holes = (gcode_bolt_holes_t *) (*block)->pdata;
  bolt_holes->pos[0] = 0.0;
  bolt_holes->pos[1] = 0.0;
  bolt_holes->num[0] = 4;
  bolt_holes->num[1] = 4;
  bolt_holes->hole_diameter = GCODE_UNITS ((*block)->gcode, 0.25);
  bolt_holes->type = GCODE_BOLT_HOLES_TYPE_RADIAL;
  bolt_holes->offset_distance = GCODE_UNITS ((*block)->gcode, 0.5);
  bolt_holes->offset_angle = 0.0;
  bolt_holes->pocket = 0;

  bolt_holes->offset.origin[0] = 0.0;
  bolt_holes->offset.origin[1] = 0.0;
  bolt_holes->offset.side = -1.0;
  bolt_holes->offset.tool = 0.0;
  bolt_holes->offset.eval = 0.0;
  bolt_holes->offset.rotation = 0.0;

  /* Create Extrusion block */
  gcode_extrusion_init (gcode, &bolt_holes->extrusion, *block);
  bolt_holes->extrusion->prev = NULL;
  bolt_holes->extrusion->next = NULL;

  /* Initialize bolt_holes List */
  bolt_holes->arc_list = NULL;

  gcode_bolt_holes_rebuild (*block);
}


void
gcode_bolt_holes_free (gcode_block_t **block)
{
  gcode_bolt_holes_t *bolt_holes;
  gcode_block_t *child_block, *tmp;

  bolt_holes = (gcode_bolt_holes_t *) (*block)->pdata;

  /* Free the extrusion list */
  bolt_holes->extrusion->free (&bolt_holes->extrusion);
  
  /* Walk the list and free */
  child_block = bolt_holes->arc_list;
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


void
gcode_bolt_holes_make (gcode_block_t *block)
{
  gcode_bolt_holes_t *bolt_holes;
  gcode_extrusion_t *extrusion;
  gcode_tool_t *tool;
  gcode_block_t *child_block;
  gcode_vec2d_t p0, p1, e0, e1;
  gfloat_t tool_rad, z, last_z;
  char string[256];
  int i;

  GCODE_CLEAR(block);
  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  bolt_holes = (gcode_bolt_holes_t *) block->pdata;

  extrusion = (gcode_extrusion_t *) bolt_holes->extrusion->pdata;   
  tool = gcode_tool_find (block);
  if (tool == NULL)
   return;
  tool_rad = tool->diam * 0.5;

  /* Update origin */
  bolt_holes->offset.origin[0] = 0.0;
  bolt_holes->offset.origin[1] = 0.0;
  if (block->offset)
  {
    bolt_holes->offset.origin[0] += block->offset->origin[0];
    bolt_holes->offset.origin[1] += block->offset->origin[1];
    bolt_holes->offset.rotation = block->offset->rotation;
  }

  bolt_holes->offset.tool = tool_rad;

  /*
  * Evaluate the Extrusion curve to provide an offset and depth to
  * each of the child block make functions.
  */
  bolt_holes->extrusion->ends (bolt_holes->extrusion, p0, p1, GCODE_GET);
  /* Swap ends if necessary so p0 is above p1 */
  if (p0[1] < p1[1])
  {
    z = p0[1];
    p0[1] = p1[1];
    p1[1] = z;
  }


  /* Can Cycle G81 Start */
  if (fabs (bolt_holes->hole_diameter - tool->diam) < GCODE_PRECISION)
  {
    /* Let the first X Y get placed on this line hence no '\n' */
    gsprintf (string, block->gcode->decimal, "G81 Z%z F%.3f R%z ", p1[1], tool->feed * tool->plunge_ratio, block->gcode->ztraverse);
    GCODE_APPEND(block, string);
  }


  i = 0;
  child_block = bolt_holes->arc_list;
  while (child_block)
  {
    if (fabs (bolt_holes->hole_diameter - tool->diam) < GCODE_PRECISION)
    {
      /* Can Cycle X,Y, GCODE_GET_WITH_OFFSET will provide center since endmill diameter equals hole diameter. */
      child_block->ends (child_block, e0, e1, GCODE_GET_WITH_OFFSET);
      gsprintf (string, block->gcode->decimal, "X%z Y%z ", e0[0], e0[1]);
      GCODE_APPEND (block, string);
      sprintf (string, "hole #%d", i);
      GCODE_COMMENT(block, string);
    }
    else
    {
      GCODE_COMMENT (block, "");
      sprintf (string, "BOLT HOLES: %s", block->comment);
      GCODE_COMMENT (block, string);
      sprintf (string, "Hole #%d", i+1);
      GCODE_COMMENT (block, string);

      /*
      * Retract - should already be retracted, but here for safety reasons.
      */
      GCODE_RETRACT(block, block->gcode->ztraverse);

      /* Mill down to the depth specified by the extrusion */
      last_z = z = p0[1];
      while (z >= p1[1])
      {
        gcode_block_t *evaluated_offset_list;

        gcode_extrusion_evaluate_offset (bolt_holes->extrusion, z, &bolt_holes->offset.eval);

        gcode_util_duplicate_list (child_block, child_block->next, &evaluated_offset_list);
        gcode_util_push_offset (evaluated_offset_list);

        /* Pocketing if mode is set */
        if (bolt_holes->pocket)
        {
          gcode_pocket_t pocket;

          gcode_pocket_init (&pocket, tool_rad);
          gcode_pocket_prep (&pocket, child_block, child_block->next);
          gcode_pocket_make (&pocket, block, z, last_z, tool);
          gcode_pocket_free (&pocket);
        }

        /*
        * If starting new hole then move into position.
        */
        if (z == p0[1] || bolt_holes->pocket)
        {
          evaluated_offset_list->ends (evaluated_offset_list, e0, e1, GCODE_GET_WITH_OFFSET);
          gsprintf (string, block->gcode->decimal, "G00 X%z Y%z ", e0[0], e0[1]);
          GCODE_APPEND (block, string);
          sprintf (string, "move to start");
          GCODE_COMMENT (block, string);
        }

        if (fabs (last_z - z) > GCODE_PRECISION || last_z > block->gcode->material_origin[2])
          GCODE_PLUNGE_RAPID(block, last_z);
        GCODE_PLUNGE(block, z, tool);

        evaluated_offset_list->offset->z[0] = z;
        evaluated_offset_list->offset->z[1] = z;
        evaluated_offset_list->make (evaluated_offset_list); /* There is only ever 1 arc */
        GCODE_APPEND(block, evaluated_offset_list->code);

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

      GCODE_RETRACT(block, block->gcode->ztraverse);
    }

    child_block = child_block->next;
    i++;
  }


  /* Can Cycle G80 End */
  if (fabs (bolt_holes->hole_diameter - tool->diam) < GCODE_PRECISION)
  {
    sprintf (string, "G80\n");
    GCODE_APPEND(block, string);
    sprintf (string, "F%.3f ", tool->feed);
    GCODE_APPEND(block, string);
    sprintf (string, "normal feed rate");
    GCODE_COMMENT (block, string);
  }


  bolt_holes->offset.tool = 0.0;
  bolt_holes->offset.eval = 0.0;
}


void
gcode_bolt_holes_save (gcode_block_t *block, FILE *fh)
{
  gcode_bolt_holes_t *bolt_holes;
  uint32_t size, marker;
  uint8_t data;

  bolt_holes = (gcode_bolt_holes_t *) block->pdata;

  /* SAVE EXTRUSION DATA */
  data = GCODE_DATA_BOLT_HOLES_EXTRUSION;
  size = 0;
  fwrite (&data, sizeof (uint8_t), 1, fh);

  /* Write block type */
  marker = ftell (fh);
  size = 0;
  fwrite (&size, sizeof (uint32_t), 1, fh);

  /* Write comment */
  data = GCODE_DATA_BLOCK_COMMENT;
  size = strlen (bolt_holes->extrusion->comment) + 1;
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (bolt_holes->extrusion->comment, sizeof (char), size, fh);

  bolt_holes->extrusion->save (bolt_holes->extrusion, fh);

  size = ftell (fh) - marker - sizeof (uint32_t);
  fseek (fh, marker, SEEK_SET);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fseek (fh, marker + size + sizeof (uint32_t), SEEK_SET);
  /***********************/
  
  data = GCODE_DATA_BOLT_HOLES_POS;
  size = 2 * sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&bolt_holes->pos[0], sizeof (gfloat_t), 1, fh);
  fwrite (&bolt_holes->pos[1], sizeof (gfloat_t), 1, fh);

  data = GCODE_DATA_BOLT_HOLES_HOLE_DIAMETER;
  size = sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&bolt_holes->hole_diameter, size, 1, fh);
   
  data = GCODE_DATA_BOLT_HOLES_OFFSET_DISTANCE;
  size = sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&bolt_holes->offset_distance, size, 1, fh);

  data = GCODE_DATA_BOLT_HOLES_TYPE;
  size = sizeof (uint8_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&bolt_holes->type, size, 1, fh);

  data = GCODE_DATA_BOLT_HOLES_NUM;
  size = 2 * sizeof (int);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (bolt_holes->num, size, 1, fh);
  
  data = GCODE_DATA_BOLT_HOLES_OFFSET_ANGLE;
  size = sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&bolt_holes->offset_angle, size, 1, fh);

  data = GCODE_DATA_BOLT_HOLES_POCKET;
  size = sizeof (uint8_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&bolt_holes->pocket, size, 1, fh);
}


void
gcode_bolt_holes_load (gcode_block_t *block, FILE *fh)
{
  gcode_bolt_holes_t *bolt_holes;
  uint32_t bsize, dsize, start;
  uint8_t data;

  bolt_holes = (gcode_bolt_holes_t *) block->pdata;

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

      case GCODE_DATA_BOLT_HOLES_EXTRUSION:
        /* Rewind 4 bytes because the extrusion wants to read in its block size too. */
        fseek (fh, -4, SEEK_CUR);
        gcode_extrusion_load (bolt_holes->extrusion, fh);
        break;

      case GCODE_DATA_BOLT_HOLES_POS:
        fread (bolt_holes->pos, dsize, 1, fh);
        break;

      case GCODE_DATA_BOLT_HOLES_HOLE_DIAMETER:
        fread (&bolt_holes->hole_diameter, dsize, 1, fh);
        break;

      case GCODE_DATA_BOLT_HOLES_OFFSET_DISTANCE:
        fread (&bolt_holes->offset_distance, dsize, 1, fh);
        break;

      case GCODE_DATA_BOLT_HOLES_TYPE:
        fread (&bolt_holes->type, dsize, 1, fh);
        break;

      case GCODE_DATA_BOLT_HOLES_NUM:
        fread (bolt_holes->num, dsize, 1, fh);
        break;

      case GCODE_DATA_BOLT_HOLES_OFFSET_ANGLE:
        fread (&bolt_holes->offset_angle, dsize, 1, fh);
        break;

      case GCODE_DATA_BOLT_HOLES_POCKET:
        fread (&bolt_holes->pocket, dsize, 1, fh);
        break;
  
      default:
        fseek (fh, dsize, SEEK_CUR);
        break;
    }
  }

  gcode_bolt_holes_rebuild (block);
}


void
gcode_bolt_holes_draw (gcode_block_t *block, gcode_block_t *selected)
{
#if GCODE_USE_OPENGL
  gcode_bolt_holes_t *bolt_holes;
  gcode_extrusion_t *extrusion;
  gcode_tool_t *tool;
  gcode_block_t *child_block;
  gcode_vec2d_t p0, p1;
  gfloat_t z;

  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  bolt_holes = (gcode_bolt_holes_t *) block->pdata;

  extrusion = (gcode_extrusion_t *) bolt_holes->extrusion->pdata;
  tool = gcode_tool_find (block);
  if (tool == NULL)
    return;

  /* Update origin */
  bolt_holes->offset.origin[0] = 0.0;
  bolt_holes->offset.origin[1] = 0.0;
  if (block->offset)
  {
    bolt_holes->offset.origin[0] += block->offset->origin[0];
    bolt_holes->offset.origin[1] += block->offset->origin[1];
    bolt_holes->offset.rotation = block->offset->rotation;
  }
  bolt_holes->offset.tool = 0.0;


  /*
  * Evaluate the Extrusion curve to provide an offset and depth to
  * each of the child block make functions.
  */
  bolt_holes->extrusion->ends (bolt_holes->extrusion, p0, p1, GCODE_GET);
  /* Swap ends if necessary so p0 is above p1 */
  if (p0[1] < p1[1])
  {
    z = p0[1];
    p0[1] = p1[1];
    p1[1] = z;
  }


  /* Draw the blocks */
  child_block = bolt_holes->arc_list;
  while (child_block)
  {
    /* Mill down to the specified depth */
    z = p0[1];

    glLoadName ((GLuint) block->name);
    while (z >= p1[1])
    {
      gcode_block_t *evaluated_offset_list;

      gcode_extrusion_evaluate_offset (bolt_holes->extrusion, z, &bolt_holes->offset.eval);
 
      gcode_util_duplicate_list (child_block, child_block->next, &evaluated_offset_list);
      gcode_util_push_offset (evaluated_offset_list);

      evaluated_offset_list->offset->z[0] = z;
      evaluated_offset_list->offset->z[1] = z;
      if (selected == block)
      {
        evaluated_offset_list->draw (evaluated_offset_list, evaluated_offset_list);
      }
      else
      {
        evaluated_offset_list->draw (evaluated_offset_list, selected);
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

      /* Free the offset that was created, this is kind of ugly having 2 lines to free right now. */
      free (evaluated_offset_list->offset);
      gcode_list_free (&evaluated_offset_list);
    }
    child_block = child_block->next;
  }


  bolt_holes->offset.eval = 0.0;
#endif
}


void
gcode_bolt_holes_duplicate (gcode_block_t *block, gcode_block_t **duplicate)
{
  gcode_bolt_holes_t *bolt_holes, *duplicate_bolt_holes;
  gcode_block_t *child_block, *new_block, *last_block;

  bolt_holes = (gcode_bolt_holes_t *) block->pdata;

  gcode_bolt_holes_init (block->gcode, duplicate, block->parent);

  strcpy ((*duplicate)->comment, block->comment);
  (*duplicate)->parent = block->parent;

  duplicate_bolt_holes = (gcode_bolt_holes_t *) (*duplicate)->pdata;

  duplicate_bolt_holes->pos[0] = bolt_holes->pos[0];
  duplicate_bolt_holes->pos[1] = bolt_holes->pos[1];
  duplicate_bolt_holes->num[0] = bolt_holes->num[0];
  duplicate_bolt_holes->num[1] = bolt_holes->num[1];
  duplicate_bolt_holes->type = bolt_holes->type;
  duplicate_bolt_holes->hole_diameter = bolt_holes->hole_diameter;
  duplicate_bolt_holes->offset_distance = bolt_holes->offset_distance;
  duplicate_bolt_holes->offset_angle = bolt_holes->offset_angle;
  duplicate_bolt_holes->offset = bolt_holes->offset;

  bolt_holes->extrusion->duplicate (bolt_holes->extrusion, &duplicate_bolt_holes->extrusion);
  duplicate_bolt_holes->arc_list = NULL;

  child_block = bolt_holes->arc_list;
  while (child_block)
  {
    child_block->duplicate (child_block, &new_block);
    new_block->parent = *duplicate;
    new_block->offset = &duplicate_bolt_holes->offset;
    if (!duplicate_bolt_holes->arc_list)
    {
      gcode_list_insert (&duplicate_bolt_holes->arc_list, new_block);
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
gcode_bolt_holes_scale (gcode_block_t *block, gfloat_t scale)
{
  gcode_bolt_holes_t *bolt_holes;
  gcode_block_t *index_block;

  bolt_holes = (gcode_bolt_holes_t *) block->pdata;

  index_block = bolt_holes->arc_list;
  while (index_block)
  {
    index_block->scale (index_block, scale);
    index_block = index_block->next;
  }
}


void
gcode_bolt_holes_rebuild (gcode_block_t *block)
{
  gcode_bolt_holes_t *bolt_holes;
  gcode_block_t *child_block, *last_block, *tmp;
  gcode_arc_t *arc;

  bolt_holes = (gcode_bolt_holes_t *) block->pdata;

  /* Walk the list and free */
  child_block = bolt_holes->arc_list;
  while (child_block)
  {
    tmp = child_block;
    child_block = child_block->next;
    tmp->free (&tmp);
  }
  bolt_holes->arc_list = NULL;

  if (bolt_holes->type == GCODE_BOLT_HOLES_TYPE_RADIAL)
  {
    int i;

    /* Rebuild the list and calculate values for each arc */
    for (i = 0; i < bolt_holes->num[0]; i++)
    {
      gfloat_t angle;

      gcode_arc_init (block->gcode, &child_block, block);
      child_block->name = (uint32_t) ((uint64_t) block - (uint64_t) block->gcode) >> 3; /* assign each arc a name ptr to bolt_holes */

      arc = (gcode_arc_t *) child_block->pdata;

      child_block->offset = &bolt_holes->offset;

      angle = bolt_holes->offset_angle + 360.0 * ((gfloat_t) i) / ((gfloat_t) bolt_holes->num[0]);

      arc->radius = bolt_holes->hole_diameter * 0.5;
      arc->pos[0] = bolt_holes->pos[0] + bolt_holes->offset_distance * cos (angle * GCODE_DEG2RAD) - arc->radius;
      arc->pos[1] = bolt_holes->pos[1] + bolt_holes->offset_distance * sin (angle * GCODE_DEG2RAD);
      arc->start_angle = 180.0;
      arc->sweep = 360.0;

      if (bolt_holes->arc_list)
      {
        gcode_list_insert (&last_block, child_block);
      }
      else
      {
        gcode_list_insert (&bolt_holes->arc_list, child_block);
      }

      last_block = child_block;
    }
  }
  else if (bolt_holes->type == GCODE_BOLT_HOLES_TYPE_MATRIX)
  {
    int i, j;

    /* Rebuild the list and calculate values for each arc */
    for (i = 0; i < bolt_holes->num[0]; i++)
      for (j = 0; j < bolt_holes->num[1]; j++)
      {
        gcode_arc_init (block->gcode, &child_block, block);
        child_block->name = (uint32_t) ((uint64_t) block - (uint64_t) block->gcode) >> 3; /* assign each arc a name ptr to bolt_holes */

        arc = (gcode_arc_t *) child_block->pdata;

        child_block->offset = &bolt_holes->offset;

        arc->radius = bolt_holes->hole_diameter * 0.5;
        arc->pos[0] = bolt_holes->pos[0] + ((gfloat_t) i) * bolt_holes->offset_distance - arc->radius;
        arc->pos[1] = bolt_holes->pos[1] + (i%2 ? (gfloat_t) (bolt_holes->num[1] - j - 1) : (gfloat_t) j) * bolt_holes->offset_distance;
        arc->start_angle = 180.0;
        arc->sweep = 360.0;

        if (bolt_holes->arc_list)
        {
          gcode_list_insert (&last_block, child_block);
        }
        else
        {
          gcode_list_insert (&bolt_holes->arc_list, child_block);
        }

        last_block = child_block;
      }
  }
}
