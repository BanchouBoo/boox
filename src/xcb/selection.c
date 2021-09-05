#include <xcb/xcb.h>
#include <xcb/shape.h>
#include <stdlib.h>

#include "connection.h"
#include "selection.h"
#include "../config.h"

mode_t selection_mode = MODE_SELECT;
xcb_window_t selection_window;
xcb_window_t constraining_window;
rect_t selection;
xcb_window_t selected_window;
int border_size;
int border_color;

void selection_window_initialize(void)
{
    int values[] = { border_color, 1,  XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY};
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
    set_rects_from_selection(rects, (rect_t) {
        .x = -border_size,
        .y = -border_size,
        .w = 0,
        .h = 0
    });
    // xcb_rectangle_t rect;

    xcb_shape_rectangles(xcb_connection, XCB_SHAPE_SO_SET,
        XCB_SHAPE_SK_BOUNDING, 0, selection_window, 0, 0, 4, rects);

    // TODO: I don't remember what this was for but everything works if I remove it?
    // xcb_shape_rectangles(xcb_connection, XCB_SHAPE_SO_SET,
    //     XCB_SHAPE_SK_INPUT, 0, selection_window, 0, 0, 1, &rect);

    xcb_map_window(xcb_connection, selection_window);
}

void set_rects_from_selection(xcb_rectangle_t *rects, rect_t dimensions)
{
    rects[0].x = dimensions.x - border_size;
    rects[0].y = dimensions.y - border_size;
    rects[0].width = border_size;
    rects[0].height = dimensions.h + border_size * 2;

    rects[1].x = dimensions.x;
    rects[1].y = dimensions.y - border_size;
    rects[1].width = dimensions.w + border_size;
    rects[1].height = border_size;

    rects[2].x = dimensions.x + dimensions.w;
    rects[2].y = dimensions.y - border_size;
    rects[2].width = border_size;
    rects[2].height = dimensions.h + border_size * 2;

    rects[3].x = dimensions.x;
    rects[3].y = dimensions.y + dimensions.h;
    rects[3].width = dimensions.w + border_size;
    rects[3].height = border_size;
}

void update_selection_window(rect_t dimensions)
{
    xcb_rectangle_t rects[4];
    set_rects_from_selection(rects, fix_rect(dimensions));
    xcb_shape_rectangles(xcb_connection, XCB_SHAPE_SO_SET,
        XCB_SHAPE_SK_BOUNDING, 0, selection_window, 0, 0, 4, rects);
    flush();
}

rect_t fix_rect(rect_t dimensions)
{
    xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(xcb_connection,
        xcb_get_geometry(xcb_connection, constraining_window), NULL);

    #define min(a,b) (((a)<(b))?(a):(b))
    #define max(a,b) (((a)>(b))?(a):(b))
    #define clamp(a, low, high) min(max(a, low), high)

    int window_left_edge = geom->x;
    int window_right_edge = window_left_edge + geom->width;
    int window_top_edge = geom->y;
    int window_bottom_edge = window_top_edge + geom->height;

    int dimensions_left_edge = min(dimensions.x, dimensions.x + dimensions.w);
    int dimensions_right_edge = max(dimensions.x, dimensions.x + dimensions.w);
    int dimensions_top_edge = min(dimensions.y, dimensions.y + dimensions.h);
    int dimensions_bottom_edge = max(dimensions.y, dimensions.y + dimensions.h);

    int left_edge = clamp(dimensions_left_edge, window_left_edge, window_right_edge);
    int right_edge = clamp(dimensions_right_edge, window_left_edge, window_right_edge);
    int top_edge = clamp(dimensions_top_edge, window_top_edge, window_bottom_edge);
    int bottom_edge = clamp(dimensions_bottom_edge, window_top_edge, window_bottom_edge);

    dimensions.x = left_edge;
    dimensions.w = right_edge - left_edge;
    dimensions.y = top_edge;
    dimensions.h = bottom_edge - top_edge;

    return dimensions;
}
