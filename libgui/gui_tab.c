/*
*  gui_tab.c
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
#include "gui_tab.h"
#include "gui_define.h"
#include "gui_endmills.h"
#include "gui_menu_util.h"
#include <unistd.h>


static void
generic_destroy_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;

  wlist = (GtkWidget **) data;
  free (wlist[0]); /* hids array */
  free (wlist);
}


static void
begin_update_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gulong *hids;
  gui_t *gui;
  gcode_block_t *block;
  gcode_begin_t *begin;
  uint16_t wind;

  wind = 0;
  wlist = (GtkWidget **) data;

  hids = (gulong *) wlist[wind];
  wind++;

  gui = (gui_t *) wlist[wind];
  wind++;

  block = (gcode_block_t *) wlist[wind];
  begin = (gcode_begin_t *) block->pdata;
  wind++;

  if (!strcmp ("None", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[wind]))))
  {
    begin->coordinate_system = GCODE_BEGIN_COORDINATE_SYSTEM_NONE;
  }
  else if (!strcmp ("Workspace 1 (G54)", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[wind]))))
  {
    begin->coordinate_system = GCODE_BEGIN_COORDINATE_SYSTEM_WORKSPACE1;
  }
  else if (!strcmp ("Workspace 2 (G55)", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[wind]))))
  {
    begin->coordinate_system = GCODE_BEGIN_COORDINATE_SYSTEM_WORKSPACE2;
  }
  else if (!strcmp ("Workspace 3 (G56)", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[wind]))))
  {
    begin->coordinate_system = GCODE_BEGIN_COORDINATE_SYSTEM_WORKSPACE3;
  }
  else if (!strcmp ("Workspace 4 (G57)", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[wind]))))
  {
    begin->coordinate_system = GCODE_BEGIN_COORDINATE_SYSTEM_WORKSPACE4;
  }
  else if (!strcmp ("Workspace 5 (G58)", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[wind]))))
  {
    begin->coordinate_system = GCODE_BEGIN_COORDINATE_SYSTEM_WORKSPACE5;
  }
  else if (!strcmp ("Workspace 6 (G59)", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[wind]))))
  {
    begin->coordinate_system = GCODE_BEGIN_COORDINATE_SYSTEM_WORKSPACE6;
  }

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);

  gui_menu_util_modified (gui, 1);
}


static void
gui_tab_begin (gui_t *gui, gcode_block_t *block)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *begin_tab;
  GtkWidget *alignment;
  GtkWidget *begin_table;
  GtkWidget *label;
  GtkWidget *cs_combo;
  gcode_begin_t *begin;
  uint16_t wind, row;

  /*
  * Begin Parameters
  */

  begin = (gcode_begin_t *) block->pdata;

  wind = 0;
  wlist = (GtkWidget **) malloc (4 * sizeof (GtkWidget *));
  hids = (gulong *) malloc (1 * sizeof (gulong));
  row = 0;

  begin_tab = gtk_frame_new ("Begin Parameters");
  g_signal_connect (begin_tab, "destroy", G_CALLBACK (generic_destroy_callback), wlist);
  gtk_container_add (GTK_CONTAINER (gui->panel_tab_vbox), begin_tab);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (begin_tab), alignment);

  begin_table = gtk_table_new (1, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (begin_table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (begin_table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (begin_table), 4);
  gtk_container_add (GTK_CONTAINER (alignment), begin_table);

  label = gtk_label_new ("Coord System");
  gtk_table_attach_defaults (GTK_TABLE (begin_table), label, 0, 1, row, row+1);
  cs_combo = gtk_combo_box_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (cs_combo), "None");
  gtk_combo_box_append_text (GTK_COMBO_BOX (cs_combo), "Workspace 1 (G54)");
  gtk_combo_box_append_text (GTK_COMBO_BOX (cs_combo), "Workspace 2 (G55)");
  gtk_combo_box_append_text (GTK_COMBO_BOX (cs_combo), "Workspace 3 (G56)");
  gtk_combo_box_append_text (GTK_COMBO_BOX (cs_combo), "Workspace 4 (G57)");
  gtk_combo_box_append_text (GTK_COMBO_BOX (cs_combo), "Workspace 5 (G58)");
  gtk_combo_box_append_text (GTK_COMBO_BOX (cs_combo), "Workspace 6 (G59)");
  gtk_combo_box_set_active (GTK_COMBO_BOX (cs_combo), begin->coordinate_system);
  hids[0] = g_signal_connect (cs_combo, "changed", G_CALLBACK (begin_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (begin_table), cs_combo, 1, 2, row, row+1);
  row++;

  wlist[wind++] = (GtkWidget *) hids;
  wlist[wind++] = (GtkWidget *) gui;
  wlist[wind++] = (GtkWidget *) block;
  wlist[wind++] = cs_combo;
}


static void
end_update_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gulong *hids;
  gui_t *gui;
  gcode_block_t *block;
  gcode_end_t *end;
  uint16_t wind;

  wind = 0;
  wlist = (GtkWidget **) data;

  hids = (gulong *) wlist[wind];
  wind++;

  gui = (gui_t *) wlist[wind];
  wind++;

  block = (gcode_block_t *) wlist[wind];
  end = (gcode_end_t *) block->pdata;
  wind++;

  end->pos[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[3]));
  end->pos[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[4]));
  end->pos[2] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[5]));

  end->home = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wlist[6]));

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);

  gui_menu_util_modified (gui, 1);
}


static void
gui_tab_end (gui_t *gui, gcode_block_t *block)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *end_tab;
  GtkWidget *alignment;
  GtkWidget *end_table;
  GtkWidget *label;
  GtkWidget *posx_spin;
  GtkWidget *posy_spin;
  GtkWidget *posz_spin;
  GtkWidget *home_check_button;
  gcode_end_t *end;
  uint16_t wind, row;

  /*
  * End Parameters
  */

  end = (gcode_end_t *) block->pdata;

  wind = 0;
  wlist = (GtkWidget **) malloc (7 * sizeof (GtkWidget *));
  hids = NULL;
  row = 0;

  end_tab = gtk_frame_new ("End Parameters");
  g_signal_connect (end_tab, "destroy", G_CALLBACK (generic_destroy_callback), wlist);
  gtk_container_add (GTK_CONTAINER (gui->panel_tab_vbox), end_tab);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (end_tab), alignment);

  end_table = gtk_table_new (3, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (end_table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (end_table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (end_table), 4);
  gtk_container_add (GTK_CONTAINER (alignment), end_table);

  label = gtk_label_new ("Retract(X)");
  posx_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, 0.0), GCODE_UNITS (block->gcode, 20.0), GCODE_UNITS (block->gcode, 0.1));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (posx_spin), 3);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (posx_spin), end->pos[0]);
  g_signal_connect (posx_spin, "value-changed", G_CALLBACK (end_update_callback), wlist);
  if ((block->gcode->machine_options & GCODE_MACHINE_OPTION_HOME_SWITCHES) == 0)
  {
    gtk_table_attach_defaults (GTK_TABLE (end_table), label, 0, 1, row, row+1);
    gtk_table_attach_defaults (GTK_TABLE (end_table), posx_spin, 1, 2, row, row+1);
    row++;
  }

  label = gtk_label_new ("Retract(Y)");
  posy_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, 0.0), GCODE_UNITS (block->gcode, 20.0), GCODE_UNITS (block->gcode, 0.1));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (posy_spin), 3);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (posy_spin), end->pos[1]);
  g_signal_connect (posy_spin, "value-changed", G_CALLBACK (end_update_callback), wlist);
  if ((block->gcode->machine_options & GCODE_MACHINE_OPTION_HOME_SWITCHES) == 0)
  {
    gtk_table_attach_defaults (GTK_TABLE (end_table), label, 0, 1, row, row+1);
    gtk_table_attach_defaults (GTK_TABLE (end_table), posy_spin, 1, 2, row, row+1);
    row++;
  }

  label = gtk_label_new ("Retract(Z)");
  posz_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, 0.0), GCODE_UNITS (block->gcode, 3.0), GCODE_UNITS (block->gcode, 0.1));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (posz_spin), 3);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (posz_spin), end->pos[2]);
  g_signal_connect (posz_spin, "value-changed", G_CALLBACK (end_update_callback), wlist);
  if ((block->gcode->machine_options & GCODE_MACHINE_OPTION_HOME_SWITCHES) == 0)
  {
    gtk_table_attach_defaults (GTK_TABLE (end_table), label, 0, 1, row, row+1);
    gtk_table_attach_defaults (GTK_TABLE (end_table), posz_spin, 1, 2, row, row+1);
    row++;
  }

  label = gtk_label_new ("Home all Axes (G28)");
  home_check_button = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (home_check_button), end->home);
  g_signal_connect (home_check_button, "toggled", G_CALLBACK (end_update_callback), wlist);
  if (block->gcode->machine_options & GCODE_MACHINE_OPTION_HOME_SWITCHES)
  {
    gtk_table_attach_defaults (GTK_TABLE (end_table), label, 0, 1, row, row+1);
    gtk_table_attach_defaults (GTK_TABLE (end_table), home_check_button, 1, 2, row, row+1);
    row++;
  }

  wlist[wind++] = (GtkWidget *) hids;
  wlist[wind++] = (GtkWidget *) gui;
  wlist[wind++] = (GtkWidget *) block;
  wlist[wind++] = posx_spin;
  wlist[wind++] = posy_spin;
  wlist[wind++] = posz_spin;
  wlist[wind++] = home_check_button;
}


