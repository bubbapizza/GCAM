/*
*  gui.c
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
#include "gui.h"
#include "gui_define.h"
#include "gui_endmills.h"
#include "gui_machines.h"
#include "gui_menu.h"
#include "gui_menu_util.h"
#include "gui_tab.h"
#include "gcode.h"
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "gcam_icon.h"


gui_t gui;


static void
comment_cell_edited (GtkCellRendererText *cell,
                      const gchar *path_string,
                      const gchar *new_text,
                      gpointer data)
{
  GtkTreeModel *model = (GtkTreeModel *) data;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  GtkTreeIter iter;
  gcode_block_t *selected_block;
  int i, j;
  char *modified_text;

  gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));

  gtk_tree_model_get_iter (model, &iter, path);

/*
  gtk_tree_model_get (model, &iter, column, &old_text, -1);
  g_free (old_text);
*/ 

  /* Replace certain characters in string */
  modified_text = (char *) malloc (strlen (new_text)+1);
  j = 0;
  for (i = 0; i < strlen (new_text); i++)
  {
    modified_text[j] = new_text[i];
    if (new_text[i] == '(')
      modified_text[j] = '[';
    if (new_text[i] == ')')
      modified_text[j] = ']';
    if (new_text[i] == ';')
      j--;
    j++;    
  }
  modified_text[j] = 0; /* Null terminate the string */
    
  gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column, modified_text, -1);

  /* Update the gcode block too. */
  get_selected_block (&gui, &selected_block, &iter);

  if (selected_block)
    strcpy (selected_block->comment, modified_text);

  gtk_tree_path_free (path);
  free (modified_text);

  gui_menu_util_modified (&gui, 1);
}


static void
gcode_tree_cursor_changed_event (GtkTreeView *treeview, gpointer ptr)
{
  gcode_block_t *selected_block;
  GtkTreeIter iter;

  get_selected_block (&gui, &selected_block, &iter);
  gui_tab_display (&gui, selected_block, 0);
  update_menu_options (&gui, selected_block);
}


static void
gcode_tree_row_collapsed_event (GtkTreeView *tree_view, GtkTreeIter *iter, GtkTreePath *path, gpointer user_data)
{
  gcode_block_t *selected_block;

  set_selected_row_with_iter (&gui, iter);
  get_selected_block (&gui, &selected_block, iter);
  gui_tab_display (&gui, selected_block, 0);
  update_menu_options (&gui, selected_block);
}


static void
opengl_context_expose_event (GtkWidget *widget, gpointer data)
{
  gcode_block_t *selected_block;
  GtkTreeIter iter;

  get_selected_block (&gui, &selected_block, &iter);

  /*
  * When resizing the exposed event is triggered.
  * It's not guaranteed what order the opengl context callback will be called in.
  * Refresh the opengl context a few times while the rest of the window is being redrawn.
  * This is kind of nasty, but it's the only solution I can think of right now without a using timer.
  */
  gui_opengl_context_redraw (&gui.opengl, selected_block);
  gui_opengl_context_redraw (&gui.opengl, selected_block);
  gui_opengl_context_redraw (&gui.opengl, selected_block);
/*  printf ("expose\n"); */
}


static gboolean
opengl_context_key_event (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  return (TRUE);
}


