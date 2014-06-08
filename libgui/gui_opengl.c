/*
*  gui_opengl.c
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
#include "gui_opengl.h"
#include "gui.h"
#include "gui_tab.h"
#include "gui_menu_util.h"
#include <GL/glu.h>

#define GRID_BORDER_LINE_COLOR	0.7
#define GRID_MAJOR_LINE_COLOR	0.4
#define GRID_MINOR_LINE_COLOR	0.2

void
gui_opengl_build_gridxy_display_list (gui_opengl_t *opengl)
{
  gfloat_t i, incr[3];

  opengl->matx_origin = -opengl->gcode->material_size[0] * 0.5;
  opengl->maty_origin = -opengl->gcode->material_size[1] * 0.5;
  opengl->matz_origin = -opengl->gcode->material_size[2] * 0.5;


  if (opengl->gcode->units == GCODE_UNITS_INCH)
  {
    if (opengl->gcode->material_size[0] >= 12.0 || opengl->gcode->material_size[1] >= 12.0)
    {
      incr[0] = 12.0;
      incr[1] = 1.0;
      incr[2] = 0.1;
    }
    else
    {
      incr[0] = 1.0;
      incr[1] = 0.1;
      incr[2] = 0.01;
    }
  }
  else
  {
    if (opengl->gcode->material_size[0] >= 1000.0 || opengl->gcode->material_size[1] >= 1000.0)
    {
      incr[0] = 100.0;
      incr[1] = 10.0;
      incr[2] = 1.0;
    }
    else
    {
      incr[0] = 10.0;
      incr[1] = 1.0;
      incr[2] = 0.1;
    }
  }


  opengl->gridxy_1_display_list = glGenLists (1);
  glNewList (opengl->gridxy_1_display_list, GL_COMPILE);

/*  glDisable (GL_DEPTH_TEST); */
    /* Lines spanning X axis */
  for (i = 0.0; i < opengl->gcode->material_size[0]; i += incr[0])
  {
    glVertex3f (opengl->matx_origin + i, opengl->maty_origin, 0.0);
    glVertex3f (opengl->matx_origin + i, opengl->maty_origin + opengl->gcode->material_size[1], 0.0);
  }

  /* Lines spanning Y axis */
  for (i = 0.0; i < opengl->gcode->material_size[1]; i += incr[0])
  {
    glVertex3f (opengl->matx_origin, opengl->maty_origin + i, 0.0);
    glVertex3f (opengl->matx_origin + opengl->gcode->material_size[0], opengl->maty_origin + i, 0.0);
  }

  /* Borders at Zero */
  glBegin (GL_LINES);
    glColor3f (GRID_BORDER_LINE_COLOR, GRID_BORDER_LINE_COLOR, GRID_BORDER_LINE_COLOR);

    glVertex3f (opengl->matx_origin, opengl->maty_origin, 0.0);
    glVertex3f (opengl->matx_origin + opengl->gcode->material_size[0], opengl->maty_origin, 0.0);

    glVertex3f (opengl->matx_origin, opengl->maty_origin + opengl->gcode->material_size[1], 0.0);
    glVertex3f (opengl->matx_origin + opengl->gcode->material_size[0], opengl->maty_origin + opengl->gcode->material_size[1], 0.0);

    glVertex3f (opengl->matx_origin, opengl->maty_origin, 0.0);
    glVertex3f (opengl->matx_origin, opengl->maty_origin + opengl->gcode->material_size[1], 0.0);

    glVertex3f (opengl->matx_origin + opengl->gcode->material_size[0], opengl->maty_origin, 0.0);
    glVertex3f (opengl->matx_origin + opengl->gcode->material_size[0], opengl->maty_origin + opengl->gcode->material_size[1], 0.0);
  glEnd ();

  /* Borders at Depth */
  glBegin (GL_LINES);
    glColor3f (GRID_BORDER_LINE_COLOR, GRID_BORDER_LINE_COLOR, GRID_BORDER_LINE_COLOR);

    glVertex3f (opengl->matx_origin, opengl->maty_origin, -opengl->gcode->material_size[2]);
    glVertex3f (opengl->matx_origin + opengl->gcode->material_size[0], opengl->maty_origin, -opengl->gcode->material_size[2]);

    glVertex3f (opengl->matx_origin, opengl->maty_origin + opengl->gcode->material_size[1], -opengl->gcode->material_size[2]);
    glVertex3f (opengl->matx_origin + opengl->gcode->material_size[0], opengl->maty_origin + opengl->gcode->material_size[1], -opengl->gcode->material_size[2]);

    glVertex3f (opengl->matx_origin, opengl->maty_origin, -opengl->gcode->material_size[2]);
    glVertex3f (opengl->matx_origin, opengl->maty_origin + opengl->gcode->material_size[1], -opengl->gcode->material_size[2]);

    glVertex3f (opengl->matx_origin + opengl->gcode->material_size[0], opengl->maty_origin, -opengl->gcode->material_size[2]);
    glVertex3f (opengl->matx_origin + opengl->gcode->material_size[0], opengl->maty_origin + opengl->gcode->material_size[1], -opengl->gcode->material_size[2]);
  glEnd ();

  /* Borders at Origin */
  glLineWidth (2);
  glBegin (GL_LINES);
    glColor3f (0.7, 0.2, 0.2);

    glVertex3f (opengl->matx_origin, opengl->maty_origin + opengl->gcode->material_origin[1], 0.0);
    glVertex3f (opengl->matx_origin + opengl->gcode->material_size[0], opengl->maty_origin +  opengl->gcode->material_origin[1], 0.0);

    glVertex3f (opengl->matx_origin + opengl->gcode->material_origin[0], opengl->maty_origin, 0.0);
    glVertex3f (opengl->matx_origin + opengl->gcode->material_origin[0], opengl->maty_origin + opengl->gcode->material_size[1], 0.0);
  glEnd ();
  glLineWidth (1);

