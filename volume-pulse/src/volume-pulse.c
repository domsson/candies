#include <stdio.h>             // fprintf
#include <stdlib.h>            // NULL, EXIT_*
#include <unistd.h>            // getopt()
#include <pulse/pulseaudio.h>

struct candy_cfg {
	pa_context *ctx;
	pa_mainloop *mlp;
	pa_mainloop_api *api;

	int space : 1;
	int unit : 1;
	int precision;
	char *muted;
};

void print_vol(double vol, int precision, int unit, int space)
{
	fprintf(stdout, "%.*f%s%s\n", precision, vol,
			space && unit ? " " : "",
			unit ? "%" : "");
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

		// We're done, let's quite main loop
		pa_mainloop_quit(cfg->mlp, 0);
		return;
	}

	// Get the sink volume
	pa_volume_t vol = pa_cvolume_avg(&i->volume);
	double percent = vol / (PA_VOLUME_NORM / 100.0);

	// Finally print the volume
	print_vol(percent, cfg->precision, cfg->unit, cfg->space); 

	// We're done, let's quit the main loop
	pa_mainloop_quit(cfg->mlp, 0);
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
	// If the state isn't ready, we've got nothing to do
	if (pa_context_get_state(c) != PA_CONTEXT_READY)
	{
		return;
	}

	// Let's query the pulse audio server information
	pa_context_get_server_info(c, cb_server_info, data);
}

void cleanup(struct candy_cfg *cfg)
{
	if (cfg->ctx != NULL)
	{
		pa_context_disconnect(cfg->ctx);
		// TODO I'm not exactly sure what the next line does
		// and if I should use it, but without it, we are leaking
		// more memory according to valgrind... so...
		pa_context_unref(cfg->ctx);
	}
	if (cfg->mlp != NULL)
	{
		pa_mainloop_free(cfg->mlp);
	}
}

int main(int argc, char **argv)
{
	// Command line argument helpers
	struct candy_cfg cfg = { 0 };
	cfg.space = 1;

	opterr = 0;
	int o;
	while ((o = getopt(argc, argv, "np:us:")) != -1)
	{
		switch(o)
		{
			case 'n':
				cfg.space = 0;
				break;
			case 'p':
				cfg.precision = atoi(optarg);
				break;
			case 'u':
				cfg.unit = 1;
				break;
			case 's':
				cfg.muted = optarg;
				break;
		}
	}

	// Get the mainloop object
	if (!(cfg.mlp = pa_mainloop_new()))
	{
		//fprintf(stderr, "pa_mainloop_new() failed.\n");
		return EXIT_FAILURE;
	}

	// Get the mainloop API object
	cfg.api = pa_mainloop_get_api(cfg.mlp);

	// Get the context object
	if (!(cfg.ctx = pa_context_new(cfg.api, NULL)))
	{
		//fprintf(stderr, "pa_context_new() failed.\n");
		cleanup(&cfg);
		return EXIT_FAILURE;
	}

	// Set the func to call on context state change
	pa_context_set_state_callback(cfg.ctx, cb_ctx_status, &cfg);

	// Connect the context to the pulse server
	pa_context_flags_t flags = { 0 };
	if (pa_context_connect(cfg.ctx, NULL, flags, NULL) < 0)
	{
		//fprintf(stderr, "pa_context_connect() failed.\n");
		cleanup(&cfg);
		return EXIT_FAILURE;
	}

	// Turn off buffering for stdout
	setbuf(stdout, NULL);

	// Run the main loop
	if (pa_mainloop_run(cfg.mlp, NULL) < 0)
	{
		//fprintf(stderr, "pa_mainloop_run() failed.\n");
		cleanup(&cfg);
		return EXIT_FAILURE;
	}

	// Disconnect, close and free things
	cleanup(&cfg);

	return EXIT_SUCCESS;
}

