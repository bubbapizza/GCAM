/*
*  gui_menu_file.c
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
#include "gui_menu_file.h"
#include "gui_menu_util.h"
#include "gui.h"
#include "gui_define.h"
#include "gui_tab.h"


static void
prep_project (gui_t *gui)
{
  gcode_block_t *selected_block;
  GtkTreeIter iter;  

  /* Refresh G-Code Block Tree */
  refresh_gcode_block_tree (gui);

  /* Highlight the Tool block */
  gtk_tree_selection_select_path (gtk_tree_view_get_selection (GTK_TREE_VIEW (gui->gcode_block_treeview)), gtk_tree_path_new_from_string  ("1"));
  get_selected_block (gui, &selected_block, &iter);
  gui_tab_display (gui, selected_block, 0);

  project_menu_options (gui, PROJECT_OPEN);
  update_menu_options (gui, selected_block);

  gui_opengl_context_prep (&gui->opengl);
  gui->opengl.ready = 1;
  gui->first_render = 1;

  gui->opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom = gui->gcode.material_size[0] * 0.66 * (gfloat_t) gui->opengl.context_w / (gfloat_t) gui->opengl.context_h;
  gui->opengl.view[GUI_OPENGL_VIEW_REGULAR].grid = gui->opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom;

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, selected_block);
}


static void
create_project_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gui_t *gui;
  gcode_block_t *block;
  gfloat_t tool_diam, material_size[3], material_origin[3];
  uint8_t units, material_type, tool_num;
  char *sp, tool_name[32], name[32];


  wlist = (GtkWidget **) data;

  gui = (gui_t *) wlist[0];

  gcode_init (&gui->gcode);

  strcpy (name, gtk_entry_get_text (GTK_ENTRY (wlist[2])));

  sp = gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[3]));
  if (!strcmp (sp, "inch"))
  {
    units = GCODE_UNITS_INCH;
  }
  else
  {
    units = GCODE_UNITS_MILLIMETER;
  }

  sp = gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[4]));
  if (!strcmp (sp, "aluminum"))
  {
    material_type = GCODE_MATERIAL_ALUMINUM;
  }
  else if (!strcmp (sp, "foam"))
  {
    material_type = GCODE_MATERIAL_FOAM;
  }
  else if (!strcmp (sp, "plastic"))
  {
    material_type = GCODE_MATERIAL_PLASTIC;
  }
  else if (!strcmp (sp, "steel"))
  {
    material_type = GCODE_MATERIAL_STEEL;
  }
  else if (!strcmp (sp, "wood"))
  {
    material_type = GCODE_MATERIAL_WOOD;
  }
  else
  {
    /* Set default to steel since it's the safest choice */
    material_type = GCODE_MATERIAL_STEEL;
  }

  material_size[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[5]));
  material_size[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[6]));
  material_size[2] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[7]));

  material_origin[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[8]));
  material_origin[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[9]));
  material_origin[2] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[10]));

  gui->gcode.units = units; /* Must be done before gui_endmills_read otherwise unit conversion will fail. */

  sp = gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[11]));
  strcpy (tool_name, &sp[6]);
  {
    gui_endmill_list_t endmill_list;
    int i;

    gui_endmills_init (&endmill_list);
    gui_endmills_read (&endmill_list, &gui->gcode);

    /* Initialize to first end mill */
    tool_diam = endmill_list.endmill[0].diameter;

    for (i = 0; i < endmill_list.num; i++)
    {
      if (!strcmp (tool_name, endmill_list.endmill[i].description))
      {
        tool_diam = endmill_list.endmill[i].diameter;
        tool_num = endmill_list.endmill[i].number;
      }
    }

    gui_endmills_free (&endmill_list);
  }

  /* Machine Name */
  sp = gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[12]));
  {
    gui_machine_list_t machine_list;
    int i;

    gui_machines_init (&machine_list);
    gui_machines_read (&machine_list);

    for (i = 0; i < machine_list.num; i++)
    {
      if (!strcmp (sp, machine_list.machine[i].name))
      {
        strcpy (gui->gcode.machine_name, machine_list.machine[i].name);
        gui->gcode.machine_options = machine_list.machine[i].options;
      }
    }

    gui_machines_free (&machine_list);
  }


  /* Z Traversal Value */
  gui->gcode.ztraverse = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[13]));

  /* Destroy the window along with all of its widgets */
  gtk_widget_destroy (wlist[1]);

  free (wlist);

  /*
  * Setup G-Code Block List and Populate with a Begin, Initial Tool, and End.
  */
  strcpy (gui->gcode.name, name);
  gui->gcode.material_type = material_type;
  gui->gcode.material_size[0] = material_size[0];
  gui->gcode.material_size[1] = material_size[1];
  gui->gcode.material_size[2] = material_size[2];

  gui->gcode.material_origin[0] = material_origin[0];
  gui->gcode.material_origin[1] = material_origin[1];
  gui->gcode.material_origin[2] = material_origin[2];

  /* Populate list with a begin, tool, and end block */
  gcode_begin_init (&gui->gcode, &block, NULL);
  block->make (block);
  gcode_list_insert (&gui->gcode.list, block);

  gcode_end_init (&gui->gcode, &block, NULL);
  block->make (block);
  gcode_list_insert (&gui->gcode.list, block);

  {
    gcode_tool_t *tool;

    gcode_tool_init (&gui->gcode, &block, NULL);

    tool = (gcode_tool_t *) block->pdata;
    tool->diam = tool_diam;
    tool->number = tool_num;
    strcpy (tool->label, tool_name);

    block->make (block);
  }

  gcode_list_insert (&gui->gcode.list, block);

  gui->gcode.progress_callback = update_progress;
  gui->gcode.voxel_res = gui->settings.voxel_resolution;
  gcode_prep (&gui->gcode);
  prep_project (gui);

  gui_menu_util_modified (gui, 1);
}