static void
line_update_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gulong *hids;
  gui_t *gui;
  gcode_block_t *block;
  gcode_line_t *line;
  gfloat_t clength, nlength, cangle, nangle;
  gcode_vec2d_t p0, p1, vec;

  wlist = (GtkWidget **) data;

  hids = (gulong *) wlist[0];
  gui = (gui_t *) wlist[1];

  block = (gcode_block_t *) wlist[2];
  line = (gcode_line_t *) block->pdata;


  p0[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[3]));
  p0[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[4]));
  p1[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[5]));
  p1[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[6]));

  /* If a value for the position changed then update them otherwise check length and angle */
  if (fabs (p0[0] - line->p0[0]) > GCODE_PRECISION ||
      fabs (p0[1] - line->p0[1]) > GCODE_PRECISION ||
      fabs (p1[0] - line->p1[0]) > GCODE_PRECISION ||
      fabs (p1[1] - line->p1[1]) > GCODE_PRECISION)
  {
    gfloat_t mag;

    /* Check the distance between new points - only proceed if length is non zero */
    GCODE_MATH_VEC2D_SUB (vec, p0, p1);
    GCODE_MATH_VEC2D_MAG (mag, vec);

    if (mag < GCODE_PRECISION)
    {
      g_signal_handler_block (wlist[3], hids[3-3]);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[3]), line->p0[0]);
      g_signal_handler_unblock (wlist[3], hids[3-3]);

      g_signal_handler_block (wlist[4], hids[4-3]);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[4]), line->p0[1]);
      g_signal_handler_unblock (wlist[4], hids[4-3]);

      g_signal_handler_block (wlist[5], hids[5-3]);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[5]), line->p1[0]);
      g_signal_handler_unblock (wlist[5], hids[5-3]);

      g_signal_handler_block (wlist[6], hids[6-3]);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[6]), line->p1[1]);
      g_signal_handler_unblock (wlist[6], hids[6-3]);
      return;
    }

    line->p0[0] = p0[0];
    line->p0[1] = p0[1];
    line->p1[0] = p1[0];
    line->p1[1] = p1[1];

    clength = sqrt ((line->p1[0] - line->p0[0])*(line->p1[0] - line->p0[0]) + (line->p1[1] - line->p0[1])*(line->p1[1] - line->p0[1]));
    gcode_math_xy_to_angle (line->p0, clength, line->p1[0], line->p1[1], &cangle);


    g_signal_handler_block (wlist[7], hids[7-3]);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[7]), clength);
    g_signal_handler_unblock (wlist[7], hids[7-3]);

    g_signal_handler_block (wlist[8], hids[8-3]);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[8]), cangle);
    g_signal_handler_unblock (wlist[8], hids[8-3]);
  }
  else
  {
    clength = sqrt ((line->p1[0] - line->p0[0])*(line->p1[0] - line->p0[0]) + (line->p1[1] - line->p0[1])*(line->p1[1] - line->p0[1]));
    gcode_math_xy_to_angle (line->p0, clength, line->p1[0], line->p1[1], &cangle);


    nlength = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[7]));
    nangle = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[8]));

    if (fabs (clength - nlength) > GCODE_PRECISION)
    {
      GCODE_MATH_VEC2D_SUB (vec, line->p1, line->p0);
      GCODE_MATH_VEC2D_UNITIZE (vec);
      GCODE_MATH_VEC2D_MUL_SCALAR (vec, vec, nlength);
      line->p1[0] = line->p0[0] + vec[0];
      line->p1[1] = line->p0[1] + vec[1];

      g_signal_handler_block (wlist[5], hids[5-3]);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[5]), line->p1[0]);
      g_signal_handler_unblock (wlist[5], hids[5-3]);

      g_signal_handler_block (wlist[6], hids[6-3]);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[6]), line->p1[1]);
      g_signal_handler_unblock (wlist[6], hids[6-3]);
    }

    if (fabs (cangle - nangle) > GCODE_PRECISION)
    {
      line->p1[0] = line->p0[0] + clength * cos (nangle * GCODE_DEG2RAD);
      line->p1[1] = line->p0[1] + clength * sin (nangle * GCODE_DEG2RAD);

      g_signal_handler_block (wlist[5], hids[5-3]);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[5]), line->p1[0]);
      g_signal_handler_unblock (wlist[5], hids[5-3]);

      g_signal_handler_block (wlist[6], hids[6-3]);
      gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[6]), line->p1[1]);
      g_signal_handler_unblock (wlist[6], hids[6-3]);
    }
  }

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);

  gui_menu_util_modified (gui, 1);
}


static void
gui_tab_line (gui_t *gui, gcode_block_t *block)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *line_tab;
  GtkWidget *alignment;
  GtkWidget *line_table;
  GtkWidget *label;
  GtkWidget *p0x_spin;
  GtkWidget *p0y_spin;
  GtkWidget *p1x_spin;
  GtkWidget *p1y_spin;
  GtkWidget *length_spin;
  GtkWidget *angle_spin;
  gcode_line_t *line;
  uint16_t wind, row;
  gfloat_t length, angle;

  /*
  * Line Parameters
  */

  line = (gcode_line_t *) block->pdata;

  wind = 0;
  wlist = (GtkWidget **) malloc (9 * sizeof (GtkWidget *));
  hids = (gulong *) malloc (6 * sizeof (gulong));
  row = 0;

  line_tab = gtk_frame_new ("Line Parameters");
  g_signal_connect (line_tab, "destroy", G_CALLBACK (generic_destroy_callback), wlist);
  gtk_container_add (GTK_CONTAINER (gui->panel_tab_vbox), line_tab);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (line_tab), alignment);

  line_table = gtk_table_new (4, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (line_table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (line_table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (line_table), 4);
  gtk_container_add (GTK_CONTAINER (alignment), line_table);

  if (block->parent->type == GCODE_TYPE_EXTRUSION)
  {
    label = gtk_label_new ("P0(X)");
    gtk_table_attach_defaults (GTK_TABLE (line_table), label, 0, 1, row, row+1);
    p0x_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (p0x_spin), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (p0x_spin), line->p0[0]);
    hids[0] = g_signal_connect (p0x_spin, "value-changed", G_CALLBACK (line_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (line_table), p0x_spin, 1, 2, row, row+1);
    row++;

    label = gtk_label_new ("P0(Y)");
    gtk_table_attach_defaults (GTK_TABLE (line_table), label, 0, 1, row, row+1);
    p0y_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_Z), GCODE_UNITS (block->gcode, MAX_DIM_Z), GCODE_UNITS (block->gcode, 0.01));
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (p0y_spin), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (p0y_spin), line->p0[1]);
    hids[1] = g_signal_connect (p0y_spin, "value-changed", G_CALLBACK (line_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (line_table), p0y_spin, 1, 2, row, row+1);
    row++;

    label = gtk_label_new ("P1(X)");
    gtk_table_attach_defaults (GTK_TABLE (line_table), label, 0, 1, row, row+1);
    p1x_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (p1x_spin), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (p1x_spin), line->p1[0]);
    hids[2] = g_signal_connect (p1x_spin, "value-changed", G_CALLBACK (line_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (line_table), p1x_spin, 1, 2, row, row+1);
    row++;

    label = gtk_label_new ("P1(Y)");
    gtk_table_attach_defaults (GTK_TABLE (line_table), label, 0, 1, row, row+1);
    p1y_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_Z), GCODE_UNITS (block->gcode, MAX_DIM_Z), GCODE_UNITS (block->gcode, 0.01));
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (p1y_spin), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (p1y_spin), line->p1[1]);
    hids[3] = g_signal_connect (p1y_spin, "value-changed", G_CALLBACK (line_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (line_table), p1y_spin, 1, 2, row, row+1);
    row++;

    length = sqrt ((line->p1[0] - line->p0[0])*(line->p1[0] - line->p0[0]) + (line->p1[1] - line->p0[1])*(line->p1[1] - line->p0[1]));
    label = gtk_label_new ("Length");
    gtk_table_attach_defaults (GTK_TABLE (line_table), label, 0, 1, row, row+1);
    length_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, GCODE_PRECISION), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (length_spin), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (length_spin), length);
    hids[4] = g_signal_connect (length_spin, "value-changed", G_CALLBACK (line_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (line_table), length_spin, 1, 2, row, row+1);
    row++;

    gcode_math_xy_to_angle (line->p0, length, line->p1[0], line->p1[1], &angle);
    label = gtk_label_new ("Angle");
    gtk_table_attach_defaults (GTK_TABLE (line_table), label, 0, 1, row, row+1);
    angle_spin = gtk_spin_button_new_with_range (0.0, 360.0, 0.01);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (angle_spin), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (angle_spin), angle);
    hids[5] = g_signal_connect (angle_spin, "value-changed", G_CALLBACK (line_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (line_table), angle_spin, 1, 2, row, row+1);
    row++;
  }
  else
  {
    label = gtk_label_new ("P0(X)");
    gtk_table_attach_defaults (GTK_TABLE (line_table), label, 0, 1, row, row+1);
    p0x_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (p0x_spin), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (p0x_spin), line->p0[0]);
    hids[0] = g_signal_connect (p0x_spin, "value-changed", G_CALLBACK (line_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (line_table), p0x_spin, 1, 2, row, row+1);
    row++;

    label = gtk_label_new ("P0(Y)");
    gtk_table_attach_defaults (GTK_TABLE (line_table), label, 0, 1, row, row+1);
    p0y_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_Y), GCODE_UNITS (block->gcode, MAX_DIM_Y), GCODE_UNITS (block->gcode, 0.01));
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (p0y_spin), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (p0y_spin), line->p0[1]);
    hids[1] = g_signal_connect (p0y_spin, "value-changed", G_CALLBACK (line_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (line_table), p0y_spin, 1, 2, row, row+1);
    row++;

    label = gtk_label_new ("P1(X)");
    gtk_table_attach_defaults (GTK_TABLE (line_table), label, 0, 1, row, row+1);
    p1x_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (p1x_spin), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (p1x_spin), line->p1[0]);
    hids[2] = g_signal_connect (p1x_spin, "value-changed", G_CALLBACK (line_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (line_table), p1x_spin, 1, 2, row, row+1);
    row++;

    label = gtk_label_new ("P1(Y)");
    gtk_table_attach_defaults (GTK_TABLE (line_table), label, 0, 1, row, row+1);
    p1y_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_Y), GCODE_UNITS (block->gcode, MAX_DIM_Y), GCODE_UNITS (block->gcode, 0.01));
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (p1y_spin), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (p1y_spin), line->p1[1]);
    hids[3] = g_signal_connect (p1y_spin, "value-changed", G_CALLBACK (line_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (line_table), p1y_spin, 1, 2, row, row+1);
    row++;

    length = sqrt ((line->p1[0] - line->p0[0])*(line->p1[0] - line->p0[0]) + (line->p1[1] - line->p0[1])*(line->p1[1] - line->p0[1]));
    label = gtk_label_new ("Length");
    gtk_table_attach_defaults (GTK_TABLE (line_table), label, 0, 1, row, row+1);
    length_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, GCODE_PRECISION), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (length_spin), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (length_spin), length);
    hids[4] = g_signal_connect (length_spin, "value-changed", G_CALLBACK (line_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (line_table), length_spin, 1, 2, row, row+1);
    row++;

    gcode_math_xy_to_angle (line->p0, length, line->p1[0], line->p1[1], &angle);
    label = gtk_label_new ("Angle");
    gtk_table_attach_defaults (GTK_TABLE (line_table), label, 0, 1, row, row+1);
    angle_spin = gtk_spin_button_new_with_range (0.0, 360.0, 0.01);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (angle_spin), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (angle_spin), angle);
    hids[5] = g_signal_connect (angle_spin, "value-changed", G_CALLBACK (line_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (line_table), angle_spin, 1, 2, row, row+1);
    row++;
  }

  wlist[wind++] = (GtkWidget *) hids;
  wlist[wind++] = (GtkWidget *) gui;
  wlist[wind++] = (GtkWidget *) block;
  wlist[wind++] = p0x_spin;
  wlist[wind++] = p0y_spin;
  wlist[wind++] = p1x_spin;
  wlist[wind++] = p1y_spin;
  wlist[wind++] = length_spin;
  wlist[wind++] = angle_spin;
}


