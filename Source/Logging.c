/**
 * @file Logging.c
 * @authors Israfil Argos (israfiel-a)
 * @brief This file implements the Iridium logging interface.
 * @since 0.0.1
 *
 * @copyright (c) 2024 the Iridium Development Team
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */

#include <execinfo.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <Logging.h>
#include <Memory.h>

/**
 * @name logs_silenced
 * @brief A flag representing whether or not logs with a severity below
 * warning are enabled.
 * @since 0.0.1
 */
static bool logs_silenced = false;

/**
 * @name ansi_allowed
 * @brief A flag representing whether or not ANSI escape codes are enabled
 * for any output, not just stdout.
 * @since 0.0.2
 */
static bool ansi_allowed = false;

/**
 * @name log_output
 * @brief The output for, at the very least, success logs and normal logs.
 * Should the error output not also be set, this is the output for all
 * logs. Note that the default value of this is stdout, it simply isn't set
 * until the first Ir_Log call.
 * @since 0.0.1
 */
static ir_output_t *log_output = nullptr;

/**
 * @name error_output
 * @brief The output for errors and panics. This is nullptr by default.
 * @since 0.0.1
 */
static ir_output_t *error_output = nullptr;

/**
 * @name stacktrace_silenced
 * @brief A flag representing whether or not stack traces are enabled on
 * logs.
 * @since 0.0.2
 */
static bool stacktrace_silenced = false;

/**
 * @name stacktrace_depth
 * @brief The depth of each stack trace.
 * @since 0.0.2
 */
static uint8_t stacktrace_depth = 7;

/**
 * @name CloseStreams
 * @authors Israfil Argos
 * @brief Close all of the open output streams. This removes the need to
 * fclose any provided stream.
 * @since 0.0.2
 */
[[gnu::destructor]]
static void CloseStreams()
{
    // We do not care if these closes fail.
    if (log_output != stdout) (void)fclose(log_output);
    if (error_output != nullptr) (void)fclose(error_output);
}

static inline ir_output_t *GetProperOutput(ir_severity_t severity)
{
    if ((severity == ir_warning || severity == ir_error ||
         severity == ir_panic) &&
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
        case ir_success: return "32";
        case ir_log:     return "39"; // Default color.
        case ir_warning: return "33";
        case ir_error:   return "31";
        default:         return "4;1;31"; // Underline, bold, red.
    }
}

void Ir_SilenceLogs(bool silence) { logs_silenced = silence; }

void Ir_AllowANSI(bool allowed) { ansi_allowed = allowed; }

void Ir_SetLogOutput(ir_output_t *output) { log_output = output; }

bool Ir_SetLogOutputS(const char *path)
{
    // Open the file in write-binary mode.
    ir_output_t *opened_output = fopen(path, "wb");
    if (opened_output == nullptr)
    {
        Ir_ReportProblem(ir_failed_file_open, ir_override_infer, nullptr);
        return false;
    }

    log_output = opened_output;
    return true;
}

void Ir_SetErrorOutput(ir_output_t *output) { error_output = output; }

bool Ir_SetErrorOutputS(const char *path)
{
    // Open the file in write-binary mode.
    ir_output_t *opened_output = fopen(path, "wb");
    if (opened_output == nullptr)
    {
        Ir_ReportProblem(ir_failed_file_open, ir_override_infer, nullptr);
        return false;
    }

    error_output = opened_output;
    return true;
}

const ir_output_t *Ir_GetLogOutput(void) { return log_output; }

const ir_output_t *Ir_GetErrorOutput(void) { return error_output; }

void Ir_SilenceStacktrace(bool silence) { stacktrace_silenced = silence; }

void Ir_SetStacktraceDepth(uint8_t depth) { stacktrace_depth = depth; }

char **Ir_GetStacktrace(void)
{
    // Because we cut off the first result (it is simply where we are now),
    // we need one more space to fill the whole depth.
    void *buffer[stacktrace_depth + 1];
    // We couldn't give less of a damn about the backtrace's truncation.
    (void)backtrace(buffer, stacktrace_depth + 1);

    char **symbols = backtrace_symbols(buffer, stacktrace_depth + 1);
    symbols[stacktrace_depth] = nullptr;
    return symbols;
}

void Ir_PrintStacktrace(ir_output_t *output)
{
    char **stacktrace = Ir_GetStacktrace();
    char **stacktrace_marker = stacktrace;

    for (char *current_entry = *stacktrace; current_entry != nullptr;
         current_entry = *++stacktrace)
    {
        char *last_fs = strrchr(current_entry, '/');
        if (last_fs == nullptr)
            (void)fprintf(output, "\t%s\n", current_entry);
        else (void)fprintf(output, "\t%s\n", last_fs + 1);
    }
    Ir_Free((void **)&stacktrace_marker);
}

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

    const char *color_code = nullptr;
    if (output == stdout || ansi_allowed)
    {
        color_code = GetProperColor(object->severity);
        (void)fprintf(output, "\033[%sm", color_code);
    }

    (void)fprintf(
        output, "\n%s, ln. %u :: %s():\n%s | %s - %s\n\tContext: %s\n",
        file, line, function, GetSeverityString(object->severity),
        object->title, object->description, object->context);
    if (output == stdout || ansi_allowed) (void)fputs("\033[0m", output);

    // Handle panics properly.
    if (object->severity == ir_panic)
    {
        (void)fputs("\n", output);
        abort();
    }

    if (!stacktrace_silenced)
    {
        if (output == stdout || ansi_allowed)
            (void)fprintf(output, "\033[%sm", color_code);
        (void)fputs("Stack trace:\n", output);

        Ir_PrintStacktrace(output);
        if (output == stdout || ansi_allowed)
            (void)fputs("\033[0m", output);
    }

    (void)fputs("\n", output);
    Ir_DestroyLoggable(object);
}