void
gui_menu_file_new_project_menuitem_callback (GtkWidget *widget, gpointer data)
{
  gui_t *gui;
  GtkWidget *window;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *name_entry;
  GtkWidget *base_unit_combo;
  GtkWidget *material_type_combo;
  GtkWidget *material_sizex_spin;
  GtkWidget *material_sizey_spin;
  GtkWidget *material_sizez_spin;
  GtkWidget *material_originx_spin;
  GtkWidget *material_originy_spin;
  GtkWidget *material_originz_spin;
  GtkWidget *end_mill_combo;
  GtkWidget *machine_combo;
  GtkWidget *ztraverse_spin;
  GtkWidget *create_button;
  GtkWidget **wlist;

  gui = (gui_t *) data;
  wlist = (GtkWidget **) malloc (14 * sizeof (GtkWidget *));

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal (GTK_WINDOW (window), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (gui->window));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
  gtk_window_set_title (GTK_WINDOW (window), "New Project");
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
  
  table = gtk_table_new (8, 4, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_container_add (GTK_CONTAINER (window), table);

  label = gtk_label_new ("Project Name");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);
  name_entry = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (name_entry), 32);
  gtk_table_attach_defaults (GTK_TABLE (table), name_entry, 1, 4, 0, 1);
  gtk_entry_set_text (GTK_ENTRY (name_entry), "Part");

  label = gtk_label_new ("Base Unit");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);
  base_unit_combo = gtk_combo_box_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (base_unit_combo), "inch");
  gtk_combo_box_append_text (GTK_COMBO_BOX (base_unit_combo), "millimeter");
  gtk_combo_box_set_active (GTK_COMBO_BOX (base_unit_combo), 0);
  g_signal_connect_swapped (base_unit_combo, "changed", G_CALLBACK (base_unit_changed_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (table), base_unit_combo, 1, 4, 1, 2);

  label = gtk_label_new ("Material Type");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 2, 3);
  material_type_combo = gtk_combo_box_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (material_type_combo), "aluminum");
  gtk_combo_box_append_text (GTK_COMBO_BOX (material_type_combo), "foam");
  gtk_combo_box_append_text (GTK_COMBO_BOX (material_type_combo), "plastic");
  gtk_combo_box_append_text (GTK_COMBO_BOX (material_type_combo), "steel");
  gtk_combo_box_append_text (GTK_COMBO_BOX (material_type_combo), "wood");
  gtk_combo_box_set_active (GTK_COMBO_BOX (material_type_combo), 0);
  gtk_table_attach_defaults (GTK_TABLE (table), material_type_combo, 1, 4, 2, 3);

  label = gtk_label_new ("Material Size (XYZ)");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 3, 4);

  material_sizex_spin = gtk_spin_button_new_with_range (0.01, MAX_DIM_X, 0.01);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (material_sizex_spin), 3);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (material_sizex_spin), 3);
  gtk_table_attach_defaults (GTK_TABLE (table), material_sizex_spin, 1, 2, 3, 4);

  material_sizey_spin = gtk_spin_button_new_with_range (0.01, MAX_DIM_Y, 0.01);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (material_sizey_spin), 3);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (material_sizey_spin), 2);
  gtk_table_attach_defaults (GTK_TABLE (table), material_sizey_spin, 2, 3, 3, 4);

  material_sizez_spin = gtk_spin_button_new_with_range (0.01, MAX_DIM_Z, 0.01);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (material_sizez_spin), 3);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (material_sizez_spin), 0.25);
  gtk_table_attach_defaults (GTK_TABLE (table), material_sizez_spin, 3, 4, 3, 4);

  label = gtk_label_new ("Material Origin (XYZ)");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 4, 5);

  material_originx_spin = gtk_spin_button_new_with_range (0.0, MAX_DIM_X, 0.01);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (material_originx_spin), 3);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (material_originx_spin), 0.0);
  gtk_table_attach_defaults (GTK_TABLE (table), material_originx_spin, 1, 2, 4, 5);

  material_originy_spin = gtk_spin_button_new_with_range (0.0, MAX_DIM_Y, 0.01);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (material_originy_spin), 3);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (material_originy_spin), 0.0);
  gtk_table_attach_defaults (GTK_TABLE (table), material_originy_spin, 2, 3, 4, 5);

  material_originz_spin = gtk_spin_button_new_with_range (0.0, MAX_DIM_Z, 0.01);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (material_originz_spin), 3);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (material_originz_spin), 0.0);
  gtk_table_attach_defaults (GTK_TABLE (table), material_originz_spin, 3, 4, 4, 5);

  {
    gui_endmill_list_t endmill_list;
    char string[32];
    int i;

    gui_endmills_init (&endmill_list);
    gui_endmills_read (&endmill_list, &gui->gcode);

    label = gtk_label_new ("End Mill");
    gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 5, 6);
    end_mill_combo = gtk_combo_box_new_text ();

    for (i = 0; i < endmill_list.num; i++)
    {
      sprintf (string, "T%.2d - %s", endmill_list.endmill[i].number, endmill_list.endmill[i].description);
      gtk_combo_box_append_text (GTK_COMBO_BOX (end_mill_combo), string);
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX (end_mill_combo), 0);
    gtk_table_attach_defaults (GTK_TABLE (table), end_mill_combo, 1, 4, 5, 6);

    gui_endmills_free (&endmill_list);
  }

  {
    gui_machine_list_t machine_list;
    int i;

    gui_machines_init (&machine_list);
    gui_machines_read (&machine_list);

    label = gtk_label_new ("Machine");
    gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 6, 7);
    machine_combo = gtk_combo_box_new_text ();

    for (i = 0; i < machine_list.num; i++)
      gtk_combo_box_append_text (GTK_COMBO_BOX (machine_combo), machine_list.machine[i].name);

    gtk_combo_box_set_active (GTK_COMBO_BOX (machine_combo), 0);
    gtk_table_attach_defaults (GTK_TABLE (table), machine_combo, 1, 4, 6, 7);

    gui_machines_free (&machine_list);
  }

  label = gtk_label_new ("Traverse(Z)");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 7, 8);
  ztraverse_spin = gtk_spin_button_new_with_range (0.0, GCODE_UNITS ((&gui->gcode), 1.0), 0.01);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (ztraverse_spin), 2);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (ztraverse_spin), 0.05);
  gtk_table_attach_defaults (GTK_TABLE (table), ztraverse_spin, 1, 4, 7, 8);

  wlist[0] = (GtkWidget *) gui;
  wlist[1] = window;
  wlist[2] = name_entry;
  wlist[3] = base_unit_combo;
  wlist[4] = material_type_combo;
  wlist[5] = material_sizex_spin;
  wlist[6] = material_sizey_spin;
  wlist[7] = material_sizez_spin;
  wlist[8] = material_originx_spin;
  wlist[9] = material_originy_spin;
  wlist[10] = material_originz_spin;
  wlist[11] = end_mill_combo;
  wlist[12] = machine_combo;
  wlist[13] = ztraverse_spin;

  create_button = gtk_button_new_with_label ("Create Project");
  gtk_table_attach_defaults (GTK_TABLE (table), create_button, 1, 4, 8, 9);
  g_signal_connect (G_OBJECT (create_button), "clicked", G_CALLBACK (create_project_callback), wlist);
  gtk_widget_show_all (window);
}


