#include "Vulkan/Vulkan.hpp"
#include "Wayland/Wayland.hpp"
#include <Window.hpp>

namespace Iridium::Windowing
{
    Window::Window(const std::string &name)
    {
        Wayland::Connect();
        Vulkan::Connect();
    }

    Window::~Window()
    {
        Wayland::Disconnect();
        Vulkan::Disconnect();
    }
}
