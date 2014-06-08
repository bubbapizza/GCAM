/*
*  gui_menu_util.c
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
#include "gui_menu_util.h"
#include "gui_define.h"


void
base_unit_changed_callback (GtkWidget **widget, gpointer data)
{
  GtkWidget **wlist;
  gcode_block_t *index_block;
  gui_t *gui;
  gfloat_t value;
  int changed = 0;
  char *bu;

  wlist = (GtkWidget **) widget;

  gui = (gui_t *) wlist[0];

  bu = gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[3]));

  /* Scale blocks to new units system */
  if (!strcmp (bu, "inch"))
  {
    if (gui->gcode.units != GCODE_UNITS_INCH)
    {
      changed = 1;
      index_block = gui->gcode.list;
      while (index_block)
      {
        if (index_block->scale)
          index_block->scale (index_block, GCODE_MM2INCH);
        index_block = index_block->next;
      }
    }
    gui->gcode.units = GCODE_UNITS_INCH;
  }
  else
  {
    if (gui->gcode.units != GCODE_UNITS_MILLIMETER)
    {
      changed = 1;
      index_block = gui->gcode.list;
      while (index_block)
      {
        if (index_block->scale)
          index_block->scale (index_block, GCODE_INCH2MM);
        index_block = index_block->next;
      }
    }

    gui->gcode.units = GCODE_UNITS_MILLIMETER;
  }

  value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[5]));
  gtk_spin_button_set_range (GTK_SPIN_BUTTON (wlist[5]), GCODE_UNITS ((&gui->gcode), 0.01), GCODE_UNITS ((&gui->gcode), MAX_DIM_X));
  gtk_spin_button_set_increments (GTK_SPIN_BUTTON (wlist[5]), GCODE_UNITS ((&gui->gcode), 0.01), GCODE_UNITS ((&gui->gcode), 0.1));
  if (changed)
    value = gui->gcode.units == GCODE_UNITS_INCH ? value * GCODE_MM2INCH : value * GCODE_INCH2MM;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[5]), value);

  value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[6]));
  gtk_spin_button_set_range (GTK_SPIN_BUTTON (wlist[6]), GCODE_UNITS ((&gui->gcode), 0.01), GCODE_UNITS ((&gui->gcode), MAX_DIM_Y));
  gtk_spin_button_set_increments (GTK_SPIN_BUTTON (wlist[6]), GCODE_UNITS ((&gui->gcode), 0.01), GCODE_UNITS ((&gui->gcode), 0.1));
  if (changed)
    value = gui->gcode.units == GCODE_UNITS_INCH ? value * GCODE_MM2INCH : value * GCODE_INCH2MM;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[6]), value);

  value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[7]));
  gtk_spin_button_set_range (GTK_SPIN_BUTTON (wlist[7]), GCODE_UNITS ((&gui->gcode), 0.01), GCODE_UNITS ((&gui->gcode), MAX_DIM_Z));
  gtk_spin_button_set_increments (GTK_SPIN_BUTTON (wlist[7]), GCODE_UNITS ((&gui->gcode), 0.01), GCODE_UNITS ((&gui->gcode), 0.1));
  if (changed)
    value = gui->gcode.units == GCODE_UNITS_INCH ? value * GCODE_MM2INCH : value * GCODE_INCH2MM;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[7]), value);

  value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[8]));
  gtk_spin_button_set_range (GTK_SPIN_BUTTON (wlist[8]), GCODE_UNITS ((&gui->gcode), 0.0), GCODE_UNITS ((&gui->gcode), MAX_DIM_X));
  gtk_spin_button_set_increments (GTK_SPIN_BUTTON (wlist[8]), GCODE_UNITS ((&gui->gcode), 0.01), GCODE_UNITS ((&gui->gcode), 0.1));
  if (changed)
    value = gui->gcode.units == GCODE_UNITS_INCH ? value * GCODE_MM2INCH : value * GCODE_INCH2MM;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[8]), value);

  value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[9]));
  gtk_spin_button_set_range (GTK_SPIN_BUTTON (wlist[9]), GCODE_UNITS ((&gui->gcode), 0.0), GCODE_UNITS ((&gui->gcode), MAX_DIM_Y));
  gtk_spin_button_set_increments (GTK_SPIN_BUTTON (wlist[9]), GCODE_UNITS ((&gui->gcode), 0.01), GCODE_UNITS ((&gui->gcode), 0.1));
  if (changed)
    value = gui->gcode.units == GCODE_UNITS_INCH ? value * GCODE_MM2INCH : value * GCODE_INCH2MM;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[9]), value);

  value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[10]));
  gtk_spin_button_set_range (GTK_SPIN_BUTTON (wlist[10]), GCODE_UNITS ((&gui->gcode), 0.0), GCODE_UNITS ((&gui->gcode), MAX_DIM_Z));
  gtk_spin_button_set_increments (GTK_SPIN_BUTTON (wlist[10]), GCODE_UNITS ((&gui->gcode), 0.01), GCODE_UNITS ((&gui->gcode), 0.1));
  if (changed)
    value = gui->gcode.units == GCODE_UNITS_INCH ? value * GCODE_MM2INCH : value * GCODE_INCH2MM;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[10]), value);

  value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[12]));
  gtk_spin_button_set_range (GTK_SPIN_BUTTON (wlist[12]), GCODE_UNITS ((&gui->gcode), 0.0), GCODE_UNITS ((&gui->gcode), 1.0));
  gtk_spin_button_set_increments (GTK_SPIN_BUTTON (wlist[12]), GCODE_UNITS ((&gui->gcode), 0.01), GCODE_UNITS ((&gui->gcode), 0.1));
  if (changed)
    value = gui->gcode.units == GCODE_UNITS_INCH ? value * GCODE_MM2INCH : value * GCODE_INCH2MM;
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[12]), value);
}


