#include "Wayland.hpp"
#include <iostream>

#include "_wl.h"

using Display = struct wl_display *;
using Registry = struct wl_registry *;

using RegistryListener = struct wl_registry_listener;

static void RegistryHandleAddition(void *data,
                                   struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version)
{
    printf("Connected device: %s, v%d\n", interface, version);
}

static void RegistryHandleRemoval(void *data, struct wl_registry *registry,
                                  uint32_t name)
{
    printf("Removed device: %u\n", name);
}

static Display display;
static Registry registry;

static const RegistryListener registry_listener = {RegistryHandleAddition,
                                                   RegistryHandleRemoval};

namespace Iridium::Wayland
{
    bool Connect()
    {
        // Attempt to connect to the preferred Wayland socket.
        display = wl_display_connect(NULL);
        if (display == NULL) exit(255);
        // Get the global registry and wait for the server to sync up.
        registry = wl_display_get_registry(display);
        wl_registry_add_listener(registry, &registry_listener, NULL);
        wl_display_roundtrip(display);

        std::cout << "Connected!" << std::endl;

        return true;
    }

    void Disconnect()
    {
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
    }
}
