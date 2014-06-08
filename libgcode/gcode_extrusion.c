/*
*  gcode_extrusion.c
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
#include "gcode_extrusion.h"
#include "gcode_line.h"
#include "gcode.h"

void
gcode_extrusion_init (GCODE_INIT_PARAMETERS)
{
  gcode_extrusion_t *extrusion;
  gcode_line_t *line;
  gcode_block_t *line_block;

  *block = (gcode_block_t *) malloc (sizeof (gcode_block_t));
  gcode_internal_init (*block, parent, gcode, GCODE_TYPE_EXTRUSION, GCODE_FLAGS_LOCK);

  (*block)->free = gcode_extrusion_free;
  (*block)->make = gcode_extrusion_make;
  (*block)->save = gcode_extrusion_save;
  (*block)->load = gcode_extrusion_load;
  (*block)->ends = gcode_extrusion_ends;
  (*block)->draw = gcode_extrusion_draw;
  (*block)->duplicate = gcode_extrusion_duplicate;
  (*block)->scale = gcode_extrusion_scale;
  (*block)->pdata = malloc (sizeof (gcode_extrusion_t));

  strcpy ((*block)->comment, "extrusion");
  strcpy ((*block)->status, "OK");
  GCODE_INIT((*block));
  GCODE_CLEAR((*block));

  /* defaults */
  extrusion = (gcode_extrusion_t *)(*block)->pdata;
  extrusion->list = NULL;
  extrusion->resolution = floor (100.0 * gcode->material_size[2]) * 0.001;
  extrusion->cut_side = GCODE_EXTRUSION_INSIDE;

  extrusion->offset.side = 1.0;
  extrusion->offset.tool = 0.0;
  extrusion->offset.eval = 0.0;
  extrusion->offset.origin[0] = 0.0;
  extrusion->offset.origin[1] = 0.0;
  extrusion->offset.rotation = 0.0;

  /* Auto populate extrusion with line from Z0 to Z-Depth */
  gcode_line_init (gcode, &line_block, *block);
  line_block->offset = &extrusion->offset;
  line = (gcode_line_t *) line_block->pdata;
  line->p0[0] = 0.0;  
  line->p0[1] = 0.0;
  line->p1[0] = 0.0;
  line->p1[1] = -gcode->material_size[2];

  gcode_list_insert (&extrusion->list, line_block);
}


void
gcode_extrusion_free (gcode_block_t **block)
{
  gcode_extrusion_t *extrusion;
  gcode_block_t *tmp, *child_block;

  extrusion = (gcode_extrusion_t *) (*block)->pdata;

  /* Walk the list and free */
  child_block = extrusion->list;
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
gcode_extrusion_make (gcode_block_t *block)
{
  gcode_extrusion_t *extrusion;
  char text[128];

  extrusion = (gcode_extrusion_t *) block->pdata;

  GCODE_CLEAR(block);
  sprintf (text, ";\n; extrusion: %s\n;\n", block->comment);
  GCODE_APPEND(block, text);
}


void
gcode_extrusion_save (gcode_block_t *block, FILE *fh)
{
  gcode_extrusion_t *extrusion;
  gcode_block_t *child_block;
  uint32_t size, num, marker;
  uint8_t data;

  extrusion = (gcode_extrusion_t *) block->pdata;

  child_block = extrusion->list;
  num = 0;
  while (child_block)
  {
    num++;
    child_block = child_block->next;
  }

  data = GCODE_DATA_EXTRUSION_NUM;
  size = sizeof (uint32_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&num, size, 1, fh);

  child_block = extrusion->list;
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

    child_block->save (child_block, fh);                                

    size = ftell (fh) - marker - sizeof (uint32_t);
    fseek (fh, marker, SEEK_SET);
    fwrite (&size, sizeof (uint32_t), 1, fh);
    fseek (fh, marker + size + sizeof (uint32_t), SEEK_SET);

    child_block = child_block->next;
  }

  data = GCODE_DATA_EXTRUSION_RESOLUTION;
  size = sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&extrusion->resolution, size, 1, fh);

  data = GCODE_DATA_EXTRUSION_CUT_SIDE;
  size = sizeof (uint8_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&extrusion->cut_side, size, 1, fh);
}


void
gcode_extrusion_load (gcode_block_t *block, FILE *fh)
{
  gcode_extrusion_t *extrusion;
  gcode_block_t *child_block, *last_block;
  uint32_t bsize, dsize, start, num, i;
  uint8_t data, type;

  extrusion = (gcode_extrusion_t *) block->pdata;
  gcode_list_free (&extrusion->list);

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

      case GCODE_DATA_EXTRUSION_NUM:
        fread (&num, sizeof (uint32_t), 1, fh);

        for (i = 0; i < num; i++)
        {
          /* Read Data */
          fread (&type, sizeof (uint8_t), 1, fh);

          switch (type)
          {
            case GCODE_TYPE_ARC:
              gcode_arc_init (block->gcode, &child_block, block);
              child_block->offset = &extrusion->offset;
              break;

            case GCODE_TYPE_LINE:
              gcode_line_init (block->gcode, &child_block, block);
              child_block->offset = &extrusion->offset;
              break;

            default:
              break;
          }

          child_block->load (child_block, fh);

          /* Add to the end of the list */
          if (extrusion->list)
          {
            gcode_list_insert (&last_block, child_block);
          }
          else
          {
            gcode_list_insert (&extrusion->list, child_block);
          }
          last_block = child_block; 
        }
        break;

      case GCODE_DATA_EXTRUSION_RESOLUTION:
        fread (&extrusion->resolution, dsize, 1, fh);
        break;

      case GCODE_DATA_EXTRUSION_CUT_SIDE:
        fread (&extrusion->cut_side, dsize, 1, fh);
        break;

      default:
        fseek (fh, dsize, SEEK_CUR);
        break;
    }
  }
}


