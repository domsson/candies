#include <stdio.h>             // fprintf
#include <stdlib.h>            // NULL, EXIT_*
#include <pulse/pulseaudio.h>

void cb_ctx_success(pa_context *c, int success, void *data)
{
	// A context operation was successful. Okay.
}

/**
 * Callback for when events occur that we subscribed to.
 */
void cb_sub_event(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *data)
{
	// We detected a change, it might be a volume or mute state change
	if (t == PA_SUBSCRIPTION_EVENT_CHANGE)
	{
		// We print something so that succade will be triggered
		fprintf(stdout, "change detected\n");
	}
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

	// Set the callback function for context events
	pa_context_set_subscribe_callback(c, cb_sub_event, data);

	// Let's subscribe to events (we're interested in sink events)
	//pa_subscription_mask_t mask = PA_SUBSCRIPTION_MASK_SINK;
	pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, cb_ctx_success, data);
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

