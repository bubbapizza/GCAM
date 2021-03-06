/*
*  gui_menu.h
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
#ifndef _GUI_MENU_H
#define _GUI_MENU_H

#include <gtk/gtk.h>
#include "gui_menu_file.h"
#include "gui_menu_edit.h"
#include "gui_menu_insert.h"
#include "gui_menu_assistant.h"
#include "gui_menu_view.h"
#include "gui_menu_help.h"

static GtkActionEntry gui_menu_entries[] = {
  { "FileMenu", 			NULL,			"_File" },
  { "New",				GTK_STOCK_NEW,		"_New Project",			"<control>N",		"Create a new GCAM Project",		G_CALLBACK (gui_menu_file_new_project_menuitem_callback) },
  { "Load",				GTK_STOCK_OPEN,		"_Load Project",		"<control>L",		"Load an existing GCAM Project",	G_CALLBACK (gui_menu_file_load_project_menuitem_callback) },
  { "Save",				GTK_STOCK_SAVE,		"_Save Project",		"<control>S",		"Save current GCAM Project",		G_CALLBACK (gui_menu_file_save_project_menuitem_callback) },
  { "Save As",				GTK_STOCK_SAVE_AS,	"Save Project _As",		"<control>A",		"Save current GCAM Project As",		G_CALLBACK (gui_menu_file_save_project_as_menuitem_callback) },
  { "Close",				GTK_STOCK_CLOSE,	"_Close Project",		"<control>W",		"Close current GCAM Project",		G_CALLBACK (gui_menu_file_close_project_menuitem_callback) },
  { "Export",				GTK_STOCK_CONVERT,	"_Export G-Code",		"<control>E",		"Export Project to G-Code",		G_CALLBACK (gui_menu_file_export_gcode_menuitem_callback) },
  { "Import GCAM",			GTK_STOCK_OPEN,		"_Import GCAM",			"<control>I",		"Import Blocks from GCAM File",		G_CALLBACK (gui_menu_file_import_gcam_menuitem_callback) },
  { "Import Gerber (RS274X)",		GTK_STOCK_OPEN,		"Import _Gerber (RS274X)",	"<control>G",		"Import Gerber (RS274X) to Sketch",	G_CALLBACK (gui_menu_file_import_gerber_menuitem_callback) },
  { "Import Excellon Drill Holes",	GTK_STOCK_OPEN,		"Import Excellon _Drill Holes",	"<control>X",		"Import Excellon Drill Holes",		G_CALLBACK (gui_menu_file_create_drill_holes_from_excellon_menuitem_callback) },
  { "Import SVG Paths",			GTK_STOCK_OPEN,		"Import SVG Paths",		NULL,			"Import SVG Paths",			G_CALLBACK (gui_menu_file_import_svg_menuitem_callback) },
  { "Import STL",			GTK_STOCK_OPEN,		"Import STL",			NULL,			"Import STL",			G_CALLBACK (gui_menu_file_import_stl_menuitem_callback) },
  { "Quit",				GTK_STOCK_QUIT,		"_Quit",			"<control>Q",		"Quit GCAM", 				G_CALLBACK (gui_menu_file_quit_menuitem_callback) },
  { "EditMenu",				NULL,			"_Edit" },
  { "Remove",				GTK_STOCK_DELETE,	"_Remove",			"<control>R",		"Remove",				G_CALLBACK (gui_menu_edit_remove_menuitem_callback) },
  { "Duplicate",			GTK_STOCK_COPY,		"_Duplicate",			"<control>D",		"Duplicate",				G_CALLBACK (gui_menu_edit_duplicate_menuitem_callback) },
  { "Scale",				NULL,			"_Scale",			NULL,			"Scale",				G_CALLBACK (gui_menu_edit_scale_menuitem_callback) },
  { "Join Previous",			GTK_STOCK_GO_BACK,	"Join _Previous",		"<alt><control>P",	"Join Previous",			G_CALLBACK (gui_menu_edit_join_previous_menuitem_callback) },
  { "Join Next",			GTK_STOCK_GO_FORWARD,	"Join _Next",			"<alt><control>N",	"Join Next",				G_CALLBACK (gui_menu_edit_join_next_menuitem_callback) },
  { "Fillet Previous",			NULL,			"Fillet Previous",		NULL,			"Fillet Previous",			G_CALLBACK (gui_menu_edit_fillet_previous_menuitem_callback) },
  { "Fillet Next",			NULL,			"Fillet Next",			NULL,			"Fillet Next",				G_CALLBACK (gui_menu_edit_fillet_next_menuitem_callback) },
  { "Pattern",				NULL,			"P_attern",			NULL,			"Pattern",				G_CALLBACK (gui_menu_edit_pattern_menuitem_callback) },
  { "Flip Direction",			NULL,			"F_lip Direction",		NULL,			"Flip Direction",			G_CALLBACK (gui_menu_edit_flip_direction_menuitem_callback) },
  { "Project Settings",			GTK_STOCK_PROPERTIES,	"_Project Settings",		"<control>P",		"Project Settings",			G_CALLBACK (gui_menu_edit_project_settings_menuitem_callback) },
  { "InsertMenu",			NULL,			"_Insert" },
  { "Tool Change",			NULL,			"Tool Change",			"<control><shift>C",	"Insert Tool Change",			G_CALLBACK (gui_menu_insert_tool_change_menuitem_callback) },
  { "Template",				NULL,			"Template",			"<control><shift>T",	"Insert Template",			G_CALLBACK (gui_menu_insert_template_menuitem_callback) },
  { "Sketch",				NULL,			"Sketch",			"<control><shift>S",	"Insert Sketch", 			G_CALLBACK (gui_menu_insert_sketch_menuitem_callback) },
  { "Arc",				NULL,			"Arc",				"<control><shift>A",	"Insert Arc", 				G_CALLBACK (gui_menu_insert_arc_menuitem_callback) },
  { "Line",				NULL,			"Line",				"<control><shift>L",	"Insert Line",				G_CALLBACK (gui_menu_insert_line_menuitem_callback) },
  { "Bolt Holes",			NULL,			"Bolt Holes",			"<control><shift>B",	"Insert Bolt Holes",			G_CALLBACK (gui_menu_insert_bolt_holes_menuitem_callback) },
  { "Drill Holes",			NULL,			"Drill Holes",			"<control><shift>D",	"Insert Drill Holes",			G_CALLBACK (gui_menu_insert_drill_holes_menuitem_callback) },
  { "Point",				NULL,			"Point",			"<control><shift>P",	"Insert Point",				G_CALLBACK (gui_menu_insert_point_menuitem_callback) },
  { "Image",				NULL,			"Image",			"<control><shift>I",	"Insert Image",				G_CALLBACK (gui_menu_insert_image_menuitem_callback) },
  { "AssistantMenu", 			NULL,			"_Assistant" },
  { "n-gon",				NULL,			"_n-gon",			NULL,			"Create n-gon sketch",			G_CALLBACK (gui_menu_assistant_ngon_menuitem_callback) },
  { "ViewMenu", 			NULL,			"_View" },
  { "Perspective",			NULL,			"_Perspective",			NULL,			"View Perspective",			G_CALLBACK (gui_menu_view_perspective_menuitem_callback) },
  { "Orthographic",			NULL,			"_Orthographic",		NULL,			"View Orthographic",			G_CALLBACK (gui_menu_view_orthographic_menuitem_callback) },
  { "Top",				NULL,			"_Top",				NULL,			"View Top",				G_CALLBACK (gui_menu_view_top_menuitem_callback) },
  { "Left",				NULL,			"_Left",			NULL,			"View Left",				G_CALLBACK (gui_menu_view_left_menuitem_callback) },
  { "Right",				NULL,			"_Right",			NULL,			"View Right",				G_CALLBACK (gui_menu_view_right_menuitem_callback) },
  { "Front",				NULL,			"_Front",			NULL,			"View Front",				G_CALLBACK (gui_menu_view_front_menuitem_callback) },
  { "Back",				NULL,			"_Back",			NULL,			"View Back",				G_CALLBACK (gui_menu_view_back_menuitem_callback) },
  { "RenderMenu", 			NULL,			"_Render" },
  { "FinalPart",			NULL,			"_Final Part",			"<control>F",		"Render Final Part",			G_CALLBACK (gui_menu_view_render_final_part_menuitem_callback) },
  { "HelpMenu", 			NULL,			"_Help" },
  { "Manual",				GTK_STOCK_HELP, 	"_Manual",			NULL,			"GCAM Manual",				G_CALLBACK (gui_menu_help_manual_menuitem_callback) },
  { "About",				0, 			"_About",			NULL,			"About GCAM",				G_CALLBACK (gui_menu_help_about_menuitem_callback) },
};


static const char *gui_menu_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='New'/>"
"      <menuitem action='Load'/>"
"      <menuitem action='Save'/>"
"      <menuitem action='Save As'/>"
"      <menuitem action='Close'/>"
"      <separator/>"
"      <menuitem action='Export'/>"
"      <separator/>"
"      <menuitem action='Import GCAM'/>"
"      <menuitem action='Import Gerber (RS274X)'/>"
"      <menuitem action='Import Excellon Drill Holes'/>"
"      <menuitem action='Import SVG Paths'/>"
"      <menuitem action='Import STL'/>"
"      <separator/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='EditMenu'>"
"      <menuitem action='Remove'/>"
"      <menuitem action='Duplicate'/>"
"      <separator/>"
"      <menuitem action='Scale'/>"
"      <separator/>"
"      <menuitem action='Join Previous'/>"
"      <menuitem action='Join Next'/>"
"      <separator/>"
"      <menuitem action='Fillet Previous'/>"
"      <menuitem action='Fillet Next'/>"
"      <separator/>"
"      <menuitem action='Pattern'/>"
"      <separator/>"
"      <menuitem action='Flip Direction'/>"
"      <separator/>"
"      <menuitem action='Project Settings'/>"
"    </menu>"
"    <menu action='InsertMenu'>"
"      <menuitem action='Tool Change'/>"
"      <separator/>"
"      <menuitem action='Template'/>"
"      <separator/>"
"      <menuitem action='Sketch'/>"
"      <menuitem action='Arc'/>"
"      <menuitem action='Line'/>"
"      <separator/>"
"      <menuitem action='Bolt Holes'/>"
"      <separator/>"
"      <menuitem action='Drill Holes'/>"
"      <menuitem action='Point'/>"
"      <separator/>"
"      <menuitem action='Image'/>"
"    </menu>"
"    <menu action='AssistantMenu'>"
"      <menuitem action='n-gon'/>"
"    </menu>"
"    <menu action='ViewMenu'>"
"      <menuitem action='Perspective'/>"
"      <menuitem action='Orthographic'/>"
"      <separator/>"
"      <menuitem action='Top'/>"
"      <menuitem action='Left'/>"
"      <menuitem action='Right'/>"
"      <menuitem action='Front'/>"
"      <menuitem action='Back'/>"
"    </menu>"
"    <menu action='RenderMenu'>"
"      <menuitem action='FinalPart'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Manual'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

#endif
