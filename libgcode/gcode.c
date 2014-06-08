/*
*  gcode.c
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
#include "gcode.h"
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include "gcode_util.h"
#include "gcode_sim.h"


void
gcode_list_insert (gcode_block_t **list, gcode_block_t *block)
{
  if (*list)
  {
    gcode_block_t *tmp;

    if ((*list)->next)
    {
      tmp = (*list)->next;

      (*list)->next = block;
      block->prev = *list;
      block->next = tmp;

      tmp->prev = block;
    }
    else
    {
      (*list)->next = block;
      block->prev = *list;
      block->next = NULL;
    }
  }
  else
  {
    *list = block;
    (*list)->prev = NULL;
    (*list)->next = NULL;
  }

  /*
  * This provides the block with direct access to the list to which it belongs.
  * If the (*list)->parent_list is NULL then this implies that the **list head has
  * been assigned the address of "block".  When this is the case "block" may not have
  * been assigned a parent_list pointer external to this function.  If this is the case
  * then the solution is to effectively point the blocks parent_list to itself.
  */
  if ((*list)->parent_list)
  {
    block->parent_list = (*list)->parent_list;
  }
  else
  {
    block->parent_list = list;
  }
}


void
gcode_list_splice (gcode_block_t **list, gcode_block_t *block)
{
  if (block->parent_list)
  {
    if (*block->parent_list == block)
    {
      if (block->next)
      {
        *block->parent_list = block->next;
      }
      else
      {
        *block->parent_list = NULL;
      }
    }
  }

  if (block->prev)
    block->prev->next = block->next;

  if (block->next)
    block->next->prev = block->prev;

  if (block == *list)
    *list = block->next;

  block->prev = NULL;
  block->next = NULL;
  block->parent = NULL;
  block->parent_list = NULL;
}


void
gcode_list_remove (gcode_block_t *block)
{
  if (block->flags & GCODE_FLAGS_LOCK)
    return;

  if (block->next)
    block->next->prev = block->prev;

  if (block->prev)
    block->prev->next = block->next;

  if (*block->parent_list == block)
    *block->parent_list = block->next;

  block->free (&block);
}


/* Move closer toward the Head of list */
void
gcode_list_move_prev (gcode_block_t *block)
{
  if (block->flags & GCODE_FLAGS_LOCK)
    return;

  if (!block->prev)
    return;

  if (block->prev->flags & GCODE_FLAGS_LOCK)
    return;

  if (block->prev->prev)
    block->prev->prev->next = block;

  /* link prev blocks */
  block->prev->next = block->next;

  /* link next block if exists */
  if (block->next)
    block->next->prev = block->prev;

  /* link block */
  block->next = block->prev;
  block->prev = block->prev->prev;

  /* Link previous blocks previous to point to this block */
  if (block->next)
    block->next->prev = block;

  if (!block->prev)
    *block->parent_list = block;
}


/* Move closer toward the Tail of list */
void
gcode_list_move_next (gcode_block_t *block)
{
  if (block->flags & GCODE_FLAGS_LOCK)
    return;

  if (!block->next)
    return;

  if (block->next->flags & GCODE_FLAGS_LOCK)
    return;

  if (block->next->next)
    block->next->next->prev = block;

  if (block->prev)
    block->prev->next = block->next;

  /* link next block */
  block->next->prev = block->prev;

  /* link block */
  block->prev = block->next;
  block->next = block->next->next;

  /* Link next blocks next to point to this block */
  block->prev->next = block;

  if (block == *block->parent_list)
    *block->parent_list = block->prev;
}


void
gcode_list_make (gcode_t *gcode)
{
  gcode_block_t *block;
  int i, num;

  num = 0;
  for (block = gcode->list; block; block = block->next)
    num++;

  /*
  * This can only be run after the list prev/next pointers are
  * set up, hence this is not done as each block is loaded.
  */
  i = 0;
  for (block = gcode->list; block; block = block->next)
  {
    /* Make the G-Code */
    block->make (block);

    if (gcode->progress_callback)
      gcode->progress_callback (gcode->gui, (gfloat_t) i / (gfloat_t) num);
    i++;
  }

  if (gcode->progress_callback)
    gcode->progress_callback (gcode->gui, 1.0);
}


