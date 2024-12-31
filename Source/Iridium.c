#include <Iridium.h>
#ifdef IRIDIUM_WAYLAND
    #include "Internal/Wayland.h"
#endif

bool Ir_StartEngine(ir_application_info_t *application_info)
{
#ifdef IRIDIUM_WAYLAND
    Ir_WaylandConnect();
#endif
    return true;
}

void Ir_EndEngine(void) {}
