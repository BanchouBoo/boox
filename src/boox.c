#include <xcb/xcb.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "xcb/connection.h"
#include "xcb/selection.h"
#include "parsing/parsing.h"

int running = 1;

static void handle_events(void) {
	xcb_generic_event_t *ev;
	xcb_query_pointer_reply_t *pointer;
	xcb_get_geometry_reply_t *geom;
	xcb_window_t hovered_win = 0;

	static int selecting = 0;

	ev = xcb_wait_for_event(xcb_connection);
	if (!ev) {
		exit(EXIT_FAILURE);
	}

	switch (ev->response_type & ~0x80) {
		case XCB_BUTTON_PRESS: {
			xcb_button_press_event_t *e = ( xcb_button_press_event_t *)ev;
			if (e->detail == 1) {
				pointer = xcb_query_pointer_reply(xcb_connection, xcb_query_pointer(xcb_connection, xcb_screen->root), 0);
				selection.x = pointer->root_x;
				selection.y = pointer->root_y;
				selecting = 1;
			} else if (e->detail == 3) {
				exit(EXIT_FAILURE);
			}
		} break;

		case XCB_MOTION_NOTIFY: {
			pointer = xcb_query_pointer_reply(xcb_connection, xcb_query_pointer(xcb_connection, xcb_screen->root), 0);
			if (selecting) {
				selection.w = pointer->root_x - selection.x;
				selection.h = pointer->root_y - selection.y;
				update_selection_window(selection);
			} else if (hovered_win != pointer->child) {
				hovered_win = pointer->child ? pointer->child : xcb_screen->root;
				geom = xcb_get_geometry_reply(xcb_connection, xcb_get_geometry(xcb_connection, hovered_win), NULL);
				update_selection_window((rect_t) {
					.x = geom->x + geom->border_width,
					.y = geom->y + geom->border_width,
					.w = geom->width,
					.h = geom->height
				});
			}
		} break;

		case XCB_BUTTON_RELEASE: {
			xcb_button_release_event_t *e = ( xcb_button_release_event_t *)ev;
			if (e->detail != 1)
				break;

			if (selection.w == 0 && selection.h == 0) {
				pointer = xcb_query_pointer_reply(xcb_connection, xcb_query_pointer(xcb_connection, xcb_screen->root), 0);
				geom = xcb_get_geometry_reply(xcb_connection,
					xcb_get_geometry(xcb_connection, pointer->child ? pointer->child : xcb_screen->root), NULL);

				selection = (rect_t) {
					.x = geom->x + geom->border_width,
					.y = geom->y + geom->border_width,
					.w = geom->width,
					.h = geom->height
				};
			}

			xcb_ungrab_pointer(xcb_connection, XCB_CURRENT_TIME);
			running = 0;
		} break;
	}
	free(ev);
}

int main(int argc, char **argv) {
	if (!xcb_initialize())
		return EXIT_FAILURE;

	while (running) {
		handle_events();
	}

	xcb_finalize();

	selection = fix_rect(selection);

	if (argc <= 1)
		formatted_print(DEFAULT_OUTPUT_FORMAT);
	else {
		formatted_print(argv[1]);
	}

	return EXIT_SUCCESS;
}
