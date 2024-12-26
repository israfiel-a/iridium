/**
 * @file Logging.cpp
 * @author Israfil Argos (israfiel-a)
 * @date 2024-12-22
 * @brief This file provides the implementation of the Iridium logging
 * interface.
 *
 * Copyright (c) 2024 Israfil Argos
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */

#include <Logging.hpp>
#include <filesystem>
#include <format>
#include <map>
#include <vector>

// Syntax sugar for the static, outside-of-namespace stuff.
using namespace Iridium::Logging;

/**
 * @brief Information about an error code like its string name and
 * severity.
 */
struct ErrorInfo
{
    public:
        /**
         * @brief The string name of the error code.
         */
        std::string name;
        /**
         * @brief The severity of the error code.
         */
        Severity severity;
};

/**
 * @brief A map of the various error codes and their information.
 */
const static std::map<ErrorCode, ErrorInfo> error_information = {
    std::make_pair(failed_null_assertion,
                   ErrorInfo("failed_null_assertion", error)),
    std::make_pair(bad_parameter, ErrorInfo("bad_parameter", error)),
    std::make_pair(destination_too_small,
                   ErrorInfo("destination_too_small", warning)),
    std::make_pair(file_open_failed, ErrorInfo("file_open_failed", error)),
    std::make_pair(wayland_connection_failed,
                   ErrorInfo("wayland_connection_failed", error))};

/**
 * @brief The application's error stack. This contains up to 10 of the most
 * recent errors thrown.
 */
static std::vector<Iridium::Logging::Error> error_stack;

/**
 * @brief A boolean flag representing whether or not we should suppress the
 * logging of errors, sans panics.
 */
static bool suppress_errors = false;

/**
 * @brief The output stream for general things; logs and successes. This is
 * also the default output for warnings, errors, and panics.
 */
static Output *general_output = &std::cout;

/**
 * @brief The output for errors/warnings. This is the general output by
 * default.
 */
static Output *error_output = general_output;

/**
 * @brief Cull the error stack. This simply removes the oldest/first
 * element in the stack should it be at the proper threshold.
 */
static void CullStack() noexcept
{
    if (error_stack.size() >= 10) error_stack.erase(error_stack.begin());
}

/**
 * @brief An inefficient function to translate the full function signature
 * provided by std::source_location into just the function name,
 * @param function The full function signature as returned by
 * std::source_location::function_name.
 * @return The name of the function passed.
 */
static std::string PrettyToUsableFunction(const char *function)
{
    std::string cleaned_function = function;

    const std::size_t name_beginning =
        cleaned_function.find_first_of(' ') + 1;
    const std::size_t name_end = cleaned_function.find_first_of('(');
    if (name_beginning == std::string::npos ||
        name_end == std::string::npos)
        return cleaned_function;

    return cleaned_function.substr(name_beginning,
                                   name_end - name_beginning);
}

/**
 * @brief Format the body string of a loggable object. The resulting object
 * will have the format of `[file_name] @ ln.[line_count]
 * cl.[column_count], "[function_signature]" - ` Note the space at the end.
 * @param severity The severity of the loggable.
 * @param location The location the loggable was initialized from.
 * @return A string representing the loggable.
 */
static std::string FormatLoggable(Severity severity,
                                  const Location &location)
{
    return std::format(
        "{}{} @ ln.{} cl.{}, {} - ",
        std::filesystem::path(location.file_name()).stem().string(),
        std::filesystem::path(location.file_name()).extension().string(),
        std::to_string(location.line()), std::to_string(location.column()),
        PrettyToUsableFunction(location.function_name()));
}

namespace Iridium::Logging
{
    Loggable::Loggable(const std::string &body_string, Severity severity,
                       const Location &location)
        : body(FormatLoggable(severity, location) + body_string),
          severity(severity)
    {
    }

    Error::Error(ErrorCode code, Severity severity_value, Context context,
                 const Location &location)
        : Loggable("", severity_value, location), code(code)
    {
        const ErrorInfo &code_information = error_information.at(code);
        if (severity_value == infer || severity_value == success)
            severity = code_information.severity;

        body += code_information.name + ", " + std::to_string(severity);
        if (!context.empty()) body += "\n\tContext: " + context;
    }

    bool SetGeneralOutput(Output *output) noexcept
    {
        if (output == nullptr)
        {
            RaiseError(failed_null_assertion);
            return false;
        }

        if (!output->good())
        {
            RaiseError(bad_parameter, infer,
                       "output stream has badbit set");
            return false;
        }

        general_output = output;
        return true;
    }

    void SetErrorOutput(Output *output) noexcept
    {
        if (output != nullptr) error_output = output;
        else error_output = general_output;
    }

    void Log(const Loggable &loggable) noexcept
    {
        switch (loggable.GetSeverity())
        {
            case success:
                (*general_output)
                    << "\033[32m" << loggable.GetBody() << "\033[0m\n";
                break;
            case warning:
                (*error_output)
                    << "\033[33m" << loggable.GetBody() << "\033[0m\n";
                break;
            case error:
                (*error_output)
                    << "\033[31m" << loggable.GetBody() << "\033[0m\n";
                break;
            case panic:
                (*error_output)
                    << "\033[31;1m" << loggable.GetBody() << "\033[0m\n";
                break;
            default:
                (*general_output) << loggable.GetBody() << "\n";
                break;
        }
    }

    void Log(const std::string &string, Severity severity,
             const Location &location) noexcept
    {
        Log(Loggable(string, severity, location));
    }

    void Log()
    {
        if (error_stack.empty())
        {
            RaiseError(destination_too_small);
            return;
        }

        const Error &last_error = GetError();
        Log(last_error);
    }

    void RaiseError(Error error)
    {
        CullStack();
        error_stack.push_back(error);

        if (!suppress_errors || error.GetSeverity() == panic) Log();
        if (error.GetSeverity() == panic) throw PanicException();
    }

    void RaiseError(ErrorCode code, Severity severity, Context context,
                    const Location &location)
    {
        CullStack();
        Error constructed_error(code, severity, context, location);
        error_stack.push_back(constructed_error);

        if (!suppress_errors || constructed_error.GetSeverity() == panic)
            Log();
        if (constructed_error.GetSeverity() == panic)
            throw PanicException();
    }

    Error PullError() noexcept
    {
        if (error_stack.empty()) RaiseError(destination_too_small);
        Error last_error = error_stack.back();
        error_stack.pop_back();
        return last_error;
    }

    const Error &GetError() noexcept
    {
        if (error_stack.empty()) RaiseError(destination_too_small);
        return error_stack.back();
    }

    const Error &GetError(std::size_t index) noexcept
    {
        if (error_stack.empty()) RaiseError(destination_too_small);
        return error_stack.at(index);
    }

    bool SuppressErrors() noexcept
    {
        suppress_errors = !suppress_errors;
        return suppress_errors;
    }
}