static gboolean   
opengl_context_button_event (GtkWidget *widget, GdkEventButton *event)
{
  gcode_block_t *selected_block;
  GtkTreeIter iter;
  guint modifiers;

  get_selected_block (&gui, &selected_block, &iter);
  if (!selected_block)
    return (TRUE);

  modifiers = gtk_accelerator_get_default_mod_mask ();


  /* Accurate to within 1/100th of a second, good 'nuff for frame dropping */
  gui.event_time = 0.001 * (double) gtk_get_current_event_time () - g_timer_elapsed (gui.timer, NULL);

  gui.mouse_x = event->x;
  gui.mouse_y = event->y;

  if (event->type == GDK_BUTTON_RELEASE)
  {
    if (event->button == 3)
    {
      gfloat_t z;
      int16_t dx, dy;

      dx = (int16_t) (event->x - gui.mouse_x);
      dy = (int16_t) (event->y - gui.mouse_y);

      z = dy < 0 ? 0.985 : 1.015;
      z = pow (z, fabs (dy));
      if (gui.opengl.projection == GUI_OPENGL_PROJECTION_PERSPECTIVE)
      {
        gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom *= z;
        if (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom > GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MAX_ZOOM))
          gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom = GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MAX_ZOOM);
        if (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom < GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MIN_ZOOM))
          gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom = GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MIN_ZOOM);
      }
      else
      {
        gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid *= z;
        if (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid > GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MAX_ZOOM))
          gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid = GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MAX_ZOOM);
        if (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid < GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MIN_ZOOM))
          gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid = GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MIN_ZOOM);
      }

      gui_opengl_context_redraw (&gui.opengl, selected_block);
    }
  }
  else if (event->type == GDK_BUTTON_PRESS)
  {
    if (event->button == 1 && (event->state & modifiers) == GDK_CONTROL_MASK)
      gui_opengl_pick (&gui.opengl, event->x, event->y);
  }

/*  printf ("event->state: %d .. %d\n", event->state, GDK_BUTTON4_MASK); */

  return (TRUE);
}


static gboolean
opengl_context_motion_event (GtkWidget *widget, GdkEventMotion *event)
{
  int16_t dx, dy;
  uint8_t update;
  gfloat_t delay;
  gcode_block_t *selected_block;
  GtkTreeIter iter;
  guint modifiers;


  get_selected_block (&gui, &selected_block, &iter);
  if (!selected_block)
    return (TRUE);

  modifiers = gtk_accelerator_get_default_mod_mask ();

  /* Only allow spin/zoom events on appropriate blocks. */
  if (selected_block->type == GCODE_TYPE_EXTRUSION)
    return (TRUE);

  update = 0;

  dx = (int16_t) (event->x - gui.mouse_x);
  dy = (int16_t) (event->y - gui.mouse_y);


  if (event->state & GDK_BUTTON1_MASK && (event->state & modifiers) != GDK_CONTROL_MASK)
  {
    gcode_vec3d_t up, side, look;
    gfloat_t celev, delta[2], coef;
  
    /* relative up/down left/right movement */
    GCODE_MATH_VEC3D_SET(up, 0.0, 0.0, 1.0);

    celev = cos (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].elev * GCODE_DEG2RAD);
    look[0] = -cos ((gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].azim+90.0) * GCODE_DEG2RAD) * celev;
    look[1] = -sin (-(gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].azim+90.0) * GCODE_DEG2RAD) * celev;
    look[2] = -sin (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].elev * GCODE_DEG2RAD);
    GCODE_MATH_VEC3D_UNITIZE(look);

    /* side vector */
    GCODE_MATH_VEC3D_CROSS(side, look, up);
    GCODE_MATH_VEC3D_UNITIZE(side);

    /* up vector */
    GCODE_MATH_VEC3D_CROSS(up, look, side);   
    GCODE_MATH_VEC3D_UNITIZE(up);

    coef = (2.0 / (gfloat_t) gui.opengl.context_w);
    if (gui.opengl.projection == GUI_OPENGL_PROJECTION_PERSPECTIVE)
    {
      delta[0] = -coef * gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom * dx;
      delta[1] = -coef * gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom * dy;
    }
    else
    {
      delta[0] = -coef * gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid * dx;
      delta[1] = -coef * gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid * dy;
    }
      
    /* left/right */
    GCODE_MATH_VEC3D_MUL_SCALAR(side, side, delta[0]);
    GCODE_MATH_VEC3D_ADD(gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].pos, gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].pos, side);

    /* up/down */
    GCODE_MATH_VEC3D_MUL_SCALAR(up, up, delta[1]);
    GCODE_MATH_VEC3D_ADD(gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].pos, gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].pos, up);
    update = 1;
  }


  if (event->state & GDK_BUTTON2_MASK)
  {
    gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].azim += (dx * 0.5);
    gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].elev += (dy * 0.5);

    if (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].elev < 0.0)
      gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].elev = 0.0;
    if (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].elev > 90.0)
      gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].elev = 90.0;
    update = 1;
  }


  if (event->state & GDK_BUTTON3_MASK)
  {
    gfloat_t z;

    z = dy < 0 ? 0.99 : 1.01;
    z = pow (z, fabs (dy));
    if (gui.opengl.projection == GUI_OPENGL_PROJECTION_PERSPECTIVE)
    {
      gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom *= z;
      if (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom > GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MAX_ZOOM))
        gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom = GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MAX_ZOOM);
      if (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom < GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MIN_ZOOM))
        gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom = GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MIN_ZOOM);
    }
    else
    {
      gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid *= z;
      if (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid > GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MAX_ZOOM))
        gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid = GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MAX_ZOOM);
      if (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid < GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MIN_ZOOM))
        gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid = GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MIN_ZOOM);
    }
    update = 1;
  }

  delay = (gui.event_time + g_timer_elapsed (gui.timer, NULL)) - 0.001 * (double) gtk_get_current_event_time ();

  /* Drop frames that are older than 20ms old */
  if (update && delay < 0.02)
  {
    gui_opengl_context_redraw (&gui.opengl, selected_block);
    gui.mouse_x = event->x;
    gui.mouse_y = event->y;
  }


  return (TRUE);
}


