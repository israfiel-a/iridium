#include "Vulkan/Vulkan.hpp"
#include "Wayland/Wayland.hpp"
#include <Window.hpp>

namespace Iridium::Windowing
{
    Window::Window(const std::string &name)
    {
        Wayland::Connect();
        Vulkan::Connect(Wayland::GetDisplay(), Wayland::GetSurface());
        Vulkan::StartSwapchain(500, 500);

        while (!Wayland::ShouldWindowClose())
        {
            Wayland::ResizeWindow();
            Vulkan::Frame(Wayland::GetWidth(), Wayland::GetHeight());
            Wayland::Sync();
        }
    }

    Window::~Window()
    {
        Vulkan::Disconnect();
        Wayland::Disconnect();
    }
}
