#include <Version.hpp>
#include <format>

namespace Iridium
{
    /**
     * @brief An array of the string representations of the release cycle
     * identifiers provided in Ignition.hpp.
     */
    const char *release_cycle_strings[4] = {"pre_alpha", "alpha", "beta",
                                            "stable"};

    std::string Version::Stringify() const
    {
        // Create a static variable so we don't have to re-calculate this
        // during every operation.
        static std::string version_string =
            std::format("{}.{}.{}-{}", this->major, this->minor,
                        this->tweak, release_cycle_strings[this->rcid]);
        return version_string;
    }
}
