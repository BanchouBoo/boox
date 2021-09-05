#ifndef SELECTION_H
#define SELECTION_H

#include <xcb/xcb.h>

typedef struct {
    int x;
    int y;
    int w;
    int h;
} rect_t;

typedef struct {
    int x;
    int y;
} point_t;

typedef enum {
    MODE_SELECT,
    MODE_POINT
} selection_mode_t;

extern selection_mode_t selection_mode;
extern xcb_window_t selection_window;
extern xcb_window_t constraining_window;
extern rect_t selection;
extern xcb_window_t selected_window;
extern int border_size;
extern int border_color;

void selection_window_initialize(void);
void set_rects_from_selection(xcb_rectangle_t *rects, rect_t dimensions);
void update_selection_window(rect_t dimensions);
rect_t fix_rect(rect_t dimensions);
xcb_window_t get_child_window(xcb_window_t wid);

#endif
