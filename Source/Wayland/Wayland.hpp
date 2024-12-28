/**
 * @file Wayland.hpp
 * @author Israfil Argos (israfiel-a)
 * @date 2024-12-27
 * @brief This file provides the Iridium Wayland client, which provides
 * functionality for creating windows under Wayland Linux.
 *
 * Copyright (c) 2024 Israfil Argos
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */
#ifndef IRIDIUM_WAYLAND_HPP
#define IRIDIUM_WAYLAND_HPP

#include <Types.hpp>
#include <string>

/**
 * @brief The Wayland display interface.
 */
using Display = struct wl_display *;

/**
 * @brief The Wayland raw surface interface.
 */
using Surface = struct wl_surface *;

/**
 * @brief The Iridium Wayland namespace. This provides proper functionality
 * for Wayland protocols within Iridium.
 */
namespace Iridium::Windowing::Wayland
{
    /**
     * @brief Information about a monitor object. This includes all
     * information pertaining to rendering on a given monitor.
     */
    struct Monitor
    {
        public:
            /**
             * @brief The width of the monitor in pixels.
             */
            Iridium::U32 width;
            /**
             * @brief The height of the monitor in pixels.
             */
            Iridium::U32 height;
            /**
             * @brief The refresh rate of the monitor in Hz.
             */
            Iridium::U16 refresh_rate;
            /**
             * @brief The content scale of the monitor.
             */
            Iridium::U16 scale;
    };

    /**
     * @brief Convert millihertz (mHz) to hertz (Hz).
     * @param mHz Millihertz to convert.
     * @return The hertz representation of the passed value.
     */
    inline constexpr Iridium::U16 mHzToHz(Iridium::U32 mHz) noexcept
    {
        return static_cast<Iridium::U16>(mHz / 1000);
    }

    /**
     * @brief Connect the Wayland server to our program. This blocks to
     * allow registry objects to connect.
     * @return true Connection was successful.
     * @return false Connection failed. Error reason was logged.
     */
    bool Connect();

    /**
     * @brief Disconnect the Wayland server and dispose of all data.
     */
    void Disconnect() noexcept;

    /**
     * @brief Wait for the display server to sync up with our application.
     * This is blocking.
     */
    void Sync() noexcept;

    /**
     * @brief Get the private Wayland display object.
     * @return The Wayland display object.
     */
    const Display &GetDisplay() noexcept;

    /**
     * @brief Get the private Wayland surface object.
     * @return The Wayland surface object.
     */
    const Surface &GetSurface() noexcept;

    /**
     * @brief Get the private monitor object. This stores all information
     * about the monitor we're rendering on.
     * @return The monitor.
     */
    const Monitor &GetMonitor() noexcept;

    /**
     * @brief Get a boolean representing whether or not the window should
     * close.
     * @return true The window should close.
     * @return false The window should continue running.
     */
    bool ShouldWindowClose() noexcept;

    /**
     * @brief Force the window to begin closing.
     */
    void CloseWindow() noexcept;

    /**
     * @brief Set the title of the window.
     * @param title The new title of the window.
     */
    void SetWindowTitle(const std::string &title) noexcept;

    /**
     * @brief Set a flag to ignore restrictions on the window manager
     * (fullscreen capabilities, etc.) and run the game. This will likely
     * cause some minor display issues as the engine tries its best to
     * rectify these issues.
     * @param fullscreen Ignore fullscreen capabilities.
     * @param minimize Ignore minimize capabilities.
     */
    void IgnoreRestrictions(bool fullscreen,
                            bool minimize = true) noexcept;
}

#endif // IRIDIUM_WAYLAND_HPP
