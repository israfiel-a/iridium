#include <Errors.h>

#include <stdlib.h>
#include <string.h>

typedef struct ir_error
{
    ir_error_code_t code;
    ir_severity_t severity;
    const char *context;
} ir_error_t;

static bool warnings_silenced = false;
static bool errors_silenced = false;

//! rollover is intentional
static size_t max_reported_errors = (size_t)-1;

static size_t reported_error_count = 0;
static ir_error_t *reported_errors = nullptr;

static const ir_severity_t severities[] = {
    [ir_no_error] = ir_success, // paradoxical lol
    [ir_unexpected_param] = ir_warning,
    [ir_failed_wayland_connection] = ir_panic,
    [ir_failed_wayland_registry] = ir_panic,
    [ir_failed_wayland_components] = ir_panic};

static size_t silenced_function_count = 0;
static const char **silenced_functions = nullptr;

static ir_severity_t GetSeverity(ir_error_code_t code,
                                 ir_severity_override_t override)
{
    switch (override)
    {
        case ir_override_infer:   return severities[code];
        case ir_override_warning: return ir_warning;
        case ir_override_error:   return ir_error;
        case ir_override_panic:   return ir_panic;
    }
    return severities[code];
}

static bool ShouldLog(ir_severity_t severity)
{
    switch (severity)
    {
        case ir_warning: return !warnings_silenced;
        case ir_error:   return !errors_silenced;
        default:         return true;
    }
}

static const char *GetCodeString(ir_error_code_t code)
{
    switch (code)
    {
        case ir_no_error:         return "ir_no_error";
        case ir_unexpected_param: return "ir_unexpected_param";
        case ir_failed_wayland_connection:
            return "ir_failed_wayland_connection";
        case ir_failed_wayland_registry:
            return "ir_failed_wayland_registry";
        case ir_failed_wayland_components:
            return "ir_failed_wayland_components";
    }
    return "ir_no_error";
}

void Ir_SilenceWarnings(bool silence) { warnings_silenced = silence; }

void Ir_SilenceErrors(bool silence) { errors_silenced = silence; }

void Ir_SilenceProblems(bool silence)
{
    warnings_silenced = silence;
    errors_silenced = silence;
}

void Ir_SetMaxProblems(size_t max)
{
    if (max == 0) max = (size_t)-1;
    max_reported_errors = max;
}

void Ir_CatchProblems(const char *function_name)
{
    if (function_name == NULL || nullptr)
    {
        Ir_ReportProblem(ir_unexpected_param, ir_override_infer,
                         "null function name");
        return;
    }

    silenced_functions =
        realloc(silenced_functions,
                sizeof(const char *) * (silenced_function_count + 1));
    silenced_functions[silenced_function_count] = function_name;
    silenced_function_count++;
}

bool Ir_ReleaseProblems(const char *function_name)
{
    if (function_name == NULL || function_name == nullptr)
    {
        silenced_function_count = 0;
        free(silenced_functions);
        silenced_functions = nullptr;
        return true;
    }

    bool collapse = false;
    // Loop until we find the item, and then shrink the buffer in on
    // itself when we do.
    for (size_t i = 0; i < silenced_function_count; i++)
    {
        if (!collapse && strcmp(function_name, silenced_functions[i]) == 0)
            collapse = true;
        if (collapse && i < silenced_function_count - 1)
            silenced_functions[i] = silenced_functions[i + 1];
    }

    // If the function was found, shrink the buffer.
    if (collapse)
    {
        silenced_function_count--;
        if (silenced_function_count == 0) Ir_ReleaseProblems(nullptr);
        else
            silenced_functions =
                realloc(silenced_functions,
                        sizeof(const char *) * silenced_function_count);
    }

    return collapse;
}

void Ir_ReportProblem(ir_error_code_t code,
                      ir_severity_override_t override, const char *context)
{
    if (code == ir_no_error)
    {
        Ir_ReportProblem(ir_unexpected_param, ir_override_infer,
                         "no_error error code");
        return;
    }

    ir_error_t error = {.code = code,
                        .severity = GetSeverity(code, override),
                        .context = context};

    if (ShouldLog(error.severity))
    {
        ir_loggable_t loggable = {.severity = error.severity,
                                  .title = "Problem Reported",
                                  .description = GetCodeString(code),
                                  .context = context};
        Ir_Log(&loggable);
    }

    // Kill the process should it be given sa panic.
    if (error.severity == ir_panic) exit(255);

    if (reported_error_count + 1 <= max_reported_errors)
    {
        reported_errors =
            realloc(reported_errors,
                    sizeof(ir_error_t) * (reported_error_count + 1));
        reported_errors[reported_error_count] = error;
        reported_error_count++;
    }
    else
    {
        for (size_t i = 0; i < reported_error_count - 1; i++)
            reported_errors[i] = reported_errors[i + 1];
        reported_errors[reported_error_count] = error;
    }
}
