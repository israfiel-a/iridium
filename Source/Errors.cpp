/**
 * @file Errors.cpp
 * @author Israfil Argos (israfiel-a)
 * @date 2024-12-22
 * @brief This file provides the implementation of Iridium's error stack
 * interface.
 *
 * Copyright (c) 2024 Israfil Argos
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */

#include <Errors.hpp>
#include <filesystem>
#include <format>
#include <vector>

/**
 * @brief The application's error stack. This contains up to 15 of the most
 * recent errors thrown.
 */
static std::vector<Iridium::Errors::Error> error_stack;

namespace Iridium::Errors
{
    namespace Information
    {
        const std::string_view &GetCodeString(Code code) noexcept
        {
            switch (code)
            {
                case failed_null_assertion: return fna_string;
            }
            // This can't ever happen, but unfortunately GCC complains.
            return fna_string;
        }

        Severity GetCodeSeverity(Code code) noexcept
        {
            switch (code)
            {
                case failed_null_assertion: return fna_severity;
            }
            // This can't ever happen, but unfortunately GCC complains.
            return fna_severity;
        }
    }

    std::string Error::Stringify() const
    {
        // Save the value statically as to not have to re-calculate
        // this every call.
        static std::string stringified_error = std::format(
            "{}{}: fn. \"{}\" ln.{} cl.{} - {}, {}",
            std::filesystem::path(location.file_name()).stem().string(),
            std::filesystem::path(location.file_name())
                .extension()
                .string(),
            location.function_name(), std::to_string(location.line()),
            std::to_string(location.column()),
            Information::GetCodeString(code), std::to_string(severity));
        return stringified_error;
    }

    void Push(Error error)
    {
        // If we've reached the stack size limit, simply remove the oldest
        // error from the stack.
        if (error_stack.size() == 15)
            error_stack.erase(error_stack.begin());
        error_stack.push_back(error);

        if (error.GetSeverity() == Information::panic)
            throw PanicException(error);
    }

    void Push(Information::Code code, Information::Severity severity,
              Information::Context context,
              const Information::Location &location)
    {
        // If we've reached the stack size limit, simply remove the oldest
        // error from the stack.
        if (error_stack.size() == 15)
            error_stack.erase(error_stack.begin());
        Error constructed_error(code, severity, location, context);
        error_stack.push_back(constructed_error);

        if (constructed_error.GetSeverity() == Information::panic)
            throw PanicException(constructed_error);
    }

    Error Pull() noexcept
    {
        Error last_error = error_stack.back();
        error_stack.pop_back();
        return last_error;
    }

    const Error &Get() noexcept { return error_stack.back(); }

    const Error &Get(std::size_t index) { return error_stack.at(index); }
}
