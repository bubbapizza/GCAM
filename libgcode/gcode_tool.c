/*
*  gcode_tool.c
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
#include "gcode_tool.h"

void
gcode_tool_init (GCODE_INIT_PARAMETERS)
{
  gcode_tool_t *tool;

  *block = (gcode_block_t *) malloc (sizeof (gcode_block_t));
  gcode_internal_init (*block, parent, gcode, GCODE_TYPE_TOOL, 0);

  (*block)->free = gcode_tool_free;
  (*block)->make = gcode_tool_make;
  (*block)->save = gcode_tool_save;
  (*block)->load = gcode_tool_load;
  (*block)->duplicate = gcode_tool_duplicate;
  (*block)->pdata = malloc (sizeof (gcode_tool_t));

  strcpy ((*block)->comment, "Tool Change");
  strcpy ((*block)->status, "OK");
  GCODE_INIT((*block));
  GCODE_CLEAR((*block));

  tool = (gcode_tool_t *) (*block)->pdata;
  tool->prompt = 0;
  strcpy (tool->label, "");
  gcode_tool_calc (*block);
  tool->change[0] = 0.0;
  tool->change[1] = 0.0;
  tool->change[2] = GCODE_UNITS ((*block)->gcode, 1.0);
  tool->number = 1;
  tool->plunge_ratio = 0.2; /* 20% */
  tool->spindle_rpm = 2000; /* 2,000 RPM */

  tool->coolant = ((*block)->gcode->machine_options & GCODE_MACHINE_OPTION_COOLANT) == 0 ? 0 : 1;
}


void
gcode_tool_free (gcode_block_t **block)
{
  free ((*block)->pdata);
  free (*block);
  *block = NULL;
}


void
gcode_tool_calc (gcode_block_t *block)
{
  gcode_tool_t *tool;

  tool = (gcode_tool_t *) block->pdata;

  switch (block->gcode->material_type)
  {
    case GCODE_MATERIAL_ALUMINUM:
      tool->feed = 3.0;
      tool->plunge_ratio = 0.2;
      break;

    case GCODE_MATERIAL_FOAM:
      tool->feed = 15.0;
      tool->plunge_ratio = 1.0;
      break;

    case GCODE_MATERIAL_PLASTIC:
      tool->feed = 7.0;
      tool->plunge_ratio = 1.0;
      break;

    case GCODE_MATERIAL_STEEL:
      tool->feed = 0.1;
      tool->plunge_ratio = 0.1;
      break;

    case GCODE_MATERIAL_WOOD:
      tool->feed = 8.0;
      tool->plunge_ratio = 0.5;
      break;
  }


#if 0
  /* Heuristic */
  tool->hinc = GCODE_TOOL_AREA_COEF * tool->diam;
  tool->vinc = GCODE_TOOL_AREA_COEF * tool->diam;

  /* Scale the tool->feed rate based on a 1/8" bit reference */
  tool->feed *= 0.1 / tool->diam;

  if (tool->feed > GCODE_TOOL_MAX_FEED)
    tool->feed = GCODE_TOOL_MAX_FEED;
#endif

  if (block->gcode->units == GCODE_UNITS_MILLIMETER)
    tool->feed *= GCODE_INCH2MM;
}


void
gcode_tool_make (gcode_block_t *block)
{
  gcode_tool_t *tool;
  char string[256];

  GCODE_CLEAR(block);
  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  tool = (gcode_tool_t *) block->pdata;

  GCODE_APPEND (block, "\n");
  sprintf (string, "Tool Change: %s", tool->label);
  GCODE_COMMENT (block, string);

  if (block->gcode->machine_options & GCODE_MACHINE_OPTION_SPINDLE_CONTROL)
  {
    GCODE_APPEND (block, "M05 ");
    GCODE_COMMENT (block, "Spindle Off");
  }

  if (tool->prompt)
  {
    GCODE_RETRACT (block, tool->change[2]);
    gsprintf (string, block->gcode->decimal, "G00 X%z Y%z ", tool->change[0], tool->change[1]);
    GCODE_APPEND(block, string);
    GCODE_COMMENT (block, "move to safe tool change location");
  }

  if (tool->prompt || (block->gcode->machine_options & GCODE_MACHINE_OPTION_AUTOMATIC_TOOL_CHANGE))
  {
    sprintf (string, "M06 T%.2d (%s)\n", tool->number, tool->label);
    GCODE_APPEND(block, string);
  }

  sprintf (string, "F%.3f ", tool->feed);
  GCODE_APPEND(block, string);
  GCODE_COMMENT (block, "Feed Rate");
  sprintf (string, "GCAM:TOOL_DIAMETER:%f", tool->diam);
  GCODE_COMMENT (block, string);

  if (block->gcode->machine_options & GCODE_MACHINE_OPTION_SPINDLE_CONTROL)
  {
    sprintf (string, "S%d\n", tool->spindle_rpm);
    GCODE_APPEND(block, string);
  }

  if (block->gcode->machine_options & GCODE_MACHINE_OPTION_SPINDLE_CONTROL)
  {
    GCODE_APPEND (block, "M03 ");
    GCODE_COMMENT (block, "Spindle On");
  }


  if (block->gcode->machine_options & GCODE_MACHINE_OPTION_COOLANT)
  {
    if (tool->coolant)
    {
      sprintf (string, "M08 ");
      GCODE_APPEND(block, string);
      GCODE_COMMENT (block, "Coolant On");
    }
    else
    {
      sprintf (string, "M09 ");
      GCODE_APPEND(block, string);
      GCODE_COMMENT (block, "Coolant Off");
    }
  }
}