void
gcode_list_free (gcode_block_t **list)
{
  gcode_block_t *tmp;

  /* Walk the list and free */
  while (*list)
  {
    tmp = *list;
    *list = (*list)->next;
    tmp->free (&tmp);
  }

  *list = NULL;
}


void
gcode_init (gcode_t *gcode)
{
  gcode->list = NULL;

  strcpy (gcode->name, "");
  strcpy (gcode->notes, "");
  gcode->material_size[0] = 0.0;
  gcode->material_size[1] = 0.0;
  gcode->material_size[2] = 0.0;

  gcode->material_origin[0] = 0.0;
  gcode->material_origin[1] = 0.0;
  gcode->material_origin[2] = 0.0;

  gcode->voxel_res = 0;
  gcode->voxel_map = NULL;

  /* Depth at which to traverse along XY plane */
  gcode->ztraverse = 0.0;

  gcode->progress_callback = NULL;

  strcpy (gcode->machine_name, "");
  gcode->machine_options = 0;

  gcode->decimal = 5;

  gcode->project_number = 0;
}


void
gcode_prep (gcode_t *gcode)
{
  uint32_t size;
  gcode_vec3d_t portion;
  gfloat_t inv_den;

  inv_den = 1.0 / (gcode->material_size[0] + gcode->material_size[1] + gcode->material_size[2]);


  portion[0] = gcode->material_size[0] * inv_den;
  portion[1] = gcode->material_size[1] * inv_den;
  portion[2] = gcode->material_size[2] * inv_den;

  /* Setup voxels */
  gcode->voxel_num[0] = (uint16_t) (gcode->voxel_res * portion[0]);
  gcode->voxel_num[1] = (uint16_t) (gcode->voxel_res * portion[1]);
  gcode->voxel_num[2] = (uint16_t) (gcode->voxel_res * portion[2]);

  if (gcode->voxel_num[0] == 0)
    gcode->voxel_num[0] = 1;
  if (gcode->voxel_num[1] == 0)
    gcode->voxel_num[1] = 1;
  if (gcode->voxel_num[2] == 0)
    gcode->voxel_num[2] = 1;

  size = gcode->voxel_num[0] * gcode->voxel_num[1] * gcode->voxel_num[2];

  gcode->voxel_map = (uint8_t *) realloc (gcode->voxel_map, size);
  memset (gcode->voxel_map, 1, size);
}


void
gcode_free (gcode_t *gcode)
{
  gcode_list_free (&gcode->list);
  free (gcode->voxel_map);
}


