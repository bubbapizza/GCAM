/*
*  gcode_util.c
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
#include "gcode_util.h"
#include <inttypes.h>
#include "gcode.h"
#include "gcode_arc.h"
#include "gcode_line.h"


int
gcode_util_qsort_compare_asc (const void *a, const void *b)
{
  gfloat_t x, y;

  x = *(gfloat_t *) a;
  y = *(gfloat_t *) b;

  if (fabs (x-y) < GCODE_PRECISION)
    return (0);

  return (x < y ? -1 : 1);
}


void
gcode_util_remove_spaces (char *string)
{
  uint32_t i, n;

  i = 0;

  while (i < strlen (string))
  {
    if (string[i] == ' ')
    {
      for (n = i; n < strlen (string); n++)
      {
        string[n] = string[n+1];
      }
    }
    else
    {
      i++;
    }
  }
}


void
gcode_util_remove_comment (char *string)
{
  uint32_t i;

  i = 0;
  while (string[i] != '\0' && string[i] != ';')
    i++;
  string[i] = 0;
}


void
gcode_util_remove_duplicate_scalars (gfloat_t *array, uint32_t *num)
{
  int32_t i, j, num2;

  num2 = *num;
  for (i = 0; i < num2-1; i++)
  {
    if (fabs (array[i+1] - array[i]) < GCODE_PRECISION)
    {
      for (j = i; j < num2-1; j++)
      {
        array[j] = array[j+1];
      }
      num2--;
    }
  }
  *num = num2;
}

void
gcode_util_duplicate_list (gcode_block_t *start_block, gcode_block_t *end_block, gcode_block_t **duplicate_list)
{
  gcode_block_t *duplicate_block, *last_block;

  *duplicate_list = NULL;

  while (start_block != end_block)
  {
    start_block->duplicate (start_block, &duplicate_block);

    if (*duplicate_list)
    {
      gcode_list_insert (&last_block, duplicate_block);
    }
    else
    {
      gcode_list_insert (duplicate_list, duplicate_block);
    }

    last_block = duplicate_block;
    start_block = start_block->next;
  }
}


static int
line_arc_intersect (gcode_block_t *line_block, gcode_block_t *arc_block, gcode_vec2d_t ip_array[2], int *ip_num)
{
  gcode_line_t *line;
  gcode_arc_t *arc;
  gcode_vec2d_t line_p0, line_p1, line_normal, arc_origin, arc_center, arc_p0, arc_p1, min, max;
  gfloat_t arc_radius, line_dx, line_dy, line_dr, line_dr_inv, line_d, line_sgn, line_disc, arc_start_angle, angle;
  int p0_test, p1_test;


  *ip_num = 0;

  line = (gcode_line_t *) line_block->pdata;
  arc = (gcode_arc_t *) arc_block->pdata;

  gcode_arc_with_offset (arc_block, arc_origin, arc_center, arc_p0, &arc_radius, &arc_start_angle);
  if (arc_radius <= GCODE_PRECISION)
    return (1);

  gcode_line_with_offset (line_block, line_p0, line_p1, line_normal);

/* printf ("line: %s, %.12f,%.12f %.12f,%.12f  normal: %.12f,%.12f  arc: %s, radius: %.12f, center: %.12f,%.12f\n", line_block->comment, line_p0[0], line_p0[1], line_p1[0], line_p1[1], line_normal[0], line_normal[1], arc_block->comment, arc_radius, arc_center[0], arc_center[1]); */

  /*
  * Circle-Line Intersection from Wolfram MathWorld.
  * Subtract circle center from line points to represent circle center as 0,0.
  */

  line_p0[0] -= arc_center[0];
  line_p0[1] -= arc_center[1];
  line_p1[0] -= arc_center[0];
  line_p1[1] -= arc_center[1];


  line_dx = line_p1[0] - line_p0[0];
  line_dy = line_p1[1] - line_p0[1];

  line_dr = sqrt (line_dx*line_dx + line_dy*line_dy);
  line_d = line_p0[0]*line_p1[1] - line_p1[0]*line_p0[1];

/* printf ("line: %.12f,%.12f %.12f,%.12f\n", line_p0[0], line_p0[1], line_p1[0], line_p1[1]); */
/* printf ("line_disc: %.12f * %.12f - %.12f\n", arc_radius, line_dr, line_d); */
  line_disc = arc_radius * arc_radius * line_dr * line_dr - line_d * line_d;

  /* Prevent floating fuzz from turning the zero discriminant into an imaginary number. */