/*  glEnable (GL_DEPTH_TEST); */
  glEndList ();




  opengl->gridxy_2_display_list = glGenLists (2);
  glNewList (opengl->gridxy_2_display_list, GL_COMPILE);

/*  glDisable (GL_DEPTH_TEST); */
    /* Lines spanning X axis */
  for (i = 0.0; i < opengl->gcode->material_size[0]; i += incr[1])
  {
    glVertex3f (opengl->matx_origin + i, opengl->maty_origin, 0.0);
    glVertex3f (opengl->matx_origin + i, opengl->maty_origin + opengl->gcode->material_size[1], 0.0);
  }

  /* Lines spanning Y axis */
  for (i = 0.0; i < opengl->gcode->material_size[1]; i += incr[1])
  {
    glVertex3f (opengl->matx_origin, opengl->maty_origin + i, 0.0);
    glVertex3f (opengl->matx_origin + opengl->gcode->material_size[0], opengl->maty_origin + i, 0.0);
  }

/*  glEnable (GL_DEPTH_TEST); */
  glEndList ();



  opengl->gridxy_3_display_list = glGenLists (3);
  glNewList (opengl->gridxy_3_display_list, GL_COMPILE);

/*  glDisable (GL_DEPTH_TEST); */
  /* Lines spanning X axis */
  for (i = 0.0; i < opengl->gcode->material_size[0]; i += incr[2])
  {
    glVertex3f (opengl->matx_origin + i, opengl->maty_origin, 0.0);
    glVertex3f (opengl->matx_origin + i, opengl->maty_origin + opengl->gcode->material_size[1], 0.0);
  }

  /* Lines spanning Y axis */
  for (i = 0.0; i < opengl->gcode->material_size[1]; i += incr[2])
  {
    glVertex3f (opengl->matx_origin, opengl->maty_origin + i, 0.0);
    glVertex3f (opengl->matx_origin + opengl->gcode->material_size[0], opengl->maty_origin + i, 0.0);
  }

/*  glEnable (GL_DEPTH_TEST); */
  glEndList ();
}


