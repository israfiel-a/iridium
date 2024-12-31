/**
 * @file Iridium.h
 * @authors Israfil Argos (israfiel-a)
 * @brief This file provides the base of the Iridium API. That includes the
 * vital engine startup functions, alongside some methods for grabbing
 * metadata.
 *
 * Copyright (c) 2024 the Iridium Development Team
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */
#ifndef IRIDIUM_IRIDIUM_H
#define IRIDIUM_IRIDIUM_H

#include <stdint.h>

/**
 * @name version
 * @brief A merged 32-bit version identifier. Can be created via the
 * MakeVersion function and exploded via the SplitVersion function.
 */
typedef uint32_t ir_version_t;

/**
 * @name application_info
 * @brief An application's information structure. This is passed to the
 * engine ignition function for metadata purposes.
 */
typedef struct application_info
{
    /**
     * @brief The name of the application. This will be copied into
     * an internal engine buffer. This is set to the title of the window
     * when created.
     */
    const char *name;
    /**
     * @brief The application's version. This is used purely for metadata
     * purposes.
     */
    ir_version_t version;
} ir_application_info_t;

/**
 * @name MakeVersion
 * @authors Israfil Argos
 * @brief Create a 32-bit version identifier from a minor, major, and tweak
 * version identifier.
 * @since 0.1.0
 *
 * @param major The major version identifier.
 * @param minor The minor version identifier.
 * @param tweak The tweak version identifier.
 * @returns The merged version number.
 */
[[nodiscard("Expression result unused.")]]
__always_inline ir_version_t Ir_MakeVersion(uint8_t major, uint8_t minor,
                                            uint8_t tweak)
{
    return (ir_version_t)major << 22U | (ir_version_t)minor << 12U |
           (ir_version_t)tweak;
}

[[nodiscard("Failure of this function is catastrophic.")]]
bool Ir_StartEngine(ir_application_info_t *application_info);

/**
 * @name EndEngine
 * @authors Israfil Argos
 * @brief End all engine processes and de-allocate all resources.
 * @since 0.1.0
 */
void Ir_EndEngine(void);

#endif // IRIDIUM_IRIDIUM_H
