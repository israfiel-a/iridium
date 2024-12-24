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
#include <source_location>
#include <string>

/**
 * @brief The Iridium logging interface. This provides all functionality to
 * do with logging messages, errors, warnings, etcetera.
 */
namespace Iridium::Logging
{
    /**
     * @brief All possible engine error codes.
     */
    enum ErrorCode
    {
        /**
         * @brief An object that shouldn't have been NULL or empty was.
         */
        failed_null_assertion,
        /**
         * @brief An object passed to the method was in some way bad.
         * This should be accompanied by extra context explaining what
         * was wrong with the object.
         */
        bad_parameter,
        /**
         * @brief The destination of some data was too small to fit said
         * data.
         */
        destination_too_small
    };

    /**
     * @brief Loggable severities.
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
         * @brief Believe it or not, something went @b right!
         */
        success,
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
     * @brief A loggable item. This can be passed to any of the logging
     * functions and produce a formatted result.
     */
    class Loggable
    {
        protected:
            /**
             * @brief The body of the loggable; this is what's actually
             * being logged.
             */
            std::string body;

            /**
             * @brief The severity of the loggable. This will decide what
             * stream it goes to.
             */
            Severity severity;

        public:
            /**
             * @brief Create a new loggable using just a string.
             * @param body The value to be logged.
             * @param severity The severity of the log. This determines
             * what stream it'll be sent to.
             * @param location The location from which the log request
             * came.
             */
            Loggable(const std::string &body, Severity severity = log,
                     const Location &location =
                         std::source_location::current());

            /**
             * @brief Create a new loggable using just a string.
             * @param body The value to be logged.
             * @param severity The severity of the log. This determines
             * what stream it'll be sent to.
             * @param location The location from which the log request
             * came.
             */
            Loggable(const char *body, Severity severity = log,
                     const Location &location =
                         std::source_location::current());

            /**
             * @brief Get the body of the loggable; what's actually going
             * to be logged. This includes raw ANSI escape codes, newline
             * characters, etcetera.
             * @return The body of the loggable.
             */
            inline const std::string &GetBody() const noexcept
            {
                return body;
            }

            /**
             * @brief Get the loggable's severity.
             * @return The severity of the loggable.
             */
            inline Severity GetSeverity() const noexcept
            {
                return severity;
            }
    };

    /**
     * @brief A loggable specifically having to do with an error/warning.
     */
    class Error : public Loggable
    {
        private:
            /**
             * @brief The code thrown.
             */
            ErrorCode code;

        public:
            /**
             * @brief Create a new error structure.
             * @param code The code of the error.
             * @param severity The severity of the error. Should this be
             * "success", the severity will be evaluated based off of the
             * error code.
             * @param context Any extra context. This is copied into the
             * structure.
             * @param location The location the error was thrown from. This
             * is copied into the structure.
             */
            Error(ErrorCode code, Severity severity = infer,
                  Context context = "",
                  Location location = std::source_location::current());

            /**
             * @brief Get the error code.
             * @return The error code.
             */
            inline ErrorCode GetCode() const noexcept { return code; }
    };

    /**
     * @brief The exception triggered when a fatal error/engine panic is
     * raised.
     */
    class PanicException : public std::exception
    {
        public:
            /**
             * @brief Create a new fatal exception.
             */
            PanicException() = default;

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
     * @brief Set the interface for which to output general messages.
     * Should the warning or error output not be set, logged warnings or
     * errors will fall back to this object.
     * @param output The output object. This cannot be null.
     * @return true The operation succeeded.
     * @return false The operation failed. Error code logged to the error
     * stack.
     *
     * @warning This function can throw both a null_assertion_failed and
     * bad_parameter error should it be provided faulty data. "Faulty" is
     * characterized by an output stream with badbit set or a straight null
     * input.
     */
    bool SetGeneralOutput(std::ostream *output = &std::cout);

    /**
     * @brief Set the interface for which to output warning messages. By
     * default, this is the general output. To reset it to the general
     * output, pass nullptr to this function.
     * @param output The output object.
     * @return true The operation succeeded.
     * @return false The operation failed. Error code logged to the error
     * stack.
     */
    void SetWarningOutput(std::ostream *output) noexcept;

    /**
     * @brief Set the interface for which to output error messages. By
     * default, this is the general output. To reset it to the general
     * output, pass nullptr to this function.
     * @param output The output object.
     * @return true The operation succeeded.
     * @return false The operation failed. Error code logged to the error
     * stack.
     */
    void SetErrorOutput(std::ostream *output) noexcept;

    /**
     * @brief Log a loggable into its proper output.
     * @param loggable The loggable to log.
     */
    void Log(const Loggable &loggable);

    /**
     * @brief Log the last error on the error stack.
     *
     * @warning Should the error stack have a size of 0, this will log
     * nothing and raise the destination_too_small warning.
     */
    void Log();

    /**
     * @brief Push an error onto the error stack. This will log the given
     * error should suppression not currently be on. Should the pushed
     * error be fatal, a FatalException will be thrown.
     * @param code The error. This is copied into the error stack.
     */
    void RaiseError(Error error);

    /**
     * @brief Push an error onto the error stack. This will log the given
     * error should suppression not currently be on. Should the pushed
     * error be fatal, a FatalException will be thrown.
     * @param code The error.
     * @param severity The severity of the error.
     * @param context Any extra context to be provided for the error.
     * @param location The source code location from which the error was
     * raised.
     */
    void RaiseError(ErrorCode code, Severity severity = infer,
                    Context context = "",
                    const Location &location = Location::current());

    /**
     * @brief Pull the last error from the error stack and return it. This
     * is useful for error handling functions that don't want to let
     * higher-level functions see an error's been hit.
     * @return The last-thrown error code.
     */
    Error PullError() noexcept;

    /**
     * @brief Get the last error from the error stack and return it. This
     * does not change the error stack, unlike Pull.
     * @return The last-thrown error code.
     *
     * @warning Should the error stack be empty this will fail the thread
     * with an std::out_of_range exception.
     */
    const Error &GetError();

    /**
     * @brief Get the error at the specified index from the error stack and
     * return it. This does not change the error stack, unlike Pull.
     * @param index The index of the error in the array.
     * @return The error retrieved.
     *
     * @warning Should the error stack be empty or should the index be out
     * of bounds for the error stack, this will fail the thread with an
     * std::out_of_range exception.
     */
    const Error &GetError(std::size_t index);

    /**
     * @brief Toggle the suppression of error logging. This does not
     * prevent them from being raised, simply from being logged. You cannot
     * suppress an engine panic from logging.
     * @return true Errors are currently being suppressed.
     * @return false Errors are not currently being suppressed.
     */
    bool SuppressErrors();
}

#endif // IRIDIUM_LOGGING_HPP
