/*
*  gcode_image.c
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
#include "gcode_image.h"
#include "gcode_point.h"
#include "gcode_tool.h"
#include "gcode.h"
#include <png.h>

void
gcode_image_init (GCODE_INIT_PARAMETERS)
{
  gcode_image_t *image;

  *block = (gcode_block_t *) malloc (sizeof (gcode_block_t));
  gcode_internal_init (*block, parent, gcode, GCODE_TYPE_IMAGE, 0);

  (*block)->free = gcode_image_free;
  (*block)->make = gcode_image_make;
  (*block)->save = gcode_image_save;
  (*block)->load = gcode_image_load;
  (*block)->draw = gcode_image_draw;
  (*block)->duplicate = gcode_image_duplicate;
  (*block)->scale = gcode_image_scale;
  (*block)->pdata = malloc (sizeof (gcode_image_t));

  strcpy ((*block)->comment, "Image");
  strcpy ((*block)->status, "OK");
  GCODE_INIT((*block));
  GCODE_CLEAR((*block));

  /* defaults */
  image = (gcode_image_t *)(*block)->pdata;
  image->res[0] = 0;
  image->res[1] = 0;
  image->size[0] = GCODE_UNITS ((*block)->gcode, 1.0);
  image->size[1] = GCODE_UNITS ((*block)->gcode, 1.0);
  image->size[2] = -gcode->material_size[2];
}


void
gcode_image_free (gcode_block_t **block)
{
  gcode_image_t *image;

  image = (gcode_image_t *) (*block)->pdata;
  free (image->dmap);

  free ((*block)->code);
  free ((*block)->pdata);
  free (*block);
  *block = NULL;
}


void
gcode_image_make (gcode_block_t *block)
{
  gcode_image_t *image;
  gcode_tool_t *tool;
  gfloat_t xpos, ypos;
  char string[256];
  int x, y;

  GCODE_CLEAR(block);
  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  image = (gcode_image_t *) block->pdata;
  tool = gcode_tool_find (block);
  if (tool == NULL)
    return;

  sprintf (string, "IMAGE: %s", block->comment);
  GCODE_COMMENT (block, string);

  xpos = ((gfloat_t) 0) * image->size[0] / (gfloat_t) image->res[0];
  ypos = ((gfloat_t) 0) * image->size[1] / (gfloat_t) image->res[1];

  GCODE_RETRACT(block, block->gcode->ztraverse);
  gsprintf (string, block->gcode->decimal, "G00 X%z Y%z\n", xpos, ypos);
  GCODE_APPEND(block, string);
  GCODE_PLUNGE_RAPID(block, 0.0);


  for (y = 0; y < image->res[1]; y++)
  {
    ypos = ((gfloat_t) y) * image->size[1] / (gfloat_t) image->res[1];

    /* Even - Left to Right */
    for (x = 0; x < image->res[0]; x++)
    {
      xpos = ((gfloat_t) x) * image->size[0] / (gfloat_t) image->res[0];

      gsprintf (string, block->gcode->decimal, "G01 X%z Y%z Z%z\n", xpos, ypos, image->size[2]*image->dmap[y*image->res[0] + x]);
      GCODE_APPEND(block, string);
    }

    y++;

    if (y < image->res[1])
    {
      /* Odd - Left to Right */
      for (x = image->res[0]-1; x >= 0; x--)
      {
        xpos = ((gfloat_t) x) * image->size[0] / (gfloat_t) image->res[0];

        gsprintf (string, block->gcode->decimal, "G01 X%z Y%z Z%z\n", xpos, ypos, image->size[2]*image->dmap[y*image->res[0] + x]);
        GCODE_APPEND(block, string);
      }
    }

  }
}


void
gcode_image_save (gcode_block_t *block, FILE *fh)
{
  gcode_image_t *image;
  uint32_t size;
  uint8_t data;

  image = (gcode_image_t *) block->pdata;

  data = GCODE_DATA_IMAGE_RESOLUTION;
  size = 2 * sizeof (int);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (image->res, sizeof (int), 2, fh);

  data = GCODE_DATA_IMAGE_SIZE;
  size = 3 * sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (image->size, sizeof (gfloat_t), 3, fh);

  data = GCODE_DATA_IMAGE_DMAP;
  size = image->res[0] * image->res[1] * sizeof (gfloat_t);
  fwrite (&data, sizeof (uint8_t), 1, fh);
  fwrite (&size, sizeof (uint32_t), 1, fh);
  fwrite (image->dmap, sizeof (gfloat_t), image->res[0]*image->res[1], fh);
}


void
gcode_image_load (gcode_block_t *block, FILE *fh)
{
  gcode_image_t *image;
  uint32_t bsize, dsize, start;
  uint8_t data;

  image = (gcode_image_t *) block->pdata;

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

      case GCODE_DATA_IMAGE_RESOLUTION:
        fread (image->res, dsize, 1, fh);
        image->dmap = (gfloat_t *) malloc (sizeof (gfloat_t) * image->res[0] * image->res[1]);
        break;

      case GCODE_DATA_IMAGE_SIZE:
        fread (image->size, dsize, 1, fh);
        break;

      case GCODE_DATA_IMAGE_DMAP:
        fread (image->dmap, dsize, 1, fh);
        break;

      default:
        fseek (fh, dsize, SEEK_CUR);
        break;
    }
  }
}


