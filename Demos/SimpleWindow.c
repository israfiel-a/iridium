#include <Errors.h>
#include <Iridium.h>
#include <Logging.h>

int main(void)
{
    Ir_ReportProblem(ir_failed_wayland_connection, ir_override_infer, "");

    return 0;
}
