#include "gcode_svg.h"
#include <stdio.h>
#include <unistd.h>
#include <expat.h>

// Temporary: config values
// Scale factor.  Coordinate values are multiplied by this.
#define SCALEFACTOR 0.01 /* 100 DPI */
// Bezier increment.  Increment to use for interpolation (1/this lines will be created)
#define BEZIERINCR 0.02
#define DEBUG 0

typedef struct line_list_s
{
  struct line_list_s * prev;
  struct line_list_s * next;
  gfloat start[2];
  gfloat end[2];
} line_list_t;


typedef struct gcode_svg_s
{
  gcode_t *gcode;
  gcode_block_t *sketch_block;
  gfloat_t width;
  gfloat_t height;
} gcode_svg_t;


static void
gcode_svg_create_line (gcode_block_t *sketch_block, gcode_svg_t *data, gfloat p0[], gfloat p1[]);


void
gcode_svg_add_reverse_lines (gcode_block_t *sketch_block, gcode_svg_t *svg, line_list_t *lines)
{
  if (lines == NULL)
    return;
 
  line_list_t * curr;
  curr = lines;
  //Go to end of list
  while(curr->next != NULL)
    {
      curr = curr->next;
    }
  //Walk backwards through list
  while(curr != NULL)
    {
      gcode_svg_create_line (sketch_block, svg, curr->start, curr->end);
      curr = curr->prev;     
    }
}

void
gcode_svg_line_list_append (line_list_t **lines, gfloat start[], gfloat end[])
{
#if DEBUG
  printf("Appending line: %f,%f -> %f,%f\n", start[0], start[1], end[0], end[1]);
#endif
  if (*lines == NULL)
  {
    (*lines) = malloc(sizeof(line_list_t));
    (*lines)->start[0] = start[0];
    (*lines)->start[1] = start[1];
    (*lines)->end[0] = end[0];
    (*lines)->end[1] = end[1];
    (*lines)->prev = NULL;
    (*lines)->next = NULL;
  }
  else
  {
    line_list_t *node = *lines;

    /* Go to end of list */
    while (node->next != NULL)
    {
      node = node->next;
    }
    /* Add new node */
    node->next = malloc (sizeof (line_list_t));
    if (node->next == NULL)
    {
#if DEBUG
      printf("Error!  Failed to allocate memory!\n");
#endif
      return;
    }

    node->next->prev = node;
    node = node->next;
    node->next = NULL;
    node->start[0] = start[0];
    node->start[1] = start[1];
    node->end[0] = end[0];
    node->end[1] = end[1];
  }
}

static void
gcode_svg_line_list_free (line_list_t * lines)
{
  if (lines == NULL)
    return;

  while (lines->next != NULL)
  {
    lines = lines->next;
    free (lines->prev);
  }

  free (lines);
}


/* Create a line and add it to the sketch */
static void
gcode_svg_create_line (gcode_block_t *sketch_block, gcode_svg_t *svg, gfloat p0[], gfloat p1[])
{
  gfloat sf = SCALEFACTOR;
  gcode_block_t *line_block;
  gcode_line_t *line;
  gcode_sketch_t *sketch;

  sketch = (gcode_sketch_t *) sketch_block->pdata;
 
#if DEBUG
  printf ("Line: (%f,%f)->(%f,%f)\n",p0[0]*sf,p0[1]*sf,p1[0]*sf,p1[1]*sf);
#endif
 
  gcode_line_init (sketch_block->gcode, &line_block, sketch_block);
  line_block->offset = &sketch->offset;
  line_block->prev = NULL;
  line_block->next = NULL;
 
  gcode_list_insert (&sketch->list, line_block);
 
  line = (gcode_line_t *) line_block->pdata;
  line->p0[0] = p0[0]*sf;
  line->p0[1] = (svg->height-p0[1])*sf;
  line->p1[0] = p1[0]*sf;
  line->p1[1] = (svg->height-p1[1])*sf;
}


/* Interpolate a cubic bezier curve and add lines to the sketch */
static void
gcode_svg_create_bezier (line_list_t **lines, gfloat p0[], gfloat p1[], gfloat p2[], gfloat p3[])
{
  gfloat t,incr = BEZIERINCR;
  gfloat curr[2]; //Current point
  gfloat last[2]; //Previous point

  last[0] = p0[0];
  last[1] = p0[1];

  for (t = incr; t <= 1; t += incr)
  {
    curr[0] = pow ((1-t),3)*p0[0] + 3*pow ((1-t),2)*t*p1[0] + 3*(1-t)*pow (t,2)*p2[0] + pow (t,3)*p3[0];
    curr[1] = pow ((1-t),3)*p0[1] + 3*pow ((1-t),2)*t*p1[1] + 3*(1-t)*pow (t,2)*p2[1] + pow (t,3)*p3[1];
#if DEBUG
    printf ("P: %f,%f\n", curr[0], curr[1]);
#endif
    gcode_svg_line_list_append (lines, last, curr);
    last[0] = curr[0];
    last[1] = curr[1];
  }
}


