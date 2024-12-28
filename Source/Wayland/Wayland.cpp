/**
 * @file Wayland.cpp
 * @author Israfil Argos (israfiel-a)
 * @date 2024-12-27
 * @brief This file provides the implementation of the Iridium Wayland
 * client.
 *
 * Copyright (c) 2024 Israfil Argos
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */
#include "Wayland.hpp"
#include <Logging.hpp>

// This pulls in the Wayland header file as well.
#include "_xsh.h"

#include <cstring>

/**
 * @brief The Wayland registry interface.
 */
using Registry = struct wl_registry *;

/**
 * @brief The Wayland compositor interface.
 */
using Compositor = struct wl_compositor *;

/**
 * @brief The Wayland output (monitor) interface.
 */
using Output = struct wl_output *;

/**
 * @brief The Wayland array type.
 */
using Array = struct wl_array *;

/**
 * @brief The XDG window manager interface.
 */
using WindowManager = struct xdg_wm_base *;

/**
 * @brief The XDG surface interface.
 */
using WindowManagerSurface = struct xdg_surface *;

/**
 * @brief The XDG toplevel window interface.
 */
using Window = struct xdg_toplevel *;

/**
 * @brief The Wayland registry listener.
 */
using RegistryListener = struct wl_registry_listener;

/**
 * @brief The Wayland output listener.
 */
using OutputListener = struct wl_output_listener;

/**
 * @brief The XDG window manager listener.
 */
using WindowManagerListener = struct xdg_wm_base_listener;

/**
 * @brief The XDG (not Wayland!) surface listener.
 */
using SurfaceListener = struct xdg_surface_listener;

/**
 * @brief The XDG toplevel window listener.
 */
using WindowListener = struct xdg_toplevel_listener;

/**
 * @brief The Wayland display object. This is basically our connection to
 * the compositor.
 */
static Display display;

/**
 * @brief The Wayland registry object. This provides methods for handling
 * new interfaces and objects connected to the program.
 */
static Registry registry;

/**
 * @brief The Wayland compositor object. This provides methods for fetching
 * surfaces.
 */
static Compositor compositor;

/**
 * @brief The root Wayland surface. On top of this is where XDG and Vulkan
 * place their data.
 */
static Surface surface;

/**
 * @brief The XDG window manager base object. This contains methods for
 * creating actual windows, intead of just "surfaces".
 */
static WindowManager window_manager;

/**
 * @brief A halfway-window. This is not quite a toplevel surface, but it
 * contains important information and proxy methods.
 */
static WindowManagerSurface wrapped_surface;

/**
 * @brief The actual window object. This is what is rendered on the screen.
 */
static Window window;

/**
 * @brief Information about the monitor we're currently rendering on. This
 * only changes should our application switch monitors.
 */
static Iridium::Windowing::Wayland::Monitor monitor;

/**
 * @brief A boolean representing whether or not the window should close.
 */
bool close = false;

/**
 * @brief A boolean representing whether or not we should panic if the
 * compositor does not have fullscreen capabilities.
 */
bool panic_on_no_fullscreen = true;

/**
 * @brief A boolean representing whether or not we should panic if the
 * compositor does not have minimization capabilities.
 */
bool panic_on_no_minimize = true;

/**
 * @brief A boolean representing whether or not the compositor supports
 * fullscreening.
 */
bool fullscreen_enabled = false;

/**
 * @brief A boolean representing whether or not the compositor supports
 * minimization.
 */
bool minimize_enabled = false;

/**
 * @brief Handle a ping request from the window manager. If we do not
 * respond to this, the WM may mark us as unresponsive and kill our
 * beautiful process.
 * @param window_manager The window manager requesting the pong.
 * @param serial The serial timestamp of the request.
 */
static void HPR(Iridium::Generic _data, WindowManager window_manager,
                Iridium::U32 serial) noexcept
{
    xdg_wm_base_pong(window_manager, serial);
}

/**
 * @brief The window manager's listener object. Functions within this
 * object will be called by the server when the manager is changed somehow.
 */
const WindowManagerListener window_manager_listener = {HPR};

/**
 * @brief Handle a configure request from the window manager. We don't do
 * much with this beyond responding with a thumbs up.
 * @param surface The surface that's been configured.
 * @param serial The serial timestamp of the request.
 */
static void HCR(Iridium::Generic _data, WindowManagerSurface surface,
                Iridium::U32 serial) noexcept
{
    xdg_surface_ack_configure(surface, serial);
}

/**
 * @brief The listener for XDG surfaces. This just handles configure
 * requests.
 */
const SurfaceListener surface_listener = {HCR};

/**
 * @brief Handle a configure event sent to the toplevel surface. This
 * usually denotes a change in state such as maximization, minimization,
 * etcetera.
 * @param window The window configured.
 */