static void
arc_update_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gulong *hids;
  gui_t *gui;
  gcode_block_t *block;
  gcode_arc_t *arc;

  wlist = (GtkWidget **) data;

  hids = (gulong *) wlist[0];
  gui = (gui_t *) wlist[1];
  block = (gcode_block_t *) wlist[2];
  arc = (gcode_arc_t *) block->pdata;

  if (!strcmp ("Sweep", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[3]))))
  {
    if (wlist[25])
    {
      /* Store the x, y, radius, sweep */
      arc->pos[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[7]));
      arc->pos[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[8]));
      arc->radius = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[9]));
      arc->start_angle = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[10]));
      arc->sweep = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[11]));
    }
    wlist[25] = (GtkWidget *) 1;
  }
  else if (!strcmp ("Radius", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[3]))))
  {
    gcode_vec2d_t vec, end_pos, center[2];
    gfloat_t dist, inv_mag, delta_angle, angle, test_angles[4];

    end_pos[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[15]));
    end_pos[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[16]));
    /*
    * Find the intersection points between 2 circles whose centers allow for
    * the start and end positions exist on the circle.
    * Choose the arc that results in the shortest length given the specified
    * sweep direction (CW/CCW).
    */
    vec[0] = end_pos[0] - gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[13]));
    vec[1] = end_pos[1] - gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[14]));
    dist = sqrt (vec[0]*vec[0] + vec[1]*vec[1]);
    /*
    * Check that the distance between start and end is <= to the diameter, otherwise
    * the circle is undefined.
    */
    if (dist <= 2.0 * gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[17])))
    {
      arc->pos[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[13]));
      arc->pos[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[14]));
      arc->radius = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[17]));

      inv_mag = 1.0 / dist;
      vec[0] *= inv_mag;
      vec[1] *= inv_mag;

      /* The angular sweep between start and end is the arc cos of distance over diameter. */
      delta_angle = GCODE_RAD2DEG * acos (dist / (2.0 * arc->radius));

      center[0][0] = 0.0;
      center[0][1] = 0.0;
      gcode_math_xy_to_angle (center[0], 1.0, vec[0], vec[1], &angle);

      center[0][0] = arc->pos[0] + arc->radius * cos ((angle - delta_angle) * GCODE_DEG2RAD);
      center[0][1] = arc->pos[1] + arc->radius * sin ((angle - delta_angle) * GCODE_DEG2RAD);
      center[1][0] = arc->pos[0] + arc->radius * cos ((angle + delta_angle) * GCODE_DEG2RAD);
      center[1][1] = arc->pos[1] + arc->radius * sin ((angle + delta_angle) * GCODE_DEG2RAD);

      gcode_math_xy_to_angle (center[0], arc->radius, arc->pos[0], arc->pos[1], &test_angles[0]);
      gcode_math_xy_to_angle (center[0], arc->radius, end_pos[0], end_pos[1], &test_angles[1]);
      gcode_math_xy_to_angle (center[1], arc->radius, arc->pos[0], arc->pos[1], &test_angles[2]);
      gcode_math_xy_to_angle (center[1], arc->radius, end_pos[0], end_pos[1], &test_angles[3]);

      /* Handles the case of test_angle[0] being 0.0 and test_angles[1] being 90.0 */
      if (test_angles[0] < GCODE_PRECISION)
        test_angles[0] += 360.0;
      if (test_angles[1] < GCODE_PRECISION)
        test_angles[1] += 360.0;
      if (test_angles[2] < GCODE_PRECISION)
        test_angles[2] += 360.0;
      if (test_angles[3] < GCODE_PRECISION)
        test_angles[3] += 360.0;

      if (!strcmp ("CW", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[12]))))
      {
        if (test_angles[0] - test_angles[1] > 0.0)
        {
          arc->start_angle = test_angles[0];
          arc->sweep = test_angles[1] - test_angles[0];
        }
        else
        {
          arc->start_angle = test_angles[2];
          if (test_angles[3] - test_angles[2] >= 180.0)
            test_angles[2] += 360.0;
          arc->sweep = test_angles[3] - test_angles[2];
        }
      }
      else
      {
        if (test_angles[0] - test_angles[1] < 0.0)
        {
          arc->start_angle = test_angles[0];
          arc->sweep = test_angles[1] - test_angles[2];
        }
        else
        {
          if (test_angles[2] - test_angles[3] >= 180.0)
            test_angles[3] += 360.0;
          arc->start_angle = test_angles[2];
          arc->sweep = test_angles[3] - test_angles[2];
        }
      }
    }

    wlist[25] = (GtkWidget *) 0;
  }
  else if (!strcmp ("Center", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[3]))))
  {
    gcode_vec2d_t center, end_pos;
    gfloat_t end_angle;

    arc->pos[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[19]));
    arc->pos[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[20]));
    end_pos[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[21]));
    end_pos[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[22]));
    center[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[23]));
    center[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[24]));
    arc->radius = sqrt ((arc->pos[0] - center[0])*(arc->pos[0] - center[0]) + (arc->pos[1] - center[1])*(arc->pos[1] - center[1]));

    gcode_math_xy_to_angle (center, arc->radius, arc->pos[0], arc->pos[1], &arc->start_angle);
    gcode_math_xy_to_angle (center, arc->radius, end_pos[0], end_pos[1], &end_angle);

    arc->sweep = end_angle - arc->start_angle;
    if (!strcmp ("CW", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[18]))))
    {
      if (arc->sweep > 0.0)
        arc->sweep -= 360.0;
    }
    else
    {
      if (arc->sweep < 0.0)
        arc->sweep += 360.0;
    }

    wlist[24] = (GtkWidget *) 0;
  }

/*  arc->auto_join = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (auto_join_check_button)); */

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);

  gui_menu_util_modified (gui, 1);
}


static void
arc_interface_update_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gulong *hids;
  gcode_block_t *block;
  gcode_arc_t *arc;

  wlist = (GtkWidget **) data;
  hids = (gulong *) wlist[0];
  block = (gcode_block_t *) wlist[2];
  arc = (gcode_arc_t *) block->pdata;

  if (!strcmp ("Sweep", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[3]))))
  {
    arc->interface = GCODE_ARC_INTERFACE_SWEEP;
    gtk_widget_set_child_visible (wlist[4], 1);
    gtk_widget_set_child_visible (wlist[5], 0);
    gtk_widget_set_child_visible (wlist[6], 0);

    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[7]), arc->pos[0]);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[8]), arc->pos[1]);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[9]), arc->radius);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[10]), arc->start_angle);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[11]), arc->sweep);
  }
  else if (!strcmp ("Radius", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[3]))))
  {
    gcode_vec2d_t origin, center, p0;
    gfloat_t arc_radius_offset, start_angle;

    arc->interface = GCODE_ARC_INTERFACE_RADIUS;
    gtk_widget_set_child_visible (wlist[4], 0);
    gtk_widget_set_child_visible (wlist[5], 1);
    gtk_widget_set_child_visible (wlist[6], 0);

    gcode_arc_with_offset (block, origin, center, p0, &arc_radius_offset, &start_angle);

    gtk_combo_box_set_active (GTK_COMBO_BOX (wlist[12]), arc->sweep > 0 ? 1 : 0);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[13]), arc->pos[0]);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[14]), arc->pos[1]);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[15]), p0[0]);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[16]), p0[1]);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[17]), arc->radius);
  }
  else if (!strcmp ("Center", gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[3]))))
  {
    gcode_vec2d_t origin, center, p0;
    gfloat_t arc_radius_offset, start_angle;

    arc->interface = GCODE_ARC_INTERFACE_CENTER;
    gtk_widget_set_child_visible (wlist[4], 0);
    gtk_widget_set_child_visible (wlist[5], 0);
    gtk_widget_set_child_visible (wlist[6], 1);

    gcode_arc_with_offset (block, origin, center, p0, &arc_radius_offset, &start_angle);

    gtk_combo_box_set_active (GTK_COMBO_BOX (wlist[18]), arc->sweep > 0 ? 1 : 0);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[19]), arc->pos[0]);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[20]), arc->pos[1]);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[21]), p0[0]);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[22]), p0[1]);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[23]), center[0]);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (wlist[24]), center[1]);
  }
}


