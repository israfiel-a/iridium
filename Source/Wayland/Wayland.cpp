#include "Wayland.hpp"
#include <Logging.hpp>
#include <cstring>

#include "../Vulkan/Vulkan.hpp"

#include "_wl.h"
#include "_xsh.h"

using Registry = struct wl_registry *;
using Compositor = struct wl_compositor *;
using WindowManager = struct xdg_wm_base *;
using WindowManagerSurface = struct xdg_surface *;
using Window = struct xdg_toplevel *;

using RegistryListener = struct wl_registry_listener;
using WindowManagerListener = struct xdg_wm_base_listener;
using WindowManagerSurfaceListener = struct xdg_surface_listener;
using WindowListener = struct xdg_toplevel_listener;

static Display display;
static Registry registry;
static Compositor compositor;
static WindowManager window_manager;

static Surface surface;
static WindowManagerSurface wm_surface;
static Window window;

bool resize = false;
bool ready_to_resize = false;
bool quit = false;

//! all null to nullptr

int32_t width = 0, height = 0;
int32_t new_width = 0, new_height = 0;

static void PingBack(void *data, WindowManager passed_wm, uint32_t serial)
{
    xdg_wm_base_pong(passed_wm, serial);
}

static void HandleWMSConfigure(void *data, WindowManagerSurface passed_wms,
                               uint32_t serial)
{
    xdg_surface_ack_configure(passed_wms, serial);
    if (resize) ready_to_resize = true;
}

static void HandleWindowConfigure(void *data, Window passed_window,
                                  int32_t passed_width,
                                  int32_t passed_height,
                                  struct wl_array *states)
{
    if (passed_width != 0 && passed_height != 0)
    {
        Iridium::Logging::Log("Window resized. New dimensions: " +
                              std::to_string(passed_width) + "x" +
                              std::to_string(passed_height));
        resize = true;
        new_width = passed_width;
        new_height = passed_height;
    }
}

static void HandleWindowClose(void *data, struct xdg_toplevel *toplevel)
{
    quit = true;
}

static void HandleWindowBoundaries(void *data, Window toplevel, int32_t x,
                                   int32_t y)
{
}

static void HandleWMCapabilities(void *data, Window toplevel,
                                 struct wl_array *array)
{
}

static const WindowManagerListener surface_listener = {PingBack};
static const WindowManagerSurfaceListener wms_listener = {
    HandleWMSConfigure};
static const WindowListener window_listener = {
    HandleWindowConfigure, HandleWindowClose, HandleWindowBoundaries,
    HandleWMCapabilities};

static void RegistryHandleAddition(void *data, Registry passed_registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version)
{
    // "Wayland global object connected"
    std::string interface_notification =
        (std::string) "WGOC: " + interface;
    Iridium::Logging::Log(interface_notification);

    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        Iridium::Logging::Log("Got Wayland compositor v" +
                                  std::to_string(version) + ".",
                              Iridium::Logging::success);
        compositor = (Compositor)wl_registry_bind(
            passed_registry, name, &wl_compositor_interface, version);
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        Iridium::Logging::Log("Got XDG shell v" + std::to_string(version) +
                                  ".",
                              Iridium::Logging::success);
        window_manager = (WindowManager)wl_registry_bind(
            registry, name, &xdg_wm_base_interface, version);
        xdg_wm_base_add_listener(window_manager, &surface_listener, NULL);
    }
}

static const RegistryListener registry_listener = {RegistryHandleAddition};

namespace Iridium::Windowing::Wayland
{
    bool Connect()
    {
        display = wl_display_connect(NULL);
        if (display == NULL)
        {
            Logging::RaiseError(Logging::wayland_connection_failed);
            return false;
        }

        registry = wl_display_get_registry(display);
        wl_registry_add_listener(registry, &registry_listener, NULL);
        // Wait for the server to sync with us.
        wl_display_roundtrip(display);

        surface = wl_compositor_create_surface(compositor);
        wm_surface = xdg_wm_base_get_xdg_surface(window_manager, surface);
        xdg_surface_add_listener(wm_surface, &wms_listener, NULL);

        window = xdg_surface_get_toplevel(wm_surface);
        xdg_toplevel_add_listener(window, &window_listener, NULL);

        xdg_toplevel_set_title(window, "SimpleWindow");
        xdg_toplevel_set_app_id(window, "SimpleWindow");

        wl_surface_commit(surface);
        wl_display_roundtrip(display);
        wl_surface_commit(surface);

        return true;
    }

    void Disconnect()
    {
        xdg_toplevel_destroy(window);
        xdg_surface_destroy(wm_surface);
        wl_surface_destroy(surface);
        xdg_wm_base_destroy(window_manager);

        wl_compositor_destroy(compositor);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
    }

    Display GetDisplay() { return display; }
    Surface GetSurface() { return surface; }
    bool ShouldWindowClose() { return quit; }

    void ResizeWindow()
    {
        if (ready_to_resize && resize)
        {
            width = new_width;
            height = new_height;

            Vulkan::WaitForIdle();

            Vulkan::EndSwapchain();
            Vulkan::StartSwapchain(width, height);

            ready_to_resize = false;
            resize = false;
            wl_surface_commit(surface);
        }
    }

    void Sync() { wl_display_roundtrip(display); }

    uint32_t GetWidth() { return width; }

    uint32_t GetHeight() { return height; }
}
