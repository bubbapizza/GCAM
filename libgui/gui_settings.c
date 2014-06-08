/*
*  gui_settings.c
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
#include "gui_settings.h"
#include <stdio.h>
#include <unistd.h>
#include <expat.h>


void
gui_settings_init (gui_settings_t *settings)
{
 settings->voxel_resolution = 250;
}


void
gui_settings_free (gui_settings_t *settings)
{
}


static void
start (void *data, const char *el, const char **attr)
{
  gui_settings_t *settings;
  int i;


  settings = (gui_settings_t *) data;
  if (strcmp ("setting", el))
    return;

  for (i = 0; attr[i]; i+= 2)
  {
    if (!strcmp ("voxel_resolution", attr[i]))
      settings->voxel_resolution = atoi (attr[i+1]);
  }
}


static void
end (void *data, const char *el)
{
}


int
gui_settings_read (gui_settings_t *settings)
{
  XML_Parser p = XML_ParserCreate ("US-ASCII");
  FILE *fh;
  int len;
  char settings_file[256], *buffer;


  if (!p)
  {
    fprintf (stderr, "Couldn't allocate memory for parser\n");
    return (1);
  }

  XML_SetElementHandler (p, start, end);
  XML_SetUserData (p, settings);

  /* Read in settings.xml file */

  sprintf (settings_file, "%s%s", SHARE_PREFIX, "settings.xml");
  fh = fopen (settings_file, "r");

  /* Try to open from current working directory */
  if (!fh)
  {
    getcwd (settings_file, 255);
    sprintf (settings_file, "%s/share/%s", settings_file, "settings.xml");
    fh = fopen (settings_file, "r");
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

  return (0);
}