void
gui_opengl_build_gridxz_display_list (gui_opengl_t *opengl)
{
  int16_t i;

  opengl->gridxz_display_list = glGenLists (4);
  glNewList (opengl->gridxz_display_list, GL_COMPILE);

  glDisable (GL_LIGHTING);

  /* Lines spanning X axis */
  glLineWidth (1);
  for (i = 0; i <= 5*opengl->gcode->material_size[0]; i++)
  {
    glBegin (GL_LINES);
      glColor3f (GRID_MAJOR_LINE_COLOR, GRID_MAJOR_LINE_COLOR, GRID_MAJOR_LINE_COLOR);

      glVertex3f (i*0.1, 0.0, 0.0);
      glVertex3f (i*0.1, -opengl->gcode->material_size[2], 0.0);
    glEnd ();
  }

  /* Lines spanning Z axis */
  for (i = 0; i < 10*opengl->gcode->material_size[2]; i++)
  {
    glBegin (GL_LINES);
      glColor3f (GRID_MAJOR_LINE_COLOR, GRID_MAJOR_LINE_COLOR, GRID_MAJOR_LINE_COLOR);
      glVertex3f (0.0, -i*0.1, 0.0);
      glVertex3f (opengl->gcode->material_size[0]*0.5, -i*0.1, 0.0);
    glEnd ();
  }

  /* Borders */
  glBegin (GL_LINES);
    glColor3f (GRID_BORDER_LINE_COLOR, GRID_BORDER_LINE_COLOR, GRID_BORDER_LINE_COLOR);

    glVertex3f (0.0, 0.0, 0.0);
    glVertex3f (opengl->gcode->material_size[0]*0.5, 0.0, 0.0);

    glVertex3f (0.0, -opengl->gcode->material_size[2], 0.0);
    glVertex3f (opengl->gcode->material_size[0]*0.5, -opengl->gcode->material_size[2], 0.0);
  glEnd ();

  /* Zero Line */
  glBegin (GL_LINES);
    glColor3f (1.0, 0.0, 0.0);
    glVertex3f (0.0, 0.0, 0.0);
    glVertex3f (0.0, -opengl->gcode->material_size[2], 0.0);
  glEnd ();

  glEndList ();
}


static void
draw_mill (gui_opengl_t *opengl)
{
  uint32_t i;
  gfloat_t angle;
  gfloat_t zaxis;


  glColor3f (0.0, 1.0, 1.0);
  /* End Mill */
  zaxis = opengl->gcode->material_size[2] + 0.25;

  glBegin (GL_LINE_LOOP);
  for (i = 0; i < 18; i++)
  {
    angle = GCODE_DEG2RAD * (((gfloat_t)i / (gfloat_t)18) * 360);
/*    glVertex3f (0.5 * gui.env->end_mill_diameter * cos (angle), 0.5 * gui.env->end_mill_diameter * sin (angle), zaxis); */
  }
  glEnd ();

  glBegin (GL_LINE_LOOP);
  for (i = 0; i < 18; i++)
  {
    angle = GCODE_DEG2RAD * (((gfloat_t)i / (gfloat_t)18) * 360);
/*    glVertex3f (0.5 * gui.env->end_mill_diameter * cos (angle), 0.5 * gui.env->end_mill_diameter * sin (angle), zaxis + gui.env->end_mill_length); */
  }
  glEnd ();

  glBegin (GL_LINES);
    angle = GCODE_DEG2RAD * 0;
/*    glVertex3f (0.5 * gui.env->end_mill_diameter * cos (angle), 0.5 * gui.env->end_mill_diameter * sin (angle), zaxis); */
/*    glVertex3f (0.5 * gui.env->end_mill_diameter * cos (angle), 0.5 * gui.env->end_mill_diameter * sin (angle), zaxis + gui.env->end_mill_length); */

    angle = GCODE_DEG2RAD * 90;
/*    glVertex3f (0.5 * gui.env->end_mill_diameter * cos (angle), 0.5 * gui.env->end_mill_diameter * sin (angle), zaxis); */
/*    glVertex3f (0.5 * gui.env->end_mill_diameter * cos (angle), 0.5 * gui.env->end_mill_diameter * sin (angle), zaxis + gui.env->end_mill_length); */

    angle = GCODE_DEG2RAD * 180;
/*    glVertex3f (0.5 * gui.env->end_mill_diameter * cos (angle), 0.5 * gui.env->end_mill_diameter * sin (angle), zaxis); */
/*    glVertex3f (0.5 * gui.env->end_mill_diameter * cos (angle), 0.5 * gui.env->end_mill_diameter * sin (angle), zaxis + gui.env->end_mill_length); */

    angle = GCODE_DEG2RAD * 270;
/*    glVertex3f (0.5 * gui.env->end_mill_diameter * cos (angle), 0.5 * gui.env->end_mill_diameter * sin (angle), zaxis); */
/*    glVertex3f (0.5 * gui.env->end_mill_diameter * cos (angle), 0.5 * gui.env->end_mill_diameter * sin (angle), zaxis + gui.env->end_mill_length); */
  glEnd ();
}