static void
gui_tab_arc (gui_t *gui, gcode_block_t *block)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *arc_tab;
  GtkWidget *alignment;
  GtkWidget *arc_table;
  GtkWidget *arc_sweep_table;
  GtkWidget *arc_radius_table;
  GtkWidget *arc_center_table;
  GtkWidget *label;
  GtkWidget *arc_interface_combo;
  GtkWidget *sweep_start_posx_spin;
  GtkWidget *sweep_start_posy_spin;
  GtkWidget *sweep_radius_spin;
  GtkWidget *sweep_start_angle_spin;
  GtkWidget *sweep_sweep_spin;
  GtkWidget *radius_direction_combo;
  GtkWidget *radius_start_posx_spin;
  GtkWidget *radius_start_posy_spin;
  GtkWidget *radius_end_posx_spin;
  GtkWidget *radius_end_posy_spin;
  GtkWidget *radius_radius_spin;
  GtkWidget *center_direction_combo;
  GtkWidget *center_start_posx_spin;
  GtkWidget *center_start_posy_spin;
  GtkWidget *center_end_posx_spin;
  GtkWidget *center_end_posy_spin;
  GtkWidget *center_center_posx_spin;
  GtkWidget *center_center_posy_spin;
  GtkWidget *radius_apply_changes_button;
  GtkWidget *center_apply_changes_button;
  gcode_arc_t *arc;
  gcode_vec2d_t origin, center, p0;
  gfloat_t arc_radius_offset, start_angle;
  uint16_t wind, row;

  /*
  * Arc Parameters
  */
  arc = (gcode_arc_t *) block->pdata;

  gcode_arc_with_offset (block, origin, center, p0, &arc_radius_offset, &start_angle);

  wind = 0;
  wlist = (GtkWidget **) malloc (26 * sizeof (GtkWidget *));
  hids = (gulong *) malloc (23 * sizeof (gulong));
  row = 0;
  /*
  * Counter for updating sweep interface because the widget change signal is called before,
  * the combo change signal, thus reverting the original changes of the sweep back.
  */
  wlist[25] = (GtkWidget *) 1;

  wlist[wind++] = (GtkWidget *) hids;
  wlist[wind++] = (GtkWidget *) gui;
  wlist[wind++] = (GtkWidget *) block;

  arc_tab = gtk_frame_new ("Arc Parameters");
  g_signal_connect (arc_tab, "destroy", G_CALLBACK (generic_destroy_callback), wlist);
  gtk_container_add (GTK_CONTAINER (gui->panel_tab_vbox), arc_tab);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (arc_tab), alignment);

  arc_table = gtk_table_new (8, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (arc_table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (arc_table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (arc_table), 4);
  gtk_container_add (GTK_CONTAINER (alignment), arc_table);

  label = gtk_label_new ("Arc Interface");
  gtk_table_attach_defaults (GTK_TABLE (arc_table), label, 0, 1, row, row+1);
  arc_interface_combo = gtk_combo_box_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (arc_interface_combo), "Sweep");
  gtk_combo_box_append_text (GTK_COMBO_BOX (arc_interface_combo), "Radius");
  gtk_combo_box_append_text (GTK_COMBO_BOX (arc_interface_combo), "Center");
  gtk_combo_box_set_active (GTK_COMBO_BOX (arc_interface_combo), arc->interface);
  g_signal_connect (arc_interface_combo, "changed", G_CALLBACK (arc_interface_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (arc_table), arc_interface_combo, 1, 2, row, row+1);
  row++;
  wlist[wind++] = arc_interface_combo;

  arc_sweep_table = gtk_table_new (5, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (arc_sweep_table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (arc_sweep_table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (arc_sweep_table), 0);
  gtk_table_attach_defaults (GTK_TABLE (arc_table), arc_sweep_table, 0, 2, row, row+1);

  arc_radius_table = gtk_table_new (5, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (arc_radius_table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (arc_radius_table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (arc_radius_table), 0);
  gtk_table_attach_defaults (GTK_TABLE (arc_table), arc_radius_table, 0, 2, row, row+1);

  arc_center_table = gtk_table_new (5, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (arc_center_table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (arc_center_table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (arc_center_table), 0);
  gtk_table_attach_defaults (GTK_TABLE (arc_table), arc_center_table, 0, 2, row, row+1);

  wlist[wind++] = arc_sweep_table;
  wlist[wind++] = arc_radius_table;
  wlist[wind++] = arc_center_table;

  gtk_widget_set_child_visible (wlist[4], 0);
  gtk_widget_set_child_visible (wlist[5], 0);
  gtk_widget_set_child_visible (wlist[6], 0);
  gtk_widget_set_child_visible (wlist[arc->interface+4], 1);


/*  arc_interface_create_table (arc->interface, tlist); */

  /*
  * SWEEP INTERFACE
  */
  row = 0;
  label = gtk_label_new ("Start Pos(X)");
  gtk_table_attach_defaults (GTK_TABLE (arc_sweep_table), label, 0, 1, row, row+1);
  sweep_start_posx_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (sweep_start_posx_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (sweep_start_posx_spin), arc->pos[0]);
  g_signal_connect (sweep_start_posx_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (arc_sweep_table), sweep_start_posx_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Start Pos(Y)");
  gtk_table_attach_defaults (GTK_TABLE (arc_sweep_table), label, 0, 1, row, row+1);
  sweep_start_posy_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_Y), GCODE_UNITS (block->gcode, MAX_DIM_Y), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (sweep_start_posy_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (sweep_start_posy_spin), arc->pos[1]);
  g_signal_connect (sweep_start_posy_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (arc_sweep_table), sweep_start_posy_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Radius");
  gtk_table_attach_defaults (GTK_TABLE (arc_sweep_table), label, 0, 1, row, row+1);
  sweep_radius_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (sweep_radius_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (sweep_radius_spin), arc->radius);
  g_signal_connect (sweep_radius_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (arc_sweep_table), sweep_radius_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Start Angle");
  gtk_table_attach_defaults (GTK_TABLE (arc_sweep_table), label, 0, 1, row, row+1);
  sweep_start_angle_spin = gtk_spin_button_new_with_range (0.0, 360.0, 0.1);
/*    gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (sweep_start_angle_spin), TRUE); */
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (sweep_start_angle_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (sweep_start_angle_spin), arc->start_angle);
  g_signal_connect (sweep_start_angle_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (arc_sweep_table), sweep_start_angle_spin, 1, 2, row, row+1);
  row++;

  if (block->parent->type == GCODE_TYPE_EXTRUSION)
  {
    label = gtk_label_new ("Sweep");
    gtk_table_attach_defaults (GTK_TABLE (arc_sweep_table), label, 0, 1, row, row+1);
    sweep_sweep_spin = gtk_spin_button_new_with_range (-90.0, 90.0, 1.0);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (sweep_sweep_spin), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (sweep_sweep_spin), arc->sweep);
    g_signal_connect (sweep_sweep_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (arc_sweep_table), sweep_sweep_spin, 1, 2, row, row+1);
    row++;
  }
  else
  {
    label = gtk_label_new ("Sweep");
    gtk_table_attach_defaults (GTK_TABLE (arc_sweep_table), label, 0, 1, row, row+1);
    sweep_sweep_spin = gtk_spin_button_new_with_range (-360.0, 360.0, 1.0);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (sweep_sweep_spin), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (sweep_sweep_spin), arc->sweep);
    g_signal_connect (sweep_sweep_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (arc_sweep_table), sweep_sweep_spin, 1, 2, row, row+1);
    row++;
  }

  wlist[wind++] = sweep_start_posx_spin;
  wlist[wind++] = sweep_start_posy_spin;
  wlist[wind++] = sweep_radius_spin;
  wlist[wind++] = sweep_start_angle_spin;
  wlist[wind++] = sweep_sweep_spin;

  /*
  * RADIUS INTERFACE
  */
  row = 0;
  label = gtk_label_new ("Direction");
  gtk_table_attach_defaults (GTK_TABLE (arc_radius_table), label, 0, 1, row, row+1);
  radius_direction_combo = gtk_combo_box_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (radius_direction_combo), "CW");
  gtk_combo_box_append_text (GTK_COMBO_BOX (radius_direction_combo), "CCW");
  gtk_combo_box_set_active (GTK_COMBO_BOX (radius_direction_combo), arc->sweep > 0 ? 1 : 0);
/*  g_signal_connect (radius_direction_combo, "changed", G_CALLBACK (arc_update_callback), wlist); */
  gtk_table_attach_defaults (GTK_TABLE (arc_radius_table), radius_direction_combo, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Start Pos(X)");
  gtk_table_attach_defaults (GTK_TABLE (arc_radius_table), label, 0, 1, row, row+1);
  radius_start_posx_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (radius_start_posx_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (radius_start_posx_spin), arc->pos[0]);
/*  g_signal_connect (radius_start_posx_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist); */
  gtk_table_attach_defaults (GTK_TABLE (arc_radius_table), radius_start_posx_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Start Pos(Y)");
  gtk_table_attach_defaults (GTK_TABLE (arc_radius_table), label, 0, 1, row, row+1);
  radius_start_posy_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_Y), GCODE_UNITS (block->gcode, MAX_DIM_Y), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (radius_start_posy_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (radius_start_posy_spin), arc->pos[1]);
/*  g_signal_connect (radius_start_posy_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist); */
  gtk_table_attach_defaults (GTK_TABLE (arc_radius_table), radius_start_posy_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("End Pos(X)");
  gtk_table_attach_defaults (GTK_TABLE (arc_radius_table), label, 0, 1, row, row+1);
  radius_end_posx_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (radius_end_posx_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (radius_end_posx_spin), p0[0]);
/*  g_signal_connect (radius_end_posx_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist); */
  gtk_table_attach_defaults (GTK_TABLE (arc_radius_table), radius_end_posx_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("End Pos(Y)");
  gtk_table_attach_defaults (GTK_TABLE (arc_radius_table), label, 0, 1, row, row+1);
  radius_end_posy_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_Y), GCODE_UNITS (block->gcode, MAX_DIM_Y), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (radius_end_posy_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (radius_end_posy_spin), p0[1]);
/*  g_signal_connect (radius_end_posy_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist); */
  gtk_table_attach_defaults (GTK_TABLE (arc_radius_table), radius_end_posy_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Radius");
  gtk_table_attach_defaults (GTK_TABLE (arc_radius_table), label, 0, 1, row, row+1);
  radius_radius_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (radius_radius_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (radius_radius_spin), arc->radius);
/*  g_signal_connect (radius_radius_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist); */
  gtk_table_attach_defaults (GTK_TABLE (arc_radius_table), radius_radius_spin, 1, 2, row, row+1);
  row++;

  radius_apply_changes_button = gtk_button_new_with_label ("Apply Changes");
  g_signal_connect (G_OBJECT (radius_apply_changes_button), "clicked", G_CALLBACK (arc_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (arc_radius_table), radius_apply_changes_button, 0, 2, row, row+1);

  wlist[wind++] = radius_direction_combo;
  wlist[wind++] = radius_start_posx_spin;
  wlist[wind++] = radius_start_posy_spin;
  wlist[wind++] = radius_end_posx_spin;
  wlist[wind++] = radius_end_posy_spin;
  wlist[wind++] = radius_radius_spin;


  /*
  * CENTER INTERFACE
  */
  row = 0;
  label = gtk_label_new ("Direction");
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), label, 0, 1, row, row+1);
  center_direction_combo = gtk_combo_box_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (center_direction_combo), "CW");
  gtk_combo_box_append_text (GTK_COMBO_BOX (center_direction_combo), "CCW");
  gtk_combo_box_set_active (GTK_COMBO_BOX (center_direction_combo), arc->sweep > 0 ? 1 : 0);
/*  g_signal_connect (center_direction_combo, "changed", G_CALLBACK (arc_update_callback), wlist); */
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), center_direction_combo, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Start Pos(X)");
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), label, 0, 1, row, row+1);
  center_start_posx_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (center_start_posx_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (center_start_posx_spin), arc->pos[0]);
/*  g_signal_connect (center_start_posx_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist); */
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), center_start_posx_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Start Pos(Y)");
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), label, 0, 1, row, row+1);
  center_start_posy_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_Y), GCODE_UNITS (block->gcode, MAX_DIM_Y), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (center_start_posy_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (center_start_posy_spin), arc->pos[1]);
/*  g_signal_connect (center_start_posy_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist); */
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), center_start_posy_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("End Pos(X)");
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), label, 0, 1, row, row+1);
  center_end_posx_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (center_end_posx_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (center_end_posx_spin), p0[0]);
/*  g_signal_connect (center_end_posx_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist); */
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), center_end_posx_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("End Pos(Y)");
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), label, 0, 1, row, row+1);
  center_end_posy_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_Y), GCODE_UNITS (block->gcode, MAX_DIM_Y), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (center_end_posy_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (center_end_posy_spin), p0[1]);
/*  g_signal_connect (center_end_posy_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist); */
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), center_end_posy_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Center Pos(X)");
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), label, 0, 1, row, row+1);
  center_center_posx_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (center_center_posx_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (center_center_posx_spin), center[0]);
/*  g_signal_connect (center_center_posx_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist); */
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), center_center_posx_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Center Pos(Y)");
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), label, 0, 1, row, row+1);
  center_center_posy_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_Y), GCODE_UNITS (block->gcode, MAX_DIM_Y), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (center_center_posy_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (center_center_posy_spin), center[1]);
/*  g_signal_connect (center_center_posy_spin, "value-changed", G_CALLBACK (arc_update_callback), wlist); */
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), center_center_posy_spin, 1, 2, row, row+1);
  row++;

  center_apply_changes_button = gtk_button_new_with_label ("Apply Changes");
  g_signal_connect (G_OBJECT (center_apply_changes_button), "clicked", G_CALLBACK (arc_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (arc_center_table), center_apply_changes_button, 0, 2, row, row+1);

  wlist[wind++] = center_direction_combo;
  wlist[wind++] = center_start_posx_spin;
  wlist[wind++] = center_start_posy_spin;
  wlist[wind++] = center_end_posx_spin;
  wlist[wind++] = center_end_posy_spin;
  wlist[wind++] = center_center_posx_spin;
  wlist[wind++] = center_center_posy_spin;

#if 0
  set_sweep_button = gtk_button_new_with_label ("Set Sweep");
  g_signal_connect (G_OBJECT (set_sweep_button), "clicked", G_CALLBACK (arc_set_sweep_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (arc_table), set_sweep_button, 0, 2, row, row+1);
#endif
}


static void
bolt_holes_update_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *posx_spin;
  GtkWidget *posy_spin;
  GtkWidget *hole_diameter_spin;
  GtkWidget *offset_distance_spin;
  GtkWidget *type_combo;
  GtkWidget *generic_spin[2];
  GtkWidget *pocket_check_button;
  gui_t *gui;
  gcode_block_t *block;
  gcode_bolt_holes_t *bolt_holes;


  wlist = (GtkWidget **) data;

  hids = (gulong *) wlist[0];

  gui = (gui_t *) wlist[1];

  block = (gcode_block_t *) wlist[2];
  bolt_holes = (gcode_bolt_holes_t *) block->pdata;

  posx_spin = wlist[3];
  posy_spin = wlist[4];
  hole_diameter_spin = wlist[5];
  offset_distance_spin = wlist[6];
  type_combo = wlist[7];
  generic_spin[0] = wlist[9];
  generic_spin[1] = wlist[11];
  pocket_check_button = wlist[12];

  bolt_holes->pos[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (posx_spin));
  bolt_holes->pos[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (posy_spin));
  bolt_holes->hole_diameter = gtk_spin_button_get_value (GTK_SPIN_BUTTON (hole_diameter_spin));
  bolt_holes->offset_distance = gtk_spin_button_get_value (GTK_SPIN_BUTTON (offset_distance_spin));


  if (!strcmp ("Radial", gtk_combo_box_get_active_text (GTK_COMBO_BOX (type_combo))) && bolt_holes->type != GCODE_BOLT_HOLES_TYPE_RADIAL)
  {
    bolt_holes->type = GCODE_BOLT_HOLES_TYPE_RADIAL;

    /* Update the labels and spin buttons */
    gtk_label_set_text (GTK_LABEL (wlist[8]), "Hole Num");
    gtk_spin_button_set_range (GTK_SPIN_BUTTON (generic_spin[0]), 1, 100);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (generic_spin[0]), 0);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (generic_spin[0]), bolt_holes->num[0]);

    gtk_label_set_text (GTK_LABEL (wlist[10]), "Offset Angle");
    gtk_spin_button_set_range (GTK_SPIN_BUTTON (generic_spin[1]), 0.0, 360.0);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (generic_spin[1]), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (generic_spin[1]), bolt_holes->offset_angle);
  }
  else if (!strcmp ("Matrix", gtk_combo_box_get_active_text (GTK_COMBO_BOX (type_combo))) && bolt_holes->type != GCODE_BOLT_HOLES_TYPE_MATRIX)
  {
    bolt_holes->type = GCODE_BOLT_HOLES_TYPE_MATRIX;

    /* Update the labels and spin buttons */
    gtk_label_set_text (GTK_LABEL (wlist[8]), "Hole Num(X)");
    gtk_spin_button_set_range (GTK_SPIN_BUTTON (generic_spin[0]), 1, 100);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (generic_spin[0]), 0);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (generic_spin[0]), bolt_holes->num[0]);

    gtk_label_set_text (GTK_LABEL (wlist[10]), "Hole Num(Y)");
    gtk_spin_button_set_range (GTK_SPIN_BUTTON (generic_spin[1]), 1, 100);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (generic_spin[1]), 0);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (generic_spin[1]), bolt_holes->num[1]);
  }


  bolt_holes->num[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (generic_spin[0]));

  if (!strcmp ("Radial", gtk_combo_box_get_active_text (GTK_COMBO_BOX (type_combo))))
  {
    bolt_holes->type = GCODE_BOLT_HOLES_TYPE_RADIAL;
    bolt_holes->offset_angle = gtk_spin_button_get_value (GTK_SPIN_BUTTON (generic_spin[1]));
  }
  else if (!strcmp ("Matrix", gtk_combo_box_get_active_text (GTK_COMBO_BOX (type_combo))))
  {
    bolt_holes->type = GCODE_BOLT_HOLES_TYPE_MATRIX;
    bolt_holes->num[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (generic_spin[1]));
  }

  bolt_holes->pocket = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pocket_check_button));

  gcode_bolt_holes_rebuild (block);

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);

  gui_menu_util_modified (gui, 1);
}


