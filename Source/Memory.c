/**
 * @file Memory.c
 * @authors Israfil Argos (israfiel-a)
 * @brief This file implements Iridium's minimal overhead memory
 * abstractions.
 * @since 0.0.2
 *
 * @copyright (c) 2024 the Iridium Development Team
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */

#include <stdlib.h>

#include <Errors.h>
#include <Memory.h>

void *Ir_Malloc(size_t size)
{
    void *temp = malloc(size);
    if (temp == NULL) abort();
    return temp;
}

void *Ir_Calloc(size_t size)
{
    void *temp = calloc(1, size);
    if (temp == NULL) abort();
    return temp;
}

void Ir_Realloc(void **ptr, size_t size)
{
    void *temp = realloc(*ptr, size);
    if (temp == NULL) abort();
    *ptr = temp;
}

void Ir_Free(void **ptr)
{
    if (*ptr == nullptr)
    {
        Ir_ReportProblem(ir_unexpected_param, ir_override_infer,
                         "ptr = null");
        return;
    }

    free(*ptr);
    *ptr = nullptr;
}
