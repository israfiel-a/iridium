#include <Ignition.hpp>

namespace Iridium
{
    bool Engine::Start(const std::string &config_path)
    {
        this->running = true;
        return true;
    }

    const Version &Engine::GetVersion() const { return this->version; }

    bool Engine::IsRunning() const { return this->running; }
}
