/**
 * @file Errors.hpp
 * @author Israfil Argos (israfiel-a)
 * @date 2024-12-22
 * @brief This file provides the Iridium error handling interface. This
 * provides a layer of abstraction over instant-failure exception throwing.
 *
 * Copyright (c) 2024 Israfil Argos
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */
#ifndef IRIDIUM_ERRORS_HPP
#define IRIDIUM_ERRORS_HPP

#include <exception>
#include <source_location>
#include <string>

/**
 * @brief The Iridium error namespace. This includes error codes, error
 * handlers, and more.
 */
namespace Iridium::Errors
{
    /**
     * @brief Information about an error.
     */
    namespace Information
    {
        /**
         * @brief All possible engine error codes.
         */
        enum Code
        {
            /**
             * @brief An object that shouldn't have been NULL or empty was.
             */
            failed_null_assertion
        };

        /**
         * @brief Error severities.
         */
        enum Severity
        {
            /**
             * @brief A severity purely representing "infer the severity of
             * this error". This is the default for all error pushing
             * functions, but is not returned from any getter functions.
             */
            infer,
            /**
             * @brief Just a log. Not worth paying any serious attention
             * to.
             */
            log,
            /**
             * @brief A warning. Developers should probably give special
             * attention to this. Functions that throw warnings don't have
             * to document them.
             */
            warning,
            /**
             * @brief A full error. This should be of special note to
             * developers and users alike. Functions that can throw errors
             * should document it unless it's a continuation of a
             * lower-level failure.
             */
            error,
            /**
             * @brief A fatal error. This kills the process, and should be
             * by far the most serious issue to happen in a program.
             * Functions that could possibly throw panics must always
             * document it.
             */
            panic
        };

        /**
         * @brief The location from which an error was thrown.
         */
        using Location = std::source_location;

        /**
         * @brief Extra context about an error.
         */
        using Context = std::string;

        /**
         * @brief The string representation of the failed_null_assertion
         * error code.
         */
        constexpr static std::string_view fna_string =
            "failed_null_assertion";

        /**
         * @brief The default severity of the failed_null_assertion error
         * code.
         */
        constexpr static Severity fna_severity = error;

        /**
         * @brief Get the string representation of the given error code.
         * @param code The error code to stringify.
         * @return The string representation of the code.
         */
        const std::string_view &GetCodeString(Code code) noexcept;

        /**
         * @brief Get the default severity of the given error code.
         * @param code The error code.
         * @return The default severity of the code.
         */
        Severity GetCodeSeverity(Code code) noexcept;
    }

    /**
     * @brief An error object. Contains basic information about where the
     * error came from, what exactly the error was, etcetera.
     */
    class Error
    {
        private:
            /**
             * @brief The code thrown.
             */
            Information::Code code;
            /**
             * @brief The error's severity.
             */
            Information::Severity severity;
            /**
             * @brief The location at which the error was thrown.
             */
            Information::Location location;

            /**
             * @brief Any extra data to be provided.
             */
            Information::Context context;

        public:
            /**
             * @brief Create a new error structure.
             * @param code The code of the error.
             * @param severity The severity of the error.
             * @param location The location the error was thrown from. This
             * is copied into the structure.
             * @param context Any extra context. This is copied into the
             * structure.
             */
            Error(Information::Code code, Information::Severity severity,
                  Information::Location location,
                  Information::Context context = "")
                : code(code), severity(severity), location(location),
                  context(context)
            {
                if (severity == Information::infer)
                    severity = GetCodeSeverity(code);
            }

            /**
             * @brief Get the error code.
             * @return The error code.
             */
            inline Information::Code GetCode() const noexcept
            {
                return code;
            }

            /**
             * @brief Get the severity of the error.
             * @return The severity of the error.
             */
            inline Information::Severity GetSeverity() const noexcept
            {
                return severity;
            }

            /**
             * @brief Get the location of the error.
             * @return The location of the error.
             */
            inline const Information::Location &
            GetLocation() const noexcept
            {
                return location;
            }

            /**
             * @brief Get extra context provided by the error's thrower.
             * @return The extra context.
             */
            inline const Information::Context &GetContext() const noexcept
            {
                return context;
            }

            /**
             * @brief Get a string representation of the error. This does
             * not include extra context provided.
             * @return A string represenation of the error.
             */
            std::string Stringify() const;
    };

    /**
     * @brief An exception triggered when a fatal error is raised. This
     * triggers the engine's FailFunction and exits the thread.
     */
    class PanicException : public std::exception
    {
        public:
            /**
             * @brief Create a new fatal exception.
             * @param raised The fatal error that was raised.
             */
            PanicException(const Error &raised)
            {
                //! This should report to the current error output.
            }

            /**
             * @brief Get an explanation of what this exception is.
             * @return The string "An engine panic was raised, check exit
             * logs."
             */
            const char *what() const throw()
            {
                return "An engine panic was raised, check exit logs.";
            }
    };

    /**
     * @brief Push an error onto the error stack. Should the pushed error
     * be fatal, a FatalException will be thrown.
     * @param code The error. This is copied into the error stack.
     */
    void Push(Error error);

    /**
     * @brief Push an error onto the error stack. Should the pushed error
     * be fatal, a FatalException will be thrown.
     * @param code The error.
     * @param severity The severity of the error.
     * @param context Any extra context to be provided for the error.
     * @param location The source code location from which the error was
     * raised.
     */
    void Push(Information::Code code,
              Information::Severity severity = Information::infer,
              Information::Context context = "",
              const Information::Location &location =
                  std::source_location::current());

    /**
     * @brief Pull the last error from the error stack and return it. This
     * is useful for error handling functions that don't want to let
     * higher-level functions see an error's been hit.
     * @return The last-thrown error code.
     */
    Error Pull() noexcept;

    /**
     * @brief Get the last error from the error stack and return it. This
     * does not change the error stack, unlike Pull.
     * @return The last-thrown error code.
     */
    const Error &Get() noexcept;

    /**
     * @brief Get the error at the specified index from the error stack and
     * return it. This does not change the error stack, unlike Pull. Should
     * the index be out of bounds of the error stack, this will @b fail the
     * thread with an @exception std::out_of_range.
     * @param index The index of the error in the array.
     * @return The error retrieved.
     */
    const Error &Get(std::size_t index);
}

#endif // IRIDIUM_ERRORS_HPP