void
update_progress (void *gui, gfloat_t progress)
{
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (((gui_t *) gui)->progress_bar), progress);
  gtk_main_iteration ();
}


void
generic_dialog (void *gui, char *message)
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new (GTK_WINDOW (((gui_t *)gui)->window),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_INFO,
                                  GTK_BUTTONS_OK,
                                  message);
  gtk_window_set_title (GTK_WINDOW (dialog), "Info");

  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (((gui_t *)gui)->window));
  g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);

  gtk_widget_show (dialog);
}


static void
set_tangent_to_previous (gcode_block_t *block)
{
  gfloat_t p0[2], p1[2], t0[2], t1[2];

  /* Auto tangency to previous on insert */
  if (block->prev)
  {
    gcode_vec2d_t tan0, tan1;

    block->ends (block, p0, p1, GCODE_GET);
    block->prev->ends (block->prev, tan0, tan1, GCODE_GET_TANGENT);
    block->prev->ends (block->prev, t0, t1, GCODE_GET);

    if (block->type == GCODE_TYPE_LINE)
    {
      gfloat_t mag;
      gcode_vec2d_t vec;

      GCODE_MATH_VEC2D_SUB (vec, p1, p0);
      GCODE_MATH_VEC2D_MAG (mag, vec);
      GCODE_MATH_VEC2D_SCALE (tan1, mag);
      GCODE_MATH_VEC2D_ADD (p1, t1, tan1);

      block->ends (block, t1, p1, GCODE_SET);
    }
    else if (block->type == GCODE_TYPE_ARC)
    {
      block->ends (block, t1, p1, GCODE_SET);
    }
  }
}


