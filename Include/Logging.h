/**
 * @file Logging.h
 * @authors Israfil Argos (israfiel-a)
 * @brief This file provides the logging API for the Iridium engine. This
 * includes an abstraction from stdout in case the user wants to pipe
 * output to a log file.
 * @since 0.0.1
 *
 * @copyright (c) 2024 the Iridium Development Team
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */
#ifndef IRIDIUM_LOGGING_H
#define IRIDIUM_LOGGING_H

#include <stdint.h>
#include <stdio.h>

/**
 * @name output
 * @brief An output stream object.
 * @since 0.0.1
 */
typedef FILE ir_output_t;

/**
 * @name severity
 * @brief The severity level of a log or problem.
 * @since 0.0.1
 */
typedef enum ir_severity
{
    /**
     * @brief An operation succeeded (green). A log marked with this can be
     * safely ignored.
     * @since 0.0.1
     */
    ir_success,
    /**
     * @brief A simple log is output (default). A log marked with this can
     * be safely ignored.
     * @since 0.0.1
     */
    ir_log,
    /**
     * @brief A warning log is output (orange). A log marked with this
     * should only be of interest to a developer.
     * @since 0.0.1
     */
    ir_warning,
    /**
     * @brief An error log is output (red). A log marked with this should
     * be of interest to both user and developer, but should not be fatal
     * to the application.
     * @since 0.0.1
     */
    ir_error,
    /**
     * @brief A panic log is output (bold, underlined red). A log marked
     * with this should be of extreme interest to developer and user
     * alike--these abort the process, and are not thrown about uselessly.
     * @since 0.0.1
     */
    ir_panic
} ir_severity_t;

/**
 * @name loggable
 * @brief A loggable object. This will be morphed into the Iridium log
 * format when output. It is not advisable to create these yourself,
 * instead use CreateLoggable or one of its sister functions.
 * @since 0.0.1
 */
typedef struct ir_loggable
{
    /**
     * @brief The severity of the log.
     * @since 0.0.1
     */
    ir_severity_t severity;
    /**
     * @brief The title of the log.
     * @since 0.0.1
     */
    const char *title;
    /**
     * @brief A description of the log.
     * @since 0.0.1
     */
    char *description;
    /**
     * @brief Extended context about the log.
     * @since 0.0.1
     */
    char *context;
} ir_loggable_t;

/**
 * @name SilenceLogs
 * @authors Israfil Argos
 * @brief Allow/disallow logging for severities below "error." Successes
 * and logs will no longer be output to @b any output stream.
 * @since 0.0.1
 *
 * @param silence The new silence flag. True to silence, false to
 * unsilence.
 */
void Ir_SilenceLogs(bool silence);

/**
 * @name AllowANSI
 * @authors Israfil Argos
 * @brief Allow ANSI color/typeface escape codes to be output anywhere. If
 * the output object is stdout ANSI sequences are enabled regardless.
 * @since 0.0.2
 *
 * @param allowed The allow flag. False to disable, true to enable.
 */
void Ir_AllowANSI(bool allowed);

/**
 * @name SetLogOutput
 * @authors Israfil Argos
 * @brief Set the output for successes, logs, and warnings. This must not
 * be fclosed, the engine will close the stream automatically on exit.
 * @since 0.0.1
 *
 * @param output The output to redirect normal logs to. This may not be
 * null. Instead, silence development logs via the SilenceLogs function.
 */
[[gnu::nonnull(1)]]
void Ir_SetLogOutput(ir_output_t *output);

/**
 * @name SetLogOutputS
 * @authors Israfil Argos
 * @brief Set the log output (see SetLogOutput) via a string path. This
 * automatically opens the file in write mode--overwriting any prior
 * contents.
 * @since 0.0.2
 *
 * @warning Should the file not exist (or fopen runs into another problem)
 * this function will throw an ir_failed_file_open error.
 *
 * @param path The path to open from.
 * @returns A boolean expression representing the success of the operation.
 */
[[gnu::nonnull(1)]]
bool Ir_SetLogOutputS(const char *path);

/**
 * @name SetErrorOutput
 * @authors Israfil Argos
 * @brief Set the output for errors. This must not be fclosed, the
 * engine will close the stream automatically on exit.
 * @since 0.0.1
 *
 * @param output The output to redirect error logs to.
 */
void Ir_SetErrorOutput(ir_output_t *output);

/**
 * @name SetErrorOutputS
 * @authors Israfil Argos
 * @brief Set the error log output (see SetErrorOutput) via a string path.
 * This automatically opens the file in write mode--overwriting any prior
 * contents.
 * @since 0.0.2
 *
 * @warning Should the file not exist (or fopen runs into another problem)
 * this function will throw an ir_failed_file_open error.
 *
 * @param path The path to open from.
 * @returns A boolean expression representing the success of the operation.
 */
[[gnu::nonnull(1)]]
bool Ir_SetErrorOutputS(const char *path);