/* printf ("line_disc: %.12f, arc_pos: %f,%f\n", line_disc, arc->pos[0], arc->pos[1]); */
  if (line_disc < 0.0 && line_disc > -GCODE_PRECISION*GCODE_PRECISION)
    line_disc = 0.0;

  if (line_disc < 0.0)
  {
/*    printf ("no intersection: %.12f\n", line_disc); */
    return (1);
  }

  line_disc = sqrt (line_disc); /* optimization */
  line_dr *= line_dr; /* optimization */
  line_dr_inv = 1.0 / line_dr;
  line_sgn = line_dy < 0.0 ? -1.0 : 1.0;

  /* Compute bounding box for line */
  if (line->p0[0] < line->p1[0])
  {
    min[0] = line->p0[0];
    max[0] = line->p1[0];
  }
  else
  {
    min[0] = line->p1[0];
    max[0] = line->p0[0];
  }

  if (line->p0[1] < line->p1[1])
  {
    min[1] = line->p0[1];
    max[1] = line->p1[1];
  }
  else
  {
    min[1] = line->p1[1];
    max[1] = line->p0[1];
  }
  min[0] -= GCODE_PRECISION;
  min[1] -= GCODE_PRECISION;
  max[0] += GCODE_PRECISION;
  max[1] += GCODE_PRECISION;



  arc_p0[0] = arc_center[0] + (line_d * line_dy + line_sgn * line_dx * line_disc) * line_dr_inv;
  arc_p0[1] = arc_center[1] + (-line_d * line_dx + fabs (line_dy) * line_disc) * line_dr_inv;
  /* Check that the point falls within the bounds of the line segment */
  p0_test = 0;
  if (arc_p0[0] >= min[0] && arc_p0[0] <= max[0] && arc_p0[1] >= min[1] && arc_p0[1] <= max[1])
  {
    gcode_math_xy_to_angle (arc_center, arc_radius, arc_p0[0], arc_p0[1], &angle);
    p0_test = gcode_math_angle_within_arc (arc_start_angle, arc->sweep, angle) ? 0 : 1;
  }

  arc_p1[0] = arc_center[0] + (line_d * line_dy - line_sgn * line_dx * line_disc) * line_dr_inv;
  arc_p1[1] = arc_center[1] + (-line_d * line_dx - fabs (line_dy) * line_disc) * line_dr_inv;
  p1_test = 0;
  if (arc_p1[0] >= min[0] && arc_p1[0] <= max[0] && arc_p1[1] >= min[1] && arc_p1[1] <= max[1])
  {
    gcode_math_xy_to_angle (arc_center, arc_radius, arc_p1[0], arc_p1[1], &angle);
    p1_test = gcode_math_angle_within_arc (arc_start_angle, arc->sweep, angle) ? 0 : 1;
  }


  if (p0_test && line_disc > GCODE_PRECISION) /* Handles Tangent case where the discriminant equals 0.0 */
  {
    ip_array[*ip_num][0] = arc_p0[0];
    ip_array[*ip_num][1] = arc_p0[1];
    (*ip_num)++;
  }

  if (p1_test)
  {
    ip_array[*ip_num][0] = arc_p1[0];
    ip_array[*ip_num][1] = arc_p1[1];
    (*ip_num)++;
  }

#if 0
if (*ip_num)
{
/*  printf ("** ipnum: %d\n", *ip_num); */
  printf ("** line_d: %.12f, line_dy: %.12f, line_sgn: %.12f, line_dx: %.12f, line_disc: %.12f, line_dr: %.12f\n", line_d, line_dy, line_sgn, line_dx, line_disc, line_dr);
  printf ("** line: %.12f,%.12f %.12f,%.12f  normal: %.12f,%.12f  arc: %s, radius: %.12f, center: %.12f,%.12f\n", line->p0[0], line->p0[1], line->p1[0], line->p1[1], line_normal[0], line_normal[1], arc_block->comment, arc_radius, arc_center[0], arc_center[1]);
}
#endif

  return (*ip_num ? 0 : 1);
}


