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
     * @brief Information about a monitor object. This includes all
     * information pertaining to rendering on a given monitor.
     */
    struct Monitor
    {
        public:
            /**
             * @brief The width of the monitor in pixels.
             */
            uint32_t width;
            /**
             * @brief The height of the monitor in pixels.
             */
            uint32_t height;
            /**
             * @brief The X-coordinate of the top left corner of the
             * monitor in screenspace. This is relevant because sometimes
             * monitors are offset by certain things like OS decoration.
             */
            uint32_t x;
            /**
             * @brief The Y-coordinate of the top left corner of the
             * monitor in screenspace. This is relevant because sometimes
             * monitors are offset by certain things like OS decoration.
             */
            uint32_t y;
            /**
             * @brief The refresh rate of the monitor in mHz.
             */
            uint16_t refresh_rate;
            /**
             * @brief The content scale of the monitor.
             */
            int16_t scale;
    };

    /**
     * @brief Convert millihertz (mHz) to megahertz (MHz).
     * @param mHz Millihertz to convert.
     * @return The megahertz representation of the passed value.
     */
    constexpr std::uint32_t mHzToMHz(std::uint32_t mHz) noexcept
    {
        return static_cast<std::uint32_t>(mHz / 1000000000);
    }

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

    const Monitor &GetMonitor() noexcept;
}

#endif // IRIDIUM_WAYLAND_HPP