static void
sum_normal (gui_opengl_t *opengl, int i, int j, int k, gcode_vec3d_t nor)
{
  int ind;

  if (i < 0 || j < 0 || k < 0 || i >= opengl->gcode->voxel_num[0] || j >= opengl->gcode->voxel_num[1] || k >= opengl->gcode->voxel_num[2])
    return;

  ind = i + j * opengl->gcode->voxel_num[0] + k * (opengl->gcode->voxel_num[0] * opengl->gcode->voxel_num[1]);


  /*
  * Compute Normal Based on Existance of Neighbors
  * -x = -1
  * +x = +1
  * -y = -opengl->gcode->voxel_num[1]
  * +y = +opengl->gcode->voxel_num[1]
  * -z = -(opengl->gcode->voxel_num[0] * opengl->gcode->voxel_num[1])
  * +z = +(opengl->gcode->voxel_num[0] * opengl->gcode->voxel_num[1])
  */

  /* X */
  if (i > 0)
  {
    if (!opengl->gcode->voxel_map[ind-1])
      nor[0] += -1.0;
  }
  else
  {
    nor[0] += -1.0;
  }

  if (i < opengl->gcode->voxel_num[0]-1)
  {
    if (!opengl->gcode->voxel_map[ind+1])
      nor[0] += 1.0;
  }
  else
  {
    nor[0] += 1.0;
  }

  /* Y */
  if (j > 0)
  {
    if (!opengl->gcode->voxel_map[ind-opengl->gcode->voxel_num[0]])
      nor[1] += -1.0;
  }
  else
  {
    nor[1] += -1.0;
  }

  if (j < opengl->gcode->voxel_num[1]-1)
  {
    if (!opengl->gcode->voxel_map[ind+opengl->gcode->voxel_num[0]])
      nor[1] += 1.0;
  }
  else
  {
    nor[1] += 1.0;
  }

  /* Z */
  if (k > 0)
  {
    if (!opengl->gcode->voxel_map[ind-opengl->gcode->voxel_num[0]*opengl->gcode->voxel_num[1]])
      nor[2] += -1.0;
  }
  else
  {
    nor[2] += -1.0;
  }

  if (k < opengl->gcode->voxel_num[2]-1)
  {
    if (!opengl->gcode->voxel_map[ind+opengl->gcode->voxel_num[0]*opengl->gcode->voxel_num[1]])
      nor[2] += 1.0;
  }
  else
  {
    nor[2] += 1.0;
  }
}