static gboolean
opengl_context_scroll_event (GtkWidget *widget, GdkEventScroll *event)
{
  gcode_block_t *selected_block;
  GtkTreeIter iter;
  gfloat_t z;

  get_selected_block (&gui, &selected_block, &iter);
  if (!selected_block)
    return (TRUE);

  z = event->direction ? 1.1 : 0.9;
  if (gui.opengl.projection == GUI_OPENGL_PROJECTION_PERSPECTIVE)
  {
    gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom *= z;
    if (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom > GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MAX_ZOOM))
      gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom = GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MAX_ZOOM);
    if (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom < GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MIN_ZOOM))
      gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].zoom = GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MIN_ZOOM);
  }
  else
  {
    gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid *= z;
    if (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid > GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MAX_ZOOM))
      gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid = GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MAX_ZOOM);
    if (gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid < GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MIN_ZOOM))
      gui.opengl.view[GUI_OPENGL_VIEW_REGULAR].grid = GCODE_UNITS ((&gui.gcode), GUI_OPENGL_MIN_ZOOM);
  }

  gui_opengl_context_redraw (&gui.opengl, selected_block);

  return (TRUE);
}


static void
suppress_toggled (GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
  gcode_block_t *selected_block;
  GtkTreeModel *model = (GtkTreeModel *) data;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  GtkTreeIter iter;
  gboolean toggle_item;
  gint *column;

  column = g_object_get_data (G_OBJECT (cell), "column");

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  set_selected_row_with_iter (&gui, &iter); /* This is done because the selected block is currently the old one */
  get_selected_block (&gui, &selected_block, &iter);
  if (gui.ignore_signals || selected_block->flags & GCODE_FLAGS_LOCK)
    return;

  gtk_tree_model_get (model, &iter, column, &toggle_item, -1);

  /* do something with the value */
  toggle_item ^= 1;

  /* set new value */
  gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column, toggle_item, -1);

  /* clean up */
  gtk_tree_path_free (path);

  /* update block flag bits */
  selected_block->flags = (selected_block->flags & ~GCODE_FLAGS_SUPPRESS) | toggle_item << 1;

  /* Update OpenGL context */
  gui.opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui.opengl, selected_block);

  gui_menu_util_modified (&gui, 1);
}


