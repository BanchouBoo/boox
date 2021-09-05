#include <xcb/xcb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "xcb/connection.h"
#include "xcb/selection.h"
#include "parsing/parsing.h"

typedef enum {
    ALT_STATE_NONE,
    ALT_STATE_WAITING,
    ALT_STATE_HORIZONTAL,
    ALT_STATE_VERTICAL
} alt_state_t;

int running = 1;

static void handle_events(void) {
    xcb_generic_event_t *ev;
    xcb_get_geometry_reply_t *geom;

    static xcb_window_t hovered_win = 0;
    static int selecting = 0;
    static point_t last_mouse_pos = { .x = 0, .y = 0 };
    static alt_state_t alt_state = ALT_STATE_NONE;

    ev = xcb_wait_for_event(xcb_connection);
    if (!ev) {
        exit(EXIT_FAILURE);
    }

    switch (ev->response_type & ~0x80) {
        case XCB_BUTTON_PRESS: {
            xcb_button_press_event_t *e = ( xcb_button_press_event_t *)ev;
            if (e->detail == 1) {
                if (selection_mode == MODE_SELECT) {
                    if (e->child &&
                        (e->state & XCB_MOD_MASK_CONTROL ||
                        e->state & XCB_MOD_MASK_SHIFT ||
                        alt_state == ALT_STATE_WAITING)) {

                        geom = xcb_get_geometry_reply(xcb_connection, xcb_get_geometry(xcb_connection, e->child), NULL);
                        selection = (rect_t) {
                            .x = geom->x + geom->border_width,
                            .y = geom->y + geom->border_width,
                            .w = geom->width,
                            .h = geom->height
                        };
                        last_mouse_pos.x = selection.x + selection.w;
                        last_mouse_pos.y = selection.y + selection.h;
                        xcb_warp_pointer(xcb_connection, XCB_NONE, xcb_screen->root, 0, 0, 0, 0, selection.x + selection.w, selection.y + selection.h);
                        flush();
                    } else {
                        selection.x = e->root_x;
                        selection.y = e->root_y;
                        last_mouse_pos.x = e->root_x;
                        last_mouse_pos.y = e->root_y;
                    }
                    selecting = 1;
                }
            } else if (e->detail == 3) {
                exit(EXIT_FAILURE);
            }
        } break;

        case XCB_KEY_PRESS: {
            xcb_key_press_event_t *e = ( xcb_key_press_event_t *)ev;
            // 64 == alt
            if (e->detail == 64) {
                alt_state = ALT_STATE_WAITING;
            }
        } break;

        case XCB_KEY_RELEASE: {
            xcb_key_release_event_t *e = ( xcb_key_release_event_t *)ev;
            // 64 == alt
            if (e->detail == 64) {
                alt_state = ALT_STATE_NONE;
                if (selecting) {
                    // set last_mouse_pos to prevent jumping after the warp while selecting with control
                    last_mouse_pos.x = selection.x + selection.w;
                    last_mouse_pos.y = selection.y + selection.h;
                    xcb_warp_pointer(xcb_connection, XCB_NONE, xcb_screen->root, 0, 0, 0, 0, last_mouse_pos.x, last_mouse_pos.y);
                    flush();
                }
            }
        } break;

        case XCB_MOTION_NOTIFY: {
            xcb_motion_notify_event_t *e = ( xcb_motion_notify_event_t *)ev;
            if (selecting) {
                const int16_t relative_x = (e->root_x - last_mouse_pos.x);
                const int16_t relative_y = (e->root_y - last_mouse_pos.y);

                if (alt_state == ALT_STATE_WAITING) {
                    if (abs(relative_x) > abs(relative_y)) alt_state = ALT_STATE_HORIZONTAL;
                    else if (abs(relative_y) > abs(relative_x)) alt_state = ALT_STATE_VERTICAL;
                }

                if (e->state & XCB_MOD_MASK_SHIFT) {
                    if (alt_state == ALT_STATE_HORIZONTAL || alt_state == ALT_STATE_NONE) {
                        selection.x += e->root_x - selection.x - selection.w;
                    }
                    if (alt_state == ALT_STATE_VERTICAL || alt_state == ALT_STATE_NONE) {
                        selection.y += e->root_y - selection.y - selection.h;
                    }
                } else if (e->state & XCB_MOD_MASK_CONTROL) {
                    if (alt_state == ALT_STATE_HORIZONTAL || alt_state == ALT_STATE_NONE) {
                        selection.x -= relative_x;
                        selection.w += relative_x * 2;
                    }
                    if (alt_state == ALT_STATE_VERTICAL || alt_state == ALT_STATE_NONE) {
                        selection.y -= relative_y;
                        selection.h += relative_y * 2;
                    }
                } else {
                    if (alt_state == ALT_STATE_HORIZONTAL || alt_state == ALT_STATE_NONE) {
                        selection.w = e->root_x - selection.x;
                    }
                    if (alt_state == ALT_STATE_VERTICAL || alt_state == ALT_STATE_NONE) {
                        selection.h = e->root_y - selection.y;
                    }
                }
                update_selection_window(selection);
            } else if (hovered_win != e->child) {
                hovered_win = e->child ? e->child : xcb_screen->root;
                geom = xcb_get_geometry_reply(xcb_connection, xcb_get_geometry(xcb_connection, hovered_win), NULL);
                update_selection_window((rect_t) {
                    .x = geom->x + geom->border_width,
                    .y = geom->y + geom->border_width,
                    .w = geom->width,
                    .h = geom->height
                });
            }
            last_mouse_pos.x = e->root_x;
            last_mouse_pos.y = e->root_y;
        } break;

        case XCB_BUTTON_RELEASE: {
            xcb_button_release_event_t *e = ( xcb_button_release_event_t *)ev;
            if (e->detail != 1)
                break;

            if (selection_mode == MODE_POINT) {
                selection.x = e->root_x;
                selection.y = e->root_y;
                selected_window = get_child_window(e->child);
                selected_window = selected_window ? selected_window : e->child;

                xcb_ungrab_pointer(xcb_connection, XCB_CURRENT_TIME);
                running = 0;
            } else if (selecting) {
                if (selection.w == 0 && selection.h == 0) {
                    geom = xcb_get_geometry_reply(xcb_connection,
                        xcb_get_geometry(xcb_connection, e->child ? e->child : xcb_screen->root), NULL);
                    selected_window = get_child_window(e->child);
                    selected_window = selected_window ? selected_window : e->child;

                    selection = (rect_t) {
                        .x = geom->x + geom->border_width,
                        .y = geom->y + geom->border_width,
                        .w = geom->width,
                        .h = geom->height
                    };
                }

                xcb_ungrab_pointer(xcb_connection, XCB_CURRENT_TIME);
                running = 0;
            }

        } break;
    }
    free(ev);
}