void
gui_opengl_build_simulate_display_list (gui_opengl_t *opengl)
{
  int16_t i, j, k;
  gfloat_t vx, vy, vz;
  gcode_vec3d_t nor;
  int ind;
  GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_diffuse[] = { 0.6, 0.6, 0.6, 1.0 };
  GLfloat mat_specular[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat mat_shininess[] = { 0.0 };


  if (!opengl->gcode->voxel_map)
    return;

  opengl->simulate_display_list = glGenLists (5);
  glNewList (opengl->simulate_display_list, GL_COMPILE);

  glLightModeli (GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);

  glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
  glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
  glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

  glPointSize (2);
  glBegin (GL_POINTS);

  ind = 0;
  for (k = 0; k < opengl->gcode->voxel_num[2]; k++)
  {
    vz = ((gfloat_t) k/(gfloat_t) opengl->gcode->voxel_num[2]) * opengl->gcode->material_size[2] - opengl->gcode->material_size[2];

    /* Update Progress based on Z for now */
    opengl->progress_callback (opengl->gcode->gui, (gfloat_t)(k+1) / (gfloat_t)opengl->gcode->voxel_num[2]);

    for (j = 0; j < opengl->gcode->voxel_num[1]; j++)
    {
      vy = -opengl->gcode->material_size[1]*0.5 + ((gfloat_t)j/(gfloat_t)opengl->gcode->voxel_num[1]) * opengl->gcode->material_size[1];
      for (i = 0; i < opengl->gcode->voxel_num[0]; i++)
      {
        vx = -opengl->gcode->material_size[0]*0.5 + ((gfloat_t)i/(gfloat_t)opengl->gcode->voxel_num[0]) * opengl->gcode->material_size[0];

        if (opengl->gcode->voxel_map[ind])
        {
          nor[0] = 0.0;
          nor[1] = 0.0;
          nor[2] = 0.0;

          sum_normal (opengl, i, j, k, nor);

          sum_normal (opengl, i-1, j, k, nor);
          sum_normal (opengl, i+1, j, k, nor);
          sum_normal (opengl, i, j-1, k, nor);
          sum_normal (opengl, i, j+1, k, nor);
          sum_normal (opengl, i, j, k-1, nor);
          sum_normal (opengl, i, j, k+1, nor);
#if 0
          sum_normal (opengl, i-2, j, k, nor);
          sum_normal (opengl, i+2, j, k, nor);

          sum_normal (opengl, i-2, j+1, k, nor);
          sum_normal (opengl, i+2, j+1, k, nor);

          sum_normal (opengl, i-2, j-1, k, nor);
          sum_normal (opengl, i+2, j-1, k, nor);

          sum_normal (opengl, i, j+2, k, nor);
          sum_normal (opengl, i, j-2, k, nor);

          sum_normal (opengl, i+1, j+2, k, nor);
          sum_normal (opengl, i+1, j-2, k, nor);

          sum_normal (opengl, i-1, j+2, k, nor);
          sum_normal (opengl, i-1, j-2, k, nor);
#endif

/*          GCODE_MATH_VEC3D_UNITIZE(nor) */

          if (fabs (nor[0]) + fabs (nor[1]) + fabs (nor[2]) > 0.0)
          {
            glNormal3f (nor[0], nor[1], nor[2]);
            glVertex3f (vx, vy, vz);
          }
        }
        ind++;
      }
    }
  }

  glEnd ();

  glEndList ();
}


static void
draw_top_level_blocks (gui_opengl_t *opengl, gcode_block_t *selected_block)
{
  gcode_block_t *block;

  glEnable (GL_DEPTH_TEST);
  if (opengl->rebuild_view_display_list)
  {
    if (opengl->view_display_list)
      glDeleteLists (opengl->view_display_list, 1);
    opengl->view_display_list = glGenLists (6);
    glNewList (opengl->view_display_list, GL_COMPILE);

    glTranslatef (opengl->matx_origin + opengl->gcode->material_origin[0], opengl->maty_origin + opengl->gcode->material_origin[1], 0.0);
    block = opengl->gcode->list;
    while (block)
    {
      if (block->draw)
        block->draw (block, selected_block);

      block = block->next;
    }
    glTranslatef (-opengl->matx_origin - opengl->gcode->material_origin[0], -opengl->maty_origin - opengl->gcode->material_origin[1], 0.0);
    glEndList ();

    opengl->rebuild_view_display_list = 0;
  }

  glCallList (opengl->view_display_list);
}


static void
set_projection (gui_opengl_t *opengl, uint8_t view)
{
  gfloat_t aspect, fov;

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  aspect = (gfloat_t) opengl->context_h / (gfloat_t) opengl->context_w;
  if (opengl->projection == GUI_OPENGL_PROJECTION_PERSPECTIVE)
  {
    fov = GCODE_UNITS (opengl->gcode, GUI_OPENGL_MIN_ZOOM) * 12.5 * GCODE_PI / 180.0;
    glFrustum (-tan (fov), tan (fov), -aspect*tan (fov), aspect*tan (fov), GCODE_UNITS (opengl->gcode, GUI_OPENGL_MIN_ZOOM) * 0.2, GCODE_UNITS (opengl->gcode, GUI_OPENGL_MAX_ZOOM) * 0.2);
  }
  else
  {
    glOrtho (-opengl->view[view].grid, opengl->view[view].grid, -opengl->view[view].grid*aspect, opengl->view[view].grid*aspect, -GCODE_UNITS (opengl->gcode, GUI_OPENGL_MIN_ZOOM) * 5.0, GCODE_UNITS (opengl->gcode, GUI_OPENGL_MAX_ZOOM) * 5.0);
  }

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
}


static void
draw_grid (gui_opengl_t *opengl, int view)
{
  gfloat_t coef, zoom;

  glDisable (GL_DEPTH_TEST);

  if (opengl->projection == GUI_OPENGL_PROJECTION_PERSPECTIVE)
  {
    zoom = 0.15 * opengl->view[view].zoom;
  }
  else
  {
    zoom = 0.15 * opengl->view[view].grid;
  }

  if (opengl->gcode->units == GCODE_UNITS_INCH)
  {
    if (opengl->gcode->material_size[0] >= 12.0 || opengl->gcode->material_size[1] >= 12.0)
      zoom *= 0.1;
  }
  else
  {
    if (opengl->gcode->material_size[0] >= 1000.0 || opengl->gcode->material_size[1] >= 1000.0)
      zoom *= 0.1;
  }


  /* Fine Grid */
  glLineWidth (1);
  if (opengl->gcode->units == GCODE_UNITS_INCH)
  {
    coef = 0.04 / zoom;
  }
  else
  {
    coef = 0.4 / zoom;
  }

  if (coef > 0.5)
  {
    glBegin (GL_LINES);
    glColor3f (0.35, 0.35, 0.35);
    glCallList (opengl->gridxy_3_display_list);
    glEnd ();
  }


  /* Medium Grid */
  if (opengl->gcode->units == GCODE_UNITS_INCH)
  {
    coef = 0.2 / zoom;
  }
  else
  {
    coef = 2.0 / zoom;
  }

  if (coef > 0.5)
  {
    glBegin (GL_LINES);
//    coef = GUI_OPENGL_CLEAR_COLOR + (coef - GUI_OPENGL_CLEAR_COLOR) * 0.4;
    glColor3f (0.55, 0.55, 0.55);
    glCallList (opengl->gridxy_2_display_list);
    glEnd ();
  }


  /* Coarse Grid */
  glLineWidth (1);
  glBegin (GL_LINES);
//  coef = 0.9;
  glColor3f (0.85, 0.85, 0.85);
  glCallList (opengl->gridxy_1_display_list);
  glEnd ();
}


static void
draw_all (gui_opengl_t *opengl, gcode_block_t *block)
{
  int view = GUI_OPENGL_VIEW_REGULAR;

  if (block->parent)
    if (block->parent->type == GCODE_TYPE_EXTRUSION)
      view = GUI_OPENGL_VIEW_EXTRUSION;
  if (block->type == GCODE_TYPE_EXTRUSION)
    view = GUI_OPENGL_VIEW_EXTRUSION;

  set_projection (opengl, view);	

  glTranslatef (0, 0, -opengl->view[view].zoom);
  glRotatef (opengl->view[view].elev-90.0, 1, 0, 0);
  glRotatef (opengl->view[view].azim, 0, 0, 1);
  glTranslatef (-opengl->view[view].pos[0], -opengl->view[view].pos[1], -opengl->view[view].pos[2]);

  if (block->type == GCODE_TYPE_EXTRUSION)
  {
    glDisable (GL_DEPTH_TEST);
    glCallList (opengl->gridxz_display_list);
    block->draw (block, 0);
  }
  else if (block->parent)
  {
    if (block->parent->type != GCODE_TYPE_EXTRUSION)
    {
      draw_grid (opengl, view);
      draw_top_level_blocks (opengl, block);
    }
    else if (block->parent->type == GCODE_TYPE_EXTRUSION)
    {
      glDisable (GL_DEPTH_TEST);
      glCallList (opengl->gridxz_display_list);
      block->parent->draw (block->parent, block);
    }
  }
  else if (block->type != GCODE_TYPE_EXTRUSION)
  {
    draw_grid (opengl, view);

/* glTranslatef (opengl->matx_origin, opengl->maty_origin, 0.0); */
    glEnable (GL_DEPTH_TEST);
/* block->draw (block, block); old behavior */
    draw_top_level_blocks (opengl, block); /* new behavior, show everything now that suppress exists */
/* glTranslatef (-opengl->matx_origin, -opengl->maty_origin, 0.0); */
  }

/*
  glCallList (opengl->point_display_list);
  draw_mill ();
*/
}


void
gui_opengl_context_redraw (gui_opengl_t *opengl, gcode_block_t *block)
{
  uint8_t view;

  view = GUI_OPENGL_VIEW_REGULAR;

  gdk_gl_drawable_gl_begin (opengl->gl_drawable, opengl->gl_context);

//  set_projection (opengl, view);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  glOrtho (-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

  /* Draw Background Gradient */
  glDisable (GL_LIGHTING);
  glBegin (GL_QUADS);
    glColor3f (0.12, 0.12, 0.12);
    glVertex3f (-1.0, -1.0, 0.0);

    glColor3f (0.14, 0.14, 0.14);
    glVertex3f (1.0, -1.0, 0.0);

    glColor3f (0.28, 0.28, 0.28);
    glVertex3f (1.0, 1.0, 0.0);

    glColor3f (0.18, 0.18, 0.18);
    glVertex3f (-1.0, 1.0, 0.0);
  glEnd ();


set_projection (opengl, view);

  /* Make sure the block is non NULL and has as a drawing function */
  if (opengl->ready)
    switch (opengl->mode)
    {
      case GUI_OPENGL_MODE_EDIT:
        glDisable (GL_LIGHTING);
        if (block)
        {
          if (block->draw)
          {
            draw_all (opengl, block);
          }
          else
          {
            glTranslatef (0, 0, -opengl->view[view].zoom);
            glRotatef (opengl->view[view].elev-90.0, 1, 0, 0);
            glRotatef (opengl->view[view].azim, 0, 0, 1);
            glTranslatef (-opengl->view[view].pos[0], -opengl->view[view].pos[1], -opengl->view[view].pos[2]);

            draw_grid (opengl, view);

            draw_top_level_blocks (opengl, block->parent);
          }
        }
        break;

      case GUI_OPENGL_MODE_RENDER:
        {
          int size;

          glTranslatef (0, 0, -opengl->view[view].zoom);
          glRotatef (opengl->view[view].elev-90.0, 1, 0, 0);
          glRotatef (opengl->view[view].azim, 0, 0, 1);
          glTranslatef (-opengl->view[view].pos[0], -opengl->view[view].pos[1], -opengl->view[view].pos[2]);

/*        draw_grid (opengl, view); */

          glEnable (GL_DEPTH_TEST);

          if (opengl->projection == GUI_OPENGL_PROJECTION_PERSPECTIVE)
          {
            size = (int) ((opengl->context_w / 500.0) * 5.0 / opengl->view[view].zoom);
          }
          else
          {
            size = (int) ((opengl->context_w / 500.0) * 5.0 / opengl->view[view].grid);
          }
/*
            glPointSize (size);
            glPointSize (1);
*/
          glCallList (opengl->simulate_display_list);
        }
        break;

      default:
        break;
    }

  gdk_gl_drawable_swap_buffers (opengl->gl_drawable);
  gdk_gl_drawable_gl_end (opengl->gl_drawable);
}


void gui_opengl_pick (gui_opengl_t *opengl, int x, int y)
{
  gfloat_t aspect, fov;
  GLuint buff[64];
  int viewport[4], hits;
  uint8_t view;

  view = GUI_OPENGL_VIEW_REGULAR;

  glSelectBuffer (64, buff);

  glGetIntegerv (GL_VIEWPORT, viewport);

  glRenderMode(GL_SELECT);

  glInitNames ();

  glPushName (0);

  /* View Configuration and Draw */
  glMatrixMode (GL_PROJECTION);

  glPushMatrix ();

  glLoadIdentity ();

  gluPickMatrix (x, viewport[3] - y, 15.0, 15.0, viewport);

  aspect = (gfloat_t) opengl->context_h / (gfloat_t) opengl->context_w;
  if (opengl->projection == GUI_OPENGL_PROJECTION_PERSPECTIVE)
  {
    fov = GCODE_UNITS (opengl->gcode, GUI_OPENGL_MIN_ZOOM) * 12.5 * GCODE_PI / 180.0;
    glFrustum (-tan (fov), tan (fov), -aspect*tan (fov), aspect*tan (fov), GCODE_UNITS (opengl->gcode, GUI_OPENGL_MIN_ZOOM) * 0.2, GCODE_UNITS (opengl->gcode, GUI_OPENGL_MAX_ZOOM) * 0.2);
  }
  else
  {
    glOrtho (-opengl->view[view].grid, opengl->view[view].grid, -opengl->view[view].grid*aspect, opengl->view[view].grid*aspect, -GCODE_UNITS (opengl->gcode, GUI_OPENGL_MIN_ZOOM) * 5.0, GCODE_UNITS (opengl->gcode, GUI_OPENGL_MAX_ZOOM) * 5.0);
  }

  glMatrixMode (GL_MODELVIEW);

  glLoadIdentity ();

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glTranslatef (0, 0, -opengl->view[view].zoom);
  glRotatef (opengl->view[view].elev-90.0, 1, 0, 0);
  glRotatef (opengl->view[view].azim, 0, 0, 1);
  glTranslatef (-opengl->view[view].pos[0], -opengl->view[view].pos[1], -opengl->view[view].pos[2]);


  draw_top_level_blocks (opengl, NULL);

  glPopMatrix ();

  hits = glRenderMode (GL_RENDER);

#if 0
{
  int i;

  printf ("x: %d, y: %d, hits: %d\n", x, y, hits);
  for (i = 0; i < hits; i++)
  {
    printf ("hit[%d]: %d, min: %d, max: %d, name: %x\n", i, buff[i*4], buff[i*4+1], buff[i*4+2], buff[i*4+3]);
  }
}
#endif

  /*
  * For now, take the first hit and search the entire block treeview for the block ptr whose value
  * matches the name value from the first hit in the list.  Once located, highlight that row.
  */
  if (hits > 0)
  {
    gcode_block_t *ptr;

    ptr = (gcode_block_t *) ((((uint64_t) buff[3]) << 3) + (uint64_t) opengl->gcode);

    set_selected_row_with_block (opengl->gcode->gui, ptr);
    gui_tab_display (opengl->gcode->gui, ptr, 0);
  }

/*
glGetIntegerv(GL_MAX_NAME_STACK_DEPTH, &foo);
printf ("MAX_DEPTH: %d\n", foo);

glGetIntegerv(GL_NAME_STACK_DEPTH, &foo);
printf ("foo: %d\n", foo);
*/
}


void
gui_opengl_context_prep (gui_opengl_t *opengl)
{
  /* Set the default extrusion orthographic view. */
  opengl->view[GUI_OPENGL_VIEW_EXTRUSION].pos[0] = opengl->gcode->material_size[0] * 0.25;
  opengl->view[GUI_OPENGL_VIEW_EXTRUSION].pos[1] = -opengl->gcode->material_size[2] * 0.5;
  opengl->view[GUI_OPENGL_VIEW_EXTRUSION].pos[2] = 0.0;

  opengl->view[GUI_OPENGL_VIEW_EXTRUSION].elev = 90;
  opengl->view[GUI_OPENGL_VIEW_EXTRUSION].azim = 0;
  opengl->projection = GUI_OPENGL_PROJECTION_ORTHOGRAPHIC;
/*  aspect = (gfloat_t) opengl->context_w > (gfloat_t) opengl->context_h ? (gfloat_t) opengl->context_h / (gfloat_t) opengl->context_w : (gfloat_t) opengl->context_w / (gfloat_t) opengl->context_h; */
  opengl->view[GUI_OPENGL_VIEW_EXTRUSION].grid = (opengl->gcode->material_size[0] > opengl->gcode->material_size[2] ? 0.5 * opengl->gcode->material_size[0] : opengl->gcode->material_size[2]);
  opengl->view[GUI_OPENGL_VIEW_EXTRUSION].zoom = 1.0;

  gui_opengl_build_gridxy_display_list (opengl);
  gui_opengl_build_gridxz_display_list (opengl);

  opengl->view[GUI_OPENGL_VIEW_REGULAR].elev = 90;
  opengl->view[GUI_OPENGL_VIEW_REGULAR].azim = 0;
  opengl->view[GUI_OPENGL_VIEW_REGULAR].pos[0] = 0.0;
  opengl->view[GUI_OPENGL_VIEW_REGULAR].pos[1] = 0.0;
  opengl->view[GUI_OPENGL_VIEW_REGULAR].pos[2] = 0.0;
}


void
gui_opengl_context_init (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
  gui_opengl_t *opengl;

  opengl = (gui_opengl_t *) data;

  opengl->context_w = (uint16_t) widget->allocation.width;
  opengl->context_h = (uint16_t) widget->allocation.height;

  opengl->gl_context = gtk_widget_get_gl_context (widget);
  opengl->gl_drawable = gtk_widget_get_gl_drawable (widget);

  opengl->rebuild_view_display_list = 1;
  opengl->view_display_list = 0;

  gdk_gl_drawable_gl_begin (opengl->gl_drawable, opengl->gl_context);

  glViewport (0, 0, opengl->context_w, opengl->context_h);
  glClearColor (GUI_OPENGL_CLEAR_COLOR, GUI_OPENGL_CLEAR_COLOR, GUI_OPENGL_CLEAR_COLOR, 1.0);

  glDisable (GL_DEPTH_TEST);
  glEnable (GL_NORMALIZE);
/*
  glEnable (GL_LINE_SMOOTH);
  glEnable (GL_POINT_SMOOTH);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
  glHint (GL_POINT_SMOOTH_HINT, GL_DONT_CARE);
  glDepthMask (GL_FALSE);
*/
  gdk_gl_drawable_gl_end (opengl->gl_drawable);

  opengl->mode = GUI_OPENGL_MODE_EDIT;

  gui_opengl_context_redraw (opengl, NULL);
}
