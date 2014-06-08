/*
*  gui_menu_util.h
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
#ifndef _GUI_MENU_UTIL_H
#define _GUI_MENU_UTIL_H

#include "gui.h"
#include <gtk/gtk.h>

void base_unit_changed_callback (GtkWidget **widget, gpointer data);
void update_progress (void *gui, gfloat_t progress);
void generic_dialog (void *gui, char *message);
int insert_primitive (gui_t *gui, gcode_block_t *block, gcode_block_t *selected_block, GtkTreeIter *iter, int action);
void destroy (void);
GtkTreeIter refresh_gcode_block_tree_recursion (gui_t *gui, gcode_block_t *block, GtkTreeIter *parent_iter, uint16_t ind, uint8_t single);
void refresh_gcode_block_tree (gui_t *gui);
void get_selected_block (gui_t *gui, gcode_block_t **selected_block, GtkTreeIter *iter);
void set_selected_row_with_iter (gui_t *gui, GtkTreeIter *iter);
void set_selected_row_with_block (gui_t *gui, gcode_block_t *block);
void update_block_tree_order_recursion (gui_t *gui, GtkTreeModel *model, GtkTreeIter *iter);
void update_block_tree_order (gui_t *gui);
void project_menu_options (gui_t *gui, uint8_t state);
void update_menu_options (gui_t *gui, gcode_block_t *selected_block);
void gui_menu_util_modified (gui_t *gui, int mod);


#endif
