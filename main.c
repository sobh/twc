//---- Includes ----------------------------------------------------------------
#include <assert.h>
#include <bits/time.h>
#include <bits/types/struct_timespec.h>
#include <stdio.h>

#include <stdlib.h>
#include <time.h>
#include <wayland-client.h>
#include <wayland-server-core.h>
#include <wayland-server.h>

#include <wlr/backend.h>
#include <wlr/types/wlr_output.h>


//---- Structs -----------------------------------------------------------------
/*!
 * @struct  twc_server
 * @brief   Holds the compositor's state.
 */
struct twc_server {
	// Wayland
	struct wl_display *wl_display;          //! Connection to the client
	struct wl_event_loop *wl_event_loop;    //! Event Loop Context

	struct wl_list outputs;                 //! List of outputs
	// Listeners
	struct wl_listener new_output;          //! New Output Signal Handler
	// wlroots
	struct wlr_backend *backend;            //! wlroots backend
};

/*!
 * @struct  twc_output
 * @brief   Compositor's Output
 */
struct twc_output {
	struct wlr_output *wlr_output;
	struct twc_server *server;
	struct timespec last_frame;

	struct wl_list link;                //! Link to => twc_server.outputs

	// Listeners
	struct wl_listener destroy;         //! Deconstructor
};

//---- Functions ---------------------------------------------------------------
//---- Handlers ----------------------------------------------------------------
/*!
 * @brief       Output Destructor
 * @param[in]   listener
 * @param[in]   data
 */
static void output_destroy_notify(struct wl_listener *listener, void *data)
{
	// Get a reference of the 'twc_output' that contains the passed 'wl_listener'
	struct twc_output *output = wl_container_of(listener, output, destroy);

	wl_list_remove(&output->link);
	wl_list_remove(&output->destroy.link);

	free(output);
}

/*!
 * @brief       New Output Addition Handler
 * @param[in]   listener
 * @param[in]   data
 */
static void new_output_notify(struct wl_listener *listener, void *data)
{
	// Get a reference of the 'twc_server' that contains the passed 'wl_listener'
	struct twc_server *server = wl_container_of(listener, server, new_output);
	struct wlr_output *wlr_output = data;

	/*!
	 * Set the output mode to last available mode, as it is usually the
	 * highest resolution, and refresh rate.
	 */
	if(!wl_list_empty(&wlr_output->modes)){
		struct wlr_output_mode *mode = wl_container_of(&wlr_output->modes.prev, mode, link);
		wlr_output_set_mode(wlr_output, mode);
	}

	/*!
	 * Setup a structure to keep track of the new output within the
	 * compositor.
	 */
	struct twc_output *output = calloc(1, sizeof(struct twc_output));
	clock_gettime(CLOCK_MONOTONIC, &output->last_frame);
	output->server = server;
	output->wlr_output = wlr_output;
	wl_list_insert(&server->outputs, &output->link);

	// Register the Output Destruction Handler
	output->destroy.notify = output_destroy_notify;
	wl_signal_add(&wlr_output->events.destroy, &output->destroy);

}

//---- Main --------------------------------------------------------------------
int main(int argc, char **argv)
{
	struct twc_server server;

	server.wl_display = wl_display_create();
	assert(server.wl_display);

	server.wl_event_loop = wl_display_get_event_loop(server.wl_display);
	assert(server.wl_event_loop);

        // Automatically choose the most appropriate backend based on the user’s
        // environment
        server.backend = wlr_backend_autocreate(server.wl_display);
	assert(server.backend);

	// Register the New Output Addition Handler
	wl_list_init(&server.outputs);
	server.new_output.notify = new_output_notify;
	wl_signal_add(&server.backend->events.new_output, &server.new_output);

	// Start the backend, and enter the Wayland event loop.
	if(!wlr_backend_start(server.backend)){
		fprintf(stderr, "Failed to start the backend.\n");
		wl_display_destroy(server.wl_display);
		return 1;
	}

	wl_display_run(server.wl_display);
	wl_display_destroy(server.wl_display);
	return 0;
}
