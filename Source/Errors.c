/**
 * @file Errors.c
 * @authors Israfil Argos (israfiel-a)
 * @brief This file provides the implementation for the Iridium problem
 * reporting interface.
 * @since 0.0.1
 *
 * @copyright (c) 2024 the Iridium Development Team
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <Errors.h>
#include <Memory.h>

/**
 * @name fatality_level
 * @brief The fatality level of the engine's problem reporting interface.
 * This decides what kills the process, and is set via the
 * SetProblemFatality function.
 * @since 0.0.1
 */
static ir_fatality_override_t fatality_level = ir_just_panic;

/**
 * @name max_reported_problems
 * @brief The maximum amount of problems that will be kept in memory. Note
 * that the rollover in this initialization is on purpose, we don't want a
 * cap by default.
 * @since 0.0.1
 */
static size_t max_reported_problems = (size_t)-1;

/**
 * @name reported_problem_count
 * @brief The count of problems currently kept in memory. This can be <=
 * the max reported problem count.
 * @since 0.0.1
 */
static size_t reported_problem_count = 0;

/**
 * @name reported_problems
 * @brief An array of the problems currently kept in memory. This array's
 * length is represented by the reported_problem_count variable.
 * @since 0.0.1
 */
static ir_problem_t *reported_problems = nullptr;

/**
 * @name problem_severities
 * @brief An array of the predetermined severities of each problem code.
 * @since 0.0.1
 */
static const ir_severity_t problem_severities[] = {
    [ir_no_error] = ir_success, // paradoxical lols
    [ir_unexpected_param] = ir_warning,
    [ir_failed_wayland_connection] = ir_panic,
    [ir_failed_wayland_registry] = ir_panic,
    [ir_failed_wayland_components] = ir_panic,
    [ir_failed_file_open] = ir_error};

/**
 * @name problem_strings
 * @brief An array of the string versions of each problem code.
 * @since 0.0.1
 */
static const char *const problem_strings[] = {
    [ir_no_error] = "ir_no_error",
    [ir_unexpected_param] = "ir_unexpected_param",
    [ir_failed_wayland_connection] = "ir_failed_wayland_connection",
    [ir_failed_wayland_registry] = "ir_failed_wayland_registry",
    [ir_failed_wayland_components] = "ir_failed_wayland_components",
    [ir_failed_file_open] = "ir_failed_file_open"};

/**
 * @name warnings_silenced
 * @brief A boolean that represents whether or not warnings will be
 * logged when the engine runs into one.
 * @since 0.0.1
 */
static bool warnings_silenced = false;

/**
 * @name errors_silenced
 * @brief A boolean that represents whether or not errors (not panics!)
 * will be logged when the engine runs into one.
 * @since 0.0.1
 */
static bool errors_silenced = false;

/**
 * @name silenced_function_count
 * @brief The count of functions currently being silenced.
 * @since 0.0.1
 */
static size_t silenced_function_count = 0;

/**
 * @name silenced_functions
 * @brief An array of raw strings, each of which correspond to a function
 * name from which thrown error will be silenced.
 * @since 0.0.1
 */
static const char **silenced_functions = nullptr;

/**
 * @name FreeStacks
 * @author Israfil Argos
 * @brief This is called once the application exits and frees both the
 * silenced function and reported problem list. This removes the need for
 * manual destruction.
 * @since 0.0.2
 */
[[gnu::destructor]]
static void FreeStacks()
{
    if (reported_problems != nullptr) Ir_Free((void **)&reported_problems);
    if (silenced_functions != nullptr)
        Ir_Free((void **)&silenced_functions);
}

/**
 * @name GetFunctionSilenced
 * @authors Israfil Argos
 * @brief See if the passed function is currently "silenced" or not.
 * @since 0.0.1
 *
 * @param function The function to check for.
 * @returns A boolean expression representing whether or not the function
 * is silenced.
 */
[[gnu::nonnull(1)]] [[nodiscard("Expression result unused.")]]
static bool GetFunctionSilenced(const char *function)
{
    for (size_t i = 0; i < silenced_function_count; i++)
        if (strcmp(function, silenced_functions[i]) == 0) return true;
    return false;
}

/**
 * @name GetSeverity
 * @authors Israfil Argos
 * @brief Get the severity of a given error code, taking into consideration
 * provided overrides.
 * @since 0.0.1
 *
 * @param code The code provided.
 * @param override An override provided by the caller that may become the
 * code's new severity.
 * @returns The severity of the given code.
 */
[[gnu::pure]] [[nodiscard("Expression result unused.")]]
static ir_severity_t GetSeverity(ir_problem_code_t code,
                                 ir_severity_override_t override)
{
    switch (override)
    {
        case ir_override_infer:   return problem_severities[code];
        case ir_override_warning: return ir_warning;
        case ir_override_error:   return ir_error;
        case ir_override_panic:   return ir_panic;
    }
    return problem_severities[code];
}