/* Parse a path data string */
static int
gcode_svg_parse_d (void *data, gchar *d)
{
  int numwords, i;
  gchar *temp, *start, *end;

  gcode_block_t *sketch_block = ((gcode_svg_t *)data)->sketch_block;

  line_list_t *lines = NULL; /* Create a Line List */
  gfloat lp[2]; /* Last Point */
  gfloat sp[2]; /* Starting Point */
  int isfirst = 1;

  /*
  sp[0] = 0.5;
  sp[1] = 0.75;
  lp[0] = 1;
  lp[1] = 1;
  gcode_svg_create_line(&lines, sp, lp);
  printf("TEST: %f,%f\n", lines->start[0], lines->start[1]);
  */

#if DEBUG
  printf ("Parsing path data: %s\n", d);
#endif

  start = d;

  int numcoords = 0; //Number of coords to look for (changes w/ different commands)

  while (1)
  {
    //Parse next command
    while (*start == ' ' && *start != NULL)
      start++;
     
    if (*start == NULL) //End of data
    {
#if DEBUG
      printf ("END\n");
#endif
      /* Append lines in reverse order */
      gcode_svg_add_reverse_lines (sketch_block, (gcode_svg_t *) data, lines);
      gcode_svg_line_list_free (lines);
      return 0;
    }
     
    gchar cmd = *start;
    start++;

    switch (cmd)
    {
      case 'z': numcoords = 0; break;
      case 'Z': numcoords = 0; break;
      case 'm': numcoords = 1; break;
      case 'M': numcoords = 1; break;
      case 'l': numcoords = 1; break;
      case 'L': numcoords = 1; break;
      case 'c': numcoords = 3; break;
      case 'C': numcoords = 3; break;
    }

    if (cmd == 'z' || cmd == 'Z')
    {
      /* Close path */
      if (isfirst)
      {
#if DEBUG
        fprintf(stderr, "Bad path data.  Can't close path that hasn't been started.\n");
#endif
        break;
      }
    
      //Close the path and start a new one
      isfirst = 1;

      //printf("Line from %lf, %lf to %lf, %lf (closing)\n", lp[0], lp[1], sp[0], sp[1]);
      //gcode_svg_line_list_append(&lines, lp, sp);
      gcode_svg_create_line (sketch_block, (gcode_svg_t *) data, lp, sp);

      //Don't read coordinates
      continue;
    }

    gfloat coords[3][2];

    for (i = 0; i < numcoords; i++)
    {
      //Parse coordinates
      while (*start == ' ' && *start != NULL)
        start++;
    
      if (*start == NULL) //End of data
      {
#if DEBUG
        printf ("END\n");
#endif
        //Append lines in reverse order
        gcode_svg_add_reverse_lines (sketch_block, (gcode_svg_t *) data, lines);
        gcode_svg_line_list_free (lines);        
        return 0;
      }
    
      end = start;
      while (*end != ' ' && *end != NULL)
        end++;
    
      gchar *temp = (char*) malloc ((end-start) * sizeof(gchar));
      memcpy (temp, start, end-start);
      temp[end-start] = NULL;
      gfloat p[2];
      sscanf (temp, "%f,%f", &coords[i][0], &coords[i][1]);
      free (temp);
    
#if DEBUG
      printf("CMD: %c, COORD %d: (%f, %f)\n", cmd, i, coords[i][0], coords[i][1]);
#endif
    
      start = end;
    }

    if (cmd == 'M' || (cmd == 'm' && isfirst))
    {
      /* Absolute moveto */
      //printf("Move to %f,%f\n", coords[0][0], coords[0][1]);
      lp[0] = coords[0][0];
      lp[1] = coords[0][1];
    }
    else if (cmd == 'm')
    {
      /* Relative moveto */
      //printf("Move from %f,%f to %f,%f\n", lp[0],lp[1],coords[0][0],coords[0][1]);
      lp[0] = coords[0][0];
      lp[1] = coords[0][1];
    }
    else if (cmd == 'L')
    {
      /* Absolute line */
      //printf("Line from %f,%f to %f,%f\n", lp[0], lp[1], coords[0][0], coords[0][1]);
      //gcode_svg_line_list_append(&lines, lp, coords[0]);
      gcode_svg_create_line (sketch_block, (gcode_svg_t *) data, lp, coords[0]);
      lp[0] = coords[0][0];
      lp[1] = coords[0][1];
    }
    else if (cmd == 'l')
    {
      /* Relative line */
      coords[0][0] += lp[0];
      coords[0][1] += lp[1];
      //printf("Line from %f,%f to %f,%f\n", lp[0], lp[1], coords[0][0], coords[0][1]);
      //gcode_svg_line_list_append(&lines, lp, coords[0]);
      gcode_svg_create_line (sketch_block, (gcode_svg_t *) data, lp, coords[0]);
      lp[0] = coords[0][0];
      lp[1] = coords[0][1];
    }
    else if (cmd == 'C')
    {
      /* Absolute cubic bezier */
#if DEBUG
      printf("Cubic bezier from %f,%f to %f,%f with control points (%f,%f), (%f,%f)\n", lp[0], lp[1], coords[2][0], coords[2][1],
       coords[0][0], coords[0][1], coords[1][0], coords[1][1]);
#endif
      gcode_svg_create_bezier (&lines, lp, coords[0], coords[1], coords[2]);
      lp[0] = coords[2][0];
      lp[1] = coords[2][1];
    }
    else if (cmd == 'c')
    {
      /* Relative cubic bezier */
    }
     
    if (isfirst)
    {
      /* Mark first point */
      isfirst = 0;
      sp[0] = coords[0][0];
      sp[1] = coords[0][1];
    }
  }
}