void
gcode_image_open (gcode_block_t *block, char *filename)
{
  gcode_image_t *image;
  FILE *fp = fopen(filename, "rb");
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  char header[8];
  int x, y, incr;

  image = (gcode_image_t *) block->pdata;

  if (!fp)
  {
    printf ("ERROR!\n");
    return;
  }

  fread (header, 1, 8, fp);

  if (png_sig_cmp (header, 0, 8))
  {
    printf ("NOT_PNG!\n");
    return;
  }

  /* initialize stuff */
  png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr)
  {
    printf ("png_create_read_struct failed");
    return;
  }

  info_ptr = png_create_info_struct (png_ptr);
  if (!info_ptr)
  {
    printf ("png_create_info_struct failed");
    return;
  }

  if (setjmp (png_jmpbuf (png_ptr)))
  {
    printf ("error during init_io\n");
    return;
  }

  png_init_io (png_ptr, fp);
  png_set_sig_bytes (png_ptr, 8);

  png_read_info (png_ptr, info_ptr);

  image->res[0] = info_ptr->width;
  image->res[1] = info_ptr->height;

/*  printf ("image size: %dx%d\n", image->res[0], image->res[1]); */

  /* read file */
  if (setjmp (png_jmpbuf (png_ptr)))
  {
    printf ("error during read");
    return;
  }

  row_pointers = (png_bytep *) malloc (sizeof (png_bytep) * image->res[1]);
  for (y = 0; y < image->res[1]; y++)
    row_pointers[y] = (png_byte*) malloc (info_ptr->rowbytes);

  png_read_image (png_ptr, row_pointers);

  image->dmap = (gfloat_t *) malloc (sizeof (gfloat_t) * image->res[0] * image->res[1]);

  incr = 1;
  if (info_ptr->color_type & PNG_COLOR_MASK_COLOR)
    incr = 3;
  if (info_ptr->color_type & PNG_COLOR_MASK_ALPHA)
    incr = 4;

  for (y = 0; y < image->res[1]; y++)
  {
    png_byte *row = row_pointers[image->res[1]-y-1];
    for (x = 0; x < image->res[0]; x++)
    {
      png_byte *ptr = &(row[x*incr]);
      if (incr == 1)
        image->dmap[y * image->res[0] + x] = 1.0 - 0.003921568627 * (gfloat_t)ptr[0]; /* divide by (255) */
      else
        image->dmap[y * image->res[0] + x] = 1.0 - 0.001307189542 * (gfloat_t)(ptr[0] + ptr[1] + ptr[2]); /* divide by (3*255) */
    }
  }

  for (y = 0; y < image->res[1]; y++)
    free (row_pointers[y]);
  free (row_pointers);

  fclose (fp);
}


void
gcode_image_draw (gcode_block_t *block, gcode_block_t *selected)
{
#if GCODE_USE_OPENGL
  gcode_image_t *image;
  gfloat_t xpos[2], ypos[2], coef;
  int x, y, sind;

  if (block->flags & GCODE_FLAGS_SUPPRESS)
    return;

  image = (gcode_image_t *) block->pdata;

  sind = 0;
  if (selected)
    if (block == selected || block->parent == selected)
      sind = 1;

  glBegin (GL_QUADS);

  for (y = 0; y < image->res[1]-1; y++)
  {
    ypos[0] = ((gfloat_t) y) * image->size[1] / (gfloat_t) image->res[1];
    ypos[1] = ((gfloat_t) y+1) * image->size[1] / (gfloat_t) image->res[1];
    for (x = 0; x < image->res[0]-1; x++)
    {
      xpos[0] = ((gfloat_t) x) * image->size[0] / (gfloat_t) image->res[0];
      xpos[1] = ((gfloat_t) x+1) * image->size[0] / (gfloat_t) image->res[0];

      coef = 1.0 - 0.25 * (image->dmap[y*image->res[0] + x] +
                           image->dmap[y*image->res[0] + x+1] +
                           image->dmap[(y+1)*image->res[0] + x+1] +
                           image->dmap[(y+1)*image->res[0] + x]);

      glColor3f (coef * GCODE_OPENGL_COLOR[sind][0], coef * GCODE_OPENGL_COLOR[sind][1], coef * GCODE_OPENGL_COLOR[sind][2]);

      glVertex3f (xpos[0], ypos[0], image->size[2]*image->dmap[y*image->res[0] + x]);
      glVertex3f (xpos[1], ypos[0], image->size[2]*image->dmap[y*image->res[0] + x+1]);
      glVertex3f (xpos[1], ypos[1], image->size[2]*image->dmap[(y+1)*image->res[0] + x+1]);
      glVertex3f (xpos[0], ypos[1], image->size[2]*image->dmap[(y+1)*image->res[0] + x]);
    }
  }

  glEnd ();
#endif
}


void
gcode_image_duplicate (gcode_block_t *block, gcode_block_t **duplicate)
{
  gcode_image_t *image, *duplicate_image;
  int i;

  image = (gcode_image_t *) block->pdata;

  gcode_image_init (block->gcode, duplicate, block->parent);
  (*duplicate)->name = block->name;

  strcpy ((*duplicate)->comment, block->comment);
  (*duplicate)->parent = block->parent;
  (*duplicate)->offset = block->offset;

  duplicate_image = (gcode_image_t *) (*duplicate)->pdata;

  /* Copy resolution and size */
  duplicate_image->res[0] = image->res[0];
  duplicate_image->res[1] = image->res[1];
  duplicate_image->size[0] = image->size[0];
  duplicate_image->size[1] = image->size[1];
  duplicate_image->size[2] = image->size[2];

  /* Copy Depth Map */
  duplicate_image->dmap = (gfloat_t *) malloc (sizeof (gfloat_t) * image->res[0] * image->res[1]);
  for (i = 0; i < image->res[0]*image->res[1]; i++)
    duplicate_image->dmap[i] = image->dmap[i];
}


void
gcode_image_scale (gcode_block_t *block, gfloat_t scale)
{
  gcode_image_t *image;

  image = (gcode_image_t *) block->pdata;

  image->size[0] *= scale;
  image->size[1] *= scale;
  image->size[2] *= scale;
}