int
insert_primitive (gui_t *gui, gcode_block_t *block, gcode_block_t *selected_block, GtkTreeIter *iter, int action)
{
  gcode_block_t *block_ind;
  GtkTreeModel *model;
  GtkTreeIter child_iter, parent_iter;
  int i;


  if (selected_block->type == GCODE_TYPE_SKETCH)
  {
    if (block->type == GCODE_TYPE_ARC || block->type == GCODE_TYPE_LINE)
    {
      gcode_sketch_t *sketch;

      sketch = (gcode_sketch_t *) selected_block->pdata;
      block->offset = &sketch->offset;
      block->parent = selected_block;

      gcode_list_insert (&sketch->list, block);
      gcode_list_move_prev (block); /* so the block is first */

      /* Insert into list */
      i = 1; /* Because extrusion is 0 */
      for (block_ind = sketch->list; block_ind != block; block_ind = block_ind->next)
        i++;

      child_iter = refresh_gcode_block_tree_recursion (gui, block, iter, i, 1);
    }
    else
    {
      block->offset = selected_block->offset;
      block->parent = selected_block->parent;
      gcode_list_insert (&selected_block, block);

      i = 0;
      for (block_ind = selected_block; block_ind; block_ind = block_ind->prev)
        i++;

      gtk_tree_model_iter_parent (gtk_tree_view_get_model (GTK_TREE_VIEW (gui->gcode_block_treeview)), &parent_iter, iter);

      if (gtk_tree_store_iter_is_valid (gui->gcode_block_store, &parent_iter))
      {
        child_iter = refresh_gcode_block_tree_recursion (gui, block, &parent_iter, i, 1);
      }
      else
      {
        child_iter = refresh_gcode_block_tree_recursion (gui, block, NULL, i, 1);
      }
    }
  }
  else if (selected_block->type == GCODE_TYPE_EXTRUSION)
  {
    gcode_extrusion_t *extrusion;

    extrusion = (gcode_extrusion_t *) selected_block->pdata;
    block->offset = &extrusion->offset;
    block->parent = selected_block;

    gcode_list_insert (&extrusion->list, block);
    gcode_list_move_prev (block); /* so the block is first */

    extrusion->list = block;

    /* Insert into list */
    child_iter = refresh_gcode_block_tree_recursion (gui, block, iter, 0, 1);
  }
  else if (selected_block->type == GCODE_TYPE_TEMPLATE && action == GUI_INSERT_INTO)
  {
    if (block->type != GCODE_TYPE_TEMPLATE)
    {
      gcode_template_t *template;

      template = (gcode_template_t *) selected_block->pdata;
      block->offset = &template->offset;
      block->parent = selected_block;

      gcode_list_insert (&template->list, block);
      gcode_list_move_prev (block); /* so the block is first */

/*
      template->list = block;
      block->parent_list = &template->list;
*/

      /* Insert into treeview list */
      child_iter = refresh_gcode_block_tree_recursion (gui, block, iter, 0, 1);
    }
    else
    {
      gcode_list_insert (&selected_block, block);

      model = gtk_tree_view_get_model (GTK_TREE_VIEW (gui->gcode_block_treeview));
      gtk_tree_model_iter_parent (model, &parent_iter, iter);

      i = 0;
      for (block_ind = selected_block; block_ind; block_ind = block_ind->prev)
        i++;

      if (gtk_tree_store_iter_is_valid (gui->gcode_block_store, &parent_iter))
      {
        child_iter = refresh_gcode_block_tree_recursion (gui, block, &parent_iter, i, 1);
      }
      else
      {
        child_iter = refresh_gcode_block_tree_recursion (gui, block, NULL, i, 1);
      }
    }
  }
  else if (selected_block->type == GCODE_TYPE_DRILL_HOLES)
  {
    if (block->type == GCODE_TYPE_POINT)
    {
      gcode_drill_holes_t *drill_holes;

      drill_holes = (gcode_drill_holes_t *) selected_block->pdata;
      block->offset = &drill_holes->offset;
      block->parent = selected_block;

      gcode_list_insert (&drill_holes->list, block);
      gcode_list_move_prev (block); /* so the block is first */

      drill_holes->list = block;

      /* Insert into treeview list */
      child_iter = refresh_gcode_block_tree_recursion (gui, block, iter, 0, 1);
    }
    else
    {
      gcode_list_insert (&selected_block, block);

      /* Does this code ever get executed?  Is there anything besides a point getting places in Drill Holes? */
      i = 0;
      for (block_ind = selected_block; block_ind; block_ind = block_ind->prev)
        i++;

      gtk_tree_model_iter_parent (gtk_tree_view_get_model (GTK_TREE_VIEW (gui->gcode_block_treeview)), &parent_iter, iter);

      if (gtk_tree_store_iter_is_valid (gui->gcode_block_store, &parent_iter))
      {
        child_iter = refresh_gcode_block_tree_recursion (gui, block, &parent_iter, i, 1);
      }
      else
      {
        child_iter = refresh_gcode_block_tree_recursion (gui, block, NULL, i, 1);
      }
    }
  }
  else if (selected_block->type == GCODE_TYPE_POINT)
  {
    gcode_drill_holes_t *drill_holes;

    drill_holes = (gcode_drill_holes_t *) selected_block->parent->pdata;

    block->offset = &drill_holes->offset;
    block->parent = selected_block->parent;
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (gui->gcode_block_treeview));

    gcode_list_insert (&selected_block, block);

    /* Insert into list */
    gtk_tree_model_iter_parent (model, &parent_iter, iter);

    i = 0;
    for (block_ind = drill_holes->list; block_ind != block; block_ind = block_ind->next)
      i++;
    child_iter = refresh_gcode_block_tree_recursion (gui, block, &parent_iter, i, 1);
  }
  else if (selected_block->type == GCODE_TYPE_ARC || selected_block->type == GCODE_TYPE_LINE)
  {
    if (selected_block->parent->type == GCODE_TYPE_SKETCH)
    {
      gcode_sketch_t *sketch;

      sketch = (gcode_sketch_t *) selected_block->parent->pdata;

      block->offset = &sketch->offset;
      block->parent = selected_block->parent;

      model = gtk_tree_view_get_model (GTK_TREE_VIEW (gui->gcode_block_treeview));

      gcode_list_insert (&selected_block, block);

      /* Insert into list */
      gtk_tree_model_iter_parent (model, &parent_iter, iter);

      i = 1;
      for (block_ind = sketch->list; block_ind != block; block_ind = block_ind->next)
        i++;
      child_iter = refresh_gcode_block_tree_recursion (gui, block, &parent_iter, i, 1);
    }
    else if (selected_block->parent->type == GCODE_TYPE_EXTRUSION)
    {
      gcode_extrusion_t *extrusion;

      extrusion = (gcode_extrusion_t *) selected_block->parent->pdata;

      block->offset = &extrusion->offset;
      block->parent = selected_block->parent;
      model = gtk_tree_view_get_model (GTK_TREE_VIEW (gui->gcode_block_treeview));

      gcode_list_insert (&selected_block, block);

      /* Insert into list */
      gtk_tree_model_iter_parent (model, &parent_iter, iter);

      i = 1;
      for (block_ind = extrusion->list; block_ind != block; block_ind = block_ind->next)
        i++;
      child_iter = refresh_gcode_block_tree_recursion (gui, block, &parent_iter, i, 1);
    }
  }
  else
  {
    gcode_list_insert (&selected_block, block);

    block->offset = NULL;
    block->parent_list = selected_block->parent_list;
    block->parent = selected_block->parent;

    /* Insert into list */
    i = 0;
    for (block_ind = selected_block; block_ind; block_ind = block_ind->prev)
      i++;

    gtk_tree_model_iter_parent (gtk_tree_view_get_model (GTK_TREE_VIEW (gui->gcode_block_treeview)), &parent_iter, iter);

    if (gtk_tree_store_iter_is_valid (gui->gcode_block_store, &parent_iter))
    {
      child_iter = refresh_gcode_block_tree_recursion (gui, block, &parent_iter, i, 1);
    }
    else
    {
      child_iter = refresh_gcode_block_tree_recursion (gui, block, NULL, i, 1);
    }
  }

  if (action & GUI_INSERT_WITH_TANGENCY)
    set_tangent_to_previous (block);

  if (gtk_tree_model_iter_parent (gtk_tree_view_get_model (GTK_TREE_VIEW (gui->gcode_block_treeview)), &parent_iter, &child_iter))
    gtk_tree_view_expand_to_path (GTK_TREE_VIEW (gui->gcode_block_treeview), gtk_tree_model_get_path (gtk_tree_view_get_model (GTK_TREE_VIEW (gui->gcode_block_treeview)), &parent_iter));

  update_menu_options (gui, block);

  update_block_tree_order (gui);

  set_selected_row_with_iter (gui, &child_iter);
  /*
  * Set the Comment cell for this block to be in editing mode as a default behavior.
  */
  gui->ignore_signals = 1;
  gtk_tree_view_set_cursor_on_cell (GTK_TREE_VIEW (gui->gcode_block_treeview),
                                    gtk_tree_model_get_path (gtk_tree_view_get_model (GTK_TREE_VIEW (gui->gcode_block_treeview)), &child_iter),
                                    gtk_tree_view_get_column (GTK_TREE_VIEW (gui->gcode_block_treeview), 4),
                                    gui->comment_cell,
                                    TRUE);
  gui->ignore_signals = 0;
  gui->modified = 1;
  return (0);
}


