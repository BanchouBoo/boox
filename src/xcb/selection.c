#include <xcb/xcb.h>
#include <xcb/shape.h>
#include <stdlib.h>
#include <stdio.h>

#include "connection.h"
#include "selection.h"
#include "../config.h"

xcb_window_t selection_window;
rect_t selection;

void selection_window_initialize(void)
{
	int values[] = { BORDER_COLOR, 1,  XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY};
	int mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;

	selection_window = xcb_generate_id(xcb_connection);
	xcb_create_window(xcb_connection, XCB_COPY_FROM_PARENT, selection_window, xcb_screen->root, 0, 0,
		xcb_screen->width_in_pixels, xcb_screen->height_in_pixels, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
		XCB_COPY_FROM_PARENT, mask, values);
	xcb_change_property(xcb_connection, XCB_PROP_MODE_REPLACE, selection_window,
		XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, 4, "boox");
	xcb_change_property(xcb_connection, XCB_PROP_MODE_REPLACE, selection_window,
		XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8, 10, "boox\0boox");


	xcb_rectangle_t rects[4];
	set_rects_from_selection(rects, selection);
	xcb_rectangle_t rect;

	xcb_shape_rectangles(xcb_connection, XCB_SHAPE_SO_SET,
		XCB_SHAPE_SK_BOUNDING, 0, selection_window, 0, 0, 4, rects);

	xcb_shape_rectangles(xcb_connection, XCB_SHAPE_SO_SET,
		XCB_SHAPE_SK_INPUT, 0, selection_window, 0, 0, 1, &rect);

	xcb_map_window(xcb_connection, selection_window);
}

void set_rects_from_selection(xcb_rectangle_t *rects, rect_t dimensions)
{
	dimensions = fix_rect(dimensions);

	rects[0].x = dimensions.x - BORDER_SIZE;
	rects[0].y = dimensions.y - BORDER_SIZE;
	rects[0].width = BORDER_SIZE;
	rects[0].height = dimensions.h + BORDER_SIZE * 2;

	rects[1].x = dimensions.x;
	rects[1].y = dimensions.y - BORDER_SIZE;
	rects[1].width = dimensions.w + BORDER_SIZE;
	rects[1].height = BORDER_SIZE;

	rects[2].x = dimensions.x + dimensions.w;
	rects[2].y = dimensions.y - BORDER_SIZE;
	rects[2].width = BORDER_SIZE;
	rects[2].height = dimensions.h + BORDER_SIZE * 2;

	rects[3].x = dimensions.x;
	rects[3].y = dimensions.y + dimensions.h;
	rects[3].width = dimensions.w + BORDER_SIZE;
	rects[3].height = BORDER_SIZE;
}

void update_selection_window(rect_t dimensions)
{
	xcb_rectangle_t rects[4];
	set_rects_from_selection(rects, dimensions);
	xcb_shape_rectangles(xcb_connection, XCB_SHAPE_SO_SET,
		XCB_SHAPE_SK_BOUNDING, 0, selection_window, 0, 0, 4, rects);
	flush();
}

rect_t fix_rect(rect_t dimensions)
{
	if (dimensions.x < 0) {
		dimensions.w += dimensions.x;
		dimensions.x = 0;
	}

	if (dimensions.y < 0) {
		dimensions.h += dimensions.y;
		dimensions.y = 0;
	}

	if (dimensions.w < 0) {
		dimensions.x = dimensions.x + dimensions.w;
		dimensions.w = abs(dimensions.w);
	}

	if (dimensions.h < 0) {
		dimensions.y = dimensions.y + dimensions.h;
		dimensions.h = abs(dimensions.h);
	}

	xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(xcb_connection, xcb_get_geometry(xcb_connection, xcb_screen->root), NULL);

	if (dimensions.x + dimensions.w > geom->width) {
		dimensions.w = geom->width - dimensions.x;
	}

	if (dimensions.y + dimensions.h > geom->height) {
		dimensions.h = geom->height - dimensions.y;
	}

	return dimensions;
}