void print_help(int exit_code) {
    fprintf(stderr, "boox: [OPTIONS]\n");

    fprintf(stderr, "  -h          print this help and exit\n\n");

    fprintf(stderr, "  -w          if the pointer is already captured, wait for it to be uncaptured instead of failing\n");
    fprintf(stderr, "  -p          select a point instead of region\n");
    fprintf(stderr, "  -f FORMAT   set output format              available values: %%x, %%y, %%w, and %%h for selection values, %%i for window ID (0 if not applicable or root)\n");
    fprintf(stderr, "                                             default selection format: '%s'\n", DEFAULT_SELECTION_OUTPUT_FORMAT);
    fprintf(stderr, "                                             default point format: '%s'\n", DEFAULT_POINT_OUTPUT_FORMAT);
    fprintf(stderr, "  -r WINDOW   restrict selection to window   valid values: root, current, or a window ID\n");
    fprintf(stderr, "                                             default: '%s'\n", "root");
    fprintf(stderr, "  -b SIZE     set selection border size      default: %d\n", DEFAULT_BORDER_SIZE);
    fprintf(stderr, "  -c COLOR    set selection border color     default: %x\n", DEFAULT_BORDER_COLOR);

    exit(exit_code);
}

int main(int argc, char **argv) {
    char *format;
    char *selection_format_env = getenv("BOOX_SELECTION_FORMAT");
    char *point_format_env = getenv("BOOX_POINT_FORMAT");

    char *border_size_env = getenv("BOOX_BORDER_SIZE");
    if (border_size_env)
        border_size = strtol(border_size_env, NULL, 10);
    else
        border_size = DEFAULT_BORDER_SIZE;

    char *border_color_env = getenv("BOOX_BORDER_COLOR");
    if (border_color_env) {
        border_color = strtol(border_color_env, NULL, 16);
    } else
        border_color = DEFAULT_BORDER_COLOR;

    int wait_until_cursor_grabbable = 0;
    char *constraining_window_tag = "root";

    char opt = 0;
    while ((opt = getopt(argc, argv, "hwpf:b:c:r:")) != -1) {
        switch (opt) {
            case 'h': {
                print_help(EXIT_SUCCESS);
            } break;
            case 'w': {
                wait_until_cursor_grabbable = 1;
            } break;
            case 'p': {
                selection_mode = MODE_POINT;
            } break;
            case 'f': {
                format = optarg;
            } break;
            case 'b': {
                border_size = strtol(optarg, NULL, 10);
            } break;
            case 'c': {
                border_color = strtol(optarg, NULL, 16);
            } break;
            case 'r': {
                constraining_window_tag = optarg;
            } break;
            case '?': print_help(EXIT_FAILURE);
        }
    }

    if (!format) {
        if (selection_mode == MODE_SELECT)
            format = selection_format_env ? selection_format_env : DEFAULT_SELECTION_OUTPUT_FORMAT;
        else if (selection_mode == MODE_POINT)
            format = point_format_env ? point_format_env : DEFAULT_POINT_OUTPUT_FORMAT;
    }

    if (!xcb_initialize(wait_until_cursor_grabbable, constraining_window_tag))
        return EXIT_FAILURE;

    while (running) {
        handle_events();
    }

    selection = fix_rect(selection);

    xcb_finalize();

    formatted_print(format);

    return EXIT_SUCCESS;
}