void
destroy ()
{
  gtk_main_quit ();
}


GtkTreeIter
refresh_gcode_block_tree_recursion (gui_t *gui, gcode_block_t *block, GtkTreeIter *parent_iter, uint16_t ind, uint8_t single)
{
  GtkTreeIter child_iter;

  for (; block; ind++)
  {
    gtk_tree_store_insert (gui->gcode_block_store, &child_iter, parent_iter, ind);

/*    gtk_tree_store_append (gui->gcode_block_store, &child_iter, parent_iter); */
    gtk_tree_store_set (gui->gcode_block_store, &child_iter, 0, ind, 1, GCODE_TYPE_STRING[block->type], 2, block->status, 3, block->flags & GCODE_FLAGS_SUPPRESS, 4, block->comment, 5, block, -1);

    if (block->type == GCODE_TYPE_SKETCH)
    {
      gcode_sketch_t *sketch;
      gcode_extrusion_t *extrusion;
      GtkTreeIter extrusion_iter;

      sketch = (gcode_sketch_t *) block->pdata;
      extrusion = (gcode_extrusion_t *) sketch->extrusion->pdata;

      refresh_gcode_block_tree_recursion (gui, sketch->list, &child_iter, 1, 0);

      /* Extrusion */
      gtk_tree_store_insert (gui->gcode_block_store, &extrusion_iter, &child_iter, 0);
      gtk_tree_store_set (gui->gcode_block_store, &extrusion_iter, 0, 0, 1, GCODE_TYPE_STRING[sketch->extrusion->type], 2, sketch->extrusion->status, 3, sketch->extrusion->flags & GCODE_FLAGS_SUPPRESS, 4, sketch->extrusion->comment, 5, sketch->extrusion, -1);
      refresh_gcode_block_tree_recursion (gui, extrusion->list, &extrusion_iter, 1, 0);
    }
    else if (block->type == GCODE_TYPE_BOLT_HOLES)
    {
      gcode_bolt_holes_t *bolt_holes;
      gcode_extrusion_t *extrusion;
      GtkTreeIter extrusion_iter;

      bolt_holes = (gcode_bolt_holes_t *) block->pdata;
      extrusion = (gcode_extrusion_t *) bolt_holes->extrusion->pdata;

      gtk_tree_store_insert (gui->gcode_block_store, &extrusion_iter, &child_iter, 0);

      /* Extrusion */
      gtk_tree_store_set (gui->gcode_block_store, &extrusion_iter, 0, 0, 1, GCODE_TYPE_STRING[bolt_holes->extrusion->type], 2, bolt_holes->extrusion->status, 3, bolt_holes->extrusion->flags & GCODE_FLAGS_SUPPRESS, 4, bolt_holes->extrusion->comment, 5, bolt_holes->extrusion, -1);
      refresh_gcode_block_tree_recursion (gui, extrusion->list, &extrusion_iter, 1, 0);

      /* Do not expose bolt hole arcs */
    }
    else if (block->type == GCODE_TYPE_TEMPLATE)
    {
      gcode_template_t *template;

      template = (gcode_template_t *) block->pdata;

      refresh_gcode_block_tree_recursion (gui, template->list, &child_iter, 1, 0);
    }
    else if (block->type == GCODE_TYPE_DRILL_HOLES)
    {
      gcode_drill_holes_t *drill_holes;

      drill_holes = (gcode_drill_holes_t *) block->pdata;

      refresh_gcode_block_tree_recursion (gui, drill_holes->list, &child_iter, 1, 0);
    }

    block = single ? NULL : block->next;
  }

  return child_iter;
}


