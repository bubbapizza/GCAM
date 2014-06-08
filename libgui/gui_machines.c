/*
*  gui_machines.c
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
#include "gui_machines.h"
#include <stdio.h>
#include <unistd.h>
#include <expat.h>


void
gui_machines_init (gui_machine_list_t *machine_list)
{
  machine_list->num = 0;
  machine_list->machine = NULL;
}


void
gui_machines_free (gui_machine_list_t *machine_list)
{
  machine_list->num = 0;
  free (machine_list->machine);
  machine_list->machine = NULL;
}


static void
start (void *data, const char *el, const char **attr)
{
  gui_machine_list_t *machine_list;
  int i;


  machine_list = (gui_machine_list_t *) data;

  if (!strcmp ("machine", el))
  {
    machine_list->machine = realloc (machine_list->machine, (machine_list->num+1) * sizeof (gui_machine_t));
    strcpy (machine_list->machine[machine_list->num].name, "");

    machine_list->machine[machine_list->num].travel[0] = 0.0;
    machine_list->machine[machine_list->num].travel[1] = 0.0;
    machine_list->machine[machine_list->num].travel[2] = 0.0;

    machine_list->machine[machine_list->num].maxipm[0] = 0.0;
    machine_list->machine[machine_list->num].maxipm[1] = 0.0;
    machine_list->machine[machine_list->num].maxipm[2] = 0.0;

    machine_list->machine[machine_list->num].options = 0;

    if (!strcmp ("name", attr[0]))
      strcpy (machine_list->machine[machine_list->num].name, attr[1]);
  }

  if (!strcmp("setting", el))
  {
    for (i = 0; attr[i]; i+= 2)
    {
      if (!strcmp ("travel_x", attr[i]))
        machine_list->machine[machine_list->num].travel[0] = atof (attr[i+1]);

      if (!strcmp ("travel_y", attr[i]))
        machine_list->machine[machine_list->num].travel[1] = atof (attr[i+1]);

      if (!strcmp ("travel_z", attr[i]))
        machine_list->machine[machine_list->num].travel[2] = atof (attr[i+1]);

      if (!strcmp ("max_ipm_x", attr[i]))
        machine_list->machine[machine_list->num].maxipm[0] = atof (attr[i+1]);

      if (!strcmp ("max_ipm_y", attr[i]))
        machine_list->machine[machine_list->num].maxipm[1] = atof (attr[i+1]);

      if (!strcmp ("max_ipm_z", attr[i]))
        machine_list->machine[machine_list->num].maxipm[2] = atof (attr[i+1]);

      if (!strcmp ("spindle_control", attr[i]))
      {
        if (!strcmp (attr[i+1], "yes"))
          machine_list->machine[machine_list->num].options |= GCODE_MACHINE_OPTION_SPINDLE_CONTROL;
      }

      if (!strcmp ("tool_change", attr[i]))
      {
        if (!strcmp (attr[i+1], "auto"))
          machine_list->machine[machine_list->num].options |= GCODE_MACHINE_OPTION_AUTOMATIC_TOOL_CHANGE;
      }

      if (!strcmp ("home_switches", attr[i]))
      {
        if (!strcmp (attr[i+1], "yes"))
          machine_list->machine[machine_list->num].options |= GCODE_MACHINE_OPTION_HOME_SWITCHES;
      }

      if (!strcmp ("coolant", attr[i]))
      {
        if (!strcmp (attr[i+1], "yes"))
          machine_list->machine[machine_list->num].options |= GCODE_MACHINE_OPTION_COOLANT;
      }
    }
  }
}


static void
end (void *data, const char *el)
{
  gui_machine_list_t *machine_list;

  machine_list = (gui_machine_list_t *) data;

  if (!strcmp ("machine", el))
    machine_list->num++;
}


int
gui_machines_read (gui_machine_list_t *machine_list)
{
  XML_Parser p = XML_ParserCreate ("US-ASCII");
  FILE *fh;
  int len;
  char machines_file[256], *buffer;


  machine_list->num = 0;

  if (!p)
  {
    fprintf (stderr, "Couldn't allocate memory for parser\n");
    return (1);
  }

  XML_SetElementHandler (p, start, end);
  XML_SetUserData (p, machine_list);

  /* Read in machines.xml file */
  sprintf (machines_file, "%s%s", SHARE_PREFIX, "machines.xml");
  fh = fopen (machines_file, "r");

  /* Try to open from current working directory */
  if (!fh)
  {
    getcwd (machines_file, 255);
    sprintf (machines_file, "%s/share/%s", machines_file, "machines.xml");
    fh = fopen (machines_file, "r");
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
