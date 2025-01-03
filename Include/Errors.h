/**
 * @file Errors.h
 * @authors Israfil Argos (israfiel-a)
 * @brief This file provides Iridium's error reporting function set. This
 * includes logic for handling panics, creating log files, and more.
 * @since 0.0.1
 *
 * @copyright (c) 2024 the Iridium Development Team
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */
#ifndef IRIDIUM_ERRORS_H
#define IRIDIUM_ERRORS_H

#include <Logging.h>

/**
 * @name problem_code
 * @brief An error code. This corresponds to a predetermined severity (see
 * ir_severity_t) and short string explanation.
 * @since 0.0.1
 */
typedef enum ir_problem_code
{
    /**
     * @brief No error reported.
     * @since 0.0.1
     */
    ir_no_error,
    /**
     * @brief A provided parameter was unexpected in the function context.
     * This is a warning.
     * @since 0.0.1
     */
    ir_unexpected_param,
    /**
     * @brief Connecting to the Wayland server failed for whatever reason.
     * ERRNO is provided alongside this error. This a panic.
     * @since 0.0.1
     */
    ir_failed_wayland_connection,
    /**
     * @brief Getting the Wayland registry failed for whatever reason.
     * ERRNO is provided alongside this error. This a panic.
     * @since 0.0.1
     */
    ir_failed_wayland_registry,
    /**
     * @brief The connected Wayland server is missing crucial components
     * (outlined in Wayland.h). This a panic.
     * @since 0.0.1
     */
    ir_failed_wayland_components,
    /**
     * @brief A file open failed.
     * @since 0.0.2
     */
    ir_failed_file_open
} ir_problem_code_t;

/**
 * @name severity_override
 * @brief An override for the default severity given to each problem code.
 * @since 0.0.1
 */
typedef enum ir_severity_override
{
    /**
     * @brief Use the default severity.
     * @since 0.0.1
     */
    ir_override_infer,
    /**
     * @brief Override the severity to be a warning.
     * @since 0.0.1
     */
    ir_override_warning,
    /**
     * @brief Override the severity to be an error.
     * @since 0.0.1
     */
    ir_override_error,
    /**
     * @brief Override the severity to be a panic.
     * @since 0.0.1
     */
    ir_override_panic,
} ir_severity_override_t;

/**
 * @name fatality_override
 * @brief An enum representing the various stages of problem fatality
 * possible.
 * @since 0.0.1
 */
typedef enum ir_fatality_override
{
    /**
     * @brief Only panics are fatal. This is the default.
     * @since 0.0.1
     */
    ir_just_panic,
    /**
     * @brief Both panics and regular errors are fatal.
     * @since 0.0.1
     */
    ir_include_errors,
    /**
     * @brief All problems, warnings, included, are fatal.
     * @since 0.0.1
     */
    ir_all_problems
} ir_fatality_override_t;

/**
 * @name problem
 * @brief A problem structure.
 * @since 0.0.1
 */
typedef struct ir_problem
{
    /**
     * @brief The problem's code.
     * @since 0.0.1
     */
    ir_problem_code_t code;
    /**
     * @brief The problems's severity.
     * @since 0.0.1
     */
    ir_severity_t severity;
    /**
     * @brief Any other context to do with the problem.
     * @since 0.0.1
     */
    const char *context;
} ir_problem_t;

/**
 * @name SilenceWarnings
 * @authors Israfil Argos
 * @brief Silence the logging of only warnings. This is
 * recommended for production builds, as it can improve performance very
 * slightly in some cases.
 * @since 0.0.1
 *
 * @param silence The new silence flag. False for off, true for on.
 */
void Ir_SilenceWarnings(bool silence);

/**
 * @name SilenceWarnings
 * @authors Israfil Argos
 * @brief Silence the logging of only errors. This is not recommended, as
 * it can confuse users running into errors.
 * @since 0.0.1
 *
 * @param silence The new silence flag. False for off, true for on.
 */
void Ir_SilenceErrors(bool silence);

/**
 * @name SilenceProblems
 * @authors Israfil Argos
 * @brief Generally silence the logging of problems. This is severly
 * discouraged, as it can confuse users running into problems. Note that
 * this does nothing for engine panics.
 * @since 0.0.1
 *
 * @param silence The new silence flag. False for off, true for on.
 */
void Ir_SilenceProblems(bool silence);

/**
 * @name SetProblemFatality
 * @authors Israfil Argos
 * @brief Set the level of fatality the program is in. It is reccomended to
 * turn this to ir_all_problems. Note that this can be overridden itself by
 * the CatchProblems function, in which case only panics will be fatal.
 * @since 0.0.1
 *
 * @param fatality The new fatality override.
 */
