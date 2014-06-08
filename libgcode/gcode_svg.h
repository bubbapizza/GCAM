#ifndef __GCODE_SVG_H_
#define __GCODE_SVG_H_

/*
 * SVG file import parser
 * Reads an SVG format XML file, extracts path data, and creates
 * line segments/arcs as appropriate (currently only line
 * segments are supported and curves are split into lines.)
 *
 * ONLY paths are supported, so objects must be converted to paths
 * to be processed
 *
 * Future possibilities:
 * -Store layer data
 * -Arc support
 */

#include "gcode.h"
#include <glib.h>

int
gcode_svg_import (gcode_t *gcode, gcode_block_t *sketch_block, char *filename);

#endif