static gboolean
row_drop_possible (GtkTreeDragDest *drag_dest, GtkTreePath *dest_path, GtkSelectionData *selection_data)
{
  GtkTreeModel *model;
  GtkTreePath *src_path, *drop_path;
  GtkTreeIter iter;
  gcode_block_t *src_block, *dest_block;
  GValue value = { 0, };
  GtkTreeViewDropPosition drop_pos;
  gcode_block_t *index_block;
  gchar *gstring, src_string[16], dest_string[16];


  model = gtk_tree_view_get_model (GTK_TREE_VIEW (gui.gcode_block_treeview));

  get_selected_block (&gui, &src_block, &iter);

  src_path = gtk_tree_model_get_path (model, &iter);

  gstring = gtk_tree_path_to_string (src_path);
  strcpy (src_string, gstring);
  g_free (gstring);

  gstring = gtk_tree_path_to_string (dest_path);
  strcpy (dest_string, gstring);
  g_free (gstring);

  gtk_tree_model_get_iter (model, &iter, dest_path);

  /* get the pointer from the tree */
  gtk_tree_model_get_value (model, &iter, 5, &value);
  dest_block = (gcode_block_t *) g_value_get_pointer (&value);
  g_value_unset (&value);

  /*
  * Determine Drop Position: Before, After or Into etc.
  * GTK_TREE_VIEW_DROP_BEFORE, GTK_TREE_VIEW_DROP_AFTER, GTK_TREE_VIEW_DROP_INTO_OR_BEFORE, GTK_TREE_VIEW_DROP_INTO_OR_AFTER
  */
  gtk_tree_view_get_drag_dest_row (GTK_TREE_VIEW (gui.gcode_block_treeview), &drop_path, &drop_pos);

  /* Prevent BEGIN and END blocks from being moved */
  if (src_block->type == GCODE_TYPE_BEGIN || src_block->type == GCODE_TYPE_END)
    return (FALSE);

  /* Prevent drop INTO or BEFORE a BEGIN block */
  if (dest_block->type == GCODE_TYPE_BEGIN)
    return (FALSE);

  /* Prevent drop INTO or AFTER an END block */
  if (dest_block->type == GCODE_TYPE_END && drop_pos != GTK_TREE_VIEW_DROP_BEFORE)
    return (FALSE);

  /* Prevent SOURCE from droping INTO itself */
  if (src_block == dest_block)
    return (FALSE);

  /* Prevent SOURCE from droping INTO itself */
  if (strstr (dest_string, src_string) == dest_string)
    return (FALSE);

  /*
  * SPECIFIC RULES
  */

  /* Prevent ANY block from being dropped INTO Tool */
  if (drop_pos == GTK_TREE_VIEW_DROP_INTO_OR_BEFORE || drop_pos == GTK_TREE_VIEW_DROP_INTO_OR_AFTER)
    if (dest_block->type == GCODE_TYPE_TOOL)
      return (FALSE);

  /*
  * Prevent ANY block EXCEPT ARCS and LINES BENEATH a SKETCH
  * Examine the destination hierarchy to determine whether on or beneath a SKETCH.
  */
  index_block = dest_block->parent;
  while (index_block)
  {
    if (index_block->type == GCODE_TYPE_SKETCH && src_block->type != GCODE_TYPE_ARC && src_block->type != GCODE_TYPE_LINE)
      return (FALSE);
    index_block = index_block->parent;
  }    


  /*
  * Similarly, Prevent ANY LINE or ARC from existing OUTSIDE of a SKETCH
  * Examine the destination hierarchy to determine whether on or beneath a SKETCH.
  */
  if (src_block->type == GCODE_TYPE_ARC || src_block->type == GCODE_TYPE_LINE)
  {
    int prevent = 1;

    index_block = dest_block->parent;
    while (index_block)
    {
      if (index_block->type == GCODE_TYPE_SKETCH)
        prevent = 0;
      index_block = index_block->parent;
    }

    if (prevent)
      return (FALSE);
  }


  /* Prevent EXTRUSION from being moved */
  if (src_block->type == GCODE_TYPE_EXTRUSION)
    return (FALSE);

  /*
  * Prevent ANY block from going BENEATH a BOLT HOLES
  * Examine the destination hierarchy to determine whether on or beneath a SKETCH.
  */
  index_block = dest_block->parent;
  while (index_block)
  {
    if (index_block->type == GCODE_TYPE_BOLT_HOLES)
      return (FALSE);
    index_block = index_block->parent;
  }    

  /* Prevent ANY block EXCEPT POINTS BENEATH a DRILL HOLES */
  index_block = dest_block->parent;
  while (index_block)
  {
    if (index_block->type == GCODE_TYPE_DRILL_HOLES && dest_block->type != GCODE_TYPE_POINT)
      return (FALSE);
    index_block = index_block->parent;
  }    

/*
  printf ("dest_string1: %s\n", dest_string);
  printf ("type1: %s, drop_pos: %d, %p\n", GCODE_TYPE_STRING[dest_block->type], drop_pos, selection_data);
*/

  /*
  * If reaching this point then the move is considered Valid
  * The source, destination, and the position are known.
  */

  return (TRUE);
}


