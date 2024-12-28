#ifndef IRIDIUM_VULKAN_HPP
#define IRIDIUM_VULKAN_HPP

namespace Iridium::Vulkan
{
    bool Connect();
    void Disconnect();

    void StartSwapchain();
    void EndSwapchain();

    void WaitForIdle();
    void Frame();
}

#endif // IRIDIUM_VULKAN_HPP