static void
gui_tab_bolt_holes (gui_t *gui, gcode_block_t *block)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *bolt_holes_tab;
  GtkWidget *alignment;
  GtkWidget *bolt_holes_table;
  GtkWidget *label;
  GtkWidget *posx_spin;
  GtkWidget *posy_spin;
  GtkWidget *type_combo;
  GtkWidget *generic_spin[2];
  GtkWidget *hole_diameter_spin;
  GtkWidget *offset_distance_spin;
  GtkWidget *pocket_check_button;
  gcode_bolt_holes_t *bolt_holes;
  uint16_t wind;

  /*
  * Bolt Holes Parameters
  */
  bolt_holes = (gcode_bolt_holes_t *) block->pdata;

  wind = 0;
  wlist = (GtkWidget **) malloc (13 * sizeof (GtkWidget *));
  hids = (gulong *) malloc (10 * sizeof (gulong));

  bolt_holes_tab = gtk_frame_new ("Bolt Holes Parameters");
  g_signal_connect (bolt_holes_tab, "destroy", G_CALLBACK (generic_destroy_callback), wlist);
  gtk_container_add (GTK_CONTAINER (gui->panel_tab_vbox), bolt_holes_tab);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (bolt_holes_tab), alignment);

  bolt_holes_table = gtk_table_new (8, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (bolt_holes_table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (bolt_holes_table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (bolt_holes_table), 4);
  gtk_container_add (GTK_CONTAINER (alignment), bolt_holes_table);

  label = gtk_label_new ("Pos(X)");
  gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), label, 0, 1, 0, 1);
  posx_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (posx_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (posx_spin), bolt_holes->pos[0]);
  g_signal_connect (posx_spin, "value-changed", G_CALLBACK (bolt_holes_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), posx_spin, 1, 2, 0, 1);

  label = gtk_label_new ("Pos(Y)");
  gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), label, 0, 1, 1, 2);
  posy_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_Y), GCODE_UNITS (block->gcode, MAX_DIM_Y), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (posy_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (posy_spin), bolt_holes->pos[1]);
  g_signal_connect (posy_spin, "value-changed", G_CALLBACK (bolt_holes_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), posy_spin, 1, 2, 1, 2);

  label = gtk_label_new ("Hole Diameter");
  gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), label, 0, 1, 2, 3);
  hole_diameter_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (hole_diameter_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (hole_diameter_spin), bolt_holes->hole_diameter);
  g_signal_connect (hole_diameter_spin, "value-changed", G_CALLBACK (bolt_holes_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), hole_diameter_spin, 1, 2, 2, 3);

  label = gtk_label_new ("Offset Distance");
  gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), label, 0, 1, 3, 4);
  offset_distance_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (offset_distance_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (offset_distance_spin), bolt_holes->offset_distance);
  g_signal_connect (offset_distance_spin, "value-changed", G_CALLBACK (bolt_holes_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), offset_distance_spin, 1, 2, 3, 4);

  wlist[wind++] = (GtkWidget *) hids;
  wlist[wind++] = (GtkWidget *) gui;
  wlist[wind++] = (GtkWidget *) block;
  wlist[wind++] = posx_spin;
  wlist[wind++] = posy_spin;
  wlist[wind++] = hole_diameter_spin;
  wlist[wind++] = offset_distance_spin;

  label = gtk_label_new ("Type");
  gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), label, 0, 1, 4, 5);
  type_combo = gtk_combo_box_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (type_combo), "Radial");
  gtk_combo_box_append_text (GTK_COMBO_BOX (type_combo), "Matrix");
  gtk_combo_box_set_active (GTK_COMBO_BOX (type_combo), bolt_holes->type);
  g_signal_connect (type_combo, "changed", G_CALLBACK (bolt_holes_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), type_combo, 1, 2, 4, 5);
  wlist[wind++] = type_combo;

  if (bolt_holes->type == GCODE_BOLT_HOLES_TYPE_RADIAL)
  {
    label = gtk_label_new ("Hole Num");
    gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), label, 0, 1, 5, 6);
    generic_spin[0] = gtk_spin_button_new_with_range (1, 100, 1);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (generic_spin[0]), 0);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (generic_spin[0]), bolt_holes->num[0]);
    g_signal_connect (generic_spin[0], "value-changed", G_CALLBACK (bolt_holes_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), generic_spin[0], 1, 2, 5, 6);
    wlist[wind++] = label;
    wlist[wind++] = generic_spin[0];

    label = gtk_label_new ("Offset Angle");
    gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), label, 0, 1, 6, 7);
    generic_spin[1] = gtk_spin_button_new_with_range (0.0, 360.0, 1.0);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (generic_spin[1]), MANTISSA);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (generic_spin[1]), bolt_holes->offset_angle);
    g_signal_connect (generic_spin[1], "value-changed", G_CALLBACK (bolt_holes_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), generic_spin[1], 1, 2, 6, 7);
    wlist[wind++] = label;
    wlist[wind++] = generic_spin[1];
  }
  else if (bolt_holes->type == GCODE_BOLT_HOLES_TYPE_MATRIX)
  {
    label = gtk_label_new ("Hole Num(X)");
    gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), label, 0, 1, 5, 6);
    generic_spin[0] = gtk_spin_button_new_with_range (1, 100, 1);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (generic_spin[0]), 0);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (generic_spin[0]), bolt_holes->num[0]);
    g_signal_connect (generic_spin[0], "value-changed", G_CALLBACK (bolt_holes_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), generic_spin[0], 1, 2, 5, 6);
    wlist[wind++] = label;
    wlist[wind++] = generic_spin[0];

    label = gtk_label_new ("Hole Num(Y)");
    gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), label, 0, 1, 6, 7);
    generic_spin[1] = gtk_spin_button_new_with_range (1, 100, 1);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (generic_spin[1]), 0);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (generic_spin[1]), bolt_holes->num[1]);
    g_signal_connect (generic_spin[1], "value-changed", G_CALLBACK (bolt_holes_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), generic_spin[1], 1, 2, 6, 7);
    wlist[wind++] = label;
    wlist[wind++] = generic_spin[1];
  }

  label = gtk_label_new ("Pocket");
  gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), label, 0, 1, 7, 8);
  pocket_check_button = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pocket_check_button), bolt_holes->pocket);
  g_signal_connect (pocket_check_button, "toggled", G_CALLBACK (bolt_holes_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (bolt_holes_table), pocket_check_button, 1, 2, 7, 8);
  wlist[wind++] = pocket_check_button;
}


static void
drill_holes_update_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gulong *hids;
  gui_t *gui;
  gcode_block_t *block;
  gcode_drill_holes_t *drill_holes;


  wlist = (GtkWidget **) data;

  hids = (gulong *) wlist[0];

  gui = (gui_t *) wlist[1];

  block = (gcode_block_t *) wlist[2];
  drill_holes = (gcode_drill_holes_t *) block->pdata;

  drill_holes->depth = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[3]));
  drill_holes->increment = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[4]));
  drill_holes->optimal_path = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wlist[5]));

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);

  gui_menu_util_modified (gui, 1);
}


static void
gui_tab_drill_holes (gui_t *gui, gcode_block_t *block)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *drill_holes_tab;
  GtkWidget *alignment;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *depth_spin;
  GtkWidget *peck_increment_spin;
  GtkWidget *optimal_path_check_button;
  gcode_drill_holes_t *drill_holes;
  uint16_t wind;

  /*
  * Drill Holes Parameters
  */
  drill_holes = (gcode_drill_holes_t *) block->pdata;

  wind = 0;
  wlist = (GtkWidget **) malloc (6 * sizeof (GtkWidget *));
  hids = (gulong *) malloc (3 * sizeof (gulong));

  drill_holes_tab = gtk_frame_new ("Drill Holes Parameters");
  g_signal_connect (drill_holes_tab, "destroy", G_CALLBACK (generic_destroy_callback), wlist);
  gtk_container_add (GTK_CONTAINER (gui->panel_tab_vbox), drill_holes_tab);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (drill_holes_tab), alignment);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_container_add (GTK_CONTAINER (alignment), table);

  label = gtk_label_new ("Depth");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);
  depth_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_Z), 0.0, GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (depth_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (depth_spin), drill_holes->depth);
  g_signal_connect (depth_spin, "value-changed", G_CALLBACK (drill_holes_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (table), depth_spin, 1, 2, 0, 1);

  label = gtk_label_new ("Peck Increment");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);
  peck_increment_spin = gtk_spin_button_new_with_range (0.0, GCODE_UNITS (block->gcode, MAX_DIM_Z), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (peck_increment_spin), 3);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (peck_increment_spin), drill_holes->increment);
  g_signal_connect (peck_increment_spin, "value-changed", G_CALLBACK (drill_holes_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (table), peck_increment_spin, 1, 2, 1, 2);

  label = gtk_label_new ("Optimal Path");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 2, 3);
  optimal_path_check_button = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (optimal_path_check_button), drill_holes->optimal_path);
  g_signal_connect (optimal_path_check_button, "toggled", G_CALLBACK (drill_holes_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (table), optimal_path_check_button, 1, 2, 2, 3);

  wlist[wind++] = (GtkWidget *) hids;
  wlist[wind++] = (GtkWidget *) gui;
  wlist[wind++] = (GtkWidget *) block;
  wlist[wind++] = (GtkWidget *) depth_spin;
  wlist[wind++] = (GtkWidget *) peck_increment_spin;
  wlist[wind++] = (GtkWidget *) optimal_path_check_button;
}


