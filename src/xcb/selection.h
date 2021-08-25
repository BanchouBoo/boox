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

extern xcb_window_t selection_window;
extern rect_t selection;

void selection_window_initialize(void);
void set_rects_from_selection(xcb_rectangle_t *rects, rect_t dimensions);
void update_selection_window(rect_t dimensions);
rect_t fix_rect(rect_t dimensions);

#endif
