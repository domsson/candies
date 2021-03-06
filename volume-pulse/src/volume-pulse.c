#include <stdio.h>             // fprintf
#include <stdlib.h>            // NULL, EXIT_*
#include <unistd.h>            // getopt()
#include <pulse/pulseaudio.h>

struct candy_cfg {
	pa_context *ctx;
	pa_mainloop *mlp;
	pa_mainloop_api *api;

	int monitor : 1;       // keep running and print on change?
	int space : 1;         // print space between value and unit?
	int unit : 1;          // print unit after value? ('%')
	int precision;         // how many decimal places to print?
	char *muted;           // string to print when sink is muted
};

/**
 * Print the given volume to stdout.
 */
void print_vol(double vol, int precision, int unit, int space)
{
	fprintf(stdout, "%.*f%s%s\n", precision, vol,
			space && unit ? " " : "",
			unit ? "%" : "");
}

/**
 * Prints usage information to stderr.
 */
void help(const char *invocation)
{
	fprintf(stdout, "Usage:\n");
	fprintf(stdout, "\t%s [OPTION...]\n", invocation);
	fprintf(stdout, "\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "\t-h Print this help text and exit.\n");
	fprintf(stdout, "\t-m Enables monitor mode, where the volume is printed every time it changes.\n");
	fprintf(stdout, "\t-p<int> Number of decimal places in the output.\n");
	fprintf(stdout, "\t-s Print a space between value and percent sign.\n");
	fprintf(stdout, "\t-u Include the percent sign in the output.\n");
	fprintf(stdout, "\t-w<string> Print this string instead of the volume level when the sink is muted.\n");
}

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
	
	struct candy_cfg *cfg = data;
	
	// Is the sink muted?
	if (i->mute && cfg->muted)
	{
		// Print the user-provided 'muted' string
		fprintf(stdout, "%s\n", cfg->muted);
	}
	else
	{
		// Get the sink volume
		pa_volume_t vol = pa_cvolume_avg(&i->volume);
		double percent = vol / (PA_VOLUME_NORM / 100.0);
	
		// Finally print the volume
		print_vol(percent, cfg->precision, cfg->unit, cfg->space); 
	}

	// If NOT in monitor mode (which means we only print once)...
	if (cfg->monitor == 0)
	{
		// We're done, let's quit the main loop
		pa_mainloop_quit(cfg->mlp, 0);
	}
}

/**
 * Callback for when we receive previously queried server information.
 * This helps us find the default sink's name, so we can query it.
 */
void cb_server_info(pa_context *c, const pa_server_info *i, void *data)
{
	// Let's query the default sink's information
	pa_operation *o = NULL;
	o = pa_context_get_sink_info_by_name(c, i->default_sink_name, cb_sink_info, data);
	pa_operation_unref(o);
}

/**
 * Callback for when a context operation was successfull.
 */
void cb_ctx_success(pa_context *c, int success, void *data)
{
	// Do nothing. This is just to please libpulse.
}

/**
 * Callback for when a subscribed event has occured. In our case, that's
 * a change regarding the default sink, for example volume or mute state.
 */
void cb_sub_event(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *data)
{
	if (t == PA_SUBSCRIPTION_EVENT_CHANGE)
	{
		// Volume and/or mute state might have changed, query server again
		pa_operation *o = NULL;
		o = pa_context_get_server_info(c, cb_server_info, data);
		pa_operation_unref(o);
	}
}

/**
 * Callback for when the context status changes.
 * Once the context is ready, we can query information.
 */
void cb_ctx_status(pa_context *c, void *data)
{
	// If the state isn't ready, we've got nothing to do
	if (pa_context_get_state(c) != PA_CONTEXT_READY)
	{
		return;
	}

	// Ask for information about the PulseAudio server. This will enable us 
	// to get the name/index of the default sink, which is hopefully the audio 
	// device in use. From there, we can then actually query the volume level.
	// We'll do this whenever the volume changes, but we also want to do it at 
	// least once on startup - right here!
	pa_operation *o = NULL;
	o = pa_context_get_server_info(c, cb_server_info, data);
	pa_operation_unref(o);
	o = NULL;

	struct candy_cfg *cfg = data;
	if (cfg->monitor)
	{
		// Now let's make sure we receive information about changes
		pa_context_set_subscribe_callback(c, cb_sub_event, data);
		o = pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, cb_ctx_success, data);
		pa_operation_unref(o);
	}
}

/**
 * Should be called on exit to free resources.
 */
void cleanup(struct candy_cfg *cfg)
{
	if (cfg->ctx != NULL)
	{
		pa_context_disconnect(cfg->ctx);
		pa_context_unref(cfg->ctx);
	}
	if (cfg->mlp != NULL)
	{
		pa_mainloop_free(cfg->mlp);
	}
}

int main(int argc, char **argv)
{
	struct candy_cfg cfg = { 0 };

	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "hmnp:suw:")) != -1)
	{
		switch(o)
		{
			case 'h':
				help(argv[0]);
				return EXIT_SUCCESS;
			case 'm':
				cfg.monitor = 1;
				break;
			case 'n':
				cfg.space = 1;
				break;
			case 'p':
				cfg.precision = atoi(optarg);
				break;
			case 's':
				cfg.space = 1;
				break;
			case 'u':
				cfg.unit = 1;
				break;
			case 'w':
				cfg.muted = optarg;
				break;
		}
	}

	// Get the mainloop object
	if (!(cfg.mlp = pa_mainloop_new()))
	{
		return EXIT_FAILURE;
	}

	// Get the mainloop API object
	cfg.api = pa_mainloop_get_api(cfg.mlp);

	// Get the context object
	if (!(cfg.ctx = pa_context_new(cfg.api, NULL)))
	{
		cleanup(&cfg);
		return EXIT_FAILURE;
	}

	// Set the func to call on context state change
	pa_context_set_state_callback(cfg.ctx, cb_ctx_status, &cfg);

	// Connect the context to the pulse server
	pa_context_flags_t flags = { 0 };
	if (pa_context_connect(cfg.ctx, NULL, flags, NULL) < 0)
	{
		cleanup(&cfg);
		return EXIT_FAILURE;
	}

	// Make sure stdout is line buffered 
	setlinebuf(stdout);

	// Run the main loop
	if (pa_mainloop_run(cfg.mlp, NULL) < 0)
	{
		cleanup(&cfg);
		return EXIT_FAILURE;
	}

	// Disconnect, close and free things
	cleanup(&cfg);

	return EXIT_SUCCESS;
}

