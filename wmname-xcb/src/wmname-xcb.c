#include <stdio.h>      // NULL, fprintf(), perror(), setlinebuf()
#include <stdlib.h>     // NULL, EXIT_FAILURE, EXIT_SUCCESS
#include <unistd.h>     // getopt(), sleep()
#include <string.h>     // strlen()
#include <xcb/xcb.h>

#define WM_CHECK "_NET_SUPPORTING_WM_CHECK"
#define WM_NAME  "_NET_WM_NAME"

#define DEFAULT_BUFFER 2048
#define WINDOW_ID_BITS   64

typedef unsigned char byte;

struct options
{
	int buffer;
	byte root : 1;
	byte monitor : 1;
};

typedef struct options opts_s;

void fetch_opts(opts_s *opts, int argc, char **argv)
{
	// Process command line options
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "b:mr")) != -1)
	{
		switch(o)
		{
			case 'b':
				opts->buffer = atoi(optarg);
				break;
			case 'm':
				opts->monitor = 1;
				break;
			case 'r':
				opts->root = 1;
				break;
		}
	}
}

/*
 * Get the property specified by `atom` from the given window `win`
 */
char *get_string_prop(xcb_connection_t *conn, xcb_window_t win, xcb_atom_t atom, char *buf, size_t len)
{
	xcb_get_property_cookie_t cookie;
	xcb_get_property_reply_t *reply;

	cookie = xcb_get_property_unchecked(conn, 0, win, atom, XCB_ATOM_ANY, 0, len / 4);
	reply  = xcb_get_property_reply(conn, cookie, NULL);
	
	if (reply == NULL) return NULL;
	
	int val_len = xcb_get_property_value_length(reply);
	char *value = xcb_get_property_value(reply);

	if (value == NULL) return NULL;

	int read = val_len > len ? len : val_len;
	strncpy(buf, value, read);
	buf[read - 1] = '\0'; // ensure null termination
	free(reply);
	return buf;
}

/*
 * Get the (ID of the) atom with the given name
 */
xcb_atom_t get_atom(xcb_connection_t *conn, char *name)
{
	xcb_intern_atom_cookie_t cookie;
	xcb_intern_atom_reply_t *reply;

	cookie = xcb_intern_atom_unchecked(conn, 1, strlen(name), name);
	reply  = xcb_intern_atom_reply(conn, cookie, NULL);

	if (reply == NULL) return XCB_ATOM_NONE;

	xcb_atom_t atom = reply->atom;
	free(reply);
	return atom;	
}

/*
 * Get the ID of the currently focused window
 */
xcb_window_t get_focused_win(xcb_connection_t *conn)
{
	xcb_get_input_focus_cookie_t cookie;
	xcb_get_input_focus_reply_t *reply;
	
	cookie = xcb_get_input_focus_unchecked(conn);
	reply  = xcb_get_input_focus_reply(conn, cookie, NULL);

	if (reply == NULL) return XCB_WINDOW_NONE;

	xcb_window_t win = reply->focus;
	free(reply);
	return win;
}

/*
 * Get the ID of the root window
 */
xcb_window_t get_root_win(xcb_connection_t *conn)
{
	xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
	return screen->root;
}

/*
 * Get the ID of the window with the window manager name
 */
xcb_window_t get_manager_win(xcb_connection_t *conn, xcb_window_t root, xcb_atom_t check)
{
	xcb_get_property_cookie_t cookie;
	xcb_get_property_reply_t *reply;

	cookie = xcb_get_property_unchecked(conn, 0, root, check, XCB_ATOM_WINDOW, 0, WINDOW_ID_BITS / 4);
	reply  = xcb_get_property_reply(conn, cookie, NULL);

	if (reply == NULL) return XCB_WINDOW_NONE;

	xcb_window_t *winp = xcb_get_property_value(reply);
	xcb_window_t  win = *winp;
	free(reply);
	return win;
}