/**
 * @name GetFatality
 * @authors Israfil Argos
 * @brief Get the fatality of the given severity when taking into account
 * the fatality level of the engine and silenced functions.
 * @since 0.0.1
 *
 * @param function The function this problem came from.s
 * @param severity The severity of the problem.
 * @returns A boolean representing if the given problem should be fatal or
 * not.
 */
[[gnu::nonnull(1)]] [[nodiscard("Expression result unused.")]]
static bool GetFatality(const char *function, ir_severity_t severity)
{
    if (severity == ir_panic) return true;
    if (fatality_level == ir_just_panic) return false;

    if (GetFunctionSilenced(function)) return false;
    if (fatality_level != ir_all_problems && severity == ir_warning)
        return false;
    return true;
}

/**
 * @name GetContext
 * @authors Israfil Argos
 * @brief Get the context of the given error code; some APIs need other
 * error codes provided, which overwrites whatever nonsense the user's
 * provided.
 * @since 0.0.2
 *
 * @param provided_context Any context that was provided by the user.
 * @param code The code reported.
 * @returns The needed context, be it ERRNO or provided context or
 * something else.
 */
static const char *GetContext(const char *provided_context,
                              ir_problem_code_t code)
{
    switch (code)
    {
        case ir_failed_wayland_connection: [[fallthrough]];
        case ir_failed_file_open:          return strerror(errno);
        default:                           return provided_context;
    }
}

/**
 * @name GetLoggability
 * @authors Israfil Argos
 * @brief Get whether or not the given severity and given caller function
 * warrant a log to the output.
 * @since 0.0.1
 *
 * @param function The function that the problem came from.
 * @param severity The severity of the problem.
 * @returns A boolean expression representing whether or not the problem
 * should be loaded.
 */
[[gnu::nonnull(1)]] [[nodiscard("Expression result unused.")]]
static bool GetLoggability(const char *function, ir_severity_t severity)
{
    if (severity == ir_panic) return true;

    if (GetFunctionSilenced(function)) return false;
    if (severity == ir_warning) return !warnings_silenced;
    if (severity == ir_error) return !errors_silenced;
    return false;
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
    max_reported_problems = max;
}

const ir_problem_t *Ir_GetProblem(size_t index)
{
    if (index == (size_t)-1 && reported_problem_count != 0)
        return &reported_problems[reported_problem_count];
    if (index >= reported_problem_count)
    {
        Ir_ReportProblem(ir_unexpected_param, ir_override_infer,
                         "index out of bounds");
        return nullptr;
    }

    return &reported_problems[index];
}

bool Ir_PullProblem(size_t index, ir_problem_t *error)
{
    if (reported_problem_count == 0)
    {
        Ir_ReportProblem(ir_unexpected_param, ir_override_infer,
                         "index out of bounds");
        return false;
    }

    if (index == (size_t)-1) index = reported_problem_count - 1;
    if (index >= reported_problem_count)
    {
        Ir_ReportProblem(ir_unexpected_param, ir_override_infer,
                         "index out of bounds");
        return false;
    }

    if (error != nullptr) *error = reported_problems[index];
    if (reported_problem_count == 1)
    {
        Ir_Free((void **)&reported_problems);
        reported_problem_count = 0;
        return true;
    }

    if (index != reported_problem_count - 1)
        (void)memmove(
            &reported_problems[index], &reported_problems[index + 1],
            sizeof(ir_problem_t) * (reported_problem_count - index - 1));

    Ir_Realloc((void **)&reported_problems,
               sizeof(ir_problem_t) * (reported_problem_count--));

    return true;
}

void Ir_CatchProblems(const char *function_name)
{
    Ir_Realloc((void **)&silenced_functions,
               sizeof(const char *) * (silenced_function_count + 1));
    silenced_functions[silenced_function_count] = function_name;
    silenced_function_count++;
}

bool Ir_ReleaseProblems(const char *function_name)
{
    if (function_name == NULL || function_name == nullptr)
    {
        silenced_function_count = 0;
        Ir_Free((void **)&silenced_functions);
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
            Ir_Realloc((void **)&silenced_functions,
                       sizeof(const char *) * silenced_function_count);
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
                          .context = GetContext(context, code)};

    if (GetLoggability(function, error.severity))
    {
        ir_loggable_t loggable = Ir_CreateLoggable(
            "Problem Reported", problem_strings[code], error.context);
        loggable.severity = error.severity;
        Ir_Log_(&loggable, filename, function, line);
    }

    // Kill the process should it be given a fatal error. However, note
    // that panics are already handled within Ir_Log, not here.
    if (GetFatality(function, error.severity)) abort();

    if (reported_problem_count + 1 <= max_reported_problems)
    {
        Ir_Realloc((void **)&reported_problems,
                   sizeof(ir_problem_t) * (reported_problem_count + 1));
        reported_problems[reported_problem_count] = error;
        reported_problem_count++;
    }
    else
    {
        (void)memmove(&reported_problems[0], &reported_problems[1],
                      reported_problem_count - 1);
        // The reported error buffer hasn't expanded, so this needs to
        // access the current last member (count - 1).
        reported_problems[reported_problem_count - 1] = error;
    }
}