static void
point_update_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gulong *hids;
  gui_t *gui;
  gcode_block_t *block;
  gcode_point_t *point;
  uint16_t wind;

  wind = 0;
  wlist = (GtkWidget **) data;

  hids = (gulong *) wlist[wind];
  wind++;

  gui = (gui_t *) wlist[wind];
  wind++;

  block = (gcode_block_t *) wlist[wind];
  point = (gcode_point_t *) block->pdata;
  wind++;

  point->p[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[wind++]));
  point->p[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[wind++]));

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);

  gui_menu_util_modified (gui, 1);
}


static void
gui_tab_point (gui_t *gui, gcode_block_t *block)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *point_tab;
  GtkWidget *alignment;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *px_spin;
  GtkWidget *py_spin;
  gcode_point_t *point;
  uint16_t wind, row;

  /*
  * Point Parameters
  */

  point = (gcode_point_t *) block->pdata;

  wind = 0;
  wlist = (GtkWidget **) malloc (5 * sizeof (GtkWidget *));
  hids = (gulong *) malloc (2 * sizeof (gulong));
  row = 0;

  point_tab = gtk_frame_new ("Point Parameters");
  g_signal_connect (point_tab, "destroy", G_CALLBACK (generic_destroy_callback), wlist);
  gtk_container_add (GTK_CONTAINER (gui->panel_tab_vbox), point_tab);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (point_tab), alignment);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_container_add (GTK_CONTAINER (alignment), table);

  label = gtk_label_new ("Pos(X)");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, row, row+1);
  px_spin = gtk_spin_button_new_with_range (0.0, GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (px_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (px_spin), point->p[0]);
  g_signal_connect (px_spin, "value-changed", G_CALLBACK (point_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (table), px_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Pos(Y)");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, row, row+1);
  py_spin = gtk_spin_button_new_with_range (0, GCODE_UNITS (block->gcode, MAX_DIM_Y), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (py_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (py_spin), point->p[1]);
  g_signal_connect (py_spin, "value-changed", G_CALLBACK (point_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (table), py_spin, 1, 2, row, row+1);
  row++;

  wlist[wind++] = (GtkWidget *) hids;
  wlist[wind++] = (GtkWidget *) gui;
  wlist[wind++] = (GtkWidget *) block;
  wlist[wind++] = px_spin;
  wlist[wind++] = py_spin;
}


static void
template_update_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gulong *hids;
  gui_t *gui;
  gcode_block_t *block;
  gcode_template_t *template;
  uint16_t wind;

  wind = 0;
  wlist = (GtkWidget **) data;

  hids = (gulong *) wlist[wind];
  wind++;

  gui = (gui_t *) wlist[wind];
  wind++;

  block = (gcode_block_t *) wlist[wind];
  template = (gcode_template_t *) block->pdata;
  wind++;

  template->position[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[wind++]));
  template->position[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[wind++]));
  template->rotation = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[wind++]));

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);

  gui_menu_util_modified (gui, 1);
}


static void
gui_tab_template (gui_t *gui, gcode_block_t *block)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *template_tab;
  GtkWidget *alignment;
  GtkWidget *template_table;
  GtkWidget *label;
  GtkWidget *positionx_spin;
  GtkWidget *positiony_spin;
  GtkWidget *rotation_spin;
  gcode_template_t *template;
  uint16_t wind, row;

  /*
  * Template Parameters
  */

  template = (gcode_template_t *) block->pdata;

  wind = 0;
  wlist = (GtkWidget **) malloc (6 * sizeof (GtkWidget *));
  hids = (gulong *) malloc (3 * sizeof (gulong));
  row = 0;

  template_tab = gtk_frame_new ("Template Parameters");
  g_signal_connect (template_tab, "destroy", G_CALLBACK (generic_destroy_callback), wlist);
  gtk_container_add (GTK_CONTAINER (gui->panel_tab_vbox), template_tab);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (template_tab), alignment);

  template_table = gtk_table_new (3, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (template_table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (template_table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (template_table), 4);
  gtk_container_add (GTK_CONTAINER (alignment), template_table);

  label = gtk_label_new ("Position(X)");
  gtk_table_attach_defaults (GTK_TABLE (template_table), label, 0, 1, row, row+1);
  positionx_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_X), GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (positionx_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (positionx_spin), template->position[0]);
  g_signal_connect (positionx_spin, "value-changed", G_CALLBACK (template_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (template_table), positionx_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Position(Y)");
  gtk_table_attach_defaults (GTK_TABLE (template_table), label, 0, 1, row, row+1);
  positiony_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_Y), GCODE_UNITS (block->gcode, MAX_DIM_Y), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (positiony_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (positiony_spin), template->position[1]);
  g_signal_connect (positiony_spin, "value-changed", G_CALLBACK (template_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (template_table), positiony_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Rotation");
  gtk_table_attach_defaults (GTK_TABLE (template_table), label, 0, 1, row, row+1);
  rotation_spin = gtk_spin_button_new_with_range (0.0, 360.0, 1.0);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (rotation_spin), 3);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (rotation_spin), template->rotation);
  g_signal_connect (rotation_spin, "value-changed", G_CALLBACK (template_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (template_table), rotation_spin, 1, 2, row, row+1);
  row++;

  wlist[wind++] = (GtkWidget *) hids;
  wlist[wind++] = (GtkWidget *) gui;
  wlist[wind++] = (GtkWidget *) block;
  wlist[wind++] = positionx_spin;
  wlist[wind++] = positiony_spin;
  wlist[wind++] = rotation_spin;
}


static void
sketch_update_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gulong *hids;
  gui_t *gui;
  gcode_block_t *block;
  gcode_sketch_t *sketch;

  wlist = (GtkWidget **) data;

  hids = (gulong *) wlist[0];

  gui = (gui_t *) wlist[1];

  block = (gcode_block_t *) wlist[2];
  sketch = (gcode_sketch_t *) block->pdata;

  sketch->taper_offset[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[3]));
  sketch->taper_offset[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[4]));
  sketch->zero_pass = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wlist[6]));

  if (fabs (sketch->taper_offset[0]) > 0.0 || fabs (sketch->taper_offset[1]) > 0.0)
  {
    gtk_widget_set_sensitive (wlist[7], 0);
    sketch->helical = 0;
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wlist[7]), sketch->helical);
  }
  else
  {
    gtk_widget_set_sensitive (wlist[7], 1);
  }


  if (sketch->pocket && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wlist[7])))
  {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wlist[5]), 0);
    sketch->pocket = 0;
    sketch->helical = 1;
  }
  else if (sketch->helical && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wlist[5])))
  {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wlist[7]), 0);
    sketch->helical = 0;
    sketch->pocket = 1;
  }
  else
  {
    sketch->pocket = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wlist[5]));
    sketch->helical = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wlist[7]));
  }

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);

  gui_menu_util_modified (gui, 1);
}


static void
gui_tab_sketch (gui_t *gui, gcode_block_t *block)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *sketch_tab;
  GtkWidget *alignment;
  GtkWidget *sketch_table;
  GtkWidget *label;
  GtkWidget *taper_offsetx_spin;
  GtkWidget *taper_offsety_spin;
  GtkWidget *pocket_check_button;
  GtkWidget *zero_pass_check_button;
  GtkWidget *helical_check_button;
  gcode_sketch_t *sketch;
  int taper_exists;
  uint16_t wind, row;

  /*
  * Sketch Parameters
  */

  sketch = (gcode_sketch_t *) block->pdata;

  wind = 0;
  wlist = (GtkWidget **) malloc (8 * sizeof (GtkWidget *));
  hids = (gulong *) malloc (4 * sizeof (gulong));
  row = 0;

  sketch_tab = gtk_frame_new ("Sketch Parameters");
  g_signal_connect (sketch_tab, "destroy", G_CALLBACK (generic_destroy_callback), wlist);
  gtk_container_add (GTK_CONTAINER (gui->panel_tab_vbox), sketch_tab);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (sketch_tab), alignment);

  sketch_table = gtk_table_new (4, 4, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (sketch_table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (sketch_table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (sketch_table), 4);
  gtk_container_add (GTK_CONTAINER (alignment), sketch_table);

  label = gtk_label_new ("Taper Offset(X)");
  gtk_table_attach_defaults (GTK_TABLE (sketch_table), label, 0, 1, row, row+1);
  taper_offsetx_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -1.0), GCODE_UNITS (block->gcode, 1.0), GCODE_UNITS (block->gcode, 0.1));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (taper_offsetx_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (taper_offsetx_spin), sketch->taper_offset[0]);
  g_signal_connect (taper_offsetx_spin, "value-changed", G_CALLBACK (sketch_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (sketch_table), taper_offsetx_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Taper Offset(Y)");
  gtk_table_attach_defaults (GTK_TABLE (sketch_table), label, 0, 1, row, row+1);
  taper_offsety_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -1.0), GCODE_UNITS (block->gcode, 1.0), GCODE_UNITS (block->gcode, 0.1));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (taper_offsety_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (taper_offsety_spin), sketch->taper_offset[1]);
  g_signal_connect (taper_offsety_spin, "value-changed", G_CALLBACK (sketch_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (sketch_table), taper_offsety_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Pocket");
  gtk_table_attach_defaults (GTK_TABLE (sketch_table), label, 0, 1, row, row+1);
  pocket_check_button = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (pocket_check_button), sketch->pocket);
  g_signal_connect (pocket_check_button, "toggled", G_CALLBACK (sketch_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (sketch_table), pocket_check_button, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Zero Pass");
  gtk_table_attach_defaults (GTK_TABLE (sketch_table), label, 0, 1, row, row+1);
  zero_pass_check_button = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (zero_pass_check_button), sketch->zero_pass);
  g_signal_connect (zero_pass_check_button, "toggled", G_CALLBACK (sketch_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (sketch_table), zero_pass_check_button, 1, 2, row, row+1);
  row++;

  taper_exists = gcode_extrusion_taper_exists (block);
  if (taper_exists || fabs (sketch->taper_offset[0]) > 0.0 || fabs (sketch->taper_offset[1]) > 0.0)
    sketch->helical = 0;

  label = gtk_label_new ("Helical");
  gtk_table_attach_defaults (GTK_TABLE (sketch_table), label, 0, 1, row, row+1);
  helical_check_button = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (helical_check_button), sketch->helical);
  g_signal_connect (helical_check_button, "toggled", G_CALLBACK (sketch_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (sketch_table), helical_check_button, 1, 2, row, row+1);
  row++;

  if (taper_exists || fabs (sketch->taper_offset[0]) > 0.0 || fabs (sketch->taper_offset[1]) > 0.0)
    gtk_widget_set_sensitive (helical_check_button, 0);

  wlist[wind++] = (GtkWidget *) hids;
  wlist[wind++] = (GtkWidget *) gui;
  wlist[wind++] = (GtkWidget *) block;
  wlist[wind++] = taper_offsetx_spin;
  wlist[wind++] = taper_offsety_spin;
  wlist[wind++] = pocket_check_button;
  wlist[wind++] = zero_pass_check_button;
  wlist[wind++] = helical_check_button;
}


static void
extrusion_update_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gulong *hids;
  gui_t *gui;
  gcode_block_t *block;
  gcode_extrusion_t *extrusion;
  uint16_t wind;
  char *cut_side;

  wind = 0;
  wlist = (GtkWidget **) data;

  hids = (gulong *) wlist[wind];
  wind++;

  gui = (gui_t *) wlist[wind];
  wind++;

  block = (gcode_block_t *) wlist[wind];
  extrusion = (gcode_extrusion_t *) block->pdata;
  wind++;

  extrusion->resolution = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[wind++]));

  cut_side = gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[wind++]));
 
  if (!strcmp (cut_side, "Inside"))
  {
    extrusion->cut_side = GCODE_EXTRUSION_INSIDE;
  }
  else if (!strcmp (cut_side, "Outside"))
  {
    extrusion->cut_side = GCODE_EXTRUSION_OUTSIDE;
  }
  else if (!strcmp (cut_side, "Along"))
  {
    extrusion->cut_side = GCODE_EXTRUSION_ALONG;
  }

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);

  gui_menu_util_modified (gui, 1);
}


