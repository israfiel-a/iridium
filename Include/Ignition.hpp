/**
 * @file Ignition.hpp
 * @author Israfil Argos (israfiel-a)
 * @date 2024-12-14
 * @brief The "ignition" file of the engine. This includes the most central
 * functions to engine functionality, like starting and ending the runtime.
 *
 * Copyright (c) 2024 Israfil Argos
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */
#ifndef IRIDIUM_IGNITION_HPP
#define IRIDIUM_IGNITION_HPP

// Versioning information.
#include "Version.hpp"

/**
 * @brief The Iridium namespace.
 */
namespace Iridium
{
    /**
     * @brief The Iridium engine class. This contains central data about
     * the Iridium runtime.
     */
    class Engine
    {
        private:
            /**
             * @brief The version of the engine currently running.
             */
            Version version;

            /**
             * @brief A boolean value that represents whether or not the
             * engine is currently running its loops.
             */
            bool running = false;

            /**
             * @brief Construct the engine.
             */
            Engine() : version(1, 0, 1, pre_alpha) {}

        public:
            // Prevent copies of the engine class from appearing by
            // deleting the functions required to do so.
            Engine(Engine const &) = delete;
            void operator=(Engine const &) = delete;

            /**
             * @brief Get the engine object.
             * @return The engine.
             */
            static Engine &Get()
            {
                static Engine instance;
                return instance;
            }

            /**
             * @brief Start the engine.
             * @param config_path The path to the engine configuration
             * file. This is also known as the application description.
             * @return true If starting the engine succeeded.
             * @return false If starting the engine failed.
             */
            bool Start(const std::string &config_path);

            /**
             * @brief Get the engine's version object.
             * @return The engine's version.
             */
            const Version &GetVersion() const;

            /**
             * @brief Get whether or not the engine is running.
             * @return true If the engine is running.
             * @return false If the engine is stopped.
             */
            bool IsRunning() const;
    };
}

#endif // IRIDIUM_IGNITION_HPP