int
gcode_extrusion_ends (gcode_block_t *block, gfloat_t p0[2], gfloat_t p1[2], uint8_t mode)
{
  gcode_extrusion_t *extrusion;
  gcode_block_t *child_block;
  gfloat_t t[2];

  extrusion = (gcode_extrusion_t *) block->pdata;

  p0[0] = 0.0;
  p0[1] = 0.0;
  p1[0] = 0.0;
  p1[1] = 0.0;

  if (!extrusion->list)
    return (1);

  child_block = extrusion->list;

  /* Get Beginning of first block */
  child_block->ends (child_block, p0, t, GCODE_GET);

  while (child_block)
  {
    /* Get End of last block */
    if (!child_block->next)
      child_block->ends (child_block, t, p1, GCODE_GET);

    child_block = child_block->next;
  }

  return (0);
}


int
gcode_extrusion_evaluate_offset (gcode_block_t *block, gfloat_t z, gfloat_t *offset)
{
  gcode_extrusion_t *extrusion;
  gcode_block_t *child_block;
  gfloat_t p0[2], p1[2], x_array[2];
  int xind;

  extrusion = (gcode_extrusion_t *) block->pdata;

  child_block = extrusion->list;

  while (child_block)
  {
    child_block->ends (child_block, p0, p1, GCODE_GET);

    if ((z >= p0[1] && z <= p1[1]) || (z >= p1[1] && z <= p0[1]))
    {
      xind = 0;
      child_block->eval (child_block, z, x_array, &xind);
      *offset = x_array[0];

      return (0);
    }

    child_block = child_block->next;
  }

  return (1);
}


void gcode_extrusion_draw (gcode_block_t *block, gcode_block_t *selected)
{
#if GCODE_USE_OPENGL
  gcode_block_t *child_block;
  gcode_extrusion_t *extrusion;

  glDisable (GL_DEPTH_TEST);

  extrusion = (gcode_extrusion_t *) block->pdata;

  child_block = extrusion->list;
  while (child_block)
  {
    child_block->draw (child_block, selected);
    child_block = child_block->next;
  }
#endif
}


void
gcode_extrusion_duplicate (gcode_block_t *block, gcode_block_t **duplicate)
{
  gcode_extrusion_t *extrusion, *duplicate_extrusion;
  gcode_block_t *child_block, *new_block, *last_block;
  extrusion = (gcode_extrusion_t *) block->pdata;
  
  gcode_extrusion_init (block->gcode, duplicate, block->parent);
  (*duplicate)->name = block->name;
  
  strcpy ((*duplicate)->comment, block->comment);
  (*duplicate)->parent = block->parent;

  duplicate_extrusion = (gcode_extrusion_t *) (*duplicate)->pdata;
  gcode_list_free (&duplicate_extrusion->list);
  
  duplicate_extrusion->resolution = extrusion->resolution;
  duplicate_extrusion->offset = extrusion->offset;
  duplicate_extrusion->cut_side = extrusion->cut_side;

  child_block = extrusion->list;
  while (child_block)
  {
    child_block->duplicate (child_block, &new_block);
    new_block->parent = *duplicate;
    new_block->offset = &duplicate_extrusion->offset;
    if (!duplicate_extrusion->list)
    {
      gcode_list_insert (&duplicate_extrusion->list, new_block);
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
gcode_extrusion_scale (gcode_block_t *block, gfloat_t scale)
{
  gcode_extrusion_t *extrusion;
  gcode_block_t *index_block;

  extrusion = (gcode_extrusion_t *) block->pdata;
  extrusion->resolution *= scale;

  index_block = extrusion->list;
  while (index_block)
  {
    index_block->scale (index_block, scale);
    index_block = index_block->next;
  }
}


/*
* Determines if a taper exists, which may imply that pocketing can occur.
*/
int
gcode_extrusion_taper_exists (gcode_block_t *block)
{
  gcode_extrusion_t *extrusion;
  gcode_block_t *index_block;
  gcode_vec2d_t e0, e1, e2;

  extrusion = (gcode_extrusion_t *) block->pdata;

  index_block = extrusion->list;

  if (index_block == NULL)
    return (0);

  index_block->ends (index_block, e0, e1, GCODE_GET); /* This will always be a line */
  while (index_block)
  {
    if (index_block->type == GCODE_TYPE_ARC)
      return (1);

    index_block->ends (index_block, e1, e2, GCODE_GET); /* This will always be a line */
    if (fabs (e1[0] - e0[0]) > GCODE_PRECISION || fabs (e2[0] - e0[0]) > GCODE_PRECISION)   /* Checking X values */
      return (1);

    index_block = index_block->next;
  }

  return (0);
}
