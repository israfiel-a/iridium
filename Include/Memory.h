/**
 * @file Memory.h
 * @authors Israfil Argos (israfiel-a)
 * @brief This file provides Iridium's very small, minimal overhead and
 * error-checked wrappers around memory operations.
 * @since 0.0.2
 *
 * @copyright (c) 2024 the Iridium Development Team
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */
#ifndef IRIDIUM_MEMORY_H
#define IRIDIUM_MEMORY_H

#include <stddef.h>

/**
 * @name Malloc
 * @authors Israfil Argos
 * @brief A small wrapper around malloc that fails the application on
 * allocation failure. This failure is not required to be noted in any
 * engine functions--the application cannot continue after running out of
 * memory.
 * @since 0.0.2
 *
 * @param size The size of the block to allocate.
 * @returns The allocated block.
 */
[[gnu::hot]] [[gnu::malloc]] [[gnu::alloc_size(
    1)]] [[gnu::returns_nonnull]]
void *Ir_Malloc(size_t size);

/**
 * @name Calloc
 * @authors Israfil Argos
 * @brief A small wrapper around calloc that fails the application on
 * allocation failure. This failure is not required to be noted in any
 * engine functions--the application cannot continue after running out of
 * memory.
 * @since 0.0.3
 *
 * @param size The size of the block to allocate.
 * @returns The allocated block.
 */
[[gnu::hot]] [[gnu::malloc]] [[gnu::alloc_size(
    1)]] [[gnu::returns_nonnull]]
void *Ir_Calloc(size_t size);

/**
 * @name Realloc
 * @authors Israfil Argos
 * @brief A small wrapper around realloc that fails the application on
 * allocation failure. This failure is not required to be noted in any
 * engine functions--the application cannot continue after running out of
 * memory
 * @since 0.0.2
 *
 * @note The passed pointer is changed.
 *
 * @param ptr A pointer to the pointer that you're trying to reallocate.
 * This can be null--this function, like realloc, also acts as a regular
 * allocator.
 * @param size The new size of the block to be allocated.
 */
[[gnu::hot]] [[gnu::nonnull(1)]]
void Ir_Realloc(void **ptr, size_t size);

/**
 * @name Free
 * @authors Israfil Argos
 * @brief A small wrapper around the free function. Only pass a value to
 * this function if you're completely sure it's been dynamically allocated.
 * @since 0.0.2
 *
 * @warning Should the pointer that the passed pointer leads to be null, an
 * ir_unexpected_param warning will be thrown, and the passed pointer will
 * not be changed.
 * @note The passed pointer is set to nullptr.
 *
 * @param ptr A pointer to the pointer that you're trying to free.
 */
[[gnu::hot]] [[gnu::nonnull(1)]]
void Ir_Free(void **ptr);

#endif // IRIDIUM_MEMORY_H
