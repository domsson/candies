#include <stdio.h>             // fprintf
#include <stdlib.h>            // NULL, EXIT_*
#include <pulse/pulseaudio.h>

/**
 * Callback for when we receive previously queried sink information.
 * This will actually let us query the queried sink's volume, yay!
 */
void cb_sink_info(pa_context *c, const pa_sink_info *i, int eol, void *data)
{
	// We're not interested in the "end of list" marker
	if (eol == 1)
	{
		return;
	}
	
	// Is the sink muted?
	if (i->mute)
	{
		// TODO
	}

	// Get the sink volume, finally!
	pa_volume_t vol = pa_cvolume_avg(&i->volume);
	char pretty_vol[16];
	fprintf(stdout, "%s\n", pa_volume_snprint(pretty_vol, 16, vol));

	// We're done, let's quit the main loop
	pa_mainloop_quit(data, 0);
}

/**
 * Callback for when we receive previously queried server information.
 * This helps us find the default sink's name, so we can query it.
 */
void cb_server_info(pa_context *c, const pa_server_info *i, void *data)
{
	// Let's query the default sink's information
	pa_context_get_sink_info_by_name(c, i->default_sink_name, cb_sink_info, data);
}

/**
 * Callback for when the context status changes.
 * Once the context is ready, we can query information.
 */
void cb_ctx_status(pa_context *c, void *data)
{
	// Get the state of the context
	pa_context_state_t state = pa_context_get_state(c);

	// If the state isn't ready, we've got nothing to do
	if (state != PA_CONTEXT_READY)
	{
		return;
	}

	// Let's query the pulse audio server information
	pa_context_get_server_info(c, cb_server_info, data);
}

int main(int argc, char **argv)
{
	// Prepare all the important things
	pa_context *ctx = NULL;
	pa_mainloop *mlp = NULL;
	pa_mainloop_api *api = NULL;

	// Get the mainloop object
	if (!(mlp = pa_mainloop_new()))
	{
		fprintf(stderr, "pa_mainloop_new() failed.\n");
		return EXIT_FAILURE;
	}

	// Get the mainloop API object
	api = pa_mainloop_get_api(mlp);

	// Get the context object
	if (!(ctx = pa_context_new(api, NULL)))
	{
		fprintf(stderr, "pa_context_new() failed.\n");
		return EXIT_FAILURE;
	}

	// Set the func to call on context state change
	pa_context_set_state_callback(ctx, cb_ctx_status, mlp);

	// Connect the context to the pulse server
	pa_context_flags_t flags = { 0 };
	if (pa_context_connect(ctx, NULL, flags, NULL) < 0)
	{
		fprintf(stderr, "pa_context_connect() failed.\n");
		return EXIT_FAILURE;
	}

	// Run the main loop
	if (pa_mainloop_run(mlp, NULL) < 0)
	{
		fprintf(stderr, "pa_mainloop_run() failed.\n");
		return EXIT_FAILURE;
	}

	// Disconnect, close and free things
	pa_context_disconnect(ctx);
	pa_mainloop_free(mlp);

	return EXIT_SUCCESS;
}

