/**
 * @file Errors.h
 * @authors Israfil Argos (israfiel-a)
 * @brief This file provides Iridium's error reporting function set. This
 * includes logic for handling panics, creating log files, and more.
 *
 * Copyright (c) 2024 the Iridium Development Team
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */
#ifndef IRIDIUM_ERRORS_H
#define IRIDIUM_ERRORS_H

#include <Logging.h>

/**
 * @name error_code
 * @brief An error code. This corresponds to a predetermined severity (see
 * ir_severity_t) and short string explanation.
 */
typedef enum ir_error_code
{
    /**
     * @brief No error reported.
     */
    ir_no_error,
    /**
     * @brief A provided parameter was unexpected in the function context.
     * This is a warning.
     */
    ir_unexpected_param,
    /**
     * @brief Connecting to the Wayland server failed for whatever reason.
     * ERRNO is provided alongside this error. This a panic.
     */
    ir_failed_wayland_connection,
    /**
     * @brief Getting the Wayland registry failed for whatever reason.
     * ERRNO is provided alongside this error. This a panic.
     */
    ir_failed_wayland_registry,
    /**
     * @brief The connected Wayland server is missing crucial components
     * (outlined in Wayland.h). This a panic.
     */
    ir_failed_wayland_components
} ir_error_code_t;

typedef enum ir_severity_override
{
    ir_override_infer,
    ir_override_warning,
    ir_override_error,
    ir_override_panic,
} ir_severity_override_t;

/**
 * @name SilenceWarnings
 * @authors Israfil Argos
 * @brief Silence the logging of only warnings. This is
 * recommended for production builds, as it can improve performance very
 * slightly in some cases.
 *
 * @param silence The new silence flag. False for off, true for on.
 */
void Ir_SilenceWarnings(bool silence);

/**
 * @name SilenceWarnings
 * @authors Israfil Argos
 * @brief Silence the logging of only errors. This is not recommended, as
 * it can confuse users running into errors.
 *
 * @param silence The new silence flag. False for off, true for on.
 */
void Ir_SilenceErrors(bool silence);

/**
 * @name SilenceProblems
 * @authors Israfil Argos
 * @brief Generally silence the logging of errors. This is severly
 * discouraged, as it can confuse users running into problems. Note that
 * this does nothing for engine panics.
 *
 * @param silence The new silence flag. False for off, true for on.
 */
void Ir_SilenceProblems(bool silence);

/**
 * @name SetMaxProblems
 * @authors Israfil Argos
 * @brief Set the maximum amount of problems that are kept in memory. The
 * recommended is at least ten. Allocating for zero will result in no cap
 * being set. Giving a value of SIZE_MAX will give the same result.
 *
 * @param max The maximum amount of problems to be kept.
 */
void Ir_SetMaxProblems(size_t max);

/**
 * @name CatchProblems
 * @authors Israfil Argos
 * @brief Don't log any errors reported within the specified function, just
 * push them to the error stack. This is provided to allow for error
 * handling within nested functions. This functionality is ended via the
 * ReleaseProblems function. If all errors are set to be fatal, this
 * function will suspend fatality for any errors except for panics.
 * @note This function does not overwrite any previous calls to itself, the
 * function name is pushed to an array.
 *
 * @param function_name The name of the function in which to catch/silence
 * errors. This is case sensitive. This also is not checked for doubles, so
 * any function that "catches" another function should specify the name of
 * that function in its documentation to prevent doubling.
 */
void Ir_CatchProblems(const char *function_name);

/**
 * @name ReleaseProblems
 * @authors Israfil Argos
 * @brief Release a function from the silence stack. This is recommended to
 * be called once the errors raised have been handled. A call to this
 * function with NULL/nullptr as function_name is made within the engine
 * end function.
 *
 * @param function_name The name of the function to release. If this is
 * NULL/nullptr, all functions are released. This is case sensitive.
 * @returns A boolean representing whether or not the given function was
 * found.
 */
bool Ir_ReleaseProblems(const char *function_name);

void Ir_ReportProblem(ir_error_code_t code,
                      ir_severity_override_t override,
                      const char *context);

#endif // IRIDIUM_ERRORS_H