static void
svg_start (void *data, const char *el, const char **attr)
{
  int i;
  gcode_svg_t *svg = (gcode_svg_t *) data;
 
  //Look for element 'path' and attr 'd' for path data
#if DEBUG
  printf ("EL: %s\n", el);
#endif
  if (!strcmp(el, "path"))
  {
#if DEBUG
    printf ("Found path:\n");
#endif
    for (i = 0; attr[i]; i+= 2)
    {
      if (!strcmp (attr[i],"d"))
      {
        gchar *d = attr[i+1];
        gcode_svg_parse_d (data, d);
        return;
      }
    }
  }
  else if (!strcmp (el, "svg"))
  {
    i = 0;
    while (attr[i])
    {
      if (!strcmp (attr[i], "width"))
        svg->width = atof (attr[++i]);

      if (!strcmp (attr[i], "height"))
        svg->height = atof (attr[++i]);

      i++;
    }
#if DEBUG
    printf ("width: %f, height: %f\n", svg->width, svg->height);
#endif
  }
}


static void
svg_end (void *data, const char *el)
{
}


int
gcode_svg_import (gcode_t *gcode, gcode_block_t *sketch_block, char * filename)
{
  gcode_svg_t svg;
  XML_Parser p = XML_ParserCreate ("US-ASCII");
  FILE *fh;
  int len;
  char *buffer;

  svg.gcode = gcode;
  svg.sketch_block = sketch_block;
  svg.width = 0.0;
  svg.height = 0.0;

  if(!p)
  {
#if DEBUG
    fprintf (stderr, "Couldn't create parser for SVG!");
#endif
    return (1);
  }

  XML_SetElementHandler (p, svg_start, svg_end);
  XML_SetUserData (p, (void *) &svg);
 
  fh = fopen(filename, "r");
  if (!fh)
  {
#if DEBUG
    fprintf (stderr, "Couldn't load file: %s\n", filename);
#endif
    XML_ParserFree (p);
    return (1);
  }
 
  fseek (fh, 0, SEEK_END);
  len = ftell (fh);
  buffer = (char *) malloc (len);
  fseek (fh, 0, SEEK_SET);
  fread (buffer, len, 1, fh);

  //File read, create sketch

  if (XML_Parse (p, buffer, len, 1) == XML_STATUS_ERROR)
  {
#if DEBUG
    fprintf (stderr, "Parse error at line %d:\n%s\n", XML_GetCurrentLineNumber(p), XML_ErrorString(XML_GetErrorCode(p)));
#endif
    return (1);
  }
 
  fclose (fh);
  free (buffer);
  XML_ParserFree (p);

  svg.gcode->material_size[0] = svg.width * SCALEFACTOR;
  svg.gcode->material_size[1] = svg.height * SCALEFACTOR;
  gcode_prep (svg.gcode);
}
