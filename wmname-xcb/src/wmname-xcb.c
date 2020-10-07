#include <stdio.h>      // NULL, fprintf(), perror(), setlinebuf()
#include <stdlib.h>     // NULL, EXIT_FAILURE, EXIT_SUCCESS
#include <unistd.h>     // getopt(), sleep()
#include <string.h>     // strlen()
#include <xcb/xcb.h>

#define WM_CHECK "_NET_SUPPORTING_WM_CHECK"
#define WM_NAME  "_NET_WM_NAME"

#define DEFAULT_BUFFER 2048
#define WINDOW_ID_BITS   64

struct options
{
	int buffer;
	unsigned char root;
};

typedef struct options opts_s;

void fetch_opts(opts_s *opts, int argc, char **argv)
{
	// Process command line options
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "b:r")) != -1)
	{
		switch(o)
		{
			case 'b':
				opts->buffer = atoi(optarg);
				break;
			case 'r':
				opts->root = 1;
				break;
		}
	}
}

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

xcb_window_t get_root_win(xcb_connection_t *conn, xcb_window_t ref)
{
	xcb_query_tree_cookie_t cookie;
	xcb_query_tree_reply_t *reply;

	cookie = xcb_query_tree_unchecked(conn, ref);
	reply  = xcb_query_tree_reply(conn, cookie, NULL);

	if (reply == NULL) return XCB_WINDOW_NONE;

	xcb_window_t win = reply->root;
	free(reply);
	return win;
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

/*
 * Get the property specified by `atom` from the given window `win`
 */
char *get_string_prop(xcb_connection_t *conn, xcb_window_t win, xcb_atom_t atom, size_t buf_len)
{
	xcb_get_property_cookie_t cookie;
	xcb_get_property_reply_t *reply;

	cookie = xcb_get_property_unchecked(conn, 0, win, atom, XCB_ATOM_ANY, 0, buf_len / 4);
	reply  = xcb_get_property_reply(conn, cookie, NULL);
	
	if (reply == NULL) return NULL;
	
	int len = xcb_get_property_value_length(reply);
	if (len == 0)
	{
		free(reply);
		return NULL;
	}

	char *value = xcb_get_property_value(reply);
	char *prop = strndup(value, len);
	free(reply);
	return prop;
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

	xcb_window_t f_window; // focused window
	xcb_window_t r_window; // root window
	xcb_window_t m_window; // manager window

	xcb_atom_t name_atom;  // _NET_WM_NAME
	xcb_atom_t check_atom; // _NET_SUPPORTING_WM_CHECK

	if ((name_atom = get_atom(conn, WM_NAME)) == XCB_ATOM_NONE) // both
	{
		return EXIT_FAILURE;
	}

	if ((f_window = get_focused_win(conn)) == XCB_WINDOW_NONE) // both
	{
		return EXIT_FAILURE;
	}

	if ((check_atom = get_atom(conn, WM_CHECK)) == XCB_ATOM_NONE) // -r
	{
		return EXIT_FAILURE;
	}

	if ((r_window = get_root_win(conn, f_window)) == XCB_WINDOW_NONE) // -r
	{
		return EXIT_FAILURE;
	}

	if ((m_window = get_manager_win(conn, r_window, check_atom)) == XCB_WINDOW_NONE) // -r
	{
		return EXIT_FAILURE;
	}
	
	char *name = get_string_prop(conn, opts.root ? m_window : f_window, name_atom, opts.buffer);
	if (name == NULL)
	{
		return EXIT_FAILURE;
	}

	fprintf(stdout, "%s\n", name);
	free(name);
	
	// clean up
	xcb_disconnect(conn);
		
	// done, bye
	return EXIT_SUCCESS;
}