void
gui_menu_file_load_project_menuitem_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog;
  GtkFileFilter *filter;
  gui_t *gui;

  gui = (gui_t *) data;

  dialog = gtk_file_chooser_dialog_new ("Load GCAM Project", GTK_WINDOW (gui->window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "*.gcam");
  gtk_file_filter_add_pattern (filter, "*.gcam");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  if (gui->current_folder[0])
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), gui->current_folder);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *filename;

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    if (gcode_load (&gui->gcode, filename))
    {
      generic_dialog (gui, "Invalid GCAM File");
    }
    else
    {
      gui->gcode.progress_callback = update_progress;
      gui->gcode.voxel_res = gui->settings.voxel_resolution;
      gcode_prep (&gui->gcode);

      strcpy (gui->save_filename, filename);

      gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Save"), 1);

      prep_project (gui);

      strcpy (gui->current_folder, gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dialog)));
    }

    g_free (filename);
  }

  gui_menu_util_modified (gui, 0);

  gtk_widget_destroy (dialog);
}


void
gui_menu_file_save_project_menuitem_callback (GtkWidget *widget, gpointer data)
{
  gui_t *gui;

  gui = (gui_t *) data;

  gcode_save (&gui->gcode, gui->save_filename);

  gui_menu_util_modified (gui, 0);
}


void
gui_menu_file_save_project_as_menuitem_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog;
  GtkFileFilter *filter;
  gui_t *gui;
  char proposed_filename[64];

  gui = (gui_t *) data;

  dialog = gtk_file_chooser_dialog_new ("Save GCAM Project As", GTK_WINDOW (gui->window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);

  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

  sprintf (proposed_filename, "%s.gcam", gui->gcode.name);
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), proposed_filename);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "*.gcam");
  gtk_file_filter_add_pattern (filter, "*.gcam");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  if (gui->current_folder[0])
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), gui->current_folder);


  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *filename;

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    strcpy (gui->save_filename, filename);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Save"), 1);
    gcode_save (&gui->gcode, filename);

    gui_menu_util_modified (gui, 0);

    g_free (filename);

    strcpy (gui->current_folder, gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dialog)));
  }

  gtk_widget_destroy (dialog);
}


static void
close_callback (GtkDialog *dialog, gint arg1, gpointer ptr)
{
  gui_t *gui;

  gui = (gui_t *) ptr;

  gtk_widget_destroy (GTK_WIDGET (dialog));

  if (arg1 == GTK_RESPONSE_YES)
  {
    if (gui->save_filename[0])
    {
      gcode_save (&gui->gcode, gui->save_filename);
    }
    else
    {
      gui_menu_file_save_project_as_menuitem_callback (NULL, gui);
    }
  }

  gcode_free (&gui->gcode);

  /* Refresh G-Code Block Tree */
  refresh_gcode_block_tree (gui);

  /* Context */
  gui->opengl.ready = 0;
  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, NULL);

  gui_tab_display (gui, NULL, 0);

  project_menu_options (gui, PROJECT_CLOSED);
}


void
gui_menu_file_close_project_menuitem_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog;
  gui_t *gui;

  gui = (gui_t *) data;

  if (gui->project_state == PROJECT_CLOSED)
  {
    destroy ();
    return;
  }

  if (!gui->modified)
  {
    gcode_free (&gui->gcode);

    /* Refresh G-Code Block Tree */
    refresh_gcode_block_tree (gui);

    /* Context */
    gui->opengl.ready = 0;
    gui->opengl.rebuild_view_display_list = 1;
    gui_opengl_context_redraw (&gui->opengl, NULL);

    gui_tab_display (gui, NULL, 0);

    project_menu_options (gui, PROJECT_CLOSED);
    return;
  }

  dialog = gtk_message_dialog_new (GTK_WINDOW (gui->window),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_QUESTION,
                                  GTK_BUTTONS_YES_NO,
                                  "Save before closing?");
  gtk_window_set_title (GTK_WINDOW (dialog), "Close Project");

  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (gui->window));
  g_signal_connect (dialog, "response", G_CALLBACK (close_callback), gui);

  gtk_widget_show (dialog);
}