int
gcode_save (gcode_t *gcode, const char *filename)
{
  FILE *fh;
  uint32_t header, fsize, version, size, marker;
  gcode_block_t *block;
  uint8_t data, type;

  fh = fopen (filename, "wb");
  if (!fh)
  {
    return (1);
  }

  header = GCODE_FILE_HEADER;
  fwrite (&header, sizeof (uint32_t), 1, fh);  

  fsize = 0;
  fwrite (&fsize, sizeof (uint32_t), 1, fh);

  /*
  * VERSION
  */
  version = GCODE_VERSION;
  fwrite (&version, sizeof (uint32_t), 1, fh);

  /*
  * GCODE_DATA
  */
  type = GCODE_DATA;
  fwrite (&type, sizeof (uint8_t), 1, fh);
  marker = ftell (fh);
  size = 0;
  fwrite (&size, sizeof (uint32_t), 1, fh);

  data = GCODE_DATA_NAME;
  size = strlen (gcode->name) + 1;
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (gcode->name, sizeof (char), size, fh);

  data = GCODE_DATA_UNITS;
  size = sizeof (uint8_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&gcode->units, size, 1, fh);

  data = GCODE_DATA_MATERIAL_TYPE;
  size = sizeof (uint8_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&gcode->material_type, size, 1, fh);

  data = GCODE_DATA_MATERIAL_SIZE;
  size = 3 * sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (gcode->material_size, size, 1, fh);

  data = GCODE_DATA_MATERIAL_ORIGIN;
  size = 3 * sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (gcode->material_origin, size, 1, fh);

  data = GCODE_DATA_ZTRAVERSE;
  size = sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&gcode->ztraverse, size, 1, fh);

  {
    uint16_t nlen;

    data = GCODE_DATA_NOTES;
    size = sizeof (uint16_t) + strlen (gcode->notes) + 1;
    fwrite (&data, sizeof (uint8_t), 1, fh);
    fwrite (&size, sizeof (uint32_t), 1, fh);
    nlen = strlen (gcode->notes) + 1;
    fwrite (&nlen, sizeof (uint16_t), 1, fh);
    fwrite (gcode->notes, nlen, 1, fh);
  }

  size = ftell (fh) - marker - sizeof (uint32_t);
  fseek (fh, marker, SEEK_SET);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fseek (fh, marker + size + sizeof (uint32_t), SEEK_SET);

  /*
  * GCODE_MACHINE_DATA
  */
  type = GCODE_DATA_MACHINE;
  fwrite (&type, sizeof (uint8_t), 1, fh);
  marker = ftell (fh);
  size = 0;
  fwrite (&size, sizeof (uint32_t), 1, fh);

  data = GCODE_DATA_MACHINE_NAME;
  size = strlen (gcode->machine_name) + 1;
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (gcode->machine_name, sizeof (char), size, fh);

  data = GCODE_DATA_MACHINE_OPTIONS;
  size = sizeof (uint8_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (&gcode->machine_options, sizeof (uint8_t), 1, fh);

  size = ftell (fh) - marker - sizeof (uint32_t);
  fseek (fh, marker, SEEK_SET);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fseek (fh, marker + size + sizeof (uint32_t), SEEK_SET);

  for (block = gcode->list; block; block = block->next)
  {
    /* Write block type */
    fwrite (&block->type, sizeof (uint8_t), 1, fh);
    marker = ftell (fh);
    size = 0;
    fwrite (&size, sizeof (uint32_t), 1, fh);

    /* Write comment */
    data = GCODE_DATA_BLOCK_COMMENT;
    size = strlen (block->comment) + 1;
    fwrite (&data, sizeof (uint8_t), 1, fh);
    fwrite (&size, sizeof (uint32_t), 1, fh);
    fwrite (block->comment, sizeof (char), size, fh);

    /* Write flags */
    data = GCODE_DATA_BLOCK_FLAGS;
    size = 1;
    fwrite (&data, sizeof (uint8_t), 1, fh);
    fwrite (&size, sizeof (uint32_t), 1, fh);
    fwrite (&block->flags, 1, 1, fh);

    block->save (block, fh);

    size = ftell (fh) - marker - sizeof (uint32_t);
    fseek (fh, marker, SEEK_SET);
    fwrite (&size, sizeof (uint32_t), 1, fh);
    fseek (fh, marker + size + sizeof (uint32_t), SEEK_SET);
  }

  /* Write the actual length */
  fsize = ftell (fh);
  fseek (fh, sizeof (uint32_t), SEEK_SET);
  fwrite (&fsize, sizeof (uint32_t), 1, fh);

  fclose (fh);

  return (0);
}


