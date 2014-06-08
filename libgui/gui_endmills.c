/*
*  gui_endmills.c
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
#include "gui_endmills.h"
#include <stdio.h>
#include <unistd.h>
#include <expat.h>


void
gui_endmills_init (gui_endmill_list_t *endmill_list)
{
  endmill_list->num = 0;
  endmill_list->endmill = NULL;
}


void
gui_endmills_free (gui_endmill_list_t *endmill_list)
{
  endmill_list->num = 0;
  free (endmill_list->endmill);
  endmill_list->endmill = NULL;
}


static void
start (void *data, const char *el, const char **attr)
{
  gui_endmill_list_t *endmill_list;
  int i;


  endmill_list = (gui_endmill_list_t *) data;
  if (strcmp ("endmill", el))
    return;

  endmill_list->endmill = realloc (endmill_list->endmill, (endmill_list->num+1) * sizeof (gui_endmill_t));
  strcpy (endmill_list->endmill[endmill_list->num].description, "");

  for (i = 0; attr[i]; i+= 2)
  {
    if (!strcmp ("number", attr[i]))
      endmill_list->endmill[endmill_list->num].number = (uint8_t) atoi (attr[i+1]);

    if (!strcmp ("diameter", attr[i]))
      endmill_list->endmill[endmill_list->num].diameter = atof (attr[i+1]);

    if (!strcmp ("unit", attr[i]))
    {
      if (!strcmp ("inch", attr[i+1]))
      {
        endmill_list->endmill[endmill_list->num].unit = GCODE_UNITS_INCH;
      }
      else if (!strcmp ("millimeter", attr[i+1]))
      {
        endmill_list->endmill[endmill_list->num].unit = GCODE_UNITS_MILLIMETER;
      }
    }

    if (!strcmp ("description", attr[i]))
      strcpy (endmill_list->endmill[endmill_list->num].description, attr[i+1]);
  }

  endmill_list->num++;
}


static void
end (void *data, const char *el)
{
}


int
gui_endmills_read (gui_endmill_list_t *endmill_list, gcode_t *gcode)
{
  XML_Parser p = XML_ParserCreate ("US-ASCII");
  FILE *fh;
  int len, i;
  char endmills_file[256], *buffer;

  endmill_list->num = 0;

  if (!p)
  {
    fprintf (stderr, "Couldn't allocate memory for parser\n");
    return (1);
  }

  XML_SetElementHandler (p, start, end);
  XML_SetUserData (p, endmill_list);

  /* Read in endmills.xml file */

  sprintf (endmills_file, "%s%s", SHARE_PREFIX, "endmills.xml");
  fh = fopen (endmills_file, "r");

  /* Try to open from current working directory */
  if (!fh)
  {
    getcwd (endmills_file, 255);
    sprintf (endmills_file, "%s/share/%s", endmills_file, "endmills.xml");
    fh = fopen (endmills_file, "r");
  }

  if (!fh)
  {
    XML_ParserFree (p);
    return (1);
  }


  fseek (fh, 0, SEEK_END);
  len = ftell (fh);
  buffer = (char *) malloc (len);
  fseek (fh, 0, SEEK_SET);
  fread (buffer, len, 1, fh);

  if (XML_Parse (p, buffer, len, 1) == XML_STATUS_ERROR)
  {
    fprintf(stderr, "Parse error at line %d:\n%s\n", (int) XML_GetCurrentLineNumber(p), XML_ErrorString(XML_GetErrorCode(p)));
    return (1);
  }

  fclose (fh);
  free (buffer);
  XML_ParserFree (p);

  /* Alter diameter of end mill to match current unit system */
  for (i = 0; i < endmill_list->num; i++)
  {
    if (endmill_list->endmill[i].unit == GCODE_UNITS_INCH && gcode->units == GCODE_UNITS_MILLIMETER)
      endmill_list->endmill[i].diameter *= GCODE_INCH2MM;

    if (endmill_list->endmill[i].unit == GCODE_UNITS_MILLIMETER && gcode->units == GCODE_UNITS_INCH)
      endmill_list->endmill[i].diameter *= GCODE_MM2INCH;
  }

  return (0);
}