static void
export_gcode_file_selector (GtkWidget *widget, gpointer ptr)
{
  GtkWidget *dialog;
  GtkWidget **wlist;
  GtkFileFilter *filter;
  gui_t *gui;
  char proposed_filename[64];
  char *format;

  wlist = (GtkWidget **) ptr;

  gui = (gui_t *) wlist[0];

  format = gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[2]));
  gui->gcode.project_number = (uint32_t) gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[3]));

  if (!strcmp (format, "EMC"))
  {
    gui->gcode.driver = GCODE_DRIVER_EMC;
  }   
  else if (!strcmp (format, "TurboCNC"))
  {
    gui->gcode.driver = GCODE_DRIVER_TURBOCNC;
  }
  else if (!strcmp (format, "Haas"))
  {
    gui->gcode.driver = GCODE_DRIVER_HAAS;
  }

  gtk_widget_destroy (wlist[1]);
  free (wlist);

  dialog = gtk_file_chooser_dialog_new ("Export G-Code", GTK_WINDOW (gui->window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);

  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

  sprintf (proposed_filename, "%s.nc", gui->gcode.name);
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), proposed_filename);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "*.nc");
  gtk_file_filter_add_pattern (filter, "*.nc");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  if (gui->current_folder[0])
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), gui->current_folder);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *filename;

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    gcode_export (&gui->gcode, filename);

    g_free (filename);
  }

  gtk_widget_destroy (dialog);
}


static void
export_format_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gui_t *gui;
  char *format;

  wlist = (GtkWidget **) data;

  gui = (gui_t *) wlist[0];

  format = gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[2]));

  if (!strcmp (format, "EMC"))
  {
    gtk_widget_set_sensitive (wlist[3], 0);
  }   
  else if (!strcmp (format, "TurboCNC"))
  {
    gtk_widget_set_sensitive (wlist[3], 0);
  }
  else if (!strcmp (format, "Haas"))
  {
    gtk_widget_set_sensitive (wlist[3], 1);
  }
}


void
gui_menu_file_export_gcode_menuitem_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *window;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *export_format_combo;
  GtkWidget *project_number_spin;
  GtkWidget *export_button;
  GtkWidget **wlist;
  gui_t *gui;

  gui = (gui_t *) data;

  wlist = (GtkWidget **) malloc (4 * sizeof (GtkWidget *));

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal (GTK_WINDOW (window), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (gui->window));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
  gtk_window_set_title (GTK_WINDOW (window), "Export Format");
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
  
  table = gtk_table_new (3, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_container_add (GTK_CONTAINER (window), table);

  label = gtk_label_new ("Format");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);
  export_format_combo = gtk_combo_box_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (export_format_combo), "EMC");
  gtk_combo_box_append_text (GTK_COMBO_BOX (export_format_combo), "TurboCNC");
  gtk_combo_box_append_text (GTK_COMBO_BOX (export_format_combo), "Haas");
  gtk_combo_box_set_active (GTK_COMBO_BOX (export_format_combo), 0);
  g_signal_connect (export_format_combo, "changed", G_CALLBACK (export_format_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (table), export_format_combo, 1, 2, 0, 1);

  label = gtk_label_new ("Project Number");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);
  project_number_spin = gtk_spin_button_new_with_range (0.0, 99999.0, 1.0);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (project_number_spin), 0);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (project_number_spin), 100.0);
  gtk_table_attach_defaults (GTK_TABLE (table), project_number_spin, 1, 2, 1, 2);

  wlist[0] = (GtkWidget *) gui;
  wlist[1] = window;
  wlist[2] = export_format_combo;
  wlist[3] = project_number_spin;

  gtk_widget_set_sensitive (wlist[3], 0);

  export_button = gtk_button_new_with_label ("Export");
  gtk_table_attach_defaults (GTK_TABLE (table), export_button, 0, 2, 2, 3);
  g_signal_connect (G_OBJECT (export_button), "clicked", G_CALLBACK (export_gcode_file_selector), wlist);
  gtk_widget_show_all (window);
}


static void
cancel_import_gcam_callback (GtkWidget *widget, gpointer ptr)
{
  GtkWidget **wlist;

  wlist = (GtkWidget **) ptr;

  gtk_widget_destroy (GTK_WIDGET (wlist[1]));

  gcode_free ((gcode_t *) wlist[3]);

  free (wlist);
}


static void
import_gcam_block_callback (GtkWidget *widget, gpointer ptr)
{
  GtkWidget **wlist;
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  GValue value = { 0, };
  gui_t *gui;
  gcode_block_t *selected_block, *duplicate_block;


  wlist = (GtkWidget **) ptr;

  gui = (gui_t *) wlist[0];

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (wlist[2]));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (wlist[2]));
  selected_block = NULL;

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
  {
    /* get the pointer from the tree */
    gtk_tree_model_get_value (model, &iter, 2, &value);
    selected_block = (gcode_block_t *) g_value_get_pointer (&value);

    g_value_unset (&value);
  }

  selected_block->duplicate (selected_block, &duplicate_block);

  get_selected_block (gui, &selected_block, &iter);
  insert_primitive (gui, duplicate_block, selected_block, &iter, GUI_INSERT_AFTER);

  gtk_widget_destroy (GTK_WIDGET (wlist[1]));

  gcode_free ((gcode_t *) wlist[3]);
  free (wlist[3]);

  free (wlist);

  gui_menu_util_modified (gui, 1);
}


