#include <Logging.hpp>
#include <filesystem>
#include <format>
#include <map>
#include <vector>

/**
 * @brief The application's error stack. This contains up to 15 of the most
 * recent errors thrown.
 */
static std::vector<Iridium::Logging::Error> error_stack;

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
        Iridium::Logging::Severity severity;
};

const static std::map<Iridium::Logging::ErrorCode, ErrorInfo>
    error_information = {
        std::make_pair(
            Iridium::Logging::failed_null_assertion,
            ErrorInfo("failed_null_assertion", Iridium::Logging::error)),
        std::make_pair(
            Iridium::Logging::bad_parameter,
            ErrorInfo("bad_parameter", Iridium::Logging::error)),
        std::make_pair(Iridium::Logging::destination_too_small,
                       ErrorInfo("destination_too_small",
                                 Iridium::Logging::warning))};

static bool suppress_errors = false;

static std::ostream *general_output = &std::cout;

static std::ostream *warning_output = general_output;

static std::ostream *error_output = general_output;

namespace Iridium::Logging
{
    Loggable::Loggable(const std::string &body_string, Severity severity,
                       const Location &location)
        : severity(severity)
    {
        body = std::format(
            "{}{} @ ln.{} cl.{}, \"{}\" - {}",
            std::filesystem::path(location.file_name()).stem().string(),
            std::filesystem::path(location.file_name())
                .extension()
                .string(),
            std::to_string(location.line()),
            std::to_string(location.column()), location.function_name(),
            body_string);
    }

    Loggable::Loggable(const char *body_string, Severity severity,
                       const Location &location)
        : severity(severity)
    {
        body = std::format(
            "{}{} @ ln.{} cl.{}, \"{}\" - {}",
            std::filesystem::path(location.file_name()).stem().string(),
            std::filesystem::path(location.file_name())
                .extension()
                .string(),
            std::to_string(location.line()),
            std::to_string(location.column()), location.function_name(),
            body_string);
    }

    Error::Error(ErrorCode code, Severity severity, Context context,
                 Location location)
        : Loggable("", severity, location), code(code)
    {
        const ErrorInfo &code_information = error_information.at(code);

        if (severity == infer || severity == success)
            this->severity = code_information.severity;
        body = std::format(
            "{}{} @ ln.{} cl.{}, \"{}\" - {}, {}",
            std::filesystem::path(location.file_name()).stem().string(),
            std::filesystem::path(location.file_name())
                .extension()
                .string(),
            std::to_string(location.line()),
            std::to_string(location.column()), location.function_name(),
            code_information.name, std::to_string(this->severity));
        if (!context.empty()) body += "\n\tContext: " + context;
    }

    bool SetGeneralOutput(std::ostream *output)
    {
        if (output == nullptr)
        {
            RaiseError(failed_null_assertion);
            return false;
        }

        if (!output->good())
        {
            RaiseError(bad_parameter, infer,
                       "output stream had badbit set");
            return false;
        }

        general_output = output;
        return true;
    }

    void SetWarningOutput(std::ostream *output) noexcept
    {
        if (output != nullptr) warning_output = output;
        else warning_output = general_output;
    }

    void SetErrorOutput(std::ostream *output) noexcept
    {
        if (output != nullptr) error_output = output;
        else error_output = general_output;
    }

    void Log(const Loggable &loggable) noexcept
    {
        switch (loggable.GetSeverity())
        {
            case infer:
            case log:
                (*general_output) << loggable.GetBody() << "\n";
                break;
            case success:
                (*general_output)
                    << "\033[32m" << loggable.GetBody() << "\033[0m\n";
                break;
            case warning:
                (*warning_output)
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
        }
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
        // If we've reached the stack size limit, simply remove the oldest
        // error from the stack.
        if (error_stack.size() == 15)
            error_stack.erase(error_stack.begin());
        error_stack.push_back(error);

        if (!suppress_errors || error.GetSeverity() == panic) Log();

        if (error.GetSeverity() == panic) throw PanicException();
    }

    void RaiseError(ErrorCode code, Severity severity, Context context,
                    const Location &location)
    {
        // If we've reached the stack size limit, simply remove the oldest
        // error from the stack.
        if (error_stack.size() == 15)
            error_stack.erase(error_stack.begin());
        Error constructed_error(code, severity, context, location);
        error_stack.push_back(constructed_error);

        if (!suppress_errors || constructed_error.GetSeverity() == panic)
            Log();

        if (constructed_error.GetSeverity() == panic)
            throw PanicException();
    }

    Error PullError()
    {
        if (error_stack.empty())
            throw std::out_of_range("error stack empty");

        Error last_error = error_stack.back();
        error_stack.pop_back();
        return last_error;
    }

    const Error &GetError()
    {
        if (error_stack.empty())
            throw std::out_of_range("error stack empty");
        return error_stack.back();
    }

    const Error &GetError(std::size_t index)
    {
        if (error_stack.empty() || index >= error_stack.size())
            throw std::out_of_range("out of error stack boundaries");
        return error_stack.at(index);
    }

    bool SuppressErrors() noexcept
    {
        suppress_errors = !suppress_errors;
        return suppress_errors;
    }
}
