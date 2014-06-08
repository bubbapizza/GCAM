/*
*  gcode_excellon.c
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
#include "gcode_gerber.h"
#include "gcode_drill_holes.h"
#include "gcode_point.h"
#include "gcode_tool.h"
#include "gcode_arc.h"
#include "gcode_line.h"
#include "gcode_util.h"
#include "gcode.h"


int
gcode_excellon_import (gcode_t *gcode, gcode_block_t ***block_array, int *block_num, char *filename)
{
  FILE *fh;
  gcode_drill_holes_t *drill_holes;
  gcode_point_t *point;
  gcode_tool_t *tool;
  gcode_excellon_tool_t *tool_array;
  gcode_block_t *point_block, *index_block;
  char *file_buf = NULL, buf[10], mesg[64];
  int i, file_buf_ind, file_buf_size, buf_ind, start, tool_num, tool_ind, hole_num;


  *block_num = 0;
  start = 0;
  tool_array = NULL;
  tool_num = 0;
  tool_ind = 0;
  hole_num = 0;

  fh = fopen (filename, "r");
  if (!fh)
    return (1);

  fseek (fh, 0, SEEK_END);
  file_buf_size = ftell (fh);
  fseek (fh, 0, SEEK_SET);
  file_buf = (char *) malloc (file_buf_size);
  fread (file_buf, 1, file_buf_size, fh);
  file_buf_ind = 0;

  while (file_buf_ind < file_buf_size)
  {
    if (file_buf[file_buf_ind] == 'T')
    {
      file_buf_ind++;

      buf[0] = file_buf[file_buf_ind];
      buf[1] = file_buf[file_buf_ind+1];
      buf[2] = 0;
      if (start)
      {
        for (i = 0; i < tool_num && atoi (buf) != tool_array[i].index; i++);
          tool_ind = i;

        /* Create both a tool and drill holes block */
        *block_array = (gcode_block_t **) realloc (*block_array, (*block_num + 2) * sizeof (gcode_block_t *));
        gcode_tool_init (gcode, &(*block_array)[*block_num+0], NULL);
        gcode_drill_holes_init (gcode, &(*block_array)[*block_num+1], NULL);

        tool = (gcode_tool_t *) (*block_array)[*block_num+0]->pdata;
        tool->diam = tool_array[tool_ind].diameter;
        tool->feed = 1.0;
        tool->prompt = 1;
        sprintf ((*block_array)[*block_num+0]->comment, "T%d %.4f\"", tool_array[tool_ind].index, tool->diam);
        sprintf (tool->label, "T%d %.4f\"", tool_array[tool_ind].index, tool->diam);
        drill_holes = (gcode_drill_holes_t *) (*block_array)[*block_num+1]->pdata;
        (*block_num) += 2;
      }
      else
      {
        tool_array = (gcode_excellon_tool_t *) realloc (tool_array, (tool_num + 1) * sizeof (gcode_excellon_tool_t));
        tool_array[tool_num].index = atoi (buf);

        file_buf_ind += 2;

        /* Skip over 'C' */
        file_buf_ind++;

        buf_ind = 0;
        while (file_buf[file_buf_ind] != '\n')
        {
          buf[buf_ind] = file_buf[file_buf_ind];
          buf_ind++;
          file_buf_ind++;
        }

        tool_array[tool_num].diameter = atof (buf);
        tool_num++;
      }
    }
    else if (file_buf[file_buf_ind] == '%')
    {
      if (tool_num > 0)
        start = 1;
      file_buf_ind++;
    }
    else if (file_buf[file_buf_ind] == 'X')
    {
      gfloat_t x, y;
      file_buf_ind++;

      buf_ind = 0;
      while (file_buf[file_buf_ind] != 'Y')
      {
        buf[buf_ind] = file_buf[file_buf_ind];
        buf_ind++;
        file_buf_ind++;
      }
      x = 0.0001 * atof (buf);
      file_buf_ind++;

      buf_ind = 0;
      while (file_buf[file_buf_ind] != '\n')
      {
        buf[buf_ind] = file_buf[file_buf_ind];
        buf_ind++;
        file_buf_ind++;
      }
      y = 0.0001 * atof (buf);
      file_buf_ind++;

      gcode_point_init (gcode, &point_block, NULL);
      point_block->offset = &drill_holes->offset;
      point = (gcode_point_t *) point_block->pdata;
      point->p[0] = x;
      point->p[1] = y;
      /* Append to the end of the current drill_holes list */
      if (drill_holes->list)
      {
        index_block = drill_holes->list;
        while (index_block->next)
          index_block = index_block->next;

        gcode_list_insert (&index_block, point_block);
      }
      else
      {
        gcode_list_insert (&drill_holes->list, point_block);
      }
      hole_num++;
    }
    else
    {
      while (file_buf[file_buf_ind] != '\n' && file_buf_ind < file_buf_size)
        file_buf_ind++;
      file_buf_ind++;
    }
  }


  sprintf (mesg, "%d Drill Holes using %d different Tools\n", hole_num, tool_num);
  gcode->message_callback (gcode->gui, mesg);

  free (tool_array);
  free (file_buf);
  fclose (fh);
  return (0);
}