static void
import_gcam_callback (gui_t *gui, char *selected_filename)
{
  gcode_block_t *iter_block;
  gcode_t *imported;
  GtkWidget *window;
  GtkWidget *table;
  GtkWidget *sw;
  GtkListStore *store;
  GtkTreeIter iter;
  GtkTreeModel *model;
  GtkWidget *treeview;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkWidget *cancel_button;
  GtkWidget *import_button;
  GtkWidget **wlist;
  int first_block;


  imported = (gcode_t *) malloc (sizeof (gcode_t));
  gcode_load (imported, selected_filename);
  gcode_prep (imported);


  /* Create window list of blocks to choose from */
  wlist = (GtkWidget **) malloc (4 * sizeof (GtkWidget *));

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal (GTK_WINDOW (window), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (gui->window));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
  gtk_window_set_title (GTK_WINDOW (window), "Select Block to Import");
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
  
  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_container_add (GTK_CONTAINER (window), table);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_table_attach_defaults (GTK_TABLE (table), sw, 0, 2, 0, 1);
  gtk_widget_set_size_request (GTK_WIDGET (sw), 400, 240);


  /* create list store */
  store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);

  /* create tree model */
  model = GTK_TREE_MODEL (store);

  treeview = gtk_tree_view_new_with_model (model);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
  g_object_unref (model);
  gtk_container_add (GTK_CONTAINER (sw), treeview);

  iter_block = imported->list;
  first_block = 1;
  while (iter_block)
  {
    if (iter_block->type != GCODE_TYPE_BEGIN && iter_block->type != GCODE_TYPE_END && iter_block->type != GCODE_TYPE_TOOL)
    {
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter, 0, GCODE_TYPE_STRING[iter_block->type], 1, iter_block->comment, 2, iter_block, -1);

      if (first_block)
      {
        gtk_tree_selection_select_iter (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview)), &iter);
        first_block = 0;
      }
    }

    iter_block = iter_block->next;
  }

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Type", renderer, "text", 0, NULL);
  gtk_tree_view_column_set_sort_column_id (column, 0);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Comment", renderer, "text", 1, NULL);
  gtk_tree_view_column_set_sort_column_id (column, 1);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  wlist[0] = (GtkWidget *) gui;
  wlist[1] = window;
  wlist[2] = treeview;
  wlist[3] = (GtkWidget *) imported;

  cancel_button = gtk_button_new_with_label ("Cancel");
  gtk_table_attach_defaults (GTK_TABLE (table), cancel_button, 0, 1, 1, 2);
  g_signal_connect (G_OBJECT (cancel_button), "clicked", G_CALLBACK (cancel_import_gcam_callback), wlist);

  import_button = gtk_button_new_with_label ("Import");
  gtk_table_attach_defaults (GTK_TABLE (table), import_button, 1, 2, 1, 2);
  g_signal_connect (G_OBJECT (import_button), "clicked", G_CALLBACK (import_gcam_block_callback), wlist);


/*  gtk_widget_set_size_request (GTK_WIDGET (window), 400, 300); */
  gtk_widget_show_all (window);
}


void
gui_menu_file_import_gcam_menuitem_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog;
  GtkFileFilter *filter;
  gui_t *gui;
  char *filename;

  gui = (gui_t *) data;

  dialog = gtk_file_chooser_dialog_new ("Import Blocks from GCAM File", GTK_WINDOW (gui->window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "*.gcam");
  gtk_file_filter_add_pattern (filter, "*.gcam");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  if (gui->current_folder[0])
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), gui->current_folder);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    gtk_widget_destroy (dialog);

    if (gcode_load (&gui->gcode, filename))
    {
      generic_dialog (gui, "Invalid GCAM File");
    }
    else
    {
      import_gcam_callback (gui, filename);
    }

    g_free (filename);
  }
  else
  {
    gtk_widget_destroy (dialog);
  }
}




static void
gerber_on_assistant_close_cancel (GtkWidget *widget, gpointer data)
{
  gtk_widget_destroy (widget);
}


static void
gerber_on_assistant_prepare (GtkWidget *widget, GtkWidget *page, gpointer data)
{
  gint current_page, n_pages;
  gchar *title;

  current_page = gtk_assistant_get_current_page (GTK_ASSISTANT (widget));
  n_pages = gtk_assistant_get_n_pages (GTK_ASSISTANT (widget));

  title = g_strdup_printf ("Import RS274X (Gerber) Step (%d of %d)", current_page + 1, n_pages);
  gtk_window_set_title (GTK_WINDOW (widget), title);
  g_free (title);
}


