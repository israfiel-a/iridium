#include <Logging.h>
#include <Memory.h>
#include <execinfo.h>
#include <stdarg.h>
#include <string.h>

static bool logs_silenced = false;

static bool ansi_allowed = false;

static ir_output_t *log_output = nullptr;

static ir_output_t *error_output = nullptr;

static ir_output_t *panic_output = nullptr;

static bool stacktrace_silenced = false;

static uint8_t stacktrace_depth = 5;

static inline ir_output_t *GetProperOutput(ir_severity_t severity)
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
        default:         return "!! panic !!";
    }
}

static inline const char *GetProperColor(ir_severity_t severity)
{
    switch (severity)
    {
        case ir_success: return "32m";
        case ir_log:     return "39m"; // Default color.
        case ir_warning: return "33m";
        case ir_error:   return "31m";
        default:         return "4;1;31m"; // Bold, underline, red.
    }
}

//! public maybe?
static void PrintStacktrace(ir_output_t *output, char **trace)
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

void Ir_SilenceLogs(bool silence) { logs_silenced = silence; }

void Ir_AllowANSI(bool allowed) { ansi_allowed = allowed; }

void Ir_SetLogOutput(ir_output_t *output) { log_output = output; }

void Ir_SetErrorOutput(ir_output_t *output) { error_output = output; }

void Ir_SetPanicOutput(ir_output_t *output) { panic_output = output; }

const ir_output_t *Ir_GetLogOutput(void) { return log_output; }

const ir_output_t *Ir_GetErrorOutput(void) { return error_output; }

const ir_output_t *Ir_GetPanicOutput(void) { return panic_output; }

void Ir_SilenceStackTrace(bool silence) { stacktrace_silenced = silence; }

void Ir_SetStackTraceDepth(size_t depth) { stacktrace_depth = depth; }

ir_loggable_t Ir_CreateLoggable(const char *title, const char *description,
                                const char *context)
{
    ir_loggable_t created_loggable = {ir_log, title,
                                      Ir_Malloc(strlen(description) + 1),
                                      Ir_Malloc(strlen(context) + 1)};
    // Safe because of the allocation here ^
    (void)strcpy(created_loggable.description, description);
    (void)strcpy(created_loggable.context, context);

    return created_loggable;
}

ir_loggable_t Ir_CreateLoggableDF(const char *title,
                                  const char *description_format, ...)
{
    ir_loggable_t created_loggable = {ir_log, title, nullptr, nullptr};

    va_list args;
    va_start(args, description_format);

    (void)vasprintf(&created_loggable.description, description_format,
                    args);
    va_end(args);

    return created_loggable;
}

void Ir_DestroyLoggable(ir_loggable_t *loggable)
{
    loggable->severity = ir_log;
    loggable->title = nullptr;

    // Description and context are de-allocated if they exist.
    Ir_Free((void **)&loggable->description);
    if (loggable->context != nullptr) Ir_Free((void **)&loggable->context);
}

void Ir_Log_(ir_loggable_t *object, const char *file, const char *function,
             uint32_t line)
{
    // Don't do the expensive string concat and logging process should it
    // be disabled.
    if (logs_silenced && object->severity != ir_error &&
        object->severity != ir_panic)
        return;

    if (log_output == nullptr) log_output = stdout;

    ir_output_t *output = GetProperOutput(object->severity);
    const char *color_code = GetProperColor(object->severity);

    fprintf(output,
            "\n\033[%s%s, ln. %u :: %s():\n%s | %s - %s\n\tContext: "
            "%s\n",
            color_code, file, line, function,
            GetSeverityString(object->severity), object->title,
            object->description, object->context);

    // Handle panics properly.
    if (object->severity == ir_panic) abort();

    if (!stacktrace_silenced)
    {
        fprintf(output, "Stack trace:\n");
        void *buffer[6];
        int ret = backtrace(buffer, 6);
        (void)ret; //! check maybe
        char **trace = backtrace_symbols(buffer, 6);

        PrintStacktrace(output, trace);
        Ir_Free((void **)&trace);
    }

    fprintf(output, "\033[0m\n");
    Ir_DestroyLoggable(object);
}
