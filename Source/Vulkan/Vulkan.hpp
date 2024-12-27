#ifndef IRIDIUM_VULKAN_HPP
#define IRIDIUM_VULKAN_HPP

#include <cstdint>
//! remove
#include "../Wayland/_wl.h"

namespace Iridium::Vulkan
{
    bool Connect(wl_display *display, wl_surface *surface);
    void Disconnect();

    void StartSwapchain(std::uint32_t width, std::uint32_t height);
    void EndSwapchain();

    void WaitForIdle();
    void Frame(std::uint32_t width, std::uint32_t height);
}

#endif // IRIDIUM_VULKAN_HPP