int
gcode_load (gcode_t *gcode, const char *filename)
{
  FILE *fh;
  uint32_t header, fsize, version, size;
  uint8_t data, type;
  gcode_block_t *block, *last_block;


  fh = fopen (filename, "rb");
  if (!fh)
    return (1);

  /*
  * Set the numeric locale to C (Minimal) so that all numeric output
  * uses the period as the decimal separator since most G-Gcode drivers
  * are only compatible with a period decimal separator.  This allows
  * the user interface in GCAM to display a comma decimal separator
  * for locales that call for that format.
  */
  setlocale (LC_NUMERIC, "C");

  gcode_init (gcode);

  block = NULL;
  last_block = NULL;


  fread (&header, sizeof (uint32_t), 1, fh);  
  if (header != GCODE_FILE_HEADER)
    return (1);

  fread (&fsize, sizeof (uint32_t), 1, fh);

  fread (&version, sizeof (uint32_t), 1, fh);

  while (ftell(fh) < fsize)
  {
    /* Read Data */
    fread (&type, sizeof (uint8_t), 1, fh);

    switch (type)
    {
      case GCODE_DATA:
        {
          uint32_t start;


          fread (&size, sizeof (uint32_t), 1, fh);

          start = ftell (fh);
          while (ftell (fh) - start < size)
          {
            uint32_t dsize;
            fread (&data, sizeof (uint8_t), 1, fh);
            fread (&dsize, sizeof (uint32_t), 1, fh);

            switch (data)
            {
              case GCODE_DATA_NAME:
                fread (&gcode->name, sizeof (char), dsize, fh);
                break;

              case GCODE_DATA_UNITS:
                fread (&gcode->units, sizeof (uint8_t), 1, fh);
                break;

              case GCODE_DATA_MATERIAL_TYPE:
                fread (&gcode->material_type, sizeof (uint8_t), 1, fh);
                break;

              case GCODE_DATA_MATERIAL_SIZE:
                fread (&gcode->material_size, dsize, 1, fh);
                break;

              case GCODE_DATA_MATERIAL_ORIGIN:
                fread (&gcode->material_origin, dsize, 1, fh);
                break;

              case GCODE_DATA_ZTRAVERSE:
                fread (&gcode->ztraverse, dsize, 1, fh);
                break;

              case GCODE_DATA_NOTES:
                {
                  uint16_t nlen;

                  fread (&nlen, sizeof (uint16_t), 1, fh);
                  fread (gcode->notes, nlen, 1, fh);
                }
                break;

              default:
                fseek (fh, dsize, SEEK_CUR);
                break;
            }
          }
        }
        break;

      case GCODE_DATA_MACHINE:
        {
          uint32_t start;


          fread (&size, sizeof (uint32_t), 1, fh);

          start = ftell (fh);
          while (ftell (fh) - start < size)
          {
            uint32_t dsize;
            fread (&data, sizeof (uint8_t), 1, fh);
            fread (&dsize, sizeof (uint32_t), 1, fh);

            switch (data)
            {
              case GCODE_DATA_MACHINE_NAME:
                fread (&gcode->machine_name, sizeof (char), dsize, fh);
                break;

              case GCODE_DATA_MACHINE_OPTIONS:
                fread (&gcode->machine_options, dsize, 1, fh);
                break;

              default:
                fseek (fh, dsize, SEEK_CUR);
                break;
            }
          }
        }
        break;

      case GCODE_TYPE_BEGIN:
        gcode_begin_init (gcode, &block, NULL);
        break;

      case GCODE_TYPE_END:
        gcode_end_init (gcode, &block, NULL);
        break;

      case GCODE_TYPE_TOOL:
        gcode_tool_init (gcode, &block, NULL);
        break;

      case GCODE_TYPE_CODE:
        gcode_code_init (gcode, &block, NULL);
        break;

      case GCODE_TYPE_EXTRUSION:
        /* should never be called as a top level block */
        break;

      case GCODE_TYPE_SKETCH:
        gcode_sketch_init (gcode, &block, NULL);
        break;

      case GCODE_TYPE_LINE:
        /* should never be called as a top level block */
        break;

      case GCODE_TYPE_ARC:
        /* should never be called as a top level block */
        break;

      case GCODE_TYPE_BOLT_HOLES:
        gcode_bolt_holes_init (gcode, &block, NULL);
        break;

      case GCODE_TYPE_TEMPLATE:
        gcode_template_init (gcode, &block, NULL);
        break;

      case GCODE_TYPE_DRILL_HOLES:
        gcode_drill_holes_init (gcode, &block, NULL);
        break;

      case GCODE_TYPE_POINT:
        /* should never be called as a top level block */
        break;

      case GCODE_TYPE_IMAGE:
        gcode_image_init (gcode, &block, NULL);
        break;

      default:
        fread (&size, sizeof (uint32_t), 1, fh);
        fseek (fh, size, SEEK_CUR);
        break;
    }

    /* Append block to list */
    if (type != GCODE_DATA && type != GCODE_DATA_MACHINE)
    {
      block->load (block, fh);

      if (last_block)
      {
        gcode_list_insert (&last_block, block);
      }
      else
      {
        gcode_list_insert (&gcode->list, block);
      }
    }
    last_block = block;
  }

  gcode_list_make (gcode);

  fclose (fh);

  /* Restore Locale to previous value */
  setlocale (LC_NUMERIC, "");

/*  gcode_prep (gcode); */

  return (0);
}


