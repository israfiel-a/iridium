#include <Logging.h>

static ir_output_t log_output = nullptr;

static ir_output_t error_output = nullptr;

static ir_output_t panic_output = nullptr;

static inline ir_output_t GetProperOutput(ir_severity_t severity)
{
    if ((severity == warning || severity == error) &&
        error_output != nullptr)
        return error_output;
    else if (severity == panic && panic_output != nullptr)
        return panic_output;
    // De-escalate the panic log to the error logger if no panic logger has
    // been set.
    else if (severity == panic && panic_output == nullptr &&
             error_output != nullptr)
        return error_output;
    return log_output;
}

static inline const char *GetSeverityString(ir_severity_t severity)
{
    switch (severity)
    {
        case success: return "success";
        case log:     return "log";
        case warning: return "warning";
        case error:   return "error";
        case panic:   return "!! panic !!";
    }
    return "log";
}

static inline const char *GetProperColor(ir_severity_t severity)
{
    switch (severity)
    {
        case success: return "32m";
        case log:     return "0m";
        case warning: return "33m";
        case error:   return "31m";
        case panic:   return "1;31m";
    }
    return 0;
}

void Ir_Log_(ir_loggable_t *object, const char *file, const char *function,
             uint32_t line)
{
    if (log_output == nullptr) log_output = stdout;

    ir_output_t output = GetProperOutput(object->severity);
    const char *color_code = GetProperColor(object->severity);
    fprintf(
        output,
        "\033[%s%s, ln. %u :: %s():\n%s | %s - %s\n\tContext: %s\n\033[0m",
        color_code, file, line, function,
        GetSeverityString(object->severity), object->title,
        object->description, object->context);
}

void Ir_SetLogOutput(ir_output_t output) { log_output = output; }

void Ir_SetErrorOutput(ir_output_t output) { error_output = output; }

void Ir_SetPanicOutput(ir_output_t output) { panic_output = output; }

const ir_output_t *Ir_GetLogOutput(void) { return &log_output; }

const ir_output_t *Ir_GetErrorOutput(void) { return &error_output; }

const ir_output_t *Ir_GetPanicOutput(void) { return &panic_output; }