static void
gerber_on_assistant_apply (GtkWidget *widget, gpointer data)
{
  gcode_block_t *template_block, *tool_block, *sketch_block, *selected_block;
  gcode_tool_t *tool;
  GtkTreeIter iter;
  gui_t *gui;
  gfloat_t offset, initial, step, max, tool_diam, material_depth;
  uint8_t tool_num;
  gcode_vec2d_t aabb_min, aabb_max;
  char *emd, filename[256];


  gui = (gui_t *) data;

  strcpy (filename, gtk_entry_get_text (GTK_ENTRY (((GtkWidget **) gui->generic_ptr)[0])));

  /* Perform a Test Run to see if File passes without errors */
  gcode_sketch_init (&gui->gcode, &sketch_block, template_block);
  if (gcode_gerber_import (sketch_block, filename, 0))
  {
    generic_dialog (gui, "Error: Missing Apertures before Traces.\nThis doesn't appear to be a valid RS274X file format.");
    sketch_block->free (&sketch_block);
    return;
  }


  emd = gtk_combo_box_get_active_text (GTK_COMBO_BOX (((GtkWidget **) gui->generic_ptr)[1]));
  {
    gui_endmill_list_t endmill_list;
    int i;

    gui_endmills_init (&endmill_list);
    gui_endmills_read (&endmill_list, &gui->gcode);

    /* Initialize to first end mill */
    tool_diam = endmill_list.endmill[0].diameter;
    tool_num = endmill_list.endmill[0].number;

    for (i = 0; i < endmill_list.num; i++)
    {
      if (!strcmp (emd, endmill_list.endmill[i].description))
      {
        tool_diam = endmill_list.endmill[i].diameter;
        tool_num = endmill_list.endmill[i].number;
      }
    }

    gui_endmills_free (&endmill_list);
  }

  get_selected_block (gui, &selected_block, &iter);
  gcode_template_init (&gui->gcode, &template_block, NULL);
  /* Set the comment as the file being opened */
  if (strrchr (filename, '/'))
  {
    strcpy (template_block->comment, strrchr (filename, '/')+1);
  }
  else if (strrchr (filename, '\\'))
  {
    strcpy (template_block->comment, strrchr (filename, '\\')+1);
  }
  else
  {
    strcpy (template_block->comment, filename);
  }
  insert_primitive (gui, template_block, selected_block, &iter, GUI_INSERT_AFTER);

  get_selected_block (gui, &selected_block, &iter);
  gcode_tool_init (&gui->gcode, &tool_block, template_block);
  insert_primitive (gui, tool_block, template_block, &iter, GUI_INSERT_INTO);

  tool = (gcode_tool_t *) tool_block->pdata;
  tool->feed = gtk_spin_button_get_value (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[2]));
  tool->diam = tool_diam;
  tool->number = tool_num;
  strcpy (tool->label, emd);

  /*
  * Temporarily change the material depth to the cutting depth so that
  * the sketch extrusions are the proper depth.
  */
  material_depth = gui->gcode.material_size[2];
  gui->gcode.material_size[2] = -gtk_spin_button_get_value (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[3]));

  get_selected_block (gui, &selected_block, &iter);
  gcode_sketch_init (&gui->gcode, &sketch_block, template_block);
  gcode_gerber_import (sketch_block, filename, tool_diam);

  insert_primitive (gui, sketch_block, selected_block, &iter, GUI_INSERT_AFTER);

  initial = gtk_spin_button_get_value (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[4]));
  step = gtk_spin_button_get_value (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[5]));
  max = gtk_spin_button_get_value (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[6]));
  for (offset = initial; offset < max; offset += step)
  {
    get_selected_block (gui, &selected_block, &iter);
    gcode_sketch_init (&gui->gcode, &sketch_block, template_block);
    gcode_gerber_import (sketch_block, filename, offset + tool_diam);
    insert_primitive (gui, sketch_block, selected_block, &iter, GUI_INSERT_INTO);
  }


  /* Final pass */
  if (max - offset > GCODE_PRECISION)
  {
    get_selected_block (gui, &selected_block, &iter);
    gcode_sketch_init (&gui->gcode, &sketch_block, template_block);
    gcode_gerber_import (sketch_block, filename, max + tool_diam);
    insert_primitive (gui, sketch_block, selected_block, &iter, GUI_INSERT_INTO);

    /* Get the bounding box for the sketch */
  }

  sketch_block->aabb (sketch_block, aabb_min, aabb_max);
  ((gcode_template_t *)template_block->pdata)->position[0] -= aabb_min[0];
  ((gcode_template_t *)template_block->pdata)->position[1] -= aabb_min[1];

  if (aabb_max[0] - aabb_min[0] > gui->gcode.material_size[0] || aabb_max[1] - aabb_min[1] > gui->gcode.material_size[1])
  {
    gui->gcode.material_size[0] = aabb_max[0] - aabb_min[0];
    gui->gcode.material_size[1] = aabb_max[1] - aabb_min[1];
  }

  gui_opengl_build_gridxy_display_list (&gui->opengl);
  gui_opengl_build_gridxz_display_list (&gui->opengl);

  gcode_prep (&gui->gcode);

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, selected_block);

  /* Change material depth back to its original value */
  gui->gcode.material_size[2] = material_depth;

  free (gui->generic_ptr);
}


static void
gerber_on_entry_changed (GtkWidget *widget, gpointer data)
{
  GtkAssistant *assistant = GTK_ASSISTANT (data);
  GtkWidget *current_page;
  gint page_number;
  const gchar *text;

  page_number = gtk_assistant_get_current_page (assistant);
  current_page = gtk_assistant_get_nth_page (assistant, page_number);
  text = gtk_entry_get_text (GTK_ENTRY (widget));

  if (text && *text)
  {
    gtk_assistant_set_page_complete (assistant, current_page, TRUE);
  }
  else
  {
    gtk_assistant_set_page_complete (assistant, current_page, FALSE);
  }
}


static void
gerber_browse_file_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog;
  GtkFileFilter *filter;
  gui_t *gui;

  gui = (gui_t *) data;

  dialog = gtk_file_chooser_dialog_new ("Select Gerber File", GTK_WINDOW (gui->window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "RS274X (*.gbr,*.gbx,*.art,*.pho)");
  gtk_file_filter_add_pattern (filter, "*.gbr");
  gtk_file_filter_add_pattern (filter, "*.gbx");
  gtk_file_filter_add_pattern (filter, "*.art");
  gtk_file_filter_add_pattern (filter, "*.pho");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "All (*.*)");
  gtk_file_filter_add_pattern (filter, "*.*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  if (gui->current_folder[0])
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), gui->current_folder);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    char *filename;

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    gtk_entry_set_text (GTK_ENTRY (((GtkWidget **) gui->generic_ptr)[0]), filename);

    g_free (filename);
  }

  gtk_widget_destroy (dialog);
}


static void
gerber_create_page1 (GtkWidget *assistant, gui_t *gui)
{
  GtkWidget *hbox, *bbox, *label, *browse_button;
  GdkPixbuf *pixbuf;

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 12);

  label = gtk_label_new ("File:");
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  ((GtkWidget **) gui->generic_ptr)[0] = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox), ((GtkWidget **) gui->generic_ptr)[0], TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (((GtkWidget **) gui->generic_ptr)[0]), "changed", G_CALLBACK (gerber_on_entry_changed), assistant);

  bbox = gtk_hbutton_box_new ();
  gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_SPREAD);
  gtk_box_set_spacing (GTK_BOX (bbox), 0);
  gtk_container_set_border_width (GTK_CONTAINER (bbox), 0);
  gtk_box_pack_start (GTK_BOX (hbox), bbox, FALSE, FALSE, 0);

  browse_button = gtk_button_new_from_stock (GTK_STOCK_OPEN);
  g_signal_connect (G_OBJECT (browse_button), "clicked", G_CALLBACK (gerber_browse_file_callback), gui);
  gtk_container_add (GTK_CONTAINER (bbox), browse_button);

  gtk_widget_show_all (hbox);
  gtk_assistant_append_page (GTK_ASSISTANT (assistant), hbox);
  gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), hbox, "Choose RS274X (Gerber) File");
  gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), hbox, GTK_ASSISTANT_PAGE_INTRO);

  pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), hbox, pixbuf);
  g_object_unref (pixbuf);
}