static void
gcode_crlf (char **string)
{
  char *crlf_string;
  int i, n, len;

  crlf_string = malloc (2 * strlen (*string)+1);

  len = strlen (*string)+1;
  n = 0;
  for (i = 0; i < len; i++)
  {
    crlf_string[n] = (*string)[i];
    n++;

    if (crlf_string[n-1] == '\n')
    {
      crlf_string[n-1] = '\r';
      crlf_string[n] = '\n';
      n++;
    }
  }  

  *string = realloc (*string, strlen (crlf_string)+1);

  memcpy (*string, crlf_string, strlen (crlf_string)+1);

  free (crlf_string);
}


int
gcode_export (gcode_t *gcode, const char *filename)
{
  gcode_block_t *block;
  FILE *fh;
  int size;

  fh = fopen (filename, "w");
  if (!fh)
    return (1);
  /*
  * Check whether this is a windows machine or not by checking whether
  * writing \n results in \r\n in the file (2 bytes).
  */
  fprintf (fh, "\n");
  fseek (fh, 0, SEEK_END);
  size = ftell (fh);

  fseek (fh, 0, SEEK_SET);

  /*
  * Set appropriate number of decimals given driver
  */
  switch (gcode->driver)
  {
    case GCODE_DRIVER_HAAS:
      gcode->decimal = 4;
      break;

    default:
      gcode->decimal = 5;
      break;
  }

  /* Make all */
  gcode_list_make (gcode);

  for (block = gcode->list; block; block = block->next)
  {
    if (size == 1)
      gcode_crlf (&block->code);
    fwrite (block->code, 1, strlen (block->code), fh);
  }
  fclose (fh);

  return (0);
}


