#include "Wayland.h"
#include "XDGShell.h"

#include <wayland-client.h>

#include <stdio.h>
#include <string.h>

static ir_wayland_display_t display = nullptr;
static ir_wayland_registry_t registry = nullptr;

static ir_wayland_compositor_t compositor = nullptr;
static ir_wayland_output_t output = nullptr;
static ir_wayland_window_manager_t window_manager = nullptr;

static void HandleGlobalAddition(void *data,
                                 ir_wayland_registry_t passed_registry,
                                 uint32_t numerical_name,
                                 const char *interface_name,
                                 uint32_t version)
{
    if (strcmp(interface_name, wl_compositor_interface.name) == 0)
    {
        puts("Found wl_compositor.");
        compositor = wl_registry_bind(passed_registry, numerical_name,
                                      &wl_compositor_interface, version);
    }
    else if (strcmp(interface_name, wl_output_interface.name) == 0)
    {
        puts("Found wl_output.");
        output = wl_registry_bind(passed_registry, numerical_name,
                                  &wl_output_interface, version);
    }
    else if (strcmp(interface_name, xdg_wm_base_interface.name) == 0)
    {
        puts("Found xdg_wm_base.");
        window_manager = wl_registry_bind(passed_registry, numerical_name,
                                          &xdg_wm_base_interface, version);
    }
}

static void HandleGlobalRemoval(void *data, ir_wayland_registry_t registry,
                                uint32_t numerical_name)
{}

/**
 * @name registry_listener
 * @brief The listener for the Wayland registry object. This explicitly
 * only cares about interface additions, not removal.
 */
static const struct wl_registry_listener registry_listener = {
    HandleGlobalAddition, HandleGlobalRemoval};

bool Ir_WaylandConnect(void)
{
    if (display != nullptr) return true;
    // Connect to the default Wayland display.
    display = wl_display_connect(0);
    if (display == NULL)
    {
        display = nullptr;
        // error handling goes here -- errno is set here!
        return false;
    }

    registry = wl_display_get_registry(display);
    if (registry == NULL)
    {
        registry = nullptr;
        // follows
        return false;
    }
    wl_registry_add_listener(registry, &registry_listener, nullptr);
    // Wait for all the global objects to bind properly.
    wl_display_roundtrip(display);

    // Make sure we got required objects.
    if (compositor == nullptr || output == nullptr ||
        window_manager == nullptr)
    {
        // error handling here!
        return false;
    }

    return true;
}

void Ir_WaylandDisconnect(void)
{
    xdg_wm_base_destroy(window_manager);
    window_manager = nullptr;

    // Note that we don't destroy the global output object because it gets
    // destroyed by its callbacks once all information has been
    // transferred.
    wl_compositor_destroy(compositor);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);
    compositor = nullptr;
    registry = nullptr;
    display = nullptr;
}
