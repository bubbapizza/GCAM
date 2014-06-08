/*
*  gui_menu_edit.h
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
#ifndef _GUI_MENU_EDIT_H
#define _GUI_MENU_EDIT_H

#include <gtk/gtk.h>

void gui_menu_edit_remove_menuitem_callback (GtkWidget *widget, gpointer data);
void gui_menu_edit_cut_block_menuitem_callback (GtkWidget *widget, gpointer data);
void gui_menu_edit_paste_block_menuitem_callback (GtkWidget *widget, gpointer data);
void gui_menu_edit_move_prev_block_menuitem_callback (GtkWidget *widget, gpointer data);
void gui_menu_edit_move_next_block_menuitem_callback (GtkWidget *widget, gpointer data);
void gui_menu_edit_duplicate_menuitem_callback (GtkWidget *widget, gpointer data);
void gui_menu_edit_scale_menuitem_callback (GtkWidget *widget, gpointer data);
void gui_menu_edit_join_previous_menuitem_callback (GtkWidget *widget, gpointer data);
void gui_menu_edit_join_next_menuitem_callback (GtkWidget *widget, gpointer data);
void gui_menu_edit_fillet_previous_menuitem_callback (GtkWidget *widget, gpointer data);
void gui_menu_edit_fillet_next_menuitem_callback (GtkWidget *widget, gpointer data);
void gui_menu_edit_pattern_menuitem_callback (GtkWidget *widget, gpointer data);
void gui_menu_edit_flip_direction_menuitem_callback (GtkWidget *widget, gpointer data);
void gui_menu_edit_project_settings_menuitem_callback (GtkWidget *widget, gpointer data);

#endif