static void
gerber_create_page2 (GtkWidget *assistant, gui_t *gui)
{
  GtkWidget *table, *label;
  GdkPixbuf *pixbuf;

  table = gtk_table_new (4, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);

  label = gtk_label_new ("The end mill diameter affects the RS274X to G-Code conversion process.\nChanging this diameter after the conversion will not update the sketches.\nIt is recommended that a \"V\" type end mill with a fine point be used.");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 2, 0, 1);

  {
    gui_endmill_list_t endmill_list;
    int i;

    gui_endmills_init (&endmill_list);
    gui_endmills_read (&endmill_list, &gui->gcode);

    label = gtk_label_new ("End Mill");
    gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);
    ((GtkWidget **) gui->generic_ptr)[1] = gtk_combo_box_new_text ();

    for (i = 0; i < endmill_list.num; i++)
      gtk_combo_box_append_text (GTK_COMBO_BOX (((GtkWidget **) gui->generic_ptr)[1]), endmill_list.endmill[i].description);

    gtk_combo_box_set_active (GTK_COMBO_BOX (((GtkWidget **) gui->generic_ptr)[1]), 0);
    gtk_table_attach_defaults (GTK_TABLE (table), ((GtkWidget **) gui->generic_ptr)[1], 1, 2, 1, 2);

    gui_endmills_free (&endmill_list);
  }

  label = gtk_label_new ("Feed Rate");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 2, 3);
  ((GtkWidget **) gui->generic_ptr)[2] = gtk_spin_button_new_with_range (GCODE_UNITS ((&gui->gcode), 0.01), GCODE_UNITS ((&gui->gcode), 30.0), GCODE_UNITS ((&gui->gcode), 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[2]), 2);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[2]), 10.0);
  gtk_table_attach_defaults (GTK_TABLE (table), ((GtkWidget **) gui->generic_ptr)[2], 1, 2, 2, 3);

  label = gtk_label_new ("Cutting Depth");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 3, 4);
  ((GtkWidget **) gui->generic_ptr)[3] = gtk_spin_button_new_with_range (-GCODE_UNITS ((&gui->gcode), gui->gcode.material_size[2]), 0.0, GCODE_UNITS ((&gui->gcode), 0.0001));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[3]), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[3]), GCODE_UNITS ((&gui->gcode), -0.0045));
  gtk_table_attach_defaults (GTK_TABLE (table), ((GtkWidget **) gui->generic_ptr)[3], 1, 2, 3, 4);


  gtk_widget_show_all (table);
  gtk_assistant_append_page (GTK_ASSISTANT (assistant), table);
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), table, TRUE);
  gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), table, "Etching Parameters");

  pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), table, pixbuf);
  g_object_unref (pixbuf);
}


static void
gerber_create_page3 (GtkWidget *assistant, gui_t *gui)
{
  GtkWidget *table, *label;
  GdkPixbuf *pixbuf;

  table = gtk_table_new (3, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);

  label = gtk_label_new ("The isolation values control the measure of separation between traces\nand surrounding copper.  The Step value should typically be less than the\ndiameter of the end mill.  A sketch for each isolation pass will be generated.");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 2, 0, 1);

  label = gtk_label_new ("Initial");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);
  ((GtkWidget **) gui->generic_ptr)[4] = gtk_spin_button_new_with_range (0.0001, 1.0, 0.0001);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[4]), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[4]), 0.0002);
  gtk_table_attach_defaults (GTK_TABLE (table), ((GtkWidget **) gui->generic_ptr)[4], 1, 2, 1, 2);

  label = gtk_label_new ("Step");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 2, 3);
  ((GtkWidget **) gui->generic_ptr)[5] = gtk_spin_button_new_with_range (0.0001, 1.0, 0.0001);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[5]), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[5]), 0.004);
  gtk_table_attach_defaults (GTK_TABLE (table), ((GtkWidget **) gui->generic_ptr)[5], 1, 2, 2, 3);

  label = gtk_label_new ("Max");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 3, 4);
  ((GtkWidget **) gui->generic_ptr)[6] = gtk_spin_button_new_with_range (0.0001, 1.0, 0.0001);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[6]), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (((GtkWidget **) gui->generic_ptr)[6]), 0.02);
  gtk_table_attach_defaults (GTK_TABLE (table), ((GtkWidget **) gui->generic_ptr)[6], 1, 2, 3, 4);

  gtk_widget_show_all (table);
  gtk_assistant_append_page (GTK_ASSISTANT (assistant), table);
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), table, TRUE);
  gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), table, "Isolation Details");
  gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), table, GTK_ASSISTANT_PAGE_CONFIRM);

  pixbuf = gtk_widget_render_icon (assistant, GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG, NULL);
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), table, pixbuf);
  g_object_unref (pixbuf);
}


