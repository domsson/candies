#include <stdio.h>      // NULL, fprintf(), perror(), setlinebuf()
#include <stdlib.h>     // NULL, EXIT_FAILURE, EXIT_SUCCESS
#include <unistd.h>     // getopt(), sleep()
#include <xcb/xcb.h>

#define DEFAULT_BUFFER 2048

struct options
{
	int buffer;
};

typedef struct options opts_s;

void fetch_opts(opts_s *opts, int argc, char **argv)
{
	// Process command line options
	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "b:")) != -1)
	{
		switch(o)
		{
			case 'b':
				opts->buffer = atoi(optarg);
				break;
		}
	}
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

	// get the ID of the currently focused window
	xcb_get_input_focus_cookie_t focus_cookie;
	xcb_get_input_focus_reply_t *focus_reply;

	focus_cookie = xcb_get_input_focus_unchecked(conn);
	focus_reply  = xcb_get_input_focus_reply(conn, focus_cookie, NULL);

	if (focus_reply == NULL)
	{
		return EXIT_FAILURE;
	}

	// get the WM_NAME property of the focused window
	xcb_get_property_cookie_t prop_cookie;
	xcb_get_property_reply_t *prop_reply;
	uint32_t len = opts.buffer / 4;
	
	prop_cookie = xcb_get_property_unchecked(conn, 0, focus_reply->focus, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 0, len);
	prop_reply  = xcb_get_property_reply(conn, prop_cookie, NULL);
	
	if (prop_reply == NULL)
	{
		return EXIT_FAILURE;
	}

	// print the WM_NAME property value
	fprintf(stdout, "%s\n", (char*) xcb_get_property_value(prop_reply));
	
	// clean up
	free(focus_reply);
	free(prop_reply);
	xcb_disconnect(conn);
		
	// done, bye
	return EXIT_SUCCESS;
}

