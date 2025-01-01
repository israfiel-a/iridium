#include <Logging.h>
#include <execinfo.h>
#include <stdlib.h>
#include <string.h>

static bool silence_logs = false;

static ir_output_t log_output = nullptr;

static ir_output_t error_output = nullptr;

static ir_output_t panic_output = nullptr;

static inline ir_output_t GetProperOutput(ir_severity_t severity)
{
    if ((severity == ir_warning || severity == ir_error) &&
        error_output != nullptr)
        return error_output;
    else if (severity == ir_panic && panic_output != nullptr)
        return panic_output;
    // De-escalate the panic log to the error logger if no panic logger has
    // been set.
    else if (severity == ir_panic && panic_output == nullptr &&
             error_output != nullptr)
        return error_output;
    return log_output;
}

static inline const char *GetSeverityString(ir_severity_t severity)
{
    switch (severity)
    {
        case ir_success: return "success";
        case ir_log:     return "log";
        case ir_warning: return "warning";
        case ir_error:   return "error";
        case ir_panic:   return "!! panic !!";
    }
    return "log";
}

static inline const char *GetProperColor(ir_severity_t severity)
{
    switch (severity)
    {
        case ir_success: return "32m";
        case ir_log:     return "0m";
        case ir_warning: return "33m";
        case ir_error:   return "31m";
        case ir_panic:   return "1;31m";
    }
    return 0;
}

//! public maybe?
static void PrintStacktrace(ir_output_t output, char **trace)
{
    //! hard coded length (5)

    for (size_t i = 1; i < 6; i++)
    {
        char *current_trace = trace[i];

        size_t last_forward_index = -1;
        size_t first_parenthesis = -1;
        size_t last_parenthesis = -1;
        for (size_t i = 0; i < strlen(current_trace); i++)
        {
            char character = current_trace[i];
            if (character == '/') last_forward_index = i;
            else if (character == '(' && first_parenthesis == (size_t)-1)
                first_parenthesis = i;
            else if (character == ')') last_parenthesis = i;
        }

        char filename[128] = {0}, symbol[128] = {0};
        strncpy(filename, current_trace + last_forward_index + 1,
                first_parenthesis - last_forward_index - 1);
        strncpy(symbol, current_trace + first_parenthesis + 1,
                last_parenthesis - first_parenthesis - 1);

        fprintf(output, "\t%s: %s\n", filename, symbol);
    }
}

void Ir_Log_(ir_loggable_t *object, const char *file, const char *function,
             uint32_t line)
{
    if (log_output == nullptr) log_output = stdout;

    // Don't do the expensive string concat and logging process should it
    // be disabled.
    if (silence_logs &&
        (object->severity == ir_log || object->severity == ir_success))
        return;

    ir_output_t output = GetProperOutput(object->severity);
    const char *color_code = GetProperColor(object->severity);

    //! DONT DO THIS SHIT WHEN PANICKING
    void *buffer[6];
    int ret = backtrace(buffer, 6);
    (void)ret; //! check maybe
    char **trace = backtrace_symbols(buffer, 6);

    fprintf(output,
            "\n\033[%s%s, ln. %u :: %s():\n%s | %s - %s\n\tContext: "
            "%s\nStack trace:\n",
            color_code, file, line, function,
            GetSeverityString(object->severity), object->title,
            object->description, object->context);
    PrintStacktrace(output, trace);
    fprintf(output, "\033[0m\n");

    free(trace);
}

void Ir_SilenceLogs(bool silence) { silence_logs = silence; }

void Ir_SetLogOutput(ir_output_t output) { log_output = output; }

void Ir_SetErrorOutput(ir_output_t output) { error_output = output; }

void Ir_SetPanicOutput(ir_output_t output) { panic_output = output; }

const ir_output_t *Ir_GetLogOutput(void) { return &log_output; }

const ir_output_t *Ir_GetErrorOutput(void) { return &error_output; }

const ir_output_t *Ir_GetPanicOutput(void) { return &panic_output; }