static void
gui_tab_extrusion (gui_t *gui, gcode_block_t *block)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *extrusion_tab;
  GtkWidget *alignment;
  GtkWidget *extrusion_table;
  GtkWidget *label;
  GtkWidget *resolution_spin;
  GtkWidget *cut_side_combo;
  gcode_extrusion_t *extrusion;
  uint16_t wind, row;
  uint8_t option = 1;

  /*
  * Extrusion Parameters
  */

  extrusion = (gcode_extrusion_t *) block->pdata;

  wind = 0;
  wlist = (GtkWidget **) malloc (5 * sizeof (GtkWidget *));
  hids = (gulong *) malloc (2 * sizeof (gulong));
  row = 0;

  extrusion_tab = gtk_frame_new ("Extrusion Parameters");
  g_signal_connect (extrusion_tab, "destroy", G_CALLBACK (generic_destroy_callback), wlist);
  gtk_container_add (GTK_CONTAINER (gui->panel_tab_vbox), extrusion_tab);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (extrusion_tab), alignment);

  extrusion_table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (extrusion_table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (extrusion_table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (extrusion_table), 4);
  gtk_container_add (GTK_CONTAINER (alignment), extrusion_table);

  label = gtk_label_new ("Resolution");
  gtk_table_attach_defaults (GTK_TABLE (extrusion_table), label, 0, 1, row, row+1);
  resolution_spin = gtk_spin_button_new_with_range (0.001, block->gcode->material_size[2], GCODE_UNITS (block->gcode, 0.001));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (resolution_spin), 3);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (resolution_spin), extrusion->resolution);
  g_signal_connect (resolution_spin, "value-changed", G_CALLBACK (extrusion_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (extrusion_table), resolution_spin, 1, 2, row, row+1);
  row++;

  
  if (block->parent->type == GCODE_TYPE_SKETCH)
    option = gcode_sketch_is_closed (block->parent);

  label = gtk_label_new ("Cut Side");
  gtk_table_attach_defaults (GTK_TABLE (extrusion_table), label, 0, 1, row, row+1);
  cut_side_combo = gtk_combo_box_new_text ();
  if (option)
  {
    gtk_combo_box_append_text (GTK_COMBO_BOX (cut_side_combo), "Inside");
    gtk_combo_box_append_text (GTK_COMBO_BOX (cut_side_combo), "Outside");
    gtk_combo_box_append_text (GTK_COMBO_BOX (cut_side_combo), "Along");
    gtk_combo_box_set_active (GTK_COMBO_BOX (cut_side_combo), extrusion->cut_side);
  }
  else
  {
    extrusion->cut_side = GCODE_EXTRUSION_ALONG;
    gtk_combo_box_append_text (GTK_COMBO_BOX (cut_side_combo), "Along");
    gtk_combo_box_set_active (GTK_COMBO_BOX (cut_side_combo), 0);
  }

  g_signal_connect (cut_side_combo, "changed", G_CALLBACK (extrusion_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (extrusion_table), cut_side_combo, 1, 2, row, row+1);
  row++;

  if (block->parent->type == GCODE_TYPE_BOLT_HOLES)
  {
    /* Do not allow the user to change this, bolt holes by definition are inside only. */
    gtk_widget_set_sensitive (cut_side_combo, 0);
  }

  wlist[wind++] = (GtkWidget *) hids;
  wlist[wind++] = (GtkWidget *) gui;
  wlist[wind++] = (GtkWidget *) block;
  wlist[wind++] = resolution_spin;
  wlist[wind++] = cut_side_combo;
}


static void
tool_update_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *prompt_combo;
  gui_t *gui;
  gcode_block_t *block;
  gcode_tool_t *tool;
  uint16_t wind;
  char *prompt, *emd;

  wind = 0;
  wlist = (GtkWidget **) data;

  hids = (gulong *) wlist[wind];
  wind++;

  gui = (gui_t *) wlist[wind];
  wind++;

  block = (gcode_block_t *) wlist[wind];
  tool = (gcode_tool_t *) block->pdata;
  wind++;


  emd = gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[3]));
  strcpy (tool->label, &emd[6]);

  {
    gui_endmill_list_t endmill_list;
    int i;

    gui_endmills_init (&endmill_list);
    gui_endmills_read (&endmill_list, &gui->gcode);

    for (i = 0; i < endmill_list.num; i++)
    {
      if (!strcmp (tool->label, endmill_list.endmill[i].description))
      {
        tool->diam = endmill_list.endmill[i].diameter;
        tool->number = endmill_list.endmill[i].number;
      }
    }

    gui_endmills_free (&endmill_list);
  }

  tool->feed = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[4]));


  prompt_combo = wlist[wind++];

  prompt = gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[5]));
  if (!strcmp (prompt, "On"))
  {
    tool->prompt = 1;
  }
  else if (!strcmp (prompt, "Off"))
  {
    tool->prompt = 0;
  }

  tool->change[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[6]));
  tool->change[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[7]));
  tool->change[2] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[8]));

  if (!strcmp (gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[9])), "100%"))
    tool->plunge_ratio = 1.0;
  if (!strcmp (gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[9])), "50%"))
    tool->plunge_ratio = 0.5;
  if (!strcmp (gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[9])), "20%"))
    tool->plunge_ratio = 0.2;
  if (!strcmp (gtk_combo_box_get_active_text (GTK_COMBO_BOX (wlist[9])), "10%"))
    tool->plunge_ratio = 0.1;

  tool->spindle_rpm = (uint32_t) gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[10]));

  tool->coolant = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wlist[11]));

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);

  gui_menu_util_modified (gui, 1);
}


static void
gui_tab_tool (gui_t *gui, gcode_block_t *block)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *tool_tab;
  GtkWidget *alignment;
  GtkWidget *tool_table;
  GtkWidget *label;
  GtkWidget *prompt_combo;
  GtkWidget *changex_spin;
  GtkWidget *changey_spin;
  GtkWidget *changez_spin;
  GtkWidget *end_mill_combo;
  GtkWidget *feed_spin;
  GtkWidget *plunge_ratio_combo;
  GtkWidget *spindle_rpm_spin;
  GtkWidget *coolant_check_button;
  gcode_tool_t *tool;
  uint16_t wind, row;
  int selind;

  /*
  * Tool Parameters
  */

  tool = (gcode_tool_t *) block->pdata;

  wind = 0;
  wlist = (GtkWidget **) malloc (12 * sizeof (GtkWidget *));
  hids = NULL;
  row = 0;

  tool_tab = gtk_frame_new ("Tool Parameters");
  g_signal_connect (tool_tab, "destroy", G_CALLBACK (generic_destroy_callback), wlist);
  gtk_container_add (GTK_CONTAINER (gui->panel_tab_vbox), tool_tab);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (tool_tab), alignment);

  tool_table = gtk_table_new (12, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (tool_table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (tool_table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (tool_table), 4);
  gtk_container_add (GTK_CONTAINER (alignment), tool_table);

  {
    gui_endmill_list_t endmill_list;
    char string[32];
    int i;
  
    gui_endmills_init (&endmill_list);
    gui_endmills_read (&endmill_list, &gui->gcode);
  
    label = gtk_label_new ("End Mill");
    gtk_table_attach_defaults (GTK_TABLE (tool_table), label, 0, 2, row, row+1);
    row++;

    end_mill_combo = gtk_combo_box_new_text ();  
    selind = -1;
    for (i = 0; i < endmill_list.num; i++)
    {
      if (!strcmp (tool->label, endmill_list.endmill[i].description))
        selind = i;
      sprintf (string, "T%.2d - %s", endmill_list.endmill[i].number, endmill_list.endmill[i].description);
      gtk_combo_box_append_text (GTK_COMBO_BOX (end_mill_combo), string);
    }

    if (selind == -1)
    {
      sprintf (string, "T%.2d - %s", tool->number, tool->label);
      gtk_combo_box_append_text (GTK_COMBO_BOX (end_mill_combo), string);
      selind = i;
    }
    gtk_combo_box_set_active (GTK_COMBO_BOX (end_mill_combo), selind);

    g_signal_connect (end_mill_combo, "changed", G_CALLBACK (tool_update_callback), wlist);
    gtk_table_attach_defaults (GTK_TABLE (tool_table), end_mill_combo, 0, 2, row, row+1);

    gui_endmills_free (&endmill_list);
  }
  row++;

  label = gtk_label_new ("Feed Rate");
  gtk_table_attach_defaults (GTK_TABLE (tool_table), label, 0, 1, row, row+1);
  feed_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, 0.01), GCODE_UNITS (block->gcode, 80.0), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (feed_spin), 2);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (feed_spin), tool->feed);
  g_signal_connect (feed_spin, "value-changed", G_CALLBACK (tool_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (tool_table), feed_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Prompt");
  gtk_table_attach_defaults (GTK_TABLE (tool_table), label, 0, 1, row, row+1);
  prompt_combo = gtk_combo_box_new_text ();
  gtk_combo_box_append_text (GTK_COMBO_BOX (prompt_combo), "Off");
  gtk_combo_box_append_text (GTK_COMBO_BOX (prompt_combo), "On");
  gtk_combo_box_set_active (GTK_COMBO_BOX (prompt_combo), tool->prompt);
  g_signal_connect (prompt_combo, "changed", G_CALLBACK (tool_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (tool_table), prompt_combo, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Change(X)");
  changex_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, 0.0), GCODE_UNITS (block->gcode, 20.0), GCODE_UNITS (block->gcode, 0.1));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (changex_spin), 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (changex_spin), tool->change[0]);
  g_signal_connect (changex_spin, "value-changed", G_CALLBACK (tool_update_callback), wlist);
  if ((block->gcode->machine_options & GCODE_MACHINE_OPTION_AUTOMATIC_TOOL_CHANGE) == 0)
  {
    gtk_table_attach_defaults (GTK_TABLE (tool_table), label, 0, 1, row, row+1);
    gtk_table_attach_defaults (GTK_TABLE (tool_table), changex_spin, 1, 2, row, row+1);
    row++;
  }

  label = gtk_label_new ("Change(Y)");
  changey_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, 0.0), GCODE_UNITS (block->gcode, 20.0), GCODE_UNITS (block->gcode, 0.1));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (changey_spin), 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (changey_spin), tool->change[1]);
  g_signal_connect (changey_spin, "value-changed", G_CALLBACK (tool_update_callback), wlist);
  if ((block->gcode->machine_options & GCODE_MACHINE_OPTION_AUTOMATIC_TOOL_CHANGE) == 0)
  {
    gtk_table_attach_defaults (GTK_TABLE (tool_table), label, 0, 1, row, row+1);
    gtk_table_attach_defaults (GTK_TABLE (tool_table), changey_spin, 1, 2, row, row+1);
    row++;
  }

  label = gtk_label_new ("Change(Z)");
  changez_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, 0.0), GCODE_UNITS (block->gcode, 3.0), GCODE_UNITS (block->gcode, 0.1));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (changez_spin), 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (changez_spin), tool->change[2]);
  g_signal_connect (changez_spin, "value-changed", G_CALLBACK (tool_update_callback), wlist);
  if ((block->gcode->machine_options & GCODE_MACHINE_OPTION_AUTOMATIC_TOOL_CHANGE) == 0)
  {
    gtk_table_attach_defaults (GTK_TABLE (tool_table), label, 0, 1, row, row+1);
    gtk_table_attach_defaults (GTK_TABLE (tool_table), changez_spin, 1, 2, row, row+1);
    row++;
  }

  label = gtk_label_new ("Plunge Ratio");
  gtk_table_attach_defaults (GTK_TABLE (tool_table), label, 0, 1, row, row+1);
  plunge_ratio_combo = gtk_combo_box_new_text ();

  gtk_combo_box_append_text (GTK_COMBO_BOX (plunge_ratio_combo), "100%");
  gtk_combo_box_append_text (GTK_COMBO_BOX (plunge_ratio_combo), "50%");
  gtk_combo_box_append_text (GTK_COMBO_BOX (plunge_ratio_combo), "20%");
  gtk_combo_box_append_text (GTK_COMBO_BOX (plunge_ratio_combo), "10%");

  if (tool->plunge_ratio == 1.0)
  {
    gtk_combo_box_set_active (GTK_COMBO_BOX (plunge_ratio_combo), 0);
  }
  else if (tool->plunge_ratio == 0.5)
  {
    gtk_combo_box_set_active (GTK_COMBO_BOX (plunge_ratio_combo), 1);
  }
  else if (tool->plunge_ratio == 0.2)
  {
    gtk_combo_box_set_active (GTK_COMBO_BOX (plunge_ratio_combo), 2);
  }
  else
  {
    gtk_combo_box_set_active (GTK_COMBO_BOX (plunge_ratio_combo), 3);
  }

  g_signal_connect (plunge_ratio_combo, "changed", G_CALLBACK (tool_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (tool_table), plunge_ratio_combo, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Spindle RPM");
  spindle_rpm_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, 15.0), GCODE_UNITS (block->gcode, 80000.0), GCODE_UNITS (block->gcode, 1.0));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (spindle_rpm_spin), 0);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spindle_rpm_spin), tool->spindle_rpm);
  g_signal_connect (spindle_rpm_spin, "value-changed", G_CALLBACK (tool_update_callback), wlist);
  if (block->gcode->machine_options & GCODE_MACHINE_OPTION_SPINDLE_CONTROL)
  {
    gtk_table_attach_defaults (GTK_TABLE (tool_table), label, 0, 1, row, row+1);
    gtk_table_attach_defaults (GTK_TABLE (tool_table), spindle_rpm_spin, 1, 2, row, row+1);
    row++;
  }

  label = gtk_label_new ("Coolant");
  coolant_check_button = gtk_check_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (coolant_check_button), tool->coolant);
  g_signal_connect (coolant_check_button, "toggled", G_CALLBACK (tool_update_callback), wlist);
  if (block->gcode->machine_options & GCODE_MACHINE_OPTION_COOLANT)
  {
    gtk_table_attach_defaults (GTK_TABLE (tool_table), label, 0, 1, row, row+1);
    gtk_table_attach_defaults (GTK_TABLE (tool_table), coolant_check_button, 1, 2, row, row+1);
    row++;
  }


  wlist[wind++] = (GtkWidget *) hids;
  wlist[wind++] = (GtkWidget *) gui;
  wlist[wind++] = (GtkWidget *) block;
  wlist[wind++] = end_mill_combo;
  wlist[wind++] = feed_spin;
  wlist[wind++] = prompt_combo;
  wlist[wind++] = changex_spin;
  wlist[wind++] = changey_spin;
  wlist[wind++] = changez_spin;
  wlist[wind++] = plunge_ratio_combo;
  wlist[wind++] = spindle_rpm_spin;
  wlist[wind++] = coolant_check_button;
}