void
gcode_tool_save (gcode_block_t *block, FILE *fh)
{
  gcode_tool_t *tool;

  tool = (gcode_tool_t *) block->pdata;

  GCODE_WRITE_DATA (fh, GCODE_DATA_TOOL_DIAM, sizeof (gfloat_t), &tool->diam);
  GCODE_WRITE_DATA (fh, GCODE_DATA_TOOL_LEN, sizeof (gfloat_t), &tool->len);
  GCODE_WRITE_DATA (fh, GCODE_DATA_TOOL_PROMPT, sizeof (uint8_t), &tool->prompt);
  GCODE_WRITE_DATA (fh, GCODE_DATA_TOOL_LABEL, 32, tool->label);
  GCODE_WRITE_DATA (fh, GCODE_DATA_TOOL_FEED, sizeof (gfloat_t), &tool->feed);
  GCODE_WRITE_DATA (fh, GCODE_DATA_TOOL_CHANGE, 3 * sizeof (gfloat_t), &tool->change);
  GCODE_WRITE_DATA (fh, GCODE_DATA_TOOL_NUMBER, sizeof (uint8_t), &tool->number);
  GCODE_WRITE_DATA (fh, GCODE_DATA_TOOL_PLUNGE_RATIO, sizeof (gfloat_t), &tool->plunge_ratio);
  GCODE_WRITE_DATA (fh, GCODE_DATA_TOOL_SPINDLE_RPM, sizeof (uint32_t), &tool->spindle_rpm);
  GCODE_WRITE_DATA (fh, GCODE_DATA_TOOL_COOLANT, sizeof (uint8_t), &tool->coolant);
}


void
gcode_tool_load (gcode_block_t *block, FILE *fh)
{
  gcode_tool_t *tool;
  uint32_t bsize, dsize, start;
  uint8_t data;

  tool = (gcode_tool_t *) block->pdata;

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

      case GCODE_DATA_TOOL_DIAM:
        fread (&tool->diam, dsize, 1, fh);
        break;

      case GCODE_DATA_TOOL_LEN:
        fread (&tool->len, dsize, 1, fh);
        break;

      case GCODE_DATA_TOOL_PROMPT:
        fread (&tool->prompt, dsize, 1, fh);
        break;

      case GCODE_DATA_TOOL_LABEL:
        fread (tool->label, dsize, 1, fh);
        break;

      case GCODE_DATA_TOOL_FEED:
        fread (&tool->feed, dsize, 1, fh);
        break;

      case GCODE_DATA_TOOL_CHANGE:
        fread (tool->change, dsize, 1, fh);
        break;

      case GCODE_DATA_TOOL_NUMBER:
        fread (&tool->number, dsize, 1, fh);
        break;

      case GCODE_DATA_TOOL_PLUNGE_RATIO:
        fread (&tool->plunge_ratio, dsize, 1, fh);
        break;

      case GCODE_DATA_TOOL_SPINDLE_RPM:
        fread (&tool->spindle_rpm, dsize, 1, fh);
        break;

      case GCODE_DATA_TOOL_COOLANT:
        fread (&tool->coolant, dsize, 1, fh);
        break;

      default:
        fseek (fh, dsize, SEEK_CUR);
        break;
    }
  }
}


void
gcode_tool_duplicate (gcode_block_t *block, gcode_block_t **duplicate)
{
  gcode_tool_t *tool, *duplicate_tool;

  tool = (gcode_tool_t *) block->pdata;

  gcode_tool_init (block->gcode, duplicate, block->parent);
  (*duplicate)->name = block->name;
    
  strcpy ((*duplicate)->comment, block->comment);
  (*duplicate)->parent = block->parent;
  (*duplicate)->offset = block->offset;

  duplicate_tool = (gcode_tool_t *) (*duplicate)->pdata;
  
  duplicate_tool->hinc = tool->hinc;
  duplicate_tool->vinc = tool->vinc;
  duplicate_tool->diam = tool->diam;
  duplicate_tool->len = tool->len;
  duplicate_tool->feed = tool->feed;
  duplicate_tool->prompt = tool->prompt;
  duplicate_tool->change[0] = tool->change[0];
  duplicate_tool->change[1] = tool->change[1];
  duplicate_tool->change[2] = tool->change[2];
  duplicate_tool->plunge_ratio = tool->plunge_ratio;
}


/*
* Locate the nearest most previous tool by walking the list backwards and using recursion.
*/
gcode_tool_t*
gcode_tool_find (gcode_block_t *block)
{
  gcode_block_t *b;

  b = block;
  while (b)
  {
    if (b->type == GCODE_TYPE_TOOL)
      return ((gcode_tool_t *) b->pdata);
    b = b->prev;
  }

  /* Next, try searching the parent list */
  if (block->parent)
    return (gcode_tool_find (block->parent));

/* printf ("no tool found, this is a bug.\n"); */
  return (NULL);
}