static gboolean
drag_data_received (GtkTreeDragDest *drag_dest, GtkTreePath *dest_path, GtkSelectionData *selection_data)
{
  GtkTreeModel *model;
  GtkTreePath *test_path;
  GtkTreeIter src_iter, iter;
  GValue value = { 0, };
  gcode_block_t *src_block, *dest_block, *test_block;
  gchar *gstring, test_string[16], dest_string[16];
  int action, dest_block_has_prev, has_parent;


  action = GUI_INSERT_AFTER;

  get_selected_block (&gui, &src_block, &src_iter);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (gui.gcode_block_treeview));

  /*
  * Test to see if dest_received created a new path at the end of the list
  * which unfortunately points to the parent if you request the pointer from that row
  * This is basically a work around for this behavior.
  */

  gtk_tree_model_get_iter (model, &iter, dest_path);
  gtk_tree_model_get_value (model, &iter, 5, &value);
  dest_block = (gcode_block_t *) g_value_get_pointer (&value);
  g_value_unset (&value);

  /*
  * Before doing the following check, make sure the path provided is not one of the
  * special case paths where the last item in a list, e.g. 1:0:4 (AFTER) is not
  * given as 1:0:5 (BEFORE).  This is done by checking to see if the one given is
  * the same as the parent.  What happens with a Sketch that contains only an extrusion?
  */
  
  test_block = NULL;
  test_path = gtk_tree_path_copy (dest_path);

  /*
  * The return value of this is broken.  If I move a top level object up I get a null path
  * and the return value is 1, clearly this is wrong.  Fortunately I can get the string to
  * see if it is null.
  */
  gtk_tree_path_up (test_path);
  gstring = gtk_tree_path_to_string (test_path);
  has_parent = gstring ? 1 : 0;	

  g_free (gstring);

  if (has_parent)
  {
    gtk_tree_model_get_iter (model, &iter, test_path);
    gtk_tree_model_get_value (model, &iter, 5, &value);
    test_block = (gcode_block_t *) g_value_get_pointer (&value);
    g_value_unset (&value);
  }

/*
printf ("has_parent: %d\n", has_parent);
printf ("test_block: %p, dest_block: %p, dest_block->prev: %p\n", test_block, dest_block, dest_block->prev);
*/

  /*
  * Comparing these two pointers for equality checks to see if the special case occurs
  * Otherwise if the special case is not occuring and the block is not the head of the list
  * then get the previous block.
  */
  dest_block_has_prev = 0;
  if (test_block == dest_block || dest_block->prev)
  {
    dest_block_has_prev = 1;
    test_path = gtk_tree_path_copy (dest_path);
    gtk_tree_path_prev (test_path);

    /* Get the pointer for the test path from the tree */
    gtk_tree_model_get_iter (model, &iter, test_path);
    gtk_tree_model_get_value (model, &iter, 5, &value);
    dest_block = (gcode_block_t *) g_value_get_pointer (&value);
    g_value_unset (&value);
  }


  gstring = gtk_tree_path_to_string (test_path);
  strcpy (test_string, gstring);
  g_free (gstring);

  gstring = gtk_tree_path_to_string (dest_path);
  strcpy (dest_string, gstring);
  g_free (gstring);

