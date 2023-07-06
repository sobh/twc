//---- Includes ----------------------------------------------------------------
#include <assert.h>
#include <stdio.h>

#include <wayland-client.h>
#include <wayland-server.h>

#include <wlr/backend.h>
//---- Structs -----------------------------------------------------------------
/*!
 * @struct  mcw_server
 * @brief   Holds the compositor's state.
 */
struct twc_server {
	// Wayland
	struct wl_display *wl_display;          //! Connection to the client
	struct wl_event_loop *wl_event_loop;    //! Event Loop Context

	// wlroots
	struct wlr_backend *backend;            //! wlroots backend
};

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