/**
 * @name SilenceStacktrace
 * @authors Israfil Argos
 * @brief Allow/disallow stack traces within logs. This flag does not
 * effect panics, which have their stack trace disabled because of their
 * quick-fail nature.
 * @since 0.0.2
 *
 * @param silence The new silence flag. True to silence, false to
 * unsilence.
 */
void Ir_SilenceStacktrace(bool silence);

/**
 * @name SetStacktraceDepth
 * @authors Israfil Argos
 * @brief Set the depth of a stack trace. The maximum suggested size is 10,
 * although the true max is 255.
 * @since 0.0.2
 *
 * @param depth The depth that stack traces should be.
 */
void Ir_SetStacktraceDepth(uint8_t depth);

/**
 * @name GetStacktrace
 * @authors Israfil Argos
 * @brief Get the string representation of a stack trace. The return value
 * of this must be freed manually.
 * @since 0.0.2
 *
 * @returns The stack trace, or null should an error have occurred. The
 * length of the array is the current stack trace depth plus one, which is
 * 7 (8) by default. The array is also ended via a nullptr.
 */
char **Ir_GetStacktrace(void);

/**
 * @name PrintStacktrace
 * @authors Israfil Argos
 * @brief Print a stacktrace to the given output. This should be an open,
 * non-bad file handle.
 * @since 0.0.2
 *
 * @param output The output stream.
 */
[[gnu::nonnull(1)]]
void Ir_PrintStacktrace(ir_output_t *output);

/**
 * @name CreateLoggable
 * @authors Israfil Argos
 * @brief Create a loggable via exactly the strings provided. The severity
 * is automatically that of a regular log. For formatted inputs, see
 * CreateLoggableDF and CreateLoggableCF.
 * @since 0.0.2
 *
 * @param title The title of the log. This is not copied from the provided
 * string.
 * @param description A description of the log. This is copied from the
 * provided string.
 * @param context Any extra context about the log. This is copied from the
 * provided string.
 * @returns A loggable object.
 */
[[gnu::nonnull(1, 2)]] [[nodiscard("Expression result unused.")]]
ir_loggable_t Ir_CreateLoggable(const char *title, const char *description,
                                const char *context);

/**
 * @name CreateLoggableDF
 * @authors Israfil Argos
 * @brief Create a loggable via the format string provided. The severity is
 * by default that of a normal log, and the context is nullptr.
 * @since 0.0.2
 *
 * @param title The title of the log. This is not copied from the provided
 * string.
 * @param description_format The format string for the log's description.
 * @param ... Arguments to be substituted in the description format string.
 * @returns A loggable object.
 */
[[gnu::format(printf, 2, 3)]] [[gnu::nonnull(1, 2)]] [[nodiscard(
    "Expression result unused.")]]
ir_loggable_t Ir_CreateLoggableDF(const char *title,
                                  const char *description_format, ...);

/**
 * @name CreateLoggableCF
 * @authors Israfil Argos
 * @brief Create a loggable via the format string provided. The severity is
 * by default that of a normal log.
 * @since 0.0.2
 *
 * @param title The title of the log. This is not copied from the provided
 * string.
 * @param description The description of the log. This is copied from the
 * provided string.
 * @param context_format The format string for the log's context.
 * @param ... Arguments to be substituted in the context format string.
 * @returns A loggable object.
 */
[[gnu::format(printf, 3, 4)]] [[gnu::nonnull(1, 2, 3)]] [[nodiscard(
    "Expression result unused.")]]
ir_loggable_t Ir_CreateLoggableCF(const char *title,
                                  const char *description,
                                  const char *context_format, ...);

/**
 * @name DestroyLoggable
 * @authors Israfil Argos
 * @brief Destroy a loggable object. This will erase everything within the
 * provided loggable.
 *
 * @param loggable The loggable to be destroyed.
 */
[[gnu::nonnull(1)]]
void Ir_DestroyLoggable(ir_loggable_t *loggable);

/**
 * @name Log_
 * @authors Israfil Argos
 * @brief Log something to its proper output (internal).
 * @since 0.0.1
 *
 * @warning Should the severity of the passed loggable be resolved to
 * panic (or should the fatality level be cranked up), this function will
 * abort the thread and generate a core dump.
 *
 * @param object The loggable item.
 * @param file The file this log came from.
 * @param function The function this log came from.
 * @param line The line this log came from.
 */
[[gnu::nonnull(1, 2, 3)]] [[gnu::hot]]
void Ir_Log_(ir_loggable_t *object, const char *file, const char *function,
             uint32_t line);

/**
 * @name Log
 * @authors Israfil Argos
 * @brief Log something to its proper output.
 * @since 0.0.1
 *
 * @warning Should the severity of the passed loggable be resolved to
 * panic (or should the fatality level be cranked up), this function will
 * abort the thread and generate a core dump.
 *
 * @param object The loggable item.
 */
#define Ir_Log(object) Ir_Log_(object, FILENAME, __func__, __LINE__)

#endif // IRIDIUM_LOGGING_H