void
gui_menu_file_import_gerber_menuitem_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *assistant;
  gui_t *gui;

  gui = (gui_t *) data;

  assistant = gtk_assistant_new ();
  gtk_window_set_default_size (GTK_WINDOW (assistant), -1, -1);
  gtk_window_set_screen (GTK_WINDOW (assistant), gtk_widget_get_screen (gui->window));
  gtk_window_set_transient_for (GTK_WINDOW (assistant), GTK_WINDOW (gui->window));

  /* Setup Global Widgets */
  gui->generic_ptr = malloc (7 * sizeof (GtkWidget *));

  gerber_create_page1 (assistant, gui);
  gerber_create_page2 (assistant, gui);
  gerber_create_page3 (assistant, gui);
  g_signal_connect (G_OBJECT (assistant), "cancel", G_CALLBACK (gerber_on_assistant_close_cancel), &assistant);
  g_signal_connect (G_OBJECT (assistant), "close", G_CALLBACK (gerber_on_assistant_close_cancel), &assistant);
  g_signal_connect (G_OBJECT (assistant), "prepare", G_CALLBACK (gerber_on_assistant_prepare), NULL);
  g_signal_connect (G_OBJECT (assistant), "apply", G_CALLBACK (gerber_on_assistant_apply), gui);

  gtk_widget_show (assistant);
}


void
gui_menu_file_create_drill_holes_from_excellon_menuitem_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog;
  GtkFileFilter *filter;
  gui_t *gui;

  gui = (gui_t *) data;

  dialog = gtk_file_chooser_dialog_new ("Load Excellon Drill File", GTK_WINDOW (gui->window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "Excellon (*.nc,*.drl)");
  gtk_file_filter_add_pattern (filter, "*.nc");
  gtk_file_filter_add_pattern (filter, "*.drl");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "All (*.*)");
  gtk_file_filter_add_pattern (filter, "*.*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  if (gui->current_folder[0])
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), gui->current_folder);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    gcode_block_t **block_array, *selected_block;
    GtkTreeIter iter;
    int block_num, i;
    char *filename;

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    get_selected_block (gui, &selected_block, &iter);

    block_array = NULL;

    gcode_excellon_import (&gui->gcode, &block_array, &block_num, filename);

    /*
    * Insert the blocks in reverse order so that the selected block
    * does not have to change for each new insertion.
    */
    for (i = block_num-1; i >= 0; i--)
    {
      if (selected_block->type == GCODE_TYPE_TEMPLATE)
        block_array[i]->parent = selected_block;

      insert_primitive (gui, block_array[i], selected_block, &iter, GUI_INSERT_AFTER);
    }

    g_free (filename);
  }

  gtk_widget_destroy (dialog);
}


void
gui_menu_file_import_svg_menuitem_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog;
  GtkFileFilter *filter;
  gui_t *gui;

  gui = (gui_t *) data;

  dialog = gtk_file_chooser_dialog_new ("Import SVG", GTK_WINDOW (gui->window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "SVG (*.svg)");
  gtk_file_filter_add_pattern (filter, "*.svg");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "All (*.*)");
  gtk_file_filter_add_pattern (filter, "*.*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  if (gui->current_folder[0])
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), gui->current_folder);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    gcode_block_t *selected_block, *sketch_block;
    GtkTreeIter iter;
    char *filename;

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    get_selected_block (gui, &selected_block, &iter);

    /* Create sketch */
    gcode_sketch_init (&gui->gcode, &sketch_block, selected_block);
   
    gcode_svg_import (&gui->gcode, sketch_block, filename);

    insert_primitive (gui, sketch_block, selected_block, &iter, GUI_INSERT_AFTER);

    g_free (filename);

    gui_opengl_build_gridxy_display_list (&gui->opengl);
    gui_opengl_build_gridxz_display_list (&gui->opengl);
   
    gcode_prep (&gui->gcode);
   
    gui->opengl.rebuild_view_display_list = 1;
    gui_opengl_context_redraw (&gui->opengl, selected_block);
  }

  gtk_widget_destroy (dialog);
}


void
gui_menu_file_import_stl_menuitem_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog;
  GtkFileFilter *filter;
  gui_t *gui;

  gui = (gui_t *) data;

  dialog = gtk_file_chooser_dialog_new ("Import STL", GTK_WINDOW (gui->window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "STL (*.stl)");
  gtk_file_filter_add_pattern (filter, "*.stl");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "All (*.*)");
  gtk_file_filter_add_pattern (filter, "*.*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  if (gui->current_folder[0])
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), gui->current_folder);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
    gcode_block_t *selected_block, *stl_block;
    GtkTreeIter iter;
    char *filename;

    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

    get_selected_block (gui, &selected_block, &iter);

    gcode_stl_init (&gui->gcode, &stl_block, selected_block);
    gcode_stl_import (stl_block, filename);

    insert_primitive (gui, stl_block, selected_block, &iter, GUI_INSERT_AFTER);

    g_free (filename);

    gui_opengl_build_gridxy_display_list (&gui->opengl);
    gui_opengl_build_gridxz_display_list (&gui->opengl);
   
    gcode_prep (&gui->gcode);

    gui->opengl.rebuild_view_display_list = 1;
    gui_opengl_context_redraw (&gui->opengl, selected_block);
  }

  gtk_widget_destroy (dialog);
}


static void
quit_callback (GtkDialog *dialog, gint arg1, gpointer ptr)
{
  gui_t *gui;

  gui = (gui_t *) ptr;

  gtk_widget_destroy (GTK_WIDGET (dialog));

  if (arg1 == GTK_RESPONSE_YES)
    gcode_save (&gui->gcode, gui->save_filename);

  destroy ();
}


void
gui_menu_file_quit_menuitem_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog;
  gui_t *gui;

  gui = (gui_t *) data;

  if (gui->project_state == PROJECT_CLOSED)
  {
    destroy ();
    return;
  }

  if (!gui->modified)
  {
    destroy ();
    return;
  }

  dialog = gtk_message_dialog_new (GTK_WINDOW (gui->window),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_QUESTION,
                                  GTK_BUTTONS_YES_NO,
                                  "Save before exiting?");
  gtk_window_set_title (GTK_WINDOW (dialog), "Quit GCAM");

  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (gui->window));
  g_signal_connect (dialog, "response", G_CALLBACK (quit_callback), gui);

  gtk_widget_show (dialog);
}