/*
  printf ("test_string2: %s\n", test_string);
  printf ("dest_string2: %s\n", dest_string);
*/

  /*
  * This deals with the special case of the destination block being at the beginning
  * of the list so that action can be set to INTO and the destination block can be
  * set to the parent for insert_primitive () to work properly.
  * Must also handle the case where the ...
  */
  if ((dest_block_has_prev == 0 && has_parent) || !strcmp (dest_string, test_string))
  {
    /* If this is TRUE then set the path to the parent */
    gtk_tree_path_up (dest_path);

    /* Get the pointer for the test path from the tree */
    gtk_tree_model_get_iter (model, &iter, dest_path);
    gtk_tree_model_get_value (model, &iter, 5, &value);
    dest_block = (gcode_block_t *) g_value_get_pointer (&value);
    g_value_unset (&value);

    action = GUI_INSERT_INTO;
/*    printf ("*** HEAD\n"); */
  }


  if (src_block == dest_block)
    return (FALSE);
/*    printf ("type2: %s\n", GCODE_TYPE_STRING[dest_block->type]); */

    /* Splice the original location */
  gcode_list_splice (src_block->parent_list, src_block);
  gtk_tree_store_remove (GTK_TREE_STORE (model), &src_iter);
  insert_primitive (&gui, src_block, dest_block, &iter, action);
/*    printf ("src_block->parent: %p\n", src_block->parent); */

  gui.opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui.opengl, dest_block);

  gui_menu_util_modified (&gui, 1);

  return (TRUE);
}


void
gui_init ()
{
  GdkGLConfig *glconfig;
  GtkWidget *window_hbox;
  GtkWidget *window_vbox_main;
  GtkWidget *window_vbox_right;
  GtkWidget *gl_context;

  gui.project_state = PROJECT_CLOSED;
  gui.gcode.message_callback = generic_dialog;
  gui.gcode.gui = (void *) &gui;
  gui.selected_block = NULL;
  strcpy (gui.save_filename, "");
  gui.panel_tab_vbox = NULL;
  gui.modified = 0;
  strcpy (gui.current_folder, "");
  gui.ignore_signals = 0;
  gui.first_render = 1;


  gui_settings_init (&gui.settings);
  gui_settings_read (&gui.settings);

  /*
  * Opengl Display Defaults
  */
  gui.opengl.context_w = WINDOW_W;
  gui.opengl.context_h = WINDOW_H - PANEL_BOTTOM_H;
  gui.opengl.gcode = &gui.gcode;
  gui.opengl.ready = 0;
  gui.opengl.projection = GUI_OPENGL_PROJECTION_ORTHOGRAPHIC;
  gui.opengl.progress_callback = &update_progress;

  gui.timer = g_timer_new ();
  g_timer_start (gui.timer);

  /* Initialize GTK */
  gtk_init (0, 0);
  gtk_gl_init (0, 0);

  glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);

  /* Create Main Window */
  gui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_size_request (GTK_WIDGET (gui.window), WINDOW_W, WINDOW_H);
  gtk_window_set_icon (GTK_WINDOW (gui.window), gdk_pixbuf_new_from_xpm_data (gcam_icon_xpm));
  gui_menu_util_modified (&gui, 0); /* Set Title */


  g_signal_connect (G_OBJECT (gui.window), "destroy", G_CALLBACK (destroy), NULL);

  /* Create a vbox for the opengl context and code block tree */
  window_vbox_main = gtk_vbox_new (FALSE, 1);
  gtk_container_set_border_width (GTK_CONTAINER (window_vbox_main), 1);
  gtk_container_add (GTK_CONTAINER (gui.window), window_vbox_main);

  /* Create menu bar */
  {
    GtkActionGroup *action_group;
    GtkAccelGroup *accel_group;
    GtkWidget *menubar;
    GError *error;

    action_group = gtk_action_group_new ("MenuActions");
    gtk_action_group_add_actions (action_group, gui_menu_entries, G_N_ELEMENTS (gui_menu_entries), &gui);
    gui.ui_manager = gtk_ui_manager_new ();
    gtk_ui_manager_insert_action_group (gui.ui_manager, action_group, 0);

    accel_group = gtk_ui_manager_get_accel_group (gui.ui_manager);
    gtk_window_add_accel_group (GTK_WINDOW (gui.window), accel_group);

    error = NULL;
    if (!gtk_ui_manager_add_ui_from_string (gui.ui_manager, gui_menu_description, -1, &error))
    {
      g_message ("building menus failed: %s", error->message);
      g_error_free (error);
      exit (EXIT_FAILURE);
    }

    menubar = gtk_ui_manager_get_widget (gui.ui_manager, "/MainMenu");
    gtk_box_pack_start (GTK_BOX (window_vbox_main), menubar, FALSE, FALSE, 0);

    /* Disable certain widgets until a project is being worked on */
    project_menu_options (&gui, PROJECT_CLOSED);

    /* Disable Save until Save As is used. */
    gtk_action_set_sensitive (gtk_ui_manager_get_action (gui.ui_manager, "/MainMenu/FileMenu/Save"), 0);
  }

  /* Create an hbox for left panel and opengl context */
  window_hbox = gtk_hbox_new (FALSE, 1);
  gtk_container_set_border_width (GTK_CONTAINER (window_hbox), 1);
  gtk_box_pack_start (GTK_BOX (window_vbox_main), window_hbox, TRUE, TRUE, 0);


  /* Create a vbox for the Left Panel */
  gui.panel_vbox = gtk_vbox_new (FALSE, 1);
  gtk_container_set_border_width (GTK_CONTAINER (gui.panel_vbox), 0);
  gtk_box_pack_start (GTK_BOX (window_hbox), gui.panel_vbox, FALSE, FALSE, 0);
  gtk_widget_set_size_request (GTK_WIDGET (gui.panel_vbox), PANEL_LEFT_W, 0);

