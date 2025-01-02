#ifndef IRIDIUM_MEMORY_H
#define IRIDIUM_MEMORY_H

#include <stdlib.h>

#include <Errors.h>

[[gnu::hot]] [[gnu::malloc]]
void *Ir_Malloc(size_t size);

[[gnu::hot]] [[gnu::nonnull(1)]]
void Ir_Realloc(void **ptr, size_t size);

// provided for null checking
[[gnu::hot]] [[gnu::nonnull(1)]]
void Ir_Free(void **ptr);

#endif // IRIDIUM_MEMORY_H