char *fetch_window_name(xcb_connection_t *conn, char *buf, size_t len)
{
	xcb_window_t window; // focused window
	xcb_atom_t   atom;   // _NET_WM_NAME

	if ((atom = get_atom(conn, WM_NAME)) == XCB_ATOM_NONE)
	{
		return NULL;
	}

	if ((window = get_focused_win(conn)) == XCB_WINDOW_NONE)
	{
		return NULL;
	}
	
	return get_string_prop(conn, window, atom, buf, len);
}

char *fetch_manager_name(xcb_connection_t *conn, char *buf, size_t len)
{
	xcb_window_t f_window; // focused window
	xcb_window_t r_window; // root window
	xcb_window_t m_window; // manager window

	xcb_atom_t name_atom;  // _NET_WM_NAME
	xcb_atom_t check_atom; // _NET_SUPPORTING_WM_CHECK

	if ((name_atom = get_atom(conn, WM_NAME)) == XCB_ATOM_NONE) // both
	{
		return NULL;
	}

	if ((f_window = get_focused_win(conn)) == XCB_WINDOW_NONE) // both
	{
		return NULL;
	}

	if ((check_atom = get_atom(conn, WM_CHECK)) == XCB_ATOM_NONE) // -r
	{
		return NULL;
	}

	if ((r_window = get_root_win(conn)) == XCB_WINDOW_NONE) // -r
	{
		return NULL;
	}

	if ((m_window = get_manager_win(conn, r_window, check_atom)) == XCB_WINDOW_NONE) // -r
	{
		return NULL;
	}
	
	return get_string_prop(conn, m_window, name_atom, buf, len);
}

int main(int argc, char **argv)
{
	// set stdout to line buffered
	setlinebuf(stdout);
	
	// parse command line arguments
	opts_s opts = { 0 };
	fetch_opts(&opts, argc, argv);
	
	// if no buffer size given, use default
	if (opts.buffer == 0)
	{
		opts.buffer = DEFAULT_BUFFER;
	}

	// connect to the X server
	xcb_connection_t *conn = xcb_connect(NULL, 0);

	if (xcb_connection_has_error(conn))
	{
		return EXIT_FAILURE;
	}

	// print the name of the window manager
	if (opts.root)
	{
		char *manager_name = malloc(opts.buffer * sizeof(char));
		fetch_manager_name(conn, manager_name, opts.buffer);
		fprintf(stdout, "%s\n", manager_name);

		free(manager_name);
		xcb_disconnect(conn);
		return EXIT_SUCCESS;
	}

	if (opts.monitor)
	{
		char *window_name  = malloc(opts.buffer * sizeof(char));
		fetch_window_name(conn, window_name, opts.buffer);
		fprintf(stdout, "%s\n", window_name);

		xcb_atom_t   active_atom = get_atom(conn, "_NET_ACTIVE_WINDOW");
		xcb_window_t root_window = get_root_win(conn);

		const uint32_t list[] = { XCB_EVENT_MASK_PROPERTY_CHANGE };
		xcb_change_window_attributes(conn, root_window, XCB_CW_EVENT_MASK, &list);
		xcb_flush(conn);

		xcb_generic_event_t *evt;
		while ((evt = xcb_wait_for_event(conn)))
		{
			if (evt->response_type == XCB_PROPERTY_NOTIFY)
			{
				xcb_property_notify_event_t *e = (void *) evt;
				if (e->atom == active_atom)
				{
					fetch_window_name(conn, window_name, opts.buffer);
					fprintf(stdout, "%s\n", window_name);
				}
			}
			free(evt);
		}
		
		free(window_name);
		xcb_disconnect(conn);
		return EXIT_SUCCESS;
	}

	else
	{
		char *window_name = malloc(opts.buffer * sizeof(char));
		fetch_window_name(conn, window_name, opts.buffer);
		fprintf(stdout, "%s\n", window_name);
		free(window_name);

		xcb_disconnect(conn);
		return EXIT_SUCCESS;
	}
}

