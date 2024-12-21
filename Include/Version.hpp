/**
 * @file Version.hpp
 * @author Israfil Argos (israfiel-a)
 * @date 2024-12-14
 * @brief This file contains the versioning framework of the engine.
 *
 * Copyright (c) 2024 Israfil Argos
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */
#ifndef IRIDIUM_VERSION_HPP
#define IRIDIUM_VERSION_HPP

// Fixed-width integers.
#include <cstdint>
// The C++ standard string class.
#include <string>

/**
 * @brief The Iridium namespace.
 */
namespace Iridium
{
    /**
     * @brief An enumerator specifying what cycle of release a version
     * object details.
     */
    enum ReleaseCycleIdentifier
    {
        /**
         * @brief The object details an object in pre-alpha. This means
         * bugs, horrific performance, the works.
         */
        pre_alpha,
        /**
         * @brief The object details an object in alpha. This means that,
         * depite the lack of proper continuity and probable bugs, it is
         * @em usable.
         */
        alpha,
        /**
         * @brief The object details an object in beta. This means the
         * object is nearing release. From here onward, the object should
         * be backwards-compatible to an extent.
         */
        beta,
        /**
         * @brief The object details an object in production. This should
         * be polished, performant, and @b documented.
         */
        stable
    };

    /**
     * @brief A class detailing the version of something, be it the engine,
     * an application, or something else.
     */
    class Version
    {
        private:
            /**
             * @brief The major identifier (X.x.x) of the version.
             */
            std::uint8_t major = 0;

            /**
             * @brief The minor identifier (x.X.x) of the version.
             */
            std::uint8_t minor = 0;

            /**
             * @brief The tweak identifier (x.x.X) of the version.
             */
            std::uint8_t tweak = 0;

            /**
             * @brief The release cycle identifier of the version.
             */
            ReleaseCycleIdentifier rcid = pre_alpha;

        public:
            /**
             * @brief Create a new version object.
             * @param major The major identifier of the version.
             * @param minor The minor identifier of the version.
             * @param tweak The tweak identifier of the version.
             * @param rcid The release cycle identifier of the version.
             */
            Version(uint8_t major, uint8_t minor, uint8_t tweak,
                    ReleaseCycleIdentifier rcid)
            {
                this->major = major;
                this->minor = minor;
                this->tweak = tweak;
                this->rcid = rcid;
            }

            /**
             * @brief Get the major version identifier.
             * @return The major version ID.
             */
            std::uint8_t GetMajor() const { return this->major; }

            /**
             * @brief Get the minor version identifier.
             * @return The minor version ID.
             */
            std::uint8_t GetMinor() const { return this->minor; }

            /**
             * @brief Get the tweak version identifier.
             * @return The tweak version ID.
             */
            std::uint8_t GetTweak() const { return this->tweak; }

            /**
             * @brief Get the release cycle identifier.
             * @return The release cycle identifier.
             */
            ReleaseCycleIdentifier GetRCID() const { return this->rcid; }

            /**
             * @brief Stringify the version.
             * @return The stringified version.
             */
            std::string Stringify() const;
    };
}

#endif // IRIDIUM_VERSION_HPP