void
refresh_gcode_block_tree (gui_t *gui)
{
  gtk_tree_store_clear (gui->gcode_block_store);

  refresh_gcode_block_tree_recursion (gui, gui->gcode.list, NULL, 1, 0);
}


void
get_selected_block (gui_t *gui, gcode_block_t **selected_block, GtkTreeIter *iter)
{
  GtkTreeModel *model;
  GtkTreeSelection *selection;
  GValue value = { 0, };

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (gui->gcode_block_treeview));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (gui->gcode_block_treeview));
  *selected_block = NULL;

  if (gtk_tree_selection_get_selected (selection, NULL, iter))
  {
    /* get the pointer from the tree */
    gtk_tree_model_get_value (model, iter, 5, &value);
    *selected_block = (gcode_block_t *) g_value_get_pointer (&value);

    g_value_unset (&value);
  }
}


void
set_selected_row_with_iter (gui_t *gui, GtkTreeIter *iter)
{
  gtk_tree_selection_select_iter (gtk_tree_view_get_selection (GTK_TREE_VIEW (gui->gcode_block_treeview)), iter);
}


static void
find_tree_row_iter_with_block_recursion (gui_t *gui, GtkTreeModel *model, GtkTreeIter *iter, GtkTreeIter *found_iter, gcode_block_t *block)
{
  gcode_block_t *test_block;
  GtkTreeIter child_iter;
  GValue value = { 0, };

  do
  {
    gtk_tree_model_get_value (model, iter, 5, &value);
    test_block = (gcode_block_t *) g_value_get_pointer (&value);
    g_value_unset (&value);

    if (block == test_block)
    {
      *found_iter = *iter;
      return;
    }

    if (gtk_tree_model_iter_children (model, &child_iter, iter))
      find_tree_row_iter_with_block_recursion (gui, model, &child_iter, found_iter, block);

  } while (gtk_tree_model_iter_next (model, iter));
}