static int
line_line_intersect (gcode_block_t *line1_block, gcode_block_t *line2_block, gcode_vec2d_t ip_array[2], int *ip_num)
{
  gcode_line_t *line1;
  gcode_line_t *line2;
  gcode_vec2d_t line1_p0, line1_p1, line1_normal, line2_p0, line2_p1, line2_normal;
  gfloat_t det[4];


  *ip_num = 0;

  line1 = (gcode_line_t *) line1_block->pdata;
  line2 = (gcode_line_t *) line2_block->pdata;

  gcode_line_with_offset (line1_block, line1_p0, line1_p1, line1_normal);
  gcode_line_with_offset (line2_block, line2_p0, line2_p1, line2_normal);

  /*
  * Line-Line Intersection from Wolfram MathWorld.
  * Solved using determinants with bound checking.
  */

  det[3] = ((line1_p0[0] - line1_p1[0])*(line2_p0[1] - line2_p1[1])) - ((line1_p0[1] - line1_p1[1])*(line2_p0[0] - line2_p1[0]));

/* printf ("line_line:\n"); */
  if (fabs (det[3]) < GCODE_PRECISION)
  {
    /*
    * Lines may be parallel and may not intersect,
    * if they are parallel then return the p0 of line2.
    */
#if 0
    if (line
    p[0] = line2_p0[0];
    p[1] = line2_p0[1];
#endif
    return (1);
  }

  det[0] = line1_p0[0]*line1_p1[1] - line1_p0[1]*line1_p1[0];
  det[1] = line2_p0[0]*line2_p1[1] - line2_p0[1]*line2_p1[0];

  det[2] = (det[0] * (line2_p0[0] - line2_p1[0])) - ((line1_p0[0] - line1_p1[0]) * det[1]);
  ip_array[*ip_num][0] = det[2] / det[3];

  det[2] = (det[0] * (line2_p0[1] - line2_p1[1])) - ((line1_p0[1] - line1_p1[1]) * det[1]);
  ip_array[*ip_num][1] = det[2] / det[3];


/*  printf ("line_line x: %.12f, y: %.12f .. %.12f,%.12f  %.12f,%.12f  %.12f,%.12f  %.12f,%.12f\n", ip_array[*ip_num][0], ip_array[*ip_num][1], line1_p0[0], line1_p0[1], line1_p1[0], line1_p1[1], line2_p0[0], line2_p0[1], line2_p1[0], line2_p1[1]); */
  /* intersection point must lie within each line segment */
  if (((ip_array[*ip_num][0] >= line1_p0[0]-GCODE_PRECISION && ip_array[*ip_num][0] <= line1_p1[0]+GCODE_PRECISION) || (ip_array[*ip_num][0] >= line1_p1[0]-GCODE_PRECISION && ip_array[*ip_num][0] <= line1_p0[0]+GCODE_PRECISION)) &&
      ((ip_array[*ip_num][1] >= line1_p0[1]-GCODE_PRECISION && ip_array[*ip_num][1] <= line1_p1[1]+GCODE_PRECISION) || (ip_array[*ip_num][1] >= line1_p1[1]-GCODE_PRECISION && ip_array[*ip_num][1] <= line1_p0[1]+GCODE_PRECISION)) &&
      ((ip_array[*ip_num][0] >= line2_p0[0]-GCODE_PRECISION && ip_array[*ip_num][0] <= line2_p1[0]+GCODE_PRECISION) || (ip_array[*ip_num][0] >= line2_p1[0]-GCODE_PRECISION && ip_array[*ip_num][0] <= line2_p0[0]+GCODE_PRECISION)) &&
      ((ip_array[*ip_num][1] >= line2_p0[1]-GCODE_PRECISION && ip_array[*ip_num][1] <= line2_p1[1]+GCODE_PRECISION) || (ip_array[*ip_num][1] >= line2_p1[1]-GCODE_PRECISION && ip_array[*ip_num][1] <= line2_p0[1]+GCODE_PRECISION)))
  {
    *ip_num = 1;
    return (0);
  }

  return (1);
}


static int
arc_arc_intersect (gcode_block_t *arc1_block, gcode_block_t *arc2_block, gcode_vec2d_t ip_array[2], int *ip_num)
{
  gcode_arc_t *arc1;
  gcode_arc_t *arc2;
  gcode_vec2d_t arc1_origin, arc1_center, arc1_p0;
  gcode_vec2d_t arc2_origin, arc2_center, arc2_p0;
  gfloat_t arc1_radius, arc1_start_angle, arc2_radius, arc2_start_angle;
  gfloat_t dx, dy, d, a, h, x2, y2, rx, ry, angle1, angle2;
  int miss;


  *ip_num = 0;

  arc1 = (gcode_arc_t *) arc1_block->pdata;
  arc2 = (gcode_arc_t *) arc2_block->pdata;

  gcode_arc_with_offset (arc1_block, arc1_origin, arc1_center, arc1_p0, &arc1_radius, &arc1_start_angle);
  gcode_arc_with_offset (arc2_block, arc2_origin, arc2_center, arc2_p0, &arc2_radius, &arc2_start_angle);

  /*
  * Circle-Circle intersection code derrived from 3/26/2005 Tim Voght.
  * http://local.wasp.uwa.edu.au/~pbourke/geometry/2circle/tvoght.c
  */

  /*
  * dx and dy are the vertical and horizontal distances between the circle centers.
  */
  dx = arc2_center[0] - arc1_center[0];
  dy = arc2_center[1] - arc1_center[1];

  /* Determine the distance between the centers. */
  d = sqrt ((dy*dy) + (dx*dx));

  /* Check for solvability. */
  if (fabs (d - (arc1_radius+arc2_radius)) < GCODE_PRECISION)
    d = arc1_radius+arc2_radius;

  if (d < GCODE_PRECISION)
  {
    /* no solution.  circles overlap completely */
    return (1);
  }

  if (d > arc1_radius + arc2_radius)
  {
    /* no solution. circles do not intersect. */
    return 1;
  }

  if (d < fabs (arc1_radius - arc2_radius) - GCODE_PRECISION)
  {
    /* no solution. one circle is contained in the other */
    return 1;
  }

  /*
  * 'point 2' is the point where the line through the circle
  * intersection points crosses the line between the circle
  * centers.  
  */

  /* Determine the distance from point 0 to point 2. */
  a = ((arc1_radius*arc1_radius) - (arc2_radius*arc2_radius) + (d*d)) / (2.0 * d);

  /* Determine the coordinates of point 2. */
  x2 = arc1_center[0] + (dx * a/d);
  y2 = arc1_center[1] + (dy * a/d);

  /*
  * Determine the distance from point 2 to either of the intersection points.
  */
  h = arc1_radius*arc1_radius - a*a;
  if (h < 0.0 && h > -GCODE_PRECISION)
    h = 0.0;
  h = sqrt (h);

  /*
  * Now determine the offsets of the intersection points from point 2.
  */
  rx = -dy * (h/d);
  ry = dx * (h/d);

  /* Determine the absolute intersection points. */
/*  printf ("x0_int: %.12f, y0_int: %.12f, x1_int: %.12f, y1_int: %.12f\n", x2+rx, y2+ry, x2-rx, y2-ry); */

  /*
  * If the intersection point lies within the both arcs then an intersection of the arcs has taken place.
  * There should never be 2 intersections of arcs because this would mean the sketch has bad continuity.
  */
  miss = 1;


  gcode_math_xy_to_angle (arc1_center, arc1_radius, (x2+rx), (y2+ry), &angle1);
  gcode_math_xy_to_angle (arc2_center, arc2_radius, (x2+rx), (y2+ry), &angle2);

#if 0
printf ("arc1_block: %s, arc2_block: %s\n", arc1_block->comment, arc2_block->comment);
printf ("  arc1_angle2: %.12f, arc2_angle2: %.12f, valid: %d\n", angle1, angle2, 
!gcode_math_angle_within_arc (arc1_start_angle, arc1->sweep, angle1) & !gcode_math_angle_within_arc (arc2_start_angle, arc2->sweep, angle2));
printf ("  %.12f, %.12f, %.12f ... %.12f %.12f %.12f\n", arc1_start_angle, arc1->sweep, angle1, arc2_start_angle, arc2->sweep, angle2);
#endif

  if (!gcode_math_angle_within_arc (arc1_start_angle, arc1->sweep, angle1) & !gcode_math_angle_within_arc (arc2_start_angle, arc2->sweep, angle2))
  {
    ip_array[*ip_num][0] = x2+rx;
    ip_array[*ip_num][1] = y2+ry;
    (*ip_num)++;
    miss = 0;
  }

  gcode_math_xy_to_angle (arc1_center, arc1_radius, (x2-rx), (y2-ry), &angle1);
  gcode_math_xy_to_angle (arc2_center, arc2_radius, (x2-rx), (y2-ry), &angle2);

#if 0
printf ("arc1_block: %s, arc2_block: %s\n", arc1_block->comment, arc2_block->comment);
printf ("  arc1_angle2: %.12f, arc2_angle2: %.12f, valid: %d\n", angle1, angle2, 
!gcode_math_angle_within_arc (arc1_start_angle, arc1->sweep, angle1) & !gcode_math_angle_within_arc (arc2_start_angle, arc2->sweep, angle2));
printf ("  %.12f, %.12f, %.12f ... %.12f %.12f %.12f\n", arc1_start_angle, arc1->sweep, angle1, arc2_start_angle, arc2->sweep, angle2);
#endif

  if (!gcode_math_angle_within_arc (arc1_start_angle, arc1->sweep, angle1) & !gcode_math_angle_within_arc (arc2_start_angle, arc2->sweep, angle2))
  {
    ip_array[*ip_num][0] = x2-rx;
    ip_array[*ip_num][1] = y2-ry;
    (*ip_num)++;
    miss = 0;
  }

  return (miss);
}


int
gcode_util_intersect (gcode_block_t *block_a, gcode_block_t *block_b, gcode_vec2d_t ip_array[2], int *ip_num)
{
/*  printf ("INTERSECT: %s and %s\n", block_a->comment, block_b->comment); */
  if (block_a->type == GCODE_TYPE_LINE && block_b->type == GCODE_TYPE_LINE)
    return line_line_intersect (block_a, block_b, ip_array, ip_num);

  if (block_a->type == GCODE_TYPE_ARC && block_b->type == GCODE_TYPE_ARC)
    return arc_arc_intersect (block_a, block_b, ip_array, ip_num);

  if (block_a->type == GCODE_TYPE_LINE && block_b->type == GCODE_TYPE_ARC)
    return line_arc_intersect (block_a, block_b, ip_array, ip_num);

  if (block_a->type == GCODE_TYPE_ARC && block_b->type == GCODE_TYPE_LINE)
     return line_arc_intersect (block_b, block_a, ip_array, ip_num);

  return -1;
}


void
gcode_util_push_offset (gcode_block_t *list)
{
  gcode_block_t *working_index_block, *next_block, *next_next_block, *prev_block, *prev_prev_block, *last_block;
  gcode_block_t *duplicate_list, *index_block;
  gcode_vec2d_t ip_array[2];
  int miss, ip_num, ip_ind;
  gcode_offset_t *zero_offset;

  /*
  * Synopsis: 2 lists, one that is left alone (the copy), and one that gets modified (the input).
  * The copy is done after the input list gets the offset 'pushed' and zero_offset applied.
  * Walk through each block and intersect / truncate etc.
  */

  zero_offset = (gcode_offset_t *) malloc (sizeof (gcode_offset_t));
  zero_offset->side = list->offset->side;
  zero_offset->tool = 0.0;
  zero_offset->eval = 0.0;
  zero_offset->origin[0] = 0.0;
  zero_offset->origin[1] = 0.0;
  zero_offset->rotation = 0.0;

  /*
  * Push Offset
  */
  index_block = list;
  while (index_block)
  {
    switch (index_block->type)
    {
      case GCODE_TYPE_LINE:
      {
        gcode_line_t *line;
        gcode_vec2d_t line_p0, line_p1, line_normal;

        line = (gcode_line_t *) index_block->pdata;
        gcode_line_with_offset (index_block, line_p0, line_p1, line_normal);

        line->p0[0] = line_p0[0];
        line->p0[1] = line_p0[1];

        line->p1[0] = line_p1[0];
        line->p1[1] = line_p1[1];
      }
      break;

      case GCODE_TYPE_ARC:
      {
        gcode_arc_t *arc;
        gcode_vec2d_t arc_origin, arc_center, arc_p0;
        gfloat_t arc_radius, arc_start_angle;

        arc = (gcode_arc_t *) index_block->pdata;
        gcode_arc_with_offset (index_block, arc_origin, arc_center, arc_p0, &arc_radius, &arc_start_angle);

        arc->radius = arc_radius;
        arc->pos[0] = arc_origin[0];
        arc->pos[1] = arc_origin[1];
        arc->start_angle = arc_start_angle;
      }
      break;
    }

    index_block->offset = zero_offset;
    index_block = index_block->next;
  }

  /*
  * If dealing with a single block then just return
  */
  if (!list->next)
    return;

  /*
  * Duplicate the pushed list into 'duplicate_list'.
  */
  index_block = list;
  while (index_block)
    index_block = index_block->next;
  gcode_util_duplicate_list (list, index_block, &duplicate_list);

  /*
  * Intersect and Truncate
  */
  last_block = duplicate_list;
  while (last_block->next)
    last_block = last_block->next;

  index_block = duplicate_list;
  working_index_block = list;
  while (index_block)
  {
    /*
    * Because certain blocks are used to create continuity between two discontinuous blocks,
    * such as a 0 radius circle, it will be the case that this block intersects nothing and
    * should be skipped.  This can only happen once right now.
    *
    * Assign the immediate next and previous blocks as well as secondary next and previous blocks.
    */
    next_block = index_block->next;
    prev_block = index_block->prev;
    if (prev_block == NULL)
      prev_block = last_block;
    if (next_block == NULL)
      next_block = duplicate_list;

    next_next_block = next_block->next;
    prev_prev_block = prev_block->prev;
    if (prev_prev_block == NULL)
      prev_prev_block = last_block;
    if (next_next_block == NULL)
      next_next_block = duplicate_list;


    switch (index_block->type)
    {
      case GCODE_TYPE_LINE:
      {
        gcode_line_t *working_line, *line;

        working_line = (gcode_line_t *) working_index_block->pdata;
        line = (gcode_line_t *) index_block->pdata;

        miss = gcode_util_intersect (index_block, prev_block, ip_array, &ip_num);
        if (miss)
          miss = gcode_util_intersect (index_block, prev_prev_block, ip_array, &ip_num);
/* printf ("linep0_miss: %d\n", miss); */
        /* assign p as first point */
        if (!miss)
        {
          ip_ind = 0;
          if (ip_num == 2)
          {
            if ((line->p0[0]-ip_array[1][0])*(line->p0[0]-ip_array[1][0]) + (line->p0[1]-ip_array[1][1])*(line->p0[1]-ip_array[1][1]) <
                (line->p0[0]-ip_array[0][0])*(line->p0[0]-ip_array[0][0]) + (line->p0[1]-ip_array[0][1])*(line->p0[1]-ip_array[0][1]))
            ip_ind = 1;
          }
          working_line->p0[0] = ip_array[ip_ind][0];
          working_line->p0[1] = ip_array[ip_ind][1];
/*          printf ("assign p0: %.12f,%.12f, %s\n", p[0], p[1], working_index_block->comment); */
        }

        miss = gcode_util_intersect (index_block, next_block, ip_array, &ip_num);
        if (miss)
          miss = gcode_util_intersect (index_block, next_next_block, ip_array, &ip_num);
/* printf ("linep1_miss: %d\n", miss); */
        /* assign p as last point */
        if (!miss)
        {
          ip_ind = 0;
          if (ip_num == 2)
          {
            if ((line->p1[0]-ip_array[1][0])*(line->p1[0]-ip_array[1][0]) + (line->p1[1]-ip_array[1][1])*(line->p1[1]-ip_array[1][1]) <
                (line->p1[0]-ip_array[0][0])*(line->p1[0]-ip_array[0][0]) + (line->p1[1]-ip_array[0][1])*(line->p1[1]-ip_array[0][1]))
            ip_ind = 1;
          }
          working_line->p1[0] = ip_array[ip_ind][0];
          working_line->p1[1] = ip_array[ip_ind][1];
/*          printf ("assign p1: %.12f,%.12f, %s\n", p[0], p[1], working_index_block->comment); */
        }
      }
      break;

      case GCODE_TYPE_ARC:
      {
        gcode_arc_t *working_arc, *arc;
        gcode_vec2d_t arc_origin, arc_center, arc_p0, end_pos;
        gfloat_t arc_radius, arc_start_angle, angle;

        gcode_arc_with_offset (index_block, arc_origin, arc_center, arc_p0, &arc_radius, &arc_start_angle);

        working_arc = (gcode_arc_t *) working_index_block->pdata;
        arc = (gcode_arc_t *) index_block->pdata;

        if (working_arc->radius <= 0.0)
          break;

        miss = gcode_util_intersect (index_block, prev_block, ip_array, &ip_num);
        if (miss)
          miss = gcode_util_intersect (index_block, prev_prev_block, ip_array, &ip_num);
        /* adjust arc start position */
        if (!miss)
        {
/* printf ("UPDATING ARC POSITION: %s to %.12f,%.12f\n", working_index_block->comment, p[0], p[1]); */
          ip_ind = 0;
          if (ip_num == 2)
          {
            if ((arc->pos[0]-ip_array[1][0])*(arc->pos[0]-ip_array[1][0]) + (arc->pos[1]-ip_array[1][1])*(arc->pos[1]-ip_array[1][1]) <
                (arc->pos[0]-ip_array[0][0])*(arc->pos[0]-ip_array[0][0]) + (arc->pos[1]-ip_array[0][1])*(arc->pos[1]-ip_array[0][1]))
            ip_ind = 1;
          }
          working_arc->pos[0] = ip_array[ip_ind][0];
          working_arc->pos[1] = ip_array[ip_ind][1];

          /* Update start angle too */
          gcode_math_xy_to_angle (arc_center, arc_radius, ip_array[ip_ind][0], ip_array[ip_ind][1], &working_arc->start_angle);
        }

        end_pos[0] = arc_center[0] + arc_radius * cos (GCODE_DEG2RAD * (arc->start_angle + arc->sweep));
        end_pos[1] = arc_center[1] + arc_radius * sin (GCODE_DEG2RAD * (arc->start_angle + arc->sweep));

        miss = gcode_util_intersect (index_block, next_block, ip_array, &ip_num);
        if (miss)
          miss = gcode_util_intersect (index_block, next_next_block, ip_array, &ip_num);

        /* adjust sweep angle */
        if (!miss)
        {
          ip_ind = 0;
          if (ip_num == 2)
          {
            if ((end_pos[0]-ip_array[1][0])*(end_pos[0]-ip_array[1][0]) + (end_pos[1]-ip_array[1][1])*(end_pos[1]-ip_array[1][1]) <
                (end_pos[0]-ip_array[0][0])*(end_pos[0]-ip_array[0][0]) + (end_pos[1]-ip_array[0][1])*(end_pos[1]-ip_array[0][1]))
            ip_ind = 1;
          }
          gcode_math_xy_to_angle (arc_center, arc_radius, ip_array[ip_ind][0], ip_array[ip_ind][1], &angle);

          /* Difference must take place in the direction of the sweep. */
          if (working_arc->sweep < 0.0)
          {
            working_arc->sweep = fmodf ((angle - 360.0) - working_arc->start_angle, 360.0);
          }
          else
          {
            /*
            * Due to precision isues there may arbitrarily be a resulting 359.9+ and 0.0+ result
            * stored in "angle" from gcode_math_xy_to_angle (), which determines a positive or
            * negative sweep angle.  Since the sweep angle is defined to be positive correct
            * if necessary by adding 360.0 to angle.
            */
            if (angle < working_arc->start_angle)
              angle += 360.0;
            working_arc->sweep = angle - working_arc->start_angle;
          }

/*          printf ("++ adjust sweep angle: %s .. %.12f .. %.12f,%.12f\n", index_block->comment, angle, p[0], p[1]); */
/*          printf ("++ start_angle: %.12f, sweep_angle: %.12f\n", arc->start_angle, arc->sweep); */
        }
        else
        {
          /* Adjust the sweep by the difference in Start angles */
          working_arc->sweep = fmod (working_arc->sweep + (arc->start_angle - working_arc->start_angle), 360.0);
        }
      }
      break;
    }

/*    printf ("index_block: %p\n", index_block); */
    index_block = index_block->next;
    working_index_block = working_index_block->next;
  }
/*  printf ("\n"); */

  gcode_list_free (&duplicate_list);
}


void
gcode_util_fillet (gcode_block_t *line1_block, gcode_block_t *line2_block, gcode_block_t *fillet_arc_block, gfloat_t radius)
{
  gcode_line_t *line1, *line2;
  gcode_arc_t *fillet_arc;
  gcode_vec2d_t vec1_u, vec2_u, vec;
  gfloat_t magnitude1, magnitude2, dot, offset, test1_angle, test2_angle;


  line1 = (gcode_line_t *) line1_block->pdata;
  line2 = (gcode_line_t *) line2_block->pdata;
  fillet_arc = (gcode_arc_t *) fillet_arc_block->pdata;

  /*
  * Assuming the end points meet at the same point then compute the slope
  * of each line and scale the end point of the current line back and push
  * the start point of the next line forward.
  * Start angle and Sweep of the fillet arc will be computed by line slopes.
  */

  /* Current Line */
  GCODE_MATH_VEC2D_SUB (vec1_u, line1->p0, line1->p1); /* Flipped for calculating the dot product */
  GCODE_MATH_VEC2D_MAG (magnitude1, vec1_u);
  GCODE_MATH_VEC2D_UNITIZE (vec1_u);

  /* Next Line */
  GCODE_MATH_VEC2D_SUB (vec2_u, line2->p1, line2->p0);
  GCODE_MATH_VEC2D_MAG (magnitude2, vec2_u);
  GCODE_MATH_VEC2D_UNITIZE (vec2_u);

  /*
  * To understand how this works take 2 intersecting lines that are tangent to a circle
  * of the filleting radius.  The arc midpt is along the half angle formed by the 2 lines.
  * A right triangle is formed from the arc center, tangent intersection pt, and intersection
  * of both lines.  You know one angle is 90, the other is the half angle, and the third angle
  * is known since the (3) must add to 180 degrees.  The tangent is opposite over adjacent, so using
  * the dot product to get the angle between the lines and arc tangent you can solve for the offset.
  */
  GCODE_MATH_VEC2D_DOT (dot, vec1_u, vec2_u);
  offset = radius * tan (GCODE_HPI - 0.5 * acos (dot));

  /* Used by Arc */
  GCODE_MATH_VEC2D_SUB (vec1_u, line1->p1, line1->p0);
  GCODE_MATH_VEC2D_UNITIZE (vec1_u);

  /* Shorten Current Line */
  GCODE_MATH_VEC2D_SUB (vec, line1->p1, line1->p0);
  GCODE_MATH_VEC2D_SCALE (vec, (1.0 - (offset / magnitude1)));
  GCODE_MATH_VEC2D_ADD (line1->p1, vec, line1->p0);

  /* Shorten Next Line */
  GCODE_MATH_VEC2D_SUB (vec, line2->p1, line2->p0);
  GCODE_MATH_VEC2D_SCALE (vec, (1.0 - (offset / magnitude2)));
  GCODE_MATH_VEC2D_SUB (line2->p0, line2->p1, vec);

  /* Fillet Arc */
  fillet_arc->pos[0] = line1->p1[0];
  fillet_arc->pos[1] = line1->p1[1];
  fillet_arc->radius = radius;
  /* Sweep angle is supplemental to dot product.  dot and -dot are supplementary */
  fillet_arc->sweep = GCODE_RAD2DEG * acos (-dot);

  GCODE_MATH_VEC3D_ANGLE (fillet_arc->start_angle, vec1_u[0], vec1_u[1]);
  fillet_arc->start_angle *= GCODE_RAD2DEG;

  /* Flip Test - If Line2 > 180 degrees in clockwise direction */
  GCODE_MATH_VEC3D_ANGLE (test1_angle, vec1_u[0], vec1_u[1]);
  GCODE_MATH_VEC3D_ANGLE (test2_angle, vec2_u[0], vec2_u[1]);


  if (fabs (test1_angle - test2_angle) > GCODE_PI)
  {
    if (test1_angle > test2_angle)
    {
      test2_angle += GCODE_2PI;
    }
    else
    {
      test1_angle += GCODE_2PI;
    }
  }

  if (test1_angle - test2_angle < 0.0)
  {
    fillet_arc->start_angle -= 90.0;
  }
  else
  {
    fillet_arc->start_angle += 90.0;
    fillet_arc->sweep *= -1.0;
  }
}


static void
flip_direction (gcode_block_t *block)
{
  if (block->type == GCODE_TYPE_LINE)
  {
    gcode_line_flip_direction (block);
  }
  else if (block->type == GCODE_TYPE_ARC)
  {
    gcode_arc_flip_direction (block);
  }
}


/*
* Correct the orientation and sequence of all blocks in the list.
*/
void
gcode_util_order_list (gcode_block_t *list)
{
  gcode_block_t *index1_block, *index2_block, *free_list;
  gcode_vec2d_t e0[2], e1[2], e2[2];
  gfloat_t dist[8];
  int match;


  if (list == NULL)
    return;

  free_list = list->next;
  free_list->prev = NULL;

  list->next = NULL; /* anchored list */

  /* While there exists blocks in the free list. */
  while (free_list)
  {
    /* compare each anchored block to all of the free blocks to see if there is an adjacent block. */
    index1_block = list;

    while (index1_block)
    {
      index1_block->ends (index1_block, e0[0], e0[1], GCODE_GET);
      index2_block = free_list;
      match = 0;

      while (index2_block && !match)
      {
        index2_block->ends (index2_block, e1[0], e1[1], GCODE_GET);

        /* index1_block NEXT check */
        dist[0] = sqrt ((e0[0][0] - e1[0][0])*(e0[0][0] - e1[0][0]) + (e0[0][1] - e1[0][1])*(e0[0][1] - e1[0][1]));
        dist[1] = sqrt ((e0[0][0] - e1[1][0])*(e0[0][0] - e1[1][0]) + (e0[0][1] - e1[1][1])*(e0[0][1] - e1[1][1]));
        dist[2] = sqrt ((e0[1][0] - e1[0][0])*(e0[1][0] - e1[0][0]) + (e0[1][1] - e1[0][1])*(e0[1][1] - e1[0][1]));
        dist[3] = sqrt ((e0[1][0] - e1[1][0])*(e0[1][0] - e1[1][0]) + (e0[1][1] - e1[1][1])*(e0[1][1] - e1[1][1]));

        /* Is there adjacency between the two blocks */
        if (dist[0] < GCODE_PRECISION || dist[1] < GCODE_PRECISION || dist[2] < GCODE_PRECISION || dist[3] < GCODE_PRECISION)
        {
          /* index1_block is adjacent to index2_block */
          match = 1;

          /*
          * if index1_block and index1_block->next are adjacent then link index2_block to index1_block->prev,
          * otherwise link index2_block to index1_block->next.
          */
          if (index1_block->next)
          {
            index1_block->next->ends (index1_block->next, e2[0], e2[1], GCODE_GET);

            dist[4] = sqrt ((e0[0][0] - e2[0][0])*(e0[0][0] - e2[0][0]) + (e0[0][1] - e2[0][1])*(e0[0][1] - e2[0][1]));
            dist[5] = sqrt ((e0[0][0] - e2[1][0])*(e0[0][0] - e2[1][0]) + (e0[0][1] - e2[1][1])*(e0[0][1] - e2[1][1]));
            dist[6] = sqrt ((e0[1][0] - e2[0][0])*(e0[1][0] - e2[0][0]) + (e0[1][1] - e2[0][1])*(e0[1][1] - e2[0][1]));
            dist[7] = sqrt ((e0[1][0] - e2[1][0])*(e0[1][0] - e2[1][0]) + (e0[1][1] - e2[1][1])*(e0[1][1] - e2[1][1]));
          }

          if (index1_block->next && (dist[4] < GCODE_PRECISION || dist[5] < GCODE_PRECISION || dist[6] < GCODE_PRECISION || dist[7] < GCODE_PRECISION))
          {
            /*
            * INSERT AS PREV BLOCK
            * Take care of linking prev and next blocks up from free list as well as inserting
            * a new block into the anchored list and linking up prev and next pointers.
            */
            if (index2_block == free_list)
            {
              free_list = index2_block->next;
              if (free_list)
                free_list->prev = NULL;
            }

            if (index2_block->prev)
              index2_block->prev->next = index2_block->next;
            if (index2_block->next)
              index2_block->next->prev = index2_block->prev;

            index2_block->prev = index1_block->prev;
            index2_block->next = index1_block;
            if (index1_block->prev)
              index1_block->prev->next = index2_block;
            index1_block->prev = index2_block;

            if (dist[2] > GCODE_PRECISION)
              flip_direction (index2_block);
          }
          else
          {
            /*
            * INSERT AS NEXT BLOCK
            * Take care of linking prev and next blocks up from free list as well as inserting
            * a new block into the anchored list and linking up prev and next pointers.
            */
            if (index2_block == free_list)
            {
              free_list = index2_block->next;
              if (free_list)
                free_list->prev = NULL;
            }

            if (index2_block->prev)
              index2_block->prev->next = index2_block->next;
            if (index2_block->next)
              index2_block->next->prev = index2_block->prev;

            index2_block->prev = index1_block;
            index2_block->next = index1_block->next;
            if (index1_block->next)
              index1_block->next->prev = index2_block;
            index1_block->next = index2_block;

            if (dist[2] > GCODE_PRECISION)
              flip_direction (index2_block);
          }
        }

        /*
        * It doesn't matter that index2_block may now have different prev/next pointers because the loop will
        * terminate as a result of match being equal to 1.
        */
        index2_block = index2_block->next;
      }


      if (!match && !index1_block->next && free_list)
      {
        gcode_block_t *temp_block;

        temp_block = free_list->next;

        /* Insert free_list head onto the end of the index1_block list. */
        free_list->prev = index1_block;
        free_list->next = index1_block->next; /* should always be NULL */
        index1_block->next = free_list;

        free_list = temp_block;
        if (free_list)
          free_list->prev = NULL;
      }

      index1_block = index1_block->next;
    }
  }
}
