#include "Vulkan/Vulkan.hpp"
#include "Wayland/Wayland.hpp"
#include <Window.hpp>

namespace Iridium::Windowing
{
    Window::Window(const std::string &name)
    {
        Wayland::Connect();
        Wayland::SetWindowTitle(name);
        Vulkan::Connect();

        while (!Wayland::ShouldWindowClose())
        {
            Vulkan::Frame();
            Wayland::Sync();
        }
    }

    Window::~Window()
    {
        Vulkan::Disconnect();
        Wayland::Disconnect();
    }
}
