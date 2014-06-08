/*
*  gui_menu_help.c
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
#include "gui_menu_help.h"
#include "gui_menu_util.h"
#include "gui.h"

void
gui_menu_help_manual_menuitem_callback (GtkWidget *widget, gpointer data)
{
  gui_t *gui;

  gui = (gui_t *) data;

  generic_dialog (gui, "Visit http://gcam.js.cx and click on Manual.\n");
}


void
gui_menu_help_about_menuitem_callback (GtkWidget *widget, gpointer data)
{
  gui_t *gui;
  char message[256];

  gui = (gui_t *) data;

  sprintf (message, "%s\nBUILD DATE: %s\nBUILD TIME: %s\n\nAUTHOR: Justin Shumaker\nWEBSITE: http://gcam.js.cx\nEMAIL: justin@js.cx", gui->title, __DATE__, __TIME__);
  generic_dialog (gui, message);
}
