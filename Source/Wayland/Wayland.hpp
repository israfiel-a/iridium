#ifndef IRIDIUM_WAYLAND_WAYLAND_HPP
#define IRIDIUM_WAYLAND_WAYLAND_HPP

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
}

#endif // IRIDIUM_WAYLAND_WAYLAND_HPP
