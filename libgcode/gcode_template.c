/*
*  gcode_template.c
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
#include "gcode_template.h"
#include "gcode.h"

void
gcode_template_init (GCODE_INIT_PARAMETERS)
{
  gcode_template_t *template;

  *block = (gcode_block_t *) malloc (sizeof (gcode_block_t));
  gcode_internal_init (*block, parent, gcode, GCODE_TYPE_TEMPLATE, 0);

  (*block)->free = gcode_template_free;
  (*block)->make = gcode_template_make;
  (*block)->save = gcode_template_save;
  (*block)->load = gcode_template_load;
  (*block)->draw = gcode_template_draw;
  (*block)->duplicate = gcode_template_duplicate;
  (*block)->scale = gcode_template_scale;
  (*block)->pdata = malloc (sizeof (gcode_template_t));

  strcpy ((*block)->comment, "Template");
  strcpy ((*block)->status, "OK");
  GCODE_INIT((*block));
  GCODE_CLEAR((*block));

  /* defaults */
  template = (gcode_template_t *)(*block)->pdata;
  template->list = NULL;
  template->position[0] = 0.0;
  template->position[1] = 0.0;
  template->rotation = 0.0;

  /* offset */
  template->offset.origin[0] = 0.0;
  template->offset.origin[1] = 0.0;
  template->offset.side = 0.0;
  template->offset.tool = 0.0;
  template->offset.eval = 0.0;
  template->offset.rotation = 0.0;
}


void
gcode_template_free (gcode_block_t **block)
{
  free (*block);
  *block = NULL;
}


void
gcode_template_make (gcode_block_t *block)
{
  gcode_template_t *template;
  gcode_block_t *iter_block;
  char string[256];

  GCODE_CLEAR(block);
  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  template = (gcode_template_t *) block->pdata;

  template->offset.origin[0] = template->position[0];
  template->offset.origin[1] = template->position[1];
  template->offset.rotation = template->rotation;

  GCODE_APPEND (block, "\n");
  sprintf (string, "TEMPLATE: %s", block->comment);
  GCODE_COMMENT (block, string);
  GCODE_APPEND (block, "\n");

  iter_block = template->list;

  while (iter_block)
  {
    iter_block->offset = &template->offset;
    iter_block->make (iter_block);
    GCODE_APPEND(block, iter_block->code);
    iter_block = iter_block->next;
  }
}


void
gcode_template_save (gcode_block_t *block, FILE *fh)
{
  gcode_template_t *template;
  gcode_block_t *child_block;
  uint32_t size, num, marker;
  uint8_t data;

  template = (gcode_template_t *) block->pdata;

  child_block = template->list;
  num = 0;
  while (child_block)
  {
    num++;
    child_block = child_block->next;
  }

  data = GCODE_DATA_TEMPLATE_NUM;
  size = sizeof (uint32_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&num, size, 1, fh);

  child_block = template->list;
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

  data = GCODE_DATA_TEMPLATE_POSITION;
  size = 2 * sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (template->position, size, 1, fh);

  data = GCODE_DATA_TEMPLATE_ROTATION;
  size = sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&template->rotation, size, 1, fh);
}


void
gcode_template_load (gcode_block_t *block, FILE *fh)
{
  gcode_template_t *template;
  gcode_block_t *child_block, *last_block;
  uint32_t bsize, dsize, start, num, i;
  uint8_t data, type;

  template = (gcode_template_t *) block->pdata;
  gcode_list_free (&template->list);

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

      case GCODE_DATA_TEMPLATE_NUM:
        fread (&num, sizeof (uint32_t), 1, fh);
 
        for (i = 0; i < num; i++)
        {
          /* Read Data */
          fread (&type, sizeof (uint8_t), 1, fh);
          switch (type)
          {
            case GCODE_TYPE_TOOL:
              gcode_tool_init (block->gcode, &child_block, block);
              break;

            case GCODE_TYPE_TEMPLATE:
              gcode_template_init (block->gcode, &child_block, block);
              break;

            case GCODE_TYPE_SKETCH:
              gcode_sketch_init (block->gcode, &child_block, block);
              break;

            case GCODE_TYPE_BOLT_HOLES:
              gcode_bolt_holes_init (block->gcode, &child_block, block);
              break;

            case GCODE_TYPE_DRILL_HOLES:
              gcode_drill_holes_init (block->gcode, &child_block, block);
              break;

            default:
              break;
          }

          child_block->load (child_block, fh);

          /* Add to the end of the list */
          if (template->list)
          {
            gcode_list_insert (&last_block, child_block);
          }
          else
          {
            gcode_list_insert (&template->list, child_block);
          }

          child_block->parent_list = &template->list;
          last_block = child_block;
        }
        break;

      case GCODE_DATA_TEMPLATE_POSITION:
        fread (template->position, dsize, 1, fh);
        break;

      case GCODE_DATA_TEMPLATE_ROTATION:
        fread (&template->rotation, dsize, 1, fh);
        break;

      default:
        fseek (fh, dsize, SEEK_CUR);
        break;   
    }
  }
}


void
gcode_template_draw (gcode_block_t *block, gcode_block_t *selected)
{
#if GCODE_USE_OPENGL
  gcode_template_t *template;
  gcode_block_t *block_iter;

  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  template = (gcode_template_t *) block->pdata;

  template->offset.origin[0] = template->position[0];
  template->offset.origin[1] = template->position[1];
  template->offset.rotation = template->rotation;

  block_iter = template->list;
  while (block_iter)
  {
    block_iter->offset = &template->offset;

    if (block_iter->draw)
    {
      if (selected == block)
      {
        block_iter->draw (block_iter, block_iter);
      }
      else
      {
        block_iter->draw (block_iter, selected);
      }
    }

    block_iter = block_iter->next;
  }

  glPointSize (7);
  glColor3f (0.0, 1.0, 1.0);
  glBegin (GL_POINTS);
    glVertex3f (template->position[0], template->position[1], 0.0);
  glEnd ();
#endif
}


void
gcode_template_duplicate (gcode_block_t *block, gcode_block_t **duplicate)
{
  gcode_template_t *template, *duplicate_template;
  gcode_block_t *child_block, *new_block, *last_block;

  template = (gcode_template_t *) block->pdata;

  gcode_template_init (block->gcode, duplicate, block->parent);
  (*duplicate)->name = block->name;

  strcpy ((*duplicate)->comment, block->comment);
  (*duplicate)->parent = block->parent;

  duplicate_template = (gcode_template_t *) (*duplicate)->pdata;

  duplicate_template->position[0] = template->position[0];
  duplicate_template->position[1] = template->position[1];
  duplicate_template->rotation = template->rotation;
  duplicate_template->offset = template->offset;
          
  duplicate_template->list = NULL;
    
  child_block = template->list;
  while (child_block)
  {
    child_block->duplicate (child_block, &new_block);
    new_block->parent = *duplicate;
    new_block->offset = &duplicate_template->offset;
    if (!duplicate_template->list)
    {
      gcode_list_insert (&duplicate_template->list, new_block);
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
gcode_template_scale (gcode_block_t *block, gfloat_t scale)
{
  gcode_template_t *template;
  gcode_block_t *index_block;

  template = (gcode_template_t *) block->pdata;
  template->position[0] *= scale;
  template->position[1] *= scale;

  index_block = template->list;
  while (index_block)
  {
    if (index_block->scale) /* Because a Tool could be part of this list */
      index_block->scale (index_block, scale);
    index_block = index_block->next;
  }
}