static void HTC(Iridium::Generic _data, Window window, Iridium::I32 _width,
                Iridium::I32 _height, Array _state) noexcept
{
    // Note that we don't call an "acknowledge" function here despite
    // reccomendation, because down the line, within the underlying
    // surface's configure event, we send an acknowledge.
    xdg_toplevel_set_fullscreen(window, nullptr);
    Iridium::Logging::Log("Game window configured.",
                          Iridium::Logging::success);
}

/**
 * @brief Handle the close ("end") of the toplevel surface. None of the
 * parameters in this function are used.
 */
static void HTE(Iridium::Generic _data, Window _window) noexcept
{
    Iridium::Windowing::Wayland::CloseWindow();
    Iridium::Logging::Log("Game window closed.",
                          Iridium::Logging::warning);
}

/**
 * @brief Function to handle the reported maximum boundaries of the
 * toplevel window. This is typically the size of the monitor minus OS
 * decorations.
 * @noop We do not utilize this function.
 */
static void HTB(Iridium::Generic _data, Window _window,
                Iridium::I32 _width, Iridium::I32 _height) noexcept
{}

/**
 * @brief Handle a report on the window manager's capabilities. We don't
 * particularly care for most of these states, minus the following;
 * - fullscreen: We need to fullscreen the window.
 * - minimize: We need to minimize the window.
 * By default these kill the thread, but this behavior can be changed via a
 * call to IgnoreRestrictions--in which case we will simply do the best we
 * can.
 * @param capabilities The reported capabilities of the WM.
 */
static void HMC(Iridium::Generic _data, Window _window, Array capabilities)
{
    // Set the cursor to the first element in the array.
    Iridium::U32 *cursor = static_cast<Iridium::U32 *>(capabilities->data);
    if (capabilities->size == 0 &&
        (panic_on_no_fullscreen || panic_on_no_minimize))
        Iridium::Logging::Log("Wayland compositor missing capabilities.",
                              Iridium::Logging::panic);

    // Loop over each following element, digesting them as we go. I don't
    // fully understand why I have to case to const char* here, but it's
    // the only cast that works. Also note that C++ whinges if I do a
    // static_cast on cursor, so that's a C cast.
    for (; (const char *)cursor <
           static_cast<const char *>(capabilities->data) +
               capabilities->size;
         ++cursor)
    {
        if (*cursor == 3)
        {
            fullscreen_enabled = true;
            Iridium::Logging::Log(
                "Wayland compositor supports fullscreen.",
                Iridium::Logging::success);
        }
        else if (*cursor == 4)
        {
            minimize_enabled = true;
            Iridium::Logging::Log(
                "Wayland compositor supports minimization.",
                Iridium::Logging::success);
        }
    }

    if (panic_on_no_fullscreen && !fullscreen_enabled)
        Iridium::Logging::Log(
            "Wayland compositor missing fullscreen support.",
            Iridium::Logging::panic);

    if (panic_on_no_minimize && !minimize_enabled)
        Iridium::Logging::Log(
            "Wayland compositor missing minimize support.",
            Iridium::Logging::panic);
}

/**
 * @brief The listener object for an XDG toplevel object. This contains
 * logic for closing the window and some handlers for WM capabilities.
 */
const WindowListener window_listener = {HTC, HTE, HTB, HMC};

/**
 * @brief Handle a report from Wayland about the output device's geometry.
 * This provides us with a significant amount of utterly useless
 * information.
 * @noop We do nothing with this function.
 */
static void HOG(Iridium::Generic _data, Output _output, Iridium::I32 _x,
                Iridium::I32 _y, Iridium::I32 _physical_width,
                Iridium::I32 _physical_height, Iridium::I32 _subpixel,
                const char *_make, const char *_model,
                Iridium::I32 _transform) noexcept
{}

/**
 * @brief Handle the monitor's video mode.
 * @param width The width of the monitor.
 * @param height The height of the monitor.
 * @param refresh The refresh rate of the monitor in mHz.
 */
static void HOM(Iridium::Generic _data, Output _output,
                Iridium::U32 _flags, Iridium::I32 width,
                Iridium::I32 height, Iridium::I32 refresh) noexcept
{
    monitor.width = static_cast<std::uint32_t>(width);
    monitor.height = static_cast<std::uint32_t>(height);
    // For some completely random reason, monitor refresh rate is reported
    // in MILIHERTZ. Conversion time.
    monitor.refresh_rate = Iridium::Windowing::Wayland::mHzToHz(
        static_cast<Iridium::U32>(refresh));
}

/**
 * @brief Handle the monitor's "done" event, meaning that all needed
 * information has been properly transferred. The output object is
 * inaccessible past the call of this function.
 * @param output The output who has finished.
 */