void
set_selected_row_with_block (gui_t *gui, gcode_block_t *block)
{
  GtkTreeModel *model;
  GtkTreeIter iter, found_iter;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (gui->gcode_block_treeview));

  gtk_tree_model_get_iter_first (model, &iter);

  find_tree_row_iter_with_block_recursion (gui, model, &iter, &found_iter, block);

  gtk_tree_view_expand_to_path (GTK_TREE_VIEW (gui->gcode_block_treeview), gtk_tree_model_get_path (model, &found_iter));

  gtk_tree_selection_select_iter (gtk_tree_view_get_selection (GTK_TREE_VIEW (gui->gcode_block_treeview)), &found_iter);

  gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (gui->gcode_block_treeview), gtk_tree_model_get_path (model, &found_iter), NULL, TRUE, 0.0, 0.0);

  update_menu_options (gui, block);

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);
}


void
update_block_tree_order_recursion (gui_t *gui, GtkTreeModel *model, GtkTreeIter *iter)
{
  gcode_block_t *block;
  GtkTreeIter child_iter;
  GValue value = { 0, };
  int i = 1;

  do
  {
    gtk_tree_model_get_value (model, iter, 5, &value);
    block = (gcode_block_t *) g_value_get_pointer (&value);
    g_value_unset (&value);

    if (block->type == GCODE_TYPE_EXTRUSION)
    {
      gtk_tree_store_set (gui->gcode_block_store, iter, 0, 0, -1);
    }
    else
    {
      gtk_tree_store_set (gui->gcode_block_store, iter, 0, i, -1);
      i++;
    }

    if (gtk_tree_model_iter_children (model, &child_iter, iter))
      update_block_tree_order_recursion (gui, model, &child_iter);

  } while (gtk_tree_model_iter_next (model, iter));
}


