/**
 * @file Logging.hpp
 * @author Israfil Argos (israfiel-a)
 * @date 2024-12-22
 * @brief This file provides the Iridium logging interface. This includes
 * things like error reporting, debug logs, and more.
 *
 * Copyright (c) 2024 Israfil Argos
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */
#ifndef IRIDIUM_LOGGING_HPP
#define IRIDIUM_LOGGING_HPP

#include <iostream>

/**
 * @brief The Iridium logging interface. This provides all functionality to
 * do with logging messages, errors, warnings, etcetera.
 */
namespace Iridium::Logging
{
    /**
     * @brief An Iridium logging interface. This provides functionality for
     * connecting to files or other output streams and logging data in
     * various formats.
     */
    class Interface
    {
        protected:
            /**
             * @brief The output stream this interface is reporting to.
             * This can be a file, stdout, or anything else.
             */
            std::ostream &output;

        public:
            /**
             * @brief Create a new logging interface targeting the given
             * output stream. This object should not be deleted during the
             * lifetime of the interface, or errors shall be thrown.
             * @param output The output stream to log to.
             */
            Interface(std::ostream &output = std::cout);

            inline bool Closed() const { return output.fail(); }
    };
}

#endif // IRIDIUM_LOGGING_HPP
