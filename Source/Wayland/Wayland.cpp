#include "Wayland.hpp"
#include <Logging.hpp>
#include <cstring>

#include "../Vulkan/Vulkan.hpp"

#include "_wl.h"
#include "_xsh.h"

static Iridium::Windowing::Wayland::Monitor monitor;

using Registry = struct wl_registry *;
using Compositor = struct wl_compositor *;
using WindowManager = struct xdg_wm_base *;
using WindowManagerSurface = struct xdg_surface *;
using Window = struct xdg_toplevel *;
using WaylandMonitor = struct wl_output *;

using RegistryListener = struct wl_registry_listener;
using WindowManagerListener = struct xdg_wm_base_listener;
using WindowManagerSurfaceListener = struct xdg_surface_listener;
using WindowListener = struct xdg_toplevel_listener;
using MonitorListener = struct wl_output_listener;

using WaylandArray = struct wl_array *;

static Display display;
static Registry registry;
static Compositor compositor;
static WindowManager window_manager;
static WaylandMonitor output;

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

static void HandleWindowConfigure(void *_d, Window window, std::int32_t _w,
                                  std::int32_t _h, WaylandArray _s)
{
    xdg_toplevel_set_fullscreen(window, NULL);
    Iridium::Logging::Log("Window configured.");
}

static void HandleWindowClose(void *data, struct xdg_toplevel *toplevel)
{
    quit = true;
}

static void HandleWindowBoundaries(void *_d, Window _w, int32_t width,
                                   int32_t height)
{
    // Iridium::Logging::Log("Monitor dimensions: " + std::to_string(width)
    // +
    //                       "x" + std::to_string(height));
    // monitor_dimensions[0] = width;
    // monitor_dimensions[1] = height;
}

static void HandleWMCapabilities(void *data, Window toplevel,
                                 struct wl_array *array)
{
}

static void HandleMonitorGeometry(void *data, WaylandMonitor wl_output,
                                  int32_t x, int32_t y,
                                  int32_t physical_width,
                                  int32_t physical_height,
                                  int32_t subpixel, const char *make,
                                  const char *model, int32_t transform)
{
    monitor.x = x;
    monitor.y = y;
}

static void HandleMonitorPixelContents(void *data,
                                       WaylandMonitor wl_output,
                                       uint32_t flags, int32_t width,
                                       int32_t height, int32_t refresh)
{
    monitor.width = static_cast<std::uint32_t>(width);
    monitor.height = static_cast<std::uint32_t>(height);
    monitor.refresh_rate = Iridium::Windowing::Wayland::mHzToMHz(
        static_cast<std::uint32_t>(refresh));
}

static void HandleMonitorInformationSent(void *data,
                                         WaylandMonitor wl_output)
{
    // No operation.
}

static void HandleMonitorScale(void *data, WaylandMonitor wl_output,
                               int32_t factor)
{
    monitor.scale = factor;
}

static void HandleMonitorName(void *data, struct wl_output *wl_output,
                              const char *name)
{
    // No operation.
}

static void HandleMonitorDescription(void *data,
                                     struct wl_output *wl_output,
                                     const char *description)
{
    // No operation.
}

const MonitorListener monitor_listener = {HandleMonitorGeometry,
                                          HandleMonitorPixelContents,
                                          HandleMonitorInformationSent,
                                          HandleMonitorScale,
                                          HandleMonitorName,
                                          HandleMonitorDescription};

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
            passed_registry, name, &xdg_wm_base_interface, version);
        xdg_wm_base_add_listener(window_manager, &surface_listener, NULL);
    }
    else if (strcmp(interface, wl_output_interface.name) == 0)
    {
        Iridium::Logging::Log("Got Wayland output v" +
                                  std::to_string(version) + ".",
                              Iridium::Logging::success);
        output = (WaylandMonitor)wl_registry_bind(
            passed_registry, name, &wl_output_interface, version);
        wl_output_add_listener(output, &monitor_listener, NULL);
    }
}

static const RegistryListener registry_listener = {RegistryHandleAddition};

namespace Iridium::Windowing::Wayland
{
    bool Connect()
    {
        display = wl_display_connect(0);
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

        wl_output_destroy(output);
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
            Vulkan::StartSwapchain();

            ready_to_resize = false;
            resize = false;
            wl_surface_commit(surface);
        }
    }

    void Sync() { wl_display_roundtrip(display); }

    uint32_t GetWidth() { return width; }

    uint32_t GetHeight() { return height; }

    const Monitor &GetMonitor() noexcept { return monitor; }
}
