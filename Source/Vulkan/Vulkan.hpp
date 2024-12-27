#ifndef IRIDIUM_VULKAN_HPP
#define IRIDIUM_VULKAN_HPP

//! remove
#include "../Wayland/_wl.h"

namespace Iridium::Vulkan
{
    bool Connect(wl_display *display, wl_surface *surface);
    void Disconnect();

    void StartSwapchain();
    void EndSwapchain();

    void WaitForIdle();
    void Frame();
}

#endif // IRIDIUM_VULKAN_HPP
