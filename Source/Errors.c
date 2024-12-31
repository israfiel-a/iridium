#include <Errors.h>

#include <stdlib.h>
#include <string.h>

static bool warnings_silenced = false;
static bool errors_silenced = false;
static ir_fatality_override_t fatality_level = ir_just_panic;

//! rollover is intentional
static size_t max_reported_errors = (size_t)-1;

static size_t reported_error_count = 0;
static ir_problem_t *reported_errors = nullptr;

static const ir_severity_t severities[] = {
    [ir_no_error] = ir_success, // paradoxical lol
    [ir_failed_allocation] = ir_panic,
    [ir_unexpected_param] = ir_warning,
    [ir_failed_wayland_connection] = ir_panic,
    [ir_failed_wayland_registry] = ir_panic,
    [ir_failed_wayland_components] = ir_panic};

static size_t silenced_function_count = 0;
static const char **silenced_functions = nullptr;

static ir_severity_t GetSeverity(ir_problem_code_t code,
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

static bool ShouldLog(const char *function, ir_severity_t severity)
{
    if (severity == ir_panic) return true;

    bool function_silenced = false;
    for (size_t i = 0; i < silenced_function_count; i++)
        if (strcmp(function, silenced_functions[i]) == 0)
        {
            function_silenced = true;
            break;
        }
    if (function_silenced) return false;

    if (severity == ir_warning) return !warnings_silenced;
    if (severity == ir_error) return !errors_silenced;
    return false;
}

static bool GetFatality(const char *function, ir_severity_t severity)
{
    if (severity == ir_panic) return true;
    if (fatality_level == ir_just_panic) return false;

    // Make sure the function isn't silenced.
    bool function_silenced = false;
    for (size_t i = 0; i < silenced_function_count; i++)
        if (strcmp(function, silenced_functions[i]) == 0)
        {
            function_silenced = true;
            break;
        }
    if (function_silenced) return false;

    if (fatality_level != ir_all_problems && severity == ir_warning)
        return false;
    return true;
}

static const char *GetCodeString(ir_problem_code_t code)
{
    switch (code)
    {
        case ir_no_error:          return "ir_no_error";
        case ir_failed_allocation: return "ir_failed_allocation";
        case ir_unexpected_param:  return "ir_unexpected_param";
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

void Ir_SetProblemFatality(ir_fatality_override_t fatality)
{
    fatality_level = fatality;
}

void Ir_SetMaxProblems(size_t max)
{
    if (max == 0) max = (size_t)-1;
    max_reported_errors = max;
}

const ir_problem_t *Ir_GetProblem(size_t index)
{
    if (index == (size_t)-1 && reported_error_count != 0)
        return &reported_errors[reported_error_count];
    if (index >= reported_error_count)
    {
        Ir_ReportProblem(ir_unexpected_param, ir_override_infer,
                         "index out of bounds");
        return nullptr;
    }

    return &reported_errors[index];
}

bool Ir_PullProblem(size_t index, ir_problem_t *error)
{
    if (index == (size_t)-1 && reported_error_count != 0)
    {
        if (error != nullptr && error != NULL)
            *error = reported_errors[reported_error_count];

        reported_errors =
            realloc(reported_errors,
                    sizeof(ir_problem_t) * (reported_error_count--));
        if (reported_errors == NULL)
            Ir_ReportProblem(ir_failed_allocation, ir_override_infer,
                             nullptr);
        return true;
    }
    if (index >= reported_error_count)
    {
        Ir_ReportProblem(ir_unexpected_param, ir_override_infer,
                         "index out of bounds");
        return false;
    }

    if (error != nullptr && error != NULL) *error = reported_errors[index];
    for (size_t i = index; i < reported_error_count - 1; i++)
        reported_errors[i] = reported_errors[i + 1];
    reported_errors = realloc(
        reported_errors, sizeof(ir_problem_t) * (reported_error_count--));

    return true;
}

void Ir_ClearProblemStack(void)
{
    reported_error_count = 0;
    free(reported_errors);
    reported_errors = nullptr;
}

void Ir_CatchProblems(const char *function_name)
{
    silenced_functions =
        realloc(silenced_functions,
                sizeof(const char *) * (silenced_function_count + 1));
    if (silenced_functions == NULL)
        Ir_ReportProblem(ir_failed_allocation, ir_override_infer, nullptr);

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
        {
            silenced_functions =
                realloc(silenced_functions,
                        sizeof(const char *) * silenced_function_count);
            if (silenced_functions == NULL)
                Ir_ReportProblem(ir_failed_allocation, ir_override_infer,
                                 nullptr);
        }
    }

    return collapse;
}

void Ir_ReportProblem_(ir_problem_code_t code,
                       ir_severity_override_t override,
                       const char *context, const char *filename,
                       const char *function, uint32_t line)
{
    if (code == ir_no_error)
    {
        Ir_ReportProblem(ir_unexpected_param, ir_override_infer,
                         "no_error error code");
        return;
    }

    ir_problem_t error = {.code = code,
                          .severity = GetSeverity(code, override),
                          .context = context};

    if (ShouldLog(function, error.severity))
    {
        ir_loggable_t loggable = {.severity = error.severity,
                                  .title = "Problem Reported",
                                  .description = GetCodeString(code),
                                  .context = context};
        Ir_Log_(&loggable, filename, function, line);
    }

    // Kill the process should it be given a panic.
    if (GetFatality(function, error.severity)) exit(255);

    if (reported_error_count + 1 <= max_reported_errors)
    {
        reported_errors =
            realloc(reported_errors,
                    sizeof(ir_problem_t) * (reported_error_count + 1));
        if (reported_errors == NULL)
            Ir_ReportProblem(ir_failed_allocation, ir_override_infer,
                             nullptr);

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
