/**
 * @file Version.cpp
 * @author Israfil Argos (israfiel-a)
 * @date 2024-12-14
 * @brief This file contains the implementation for the versioning
 * framework of the Iridium engine.
 *
 * Copyright (c) 2024 Israfil Argos
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */

#include <Version.hpp>
#include <format>

/**
 * @brief An array of the string representations of the release cycle
 * identifiers provided in Ignition.hpp.
 */
constexpr static std::string_view release_cycle_strings[4] = {
    "pre_alpha", "alpha", "beta", "stable"};

namespace Iridium
{
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
