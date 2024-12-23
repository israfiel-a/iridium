#ifndef IRIDIUM_WINDOW_HPP
#define IRIDIUM_WINDOW_HPP

#include <string>

namespace Iridium
{
    class Window
    {
        public:
            /**
             * @brief Create a window.
             * @param title The title of the window.
             */
            Window(const std::string &title);

            /**
             * @brief Disconnect Wayland resources and close the window
             * properly.
             */
            ~Window();
    };
}

#endif // IRIDIUM_WINDOW_HPP
