#include <Memory.h>

void *Ir_Malloc(size_t size)
{
    if (size == 0)
    {
        Ir_ReportProblem(ir_unexpected_param, ir_override_infer,
                         "size = 0");
        return nullptr;
    }
    void *temp = malloc(size);
    if (temp == NULL) abort();

    return temp;
}

void Ir_Realloc(void **ptr, size_t size)
{
    if (size == 0)
    {
        Ir_ReportProblem(ir_unexpected_param, ir_override_infer,
                         "size = 0");
        return;
    }
    if (*ptr == nullptr)
    {
        Ir_ReportProblem(ir_unexpected_param, ir_override_infer,
                         "ptr = null");
        return;
    }

    void *temp = realloc(*ptr, size);
    if (temp == NULL) abort();
    // ptr == temp :)
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