static void HOD(Iridium::Generic _data, Output output) noexcept
{
    // We've gained all the information we need from this.
    wl_output_destroy(output);
    Iridium::Logging::Log(
        "Monitor attached: " + std::to_string(monitor.width) + "x" +
        std::to_string(monitor.height) + " @ " +
        std::to_string(monitor.refresh_rate) + "Hz.");
}

/**
 * @brief Handle the scale factor of a monitor. This is probably not
 * particularly useful, but we note it down anyways because 2 bytes is
 * truly not the end of the world and someone may need it.
 * @param factor The scale factor.
 */
static void HOS(Iridium::Generic _data, Output _output,
                Iridium::I32 factor) noexcept
{
    monitor.scale = static_cast<Iridium::U16>(factor);
}

/**
 * @brief Handle the model name of the monitor.
 * @noop We do nothing with this function.
 */
static void HON(Iridium::Generic _data, Output _output,
                const char *_name) noexcept
{}

/**
 * @brief Handle the biography of the monitor.
 * @noop We do nothing with this function.
 */
static void HOB(Iridium::Generic _data, Output _output,
                const char *_description) noexcept
{}

/**
 * @brief The listener for the Wayland output (monitor) object. This simply
 * notes down the information we need and then kills its parent object.
 */
const OutputListener output_listener = {HOG, HOM, HOD, HOS, HON, HOB};

/**
 * @brief Handle the connection of a Wayland interface to the global
 * Wayland registry.
 * @param registry The registry the object was connected to.
 * @param name The numerical name of the interface.
 * @param interface The interface name.
 * @param version The version of the interface connected.
 */
static void HRC(Iridium::Generic _data, Registry registry,
                Iridium::U32 name, const char *interface,
                Iridium::U32 version)
{
    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        Iridium::Logging::Log("Got Wayland compositor v" +
                                  std::to_string(version) + ".",
                              Iridium::Logging::success);
        // C++ whinges about these casts unless we explicitly turn each
        // void* into a type.
        compositor = (Compositor)wl_registry_bind(
            registry, name, &wl_compositor_interface, version);
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        Iridium::Logging::Log("Got XDG shell v" + std::to_string(version) +
                                  ".",
                              Iridium::Logging::success);
        window_manager = (WindowManager)wl_registry_bind(
            registry, name, &xdg_wm_base_interface, version);
        xdg_wm_base_add_listener(window_manager, &window_manager_listener,
                                 nullptr);
    }
    else if (strcmp(interface, wl_output_interface.name) == 0)
    {
        Iridium::Logging::Log("Got Wayland output v" +
                                  std::to_string(version) + ".",
                              Iridium::Logging::success);
        // We do not need this object in the global scope; it's
        // automatically destroyed within its callbacks.
        Output output = (Output)wl_registry_bind(
            registry, name, &wl_output_interface, version);
        wl_output_add_listener(output, &output_listener, nullptr);
    }
}

/**
 * @brief Handle the removal of a global object.
 * @noop We do not utilize this function.
 */
static void HRR(Iridium::Generic _data, Registry _registry,
                Iridium::U32 _name)
{}

/**
 * @brief The listener for the global Wayland registry. This handles the
 * connection/disconnection of objects.
 */
const RegistryListener registry_listener = {HRC, HRR};

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
        wl_registry_add_listener(registry, &registry_listener, nullptr);
        Sync();

        surface = wl_compositor_create_surface(compositor);

        wrapped_surface =
            xdg_wm_base_get_xdg_surface(window_manager, surface);
        xdg_surface_add_listener(wrapped_surface, &surface_listener,
                                 nullptr);

        window = xdg_surface_get_toplevel(wrapped_surface);
        xdg_toplevel_add_listener(window, &window_listener, nullptr);

        // Wait for all of our changes to properly take place.
        wl_surface_commit(surface);
        Sync();

        return true;
    }

    void Disconnect() noexcept
    {
        // Destroy XDG in ascending order, starting with the toplevel and
        // ending with the base interface.
        xdg_toplevel_destroy(window);
        xdg_surface_destroy(wrapped_surface);
        wl_surface_destroy(surface);
        xdg_wm_base_destroy(window_manager);

        wl_compositor_destroy(compositor);
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
    }

    void Sync() noexcept { wl_display_roundtrip(display); }

    const Display &GetDisplay() noexcept { return display; }

    const Surface &GetSurface() noexcept { return surface; }

    const Monitor &GetMonitor() noexcept { return monitor; }

    bool ShouldWindowClose() noexcept { return close; }

    void CloseWindow() noexcept { close = true; }

    void SetWindowTitle(const std::string &title) noexcept
    {
        xdg_toplevel_set_title(window, title.c_str());
        xdg_toplevel_set_app_id(window, title.c_str());
    }

    void IgnoreRestrictions(bool fullscreen, bool minimize) noexcept
    {
        panic_on_no_fullscreen = !fullscreen;
        panic_on_no_minimize = !minimize;
    }
}
