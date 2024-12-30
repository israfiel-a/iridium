#ifndef IRIDIUM_VULKAN_HPP
#define IRIDIUM_VULKAN_HPP

#include <string>

namespace Iridium::Vulkan
{
    bool Connect(const std::string &application_name);
    void Disconnect();

    void StartSwapchain();
    void EndSwapchain();

    void WaitForIdle();
    void Frame();
}

#endif // IRIDIUM_VULKAN_HPP