#if 0
  /* Left Panel Notebook */
  gui.notebook = gtk_notebook_new ();
  gtk_box_pack_start (GTK_BOX (panel_vbox), gui.notebook, TRUE, TRUE, 0);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gui.notebook), FALSE);
#endif

  /* Create a vbox for the opengl context and block tree on the right side of the window */
  window_vbox_right = gtk_vbox_new (FALSE, 1);
  gtk_container_set_border_width (GTK_CONTAINER (window_vbox_right), 0);
  gtk_box_pack_end (GTK_BOX (window_hbox), window_vbox_right, TRUE, TRUE, 0);

  /* Create the gl context */
  gl_context = gtk_drawing_area_new ();
  gtk_box_pack_start (GTK_BOX (window_vbox_right), gl_context, TRUE, TRUE, 0);

  /* Give gl-capability to the widget. */
  gtk_widget_set_gl_capability (gl_context, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);

  gtk_widget_add_events (gl_context, GDK_VISIBILITY_NOTIFY_MASK);
  g_signal_connect (G_OBJECT (gl_context), "configure_event", G_CALLBACK (gui_opengl_context_init), &gui.opengl);
  g_signal_connect (G_OBJECT (gl_context), "expose_event", G_CALLBACK (opengl_context_expose_event), NULL);
  g_signal_connect_swapped (G_OBJECT (gl_context), "key_press_event", G_CALLBACK (opengl_context_key_event), NULL);
  g_signal_connect (G_OBJECT (gl_context), "button_press_event", G_CALLBACK (opengl_context_button_event), NULL);
  g_signal_connect (G_OBJECT (gl_context), "button_release_event", G_CALLBACK (opengl_context_button_event), NULL);
  g_signal_connect (G_OBJECT (gl_context), "motion_notify_event", G_CALLBACK (opengl_context_motion_event), NULL);
  g_signal_connect (G_OBJECT (gl_context), "scroll_event", G_CALLBACK (opengl_context_scroll_event), NULL);

  gtk_widget_set_events (gl_context, GDK_EXPOSURE_MASK |
                                     GDK_LEAVE_NOTIFY_MASK |
                                     GDK_BUTTON_PRESS_MASK |
				     GDK_BUTTON_RELEASE_MASK |
                                     GDK_SCROLL_MASK |
                                     GDK_POINTER_MOTION_MASK);


  /* Code Block Tree List */
  {
    GtkWidget *sw;
    GtkTreeModel *tree;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_end (GTK_BOX (window_vbox_right), sw, FALSE, FALSE, 0);

    /* Make room for horizontal scroll bar */
    gtk_widget_set_size_request (GTK_WIDGET (sw), WINDOW_W, PANEL_BOTTOM_H - 28);
    /* Create Code Block Tree List */
    gui.gcode_block_store = gtk_tree_store_new (6, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_POINTER);
    tree = GTK_TREE_MODEL (gui.gcode_block_store);

    /* create tree view */
    gui.gcode_block_treeview = gtk_tree_view_new_with_model (tree);
#if GTK_MAJOR_VERSION >= 2
#if GTK_MINOR_VERSION >= 10
    gtk_tree_view_set_enable_tree_lines (GTK_TREE_VIEW (gui.gcode_block_treeview), 1);
#endif
#endif
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (gui.gcode_block_treeview), TRUE);
    gtk_tree_view_set_search_column (GTK_TREE_VIEW (gui.gcode_block_treeview), 0);
    gtk_tree_view_set_reorderable (GTK_TREE_VIEW (gui.gcode_block_treeview), TRUE);
    GTK_TREE_DRAG_DEST_GET_IFACE (gui.gcode_block_store)->row_drop_possible = row_drop_possible;
    GTK_TREE_DRAG_DEST_GET_IFACE (gui.gcode_block_store)->drag_data_received = drag_data_received;


    g_object_unref (tree);
    g_signal_connect (G_OBJECT (gui.gcode_block_treeview), "cursor-changed", G_CALLBACK (gcode_tree_cursor_changed_event), NULL);
    g_signal_connect (G_OBJECT (gui.gcode_block_treeview), "row-collapsed", G_CALLBACK (gcode_tree_row_collapsed_event), NULL);
    gtk_container_add (GTK_CONTAINER (sw), gui.gcode_block_treeview);


    /* ORDER COLUMN */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Order", renderer, "text", 0, NULL);