void
gcode_render_final (gcode_t *gcode, gfloat_t *time_elapsed)
{
  char *source, line[256], *sp, *tsp, *gv;
  gcode_block_t *block;
  gcode_sim_t sim;
  uint32_t size, line_num, line_ind, mode = 0;
  gfloat_t G83_depth = 0.0, G83_retract = 0.0;

  /* Make all */
  gcode_list_make (gcode);

  gcode_sim_init (gcode, &sim);

  GCODE_MATH_VEC3D_SET (sim.vn_inv, 1.0 / (gfloat_t) gcode->voxel_num[0], 1.0 / (gfloat_t) gcode->voxel_num[1], 1.0 / (gfloat_t) gcode->voxel_num[2]);

  /* Turn all the voxels back on */
  size = gcode->voxel_num[0] * gcode->voxel_num[1] * gcode->voxel_num[2];
  memset (gcode->voxel_map, 1, size);

  source = (char *) malloc (1);
  source[0] = 0;

  for (block = gcode->list; block; block = block->next)
  {
    source = (char *) realloc (source, strlen (source) + strlen (block->code) + 1);
    strcat (source, block->code);
  }

  /* Count the number of lines */
  line_num = 0;
  sp = source;
  while ((tsp = strchr (sp, '\n')))
  {
    sp = tsp + 1;
    line_num++;
  }

  /* Isolate each line */
  line_ind = 0;
  sp = source;
  while ((tsp = strchr (sp, '\n')))
  {
    uint8_t sind;

    if (gcode->progress_callback)
      gcode->progress_callback (gcode->gui, (gfloat_t) line_ind++ / (gfloat_t) line_num);

    memset (line, 0, 256);
    memcpy (line, sp, tsp - sp);
    sp = tsp+1;
    sind = 0;

    /*
    * Parse the line
    */

    /* Remove the spaces */
    gcode_util_remove_spaces (line);

    /* Scan for GCAM Variables of the form (GCAM:TOOL_DIAMETER:VALUE) */
    gv = strstr (line, "GCAM:TOOL_DIAMETER:");
    if (gv)
    {
      char string[256];
      uint8_t len;

      gv = strpbrk (line, ".0123456789");
      len = strspn (gv, ".0123456789");
      memset (string, 0, 256);
      memcpy (string, gv, len);
      sim.tool_diameter = atof (string);
    }

    /* Scan for GCAM Variables of the form (GCAM:ORIGIN:VALUE VALUE VALUE) */
    gv = strstr (line, "GCAM:ORIGIN:");
    if (gv)
    {
      char string[256];
      uint8_t len;

      gv = strpbrk (gv, ".0123456789");
      len = strspn (gv, ".0123456789");
      memset (string, 0, 256);
      memcpy (string, gv, len);
      sim.origin[0] = atof (string);
      sim.pos[0] += sim.origin[0];

      gv += len;
      gv = strpbrk (gv, ".0123456789");
      len = strspn (gv, ".0123456789");
      memset (string, 0, 256);
      memcpy (string, gv, len);
      sim.origin[1] = atof (string);
      sim.pos[1] += sim.origin[1];

      gv += len;
      gv = strpbrk (gv, ".0123456789");
      len = strspn (gv, ".0123456789");
      memset (string, 0, 256);
      memcpy (string, gv, len);
      sim.origin[2] = atof (string);
      sim.pos[2] += sim.origin[2];
    }

    /* Strip comments */
    gcode_util_remove_comment (line);

/*    printf ("line: %s\n", line);*/
    switch (line[sind])
    {
      case 'G':
        {
          uint8_t len;
          char string[256];

          sind++;

          /* The Number */
          len = strspn (&line[sind], "0123456789");
          memset (string, 0, 256);
          memcpy (string, &line[sind], len);
          sind += len;
/*          printf ("code: %s, %d\n", string, sind); */

          switch (atoi (string))
          {
            case 0:
              gcode_sim_G00 (gcode, &sim, &line[sind]);
              break;

            case 1:
              gcode_sim_G01 (gcode, &sim, &line[sind]);
              break;

            case 2:
              gcode_sim_G02 (gcode, &sim, &line[sind]);
              break;

            case 3:
              gcode_sim_G03 (gcode, &sim, &line[sind]);
              break;

            case 4:
              /* Dwell */
              break;

            case 20:
              break;

            case 21:
              break;

            case 81:
            case 83:
              gcode_sim_G83 (gcode, &sim, &line[sind], &G83_depth, &G83_retract, 1);
              mode = 83;
              break;

            case 90:
              sim.absolute = 1;
              break;

            case 91:
              sim.absolute = 0;
              break;

            default:
              break;
          }
        }
        break;

      case 'F':
        {
          char string[256];
          uint8_t len;

          sind++;

          len = strspn (&line[sind], ".0123456789");
          memset (string, 0, 256);
          memcpy (string, &line[sind], len);
          sim.feed = atof (string);
        }
        break;

      case 'X':
      case 'Y':
        {
          if (mode == 83)
          {
            gcode_sim_G83 (gcode, &sim, &line[sind], &G83_depth, &G83_retract, 0);
          }
        }
        break;

      default:
        break;
    }
  }

  free (source);

  /* Calculate elapsed time */
  sim.time_elapsed = 60 * sim.time_elapsed / sim.feed;

  *time_elapsed = sim.time_elapsed;
  gcode_sim_free (&sim);
}