static void
image_update_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gulong *hids;
  gui_t *gui;
  gcode_block_t *block;
  gcode_image_t *image;
  uint16_t wind;

  wind = 0;
  wlist = (GtkWidget **) data;

  hids = (gulong *) wlist[wind];
  wind++;

  gui = (gui_t *) wlist[wind];
  wind++;

  block = (gcode_block_t *) wlist[wind];
  image = (gcode_image_t *) block->pdata;
  wind++;

  image->size[0] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[wind++]));
  image->size[1] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[wind++]));
  image->size[2] = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[wind++]));

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);

  gui_menu_util_modified (gui, 1);
}


static void
gui_tab_image (gui_t *gui, gcode_block_t *block)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *image_tab;
  GtkWidget *alignment;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *sizex_spin;
  GtkWidget *sizey_spin;
  GtkWidget *depthz_spin;
  gcode_image_t *image;
  uint16_t wind, row;

  /*
  * Image Paramters
  */

  image = (gcode_image_t *) block->pdata;

  wind = 0;
  wlist = (GtkWidget **) malloc (6 * sizeof (GtkWidget *));
  hids = (gulong *) malloc (2 * sizeof (gulong));
  row = 0;

  image_tab = gtk_frame_new ("Image Parameters");
  g_signal_connect (image_tab, "destroy", G_CALLBACK (generic_destroy_callback), wlist);
  gtk_container_add (GTK_CONTAINER (gui->panel_tab_vbox), image_tab);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (image_tab), alignment);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_container_add (GTK_CONTAINER (alignment), table);

  label = gtk_label_new ("Size(X)");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, row, row+1);
  sizex_spin = gtk_spin_button_new_with_range (0.0, GCODE_UNITS (block->gcode, MAX_DIM_X), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (sizex_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (sizex_spin), image->size[0]);
  g_signal_connect (sizex_spin, "value-changed", G_CALLBACK (image_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (table), sizex_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Size(Y)");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, row, row+1);
  sizey_spin = gtk_spin_button_new_with_range (0, GCODE_UNITS (block->gcode, MAX_DIM_Y), GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (sizey_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (sizey_spin), image->size[1]);
  g_signal_connect (sizey_spin, "value-changed", G_CALLBACK (image_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (table), sizey_spin, 1, 2, row, row+1);
  row++;

  label = gtk_label_new ("Depth(Z)");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, row, row+1);
  depthz_spin = gtk_spin_button_new_with_range (GCODE_UNITS (block->gcode, -MAX_DIM_Z), 0, GCODE_UNITS (block->gcode, 0.01));
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (depthz_spin), MANTISSA);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (depthz_spin), image->size[2]);
  g_signal_connect (depthz_spin, "value-changed", G_CALLBACK (image_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (table), depthz_spin, 1, 2, row, row+1);
  row++;

  wlist[wind++] = (GtkWidget *) hids;
  wlist[wind++] = (GtkWidget *) gui;
  wlist[wind++] = (GtkWidget *) block;
  wlist[wind++] = sizex_spin;
  wlist[wind++] = sizey_spin;
  wlist[wind++] = depthz_spin;
}


static void
stl_update_callback (GtkWidget *widget, gpointer data)
{
  GtkWidget **wlist;
  gulong *hids;
  gui_t *gui;
  gcode_block_t *block;
  gcode_stl_t *stl;
  uint16_t wind;

  wind = 0;
  wlist = (GtkWidget **) data;

  hids = (gulong *) wlist[wind];
  wind++;

  gui = (gui_t *) wlist[wind];
  wind++;

  block = (gcode_block_t *) wlist[wind];
  stl = (gcode_stl_t *) block->pdata;
  wind++;

  stl->slices = gtk_spin_button_get_value (GTK_SPIN_BUTTON (wlist[wind++]));

  gcode_stl_generate_slice_contours (block);

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);

  gui_menu_util_modified (gui, 1);
}


static void
gui_tab_stl (gui_t *gui, gcode_block_t *block)
{
  GtkWidget **wlist;
  gulong *hids;
  GtkWidget *stl_tab;
  GtkWidget *alignment;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget *slices_spin;
  gcode_stl_t *stl;
  uint16_t wind, row;

  /*
  * STL Paramters
  */

  stl = (gcode_stl_t *) block->pdata;

  wind = 0;
  wlist = (GtkWidget **) malloc (4 * sizeof (GtkWidget *));
/*  hids = (gulong *) malloc (2 * sizeof (gulong)); */
  hids = NULL;
  row = 0;

  stl_tab = gtk_frame_new ("STL Parameters");
  g_signal_connect (stl_tab, "destroy", G_CALLBACK (generic_destroy_callback), wlist);
  gtk_container_add (GTK_CONTAINER (gui->panel_tab_vbox), stl_tab);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (stl_tab), alignment);

  table = gtk_table_new (2, 2, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_table_set_row_spacings (GTK_TABLE (table), TABLE_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_container_add (GTK_CONTAINER (alignment), table);

  label = gtk_label_new ("Slices");
  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, row, row+1);
  slices_spin = gtk_spin_button_new_with_range (2, 1000, 1);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (slices_spin), 0);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (slices_spin), stl->slices);
  g_signal_connect (slices_spin, "value-changed", G_CALLBACK (stl_update_callback), wlist);
  gtk_table_attach_defaults (GTK_TABLE (table), slices_spin, 1, 2, row, row+1);
  row++;

  wlist[wind++] = (GtkWidget *) hids;
  wlist[wind++] = (GtkWidget *) gui;
  wlist[wind++] = (GtkWidget *) block;
  wlist[wind++] = slices_spin;
}


void
gui_tab_display (gui_t *gui, gcode_block_t *block, int force)
{
  if (block == gui->selected_block && !force)
    return;

  if (gui->panel_tab_vbox)
    gtk_widget_destroy (gui->panel_tab_vbox);

  gui->panel_tab_vbox = gtk_vbox_new (FALSE, 1);
  gtk_container_set_border_width (GTK_CONTAINER (gui->panel_tab_vbox), 1);
  gtk_container_add (GTK_CONTAINER (gui->panel_vbox), gui->panel_tab_vbox);

  if (!block)
    return;

  switch (block->type)
  {
    case GCODE_TYPE_BEGIN:
      gui_tab_begin (gui, block);
      break;

    case GCODE_TYPE_END:
      gui_tab_end (gui, block);
      break;

    case GCODE_TYPE_LINE:
      gui_tab_line (gui, block);
      break;

    case GCODE_TYPE_ARC:
      gui_tab_arc (gui, block);
      break;

    case GCODE_TYPE_BOLT_HOLES:
      gui_tab_bolt_holes (gui, block);
      break;

    case GCODE_TYPE_DRILL_HOLES:
      gui_tab_drill_holes (gui, block);
      break;

    case GCODE_TYPE_POINT:
      gui_tab_point (gui, block);
      break;

    case GCODE_TYPE_TEMPLATE:
      gui_tab_template (gui, block);
      break;

    case GCODE_TYPE_SKETCH:
      gui_tab_sketch (gui, block);
      break;

    case GCODE_TYPE_EXTRUSION:
      gui_tab_extrusion (gui, block);
      break;

    case GCODE_TYPE_TOOL:
      gui_tab_tool (gui, block);
      break;

    case GCODE_TYPE_IMAGE:
      gui_tab_image (gui, block);
      break;

    case GCODE_TYPE_STL:
      gui_tab_stl (gui, block);
      break;

    default:
      break;
  }

  gtk_widget_show_all (gui->panel_vbox);

  gui->opengl.mode = GUI_OPENGL_MODE_EDIT;
  gui->selected_block = block;

  gui->opengl.rebuild_view_display_list = 1;
  gui_opengl_context_redraw (&gui->opengl, block);
}
