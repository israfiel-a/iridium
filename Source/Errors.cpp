#include <Errors.hpp>
#include <vector>

static std::vector<Iridium::Errors::Error> error_stack;

namespace CodeStrings
{
    constexpr static std::string_view failed_null_assertion =
        "failed_null_assertion";
}

namespace CodeSeverities
{
    constexpr static Iridium::Errors::Severity failed_null_assertion =
        Iridium::Errors::error;
}

namespace Iridium::Errors
{

    const std::string_view &GetCodeString(Code code) noexcept
    {
        switch (code)
        {
            case failed_null_assertion:
                return CodeStrings::failed_null_assertion;
        }
        // This can't ever happen, but unfortunately GCC complains.
        return CodeStrings::failed_null_assertion;
    }

    Severity GetCodeSeverity(Code code) noexcept
    {
        switch (code)
        {
            case failed_null_assertion:
                return CodeSeverities::failed_null_assertion;
        }
        // This can't ever happen, but unfortunately GCC complains.
        return CodeSeverities::failed_null_assertion;
    }

    void Push(Error error, const Location &_location)
    {
        error_stack.push_back(error);
    }

    void Push(Code error, Severity severity, std::string context,
              const Location &location)
    {
        error_stack.push_back(Error(error, severity, location, context));
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