/*    gtk_tree_view_column_set_sort_column_id (column, 0); */
    gtk_tree_view_append_column (GTK_TREE_VIEW (gui.gcode_block_treeview), column);

    /* TYPE COLUMN */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Type", renderer, "text", 1, NULL);
/*    gtk_tree_view_column_set_sort_column_id (column, 1); */
    gtk_tree_view_append_column (GTK_TREE_VIEW (gui.gcode_block_treeview), column);

    /* STATUS COLUMN */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Status", renderer, "text", 2, NULL);
/*    gtk_tree_view_column_set_sort_column_id (column, 2); */
    gtk_tree_view_append_column (GTK_TREE_VIEW (gui.gcode_block_treeview), column);

    /* SUPPRESS COLUMN */
    renderer = gtk_cell_renderer_toggle_new ();
    g_object_set_data (G_OBJECT (renderer), "column", (gint *) 3);
    column = gtk_tree_view_column_new_with_attributes ("Suppress", renderer, "active", 3, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (gui.gcode_block_treeview), column);
    g_signal_connect (renderer, "toggled", G_CALLBACK (suppress_toggled), tree);
/*
    g_object_set (renderer, "xalign", 0.0, NULL);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 50);
*/
    gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);


    /* COMMENT COLUMN */
    renderer = gtk_cell_renderer_text_new ();
    gui.comment_cell = renderer;
    column = gtk_tree_view_column_new_with_attributes ("Comment", renderer, "text", 4, NULL);
    g_object_set (renderer, "editable", TRUE, NULL);
    g_signal_connect (renderer, "edited", G_CALLBACK (comment_cell_edited), tree);
    g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (4));
/*    gtk_tree_view_column_set_sort_column_id (column, 4); */
    gtk_tree_view_append_column (GTK_TREE_VIEW (gui.gcode_block_treeview), column);
  }

  /* Progress Bar */
  {
    gui.progress_bar = gtk_progress_bar_new ();
    gtk_box_pack_end (GTK_BOX (window_vbox_main), gui.progress_bar, FALSE, FALSE, 0);
  }

/*  gtk_widget_set_size_request (GTK_WIDGET (pbar), 320, 20); */

  gtk_widget_show_all (gui.window);

  gui_menu_util_modified (&gui, 0); /* Set Title */

  gtk_main ();
}