void
update_block_tree_order (gui_t *gui)
{
  GtkTreeModel *model;
  GtkTreeIter iter;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (gui->gcode_block_treeview));

  gtk_tree_model_get_iter_first (model, &iter);
  update_block_tree_order_recursion (gui, model, &iter);
}


void
project_menu_options (gui_t *gui, uint8_t state)
{
    uint8_t mode;

    gui->project_state = state;

    mode = gui->project_state == PROJECT_OPEN ? TRUE : FALSE;

    /* Widgets to display when project is open */
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/New"), !mode);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Load"), !mode);

    /* Widgets to display when project is closed */
    if (!mode)
    {
      gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Save"), 0);
    }

    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Save As"), mode);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Close"), mode);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import GCAM"), mode);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import Gerber (RS274X)"), mode);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import Excellon Drill Holes"), mode);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import SVG Paths"), mode);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import STL"), mode);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Export"), mode);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu"), mode);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu"), mode);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/AssistantMenu"), mode);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/ViewMenu"), mode);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/RenderMenu"), mode);
}


void
update_menu_options (gui_t *gui, gcode_block_t *selected_block)
{
  /* 
  * This check is here incase a block is deleted right after a new block is inserted/duplicated
  * and  the comment field is highlighted.
  */
  if (!selected_block)
    return;

  /*
  * Toggle menu items that should reflect actions available to the current selected block.
  */
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import GCAM"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import Gerber (RS274X)"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import Excellon Drill Holes"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import SVG Paths"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import STL"), 0);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Remove"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Duplicate"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Scale"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Join Previous"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Join Next"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Fillet Previous"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Fillet Next"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Flip Direction"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Pattern"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Tool Change"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Template"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Sketch"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Arc"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Line"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Bolt Holes"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Drill Holes"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Point"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Image"), 1);
  gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/ViewMenu"), 1);

  /* FILLETING */
  if (selected_block->type == GCODE_TYPE_LINE)
  {
    gcode_block_t *connected;

    connected = gcode_sketch_prev_connected (selected_block);
    if (connected)
    {
      if (connected->type != GCODE_TYPE_LINE)
      {
        gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Fillet Previous"), 0);
      }
    }
    else
    {
      gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Fillet Previous"), 0);
    }

    connected = gcode_sketch_next_connected (selected_block);
    if (connected)
    {
      if (connected->type != GCODE_TYPE_LINE)
      {
        gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Fillet Next"), 0);
      }
    }
    else
    {
      gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Fillet Next"), 0);
    }
  }
  else
  {
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Fillet Previous"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Fillet Next"), 0);
  }

  /* DRILL POINTS AND POINT */
  if (selected_block->type != GCODE_TYPE_DRILL_HOLES && selected_block->type != GCODE_TYPE_POINT)
  {
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Point"), 0);
  }

  if (selected_block->type == GCODE_TYPE_POINT)
  {
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Tool Change"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Template"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Sketch"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Arc"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Line"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Bolt Holes"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Drill Holes"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Image"), 0);
  }


  /* EXTRUSION */
  if (selected_block->parent)
    if (selected_block->parent->type == GCODE_TYPE_EXTRUSION)
    {
      gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/ViewMenu"), 0);
    }

  if (selected_block->type == GCODE_TYPE_EXTRUSION)
  {
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/ViewMenu"), 0);
  }

  /* EDIT MENU */
  if (selected_block->flags & GCODE_FLAGS_LOCK)
  {
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Remove"), 0);
  }

  if (selected_block->type != GCODE_TYPE_SKETCH && selected_block->type != GCODE_TYPE_DRILL_HOLES)
  {
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Pattern"), 0);
  }

  if (selected_block->type != GCODE_TYPE_ARC && selected_block->type != GCODE_TYPE_LINE)
  {
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Flip Direction"), 0);
  }

  /* LINE AND ARC */
  if (selected_block->type == GCODE_TYPE_BEGIN ||
      selected_block->type == GCODE_TYPE_END ||
      selected_block->type == GCODE_TYPE_TOOL ||
      selected_block->type == GCODE_TYPE_TEMPLATE ||
      selected_block->type == GCODE_TYPE_BOLT_HOLES ||
      selected_block->type == GCODE_TYPE_DRILL_HOLES ||
      selected_block->type == GCODE_TYPE_IMAGE)
  {
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Line"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Arc"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Join Previous"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Join Next"), 0);
  }

  /* JOIN NEXT and JOIN PREVIOUS */
  if (selected_block->type == GCODE_TYPE_SKETCH || selected_block->type == GCODE_TYPE_EXTRUSION)
  {
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Join Previous"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Join Next"), 0);
  }


  /* DUPLICATE */
  if (selected_block->type == GCODE_TYPE_BEGIN || selected_block->type == GCODE_TYPE_END || selected_block->type == GCODE_TYPE_EXTRUSION)
  {
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Duplicate"), 0);
  }

  /* SCALE */
  if (selected_block->type != GCODE_TYPE_TEMPLATE && selected_block->type != GCODE_TYPE_SKETCH &&
      selected_block->type != GCODE_TYPE_BOLT_HOLES && selected_block->type != GCODE_TYPE_DRILL_HOLES &&
      selected_block->type != GCODE_TYPE_LINE && selected_block->type != GCODE_TYPE_ARC &&
      selected_block->type != GCODE_TYPE_IMAGE && selected_block->type != GCODE_TYPE_STL)
  {
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/EditMenu/Scale"), 0);
  }

  /* End - Nothing allowed to be inserted or pasted */
  if (selected_block->type == GCODE_TYPE_END)
  {
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import GCAM"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import Gerber (RS274X)"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import Excellon Drill Holes"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import SVG Paths"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Import STL"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Tool Change"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Sketch"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Bolt Holes"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Template"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Arc"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Line"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Drill Holes"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Image"), 0);
  }

  if (selected_block->type == GCODE_TYPE_LINE ||
      selected_block->type == GCODE_TYPE_ARC ||
      selected_block->type == GCODE_TYPE_EXTRUSION)
  {
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Tool Change"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Sketch"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Bolt Holes"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Template"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Drill Holes"), 0);
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/InsertMenu/Image"), 0);
  }
}


void
gui_menu_util_modified (gui_t *gui, int mod)
{
  if (mod)
  {
    if (gui->project_state == PROJECT_OPEN)
      gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Save"), 1);

    if (gui->gcode.name[0])
    {
      sprintf (gui->title, "* GCAM v%s - %s", VERSION, gui->gcode.name);
    }
    else
    {
      sprintf (gui->title, "* GCAM v%s", VERSION);
    }
  }
  else
  {
    if (gui->project_state == PROJECT_OPEN)
      gtk_action_set_sensitive (gtk_ui_manager_get_action (gui->ui_manager, "/MainMenu/FileMenu/Save"), 0);

    if (gui->gcode.name[0])
    {
      sprintf (gui->title, "GCAM v%s - %s", VERSION, gui->gcode.name);
    }
    else
    {
      sprintf (gui->title, "GCAM v%s", VERSION);
    }
  }

  gtk_window_set_title (GTK_WINDOW (gui->window), gui->title);
  gui->modified = mod;
}
