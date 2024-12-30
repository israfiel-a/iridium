#include "Private/Vulkan.hpp"
#include "Private/Wayland.hpp"
#include <Window.hpp>

namespace Iridium::Windowing
{
    Window::Window(const std::string &name)
    {
        if (!Wayland::Connect()) exit(255);
        // Wayland::SetWindowTitle(name);
        Vulkan::Connect(name);

        while (!Wayland::ShouldWindowClose())
        {
            Vulkan::Frame();
            // Sync our display every frame to prevent screen tearing.
            Wayland::Sync();
        }
    }

    Window::~Window()
    {
        Vulkan::Disconnect();
        Wayland::Disconnect();
    }
}
