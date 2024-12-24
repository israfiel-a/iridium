#include "Wayland/Wayland.hpp"
#include <Window.hpp>

namespace Iridium::Windowing
{
    Window::Window(const std::string &name) { Wayland::Connect(); }

    Window::~Window() { Wayland::Disconnect(); }
}
