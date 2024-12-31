/**
 * @file Logging.h
 * @authors Israfil Argos (israfiel-a)
 * @brief This file provides the logging API for the Iridium engine. This
 * includes an abstraction from stdout in case the user wants to pipe
 * output to a log file.
 *
 * Copyright (c) 2024 the Iridium Development Team
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */
#ifndef IRIDIUM_LOGGING_H
#define IRIDIUM_LOGGING_H

#include <stdint.h>
#include <stdio.h>

typedef FILE *ir_output_t;

typedef enum ir_severity
{
    ir_success,
    ir_log,
    ir_warning,
    ir_error,
    /**
     * @brief document panic-throwers
     */
    ir_panic
} ir_severity_t;

typedef struct ir_loggable
{
    ir_severity_t severity;
    const char *title;
    const char *description;
    const char *context;
} ir_loggable_t;

void Ir_Log_(ir_loggable_t *object, const char *file, const char *function,
             uint32_t line);

#define Ir_Log(object) Ir_Log_(object, FILENAME, __func__, __LINE__)

/**
 * @name SilenceLogs
 * @authors Israfil Argos
 * @brief Silence all logs that aren't warnings, errors, or panics. This is
 * reccomended for production builds, as it can increase performance
 * substantially on some systems.
 *
 * @param silence The new silence flag. False for off, true for on.
 */
void Ir_SilenceLogs(bool silence);

void Ir_SetLogOutput(ir_output_t output);

void Ir_SetErrorOutput(ir_output_t output);

void Ir_SetPanicOutput(ir_output_t output);

const ir_output_t *Ir_GetLogOutput(void);

const ir_output_t *Ir_GetErrorOutput(void);

const ir_output_t *Ir_GetPanicOutput(void);

#endif // IRIDIUM_LOGGING_H
