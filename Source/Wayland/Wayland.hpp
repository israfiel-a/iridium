#ifndef IRIDIUM_WAYLAND_HPP
#define IRIDIUM_WAYLAND_HPP

#include <cstdint>

using Display = struct wl_display *;
using Surface = struct wl_surface *;

/**
 * @brief The Iridium Wayland namespace. This provides proper functionality
 * for Wayland protocols within Iridium.
 */
namespace Iridium::Windowing::Wayland
{
    /**
     * @brief Connect the Wayland server to our program. This blocks to
     * allow registry objects to connect.
     * @return true Connection was successful.
     * @return false Connection failed. Error reason was pushed to the
     * error interface.
     */
    bool Connect();

    /**
     * @brief Disconnect the Wayland server and dispose of all data.
     */
    void Disconnect();

    Display GetDisplay();
    Surface GetSurface();
    bool ShouldWindowClose();
    void ResizeWindow();
    void Sync();

    uint32_t GetWidth();
    uint32_t GetHeight();
}

#endif // IRIDIUM_WAYLAND_HPP