void Ir_SetProblemFatality(ir_fatality_override_t fatality);

/**
 * @name SetMaxProblems
 * @authors Israfil Argos
 * @brief Set the maximum amount of problems that are kept in memory. The
 * recommended is at least ten. Allocating for zero will result in no cap
 * being set. Giving a value of SIZE_MAX will give the same result.
 * @since 0.0.1
 *
 * @param max The maximum amount of problems to be kept.
 */
void Ir_SetMaxProblems(size_t max);

/**
 * @name GetProblem
 * @authors Israfil Argos
 * @brief Get a problem from the problem stack at the given index.
 * @since 0.0.1
 *
 * @warning Should the index be out of bounds, an ir_unexpected_param
 * warning will be thrown.
 *
 * @param index The index to get the problem from. A value of -1 (SIZE_MAX)
 * will grab the last problem reported.
 * @returns The problem at the given index, or nullptr if an error
 * occurred.
 */
[[nodiscard("Expression result unused.")]]
const ir_problem_t *Ir_GetProblem(size_t index);

/**
 * @name PullProblem
 * @authors Israfil Argos
 * @brief Pull a problem off the stack at the given index.
 * @since 0.0.1
 *
 * @warning Should the index be out of bounds, an ir_unexpected_param
 * warning will be thrown.
 * @note Do not print the pulled error without first checking if its
 * context is nullptr, as that is possible. Printing nullptr will almost
 * always end up throwing a segfault.
 *
 * @param index The index to get the problem from. A value of -1 (SIZE_MAX)
 * will grab the last problem reported.
 * @param problem A storage object for the problem, or NULL/nullptr if you
 * do not wish to store the pulled problem.
 * @returns A boolean representing the success of the operation.
 */
bool Ir_PullProblem(size_t index, ir_problem_t *problem);

/**
 * @name CatchProblems
 * @authors Israfil Argos
 * @brief Don't log any problems reported within the specified function,
 * just push them to the error stack. This is provided to allow for error
 * handling within nested functions. This functionality is ended via the
 * ReleaseProblems function. If all problems are set to be fatal, this
 * function will suspend fatality for any errors except for panics.
 * @since 0.0.1
 *
 * @note This function does not overwrite any previous calls to itself, the
 * function name is pushed to an array.
 *
 * @param function_name The name of the function in which to catch/silence
 * errors. This is case sensitive. This also is not checked for doubles, so
 * any function that "catches" another function should specify the name of
 * that function in its documentation to prevent doubling.
 */
[[gnu::nonnull(1)]]
void Ir_CatchProblems(const char *function_name);

/**
 * @name ReleaseProblems
 * @authors Israfil Argos
 * @brief Release a function from the silence stack. This is recommended to
 * be called once the errors raised have been handled. A call to this
 * function with NULL/nullptr as function_name is made within the engine
 * end function.
 * @since 0.0.1
 *
 * @param function_name The name of the function to release. If this is
 * NULL/nullptr, all functions are released. This is case sensitive.
 * @returns A boolean representing whether or not the given function was
 * found.
 */
bool Ir_ReleaseProblems(const char *function_name);

/**
 * @name ReportProblem_
 * @brief Report that a problem occurred (internal).
 * @since 0.0.1
 *
 * @warning Should the severity of this problem be resolved to panic (or
 * should the fatality level be cranked up), this function will abort the
 * thread and generate a core dump.
 *
 * @param code The code of the problem.
 * @param override An override for the code's default severity. If you do
 * not want to to override, set this to ir_override_infer.
 * @param context Any extra context for the problem. This can be NULL.
 * Should the reported error code be noted as being accompanied by ERRNO,
 * this value is replaced by ERRNO.
 * @param filename The name of the file from which the problem was sent.
 * This is defined during the build process as the file's basename + its
 * extension.
 * @param function The name of the function from which the problem was
 * sent.
 * @param line The line from which the problem was sent.
 */
[[gnu::nonnull(4, 5)]] [[gnu::hot]]
void Ir_ReportProblem_(ir_problem_code_t code,
                       ir_severity_override_t override,
                       const char *context, const char *filename,
                       const char *function, uint32_t line);

/**
 * @name ReportProblem
 * @brief Report that a problem occurred.
 * @since 0.0.1
 *
 * @param code The code of the problem.
 * @param override An override for the code's default severity. If you do
 * not want to to override, set this to ir_override_infer.
 * @param context Any extra context for the problem. This can be NULL.
 * Should the reported error code be noted as being accompanied by ERRNO,
 * this value is replaced by ERRNO.
 */
#define Ir_ReportProblem(code, override, context)                         \
    Ir_ReportProblem_(code, override, context, FILENAME, __func__,        \
                      __LINE__)

#endif // IRIDIUM_ERRORS_H
