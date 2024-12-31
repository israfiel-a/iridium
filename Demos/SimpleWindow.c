#include <Errors.h>
#include <Iridium.h>

int main(void)
{
    Ir_ReportProblem(ir_failed_wayland_connection, ir_override_warning,
                     "hello world");
    Ir_ReportProblem(ir_failed_wayland_connection, ir_override_infer,
                     NULL);
    Ir_ReportProblem(ir_failed_wayland_connection, ir_override_infer,
                     NULL);
    Ir_ReportProblem(ir_failed_wayland_connection, ir_override_infer,
                     NULL);
    Ir_ReportProblem(ir_failed_wayland_connection, ir_override_infer,
                     NULL);
    Ir_ReportProblem(ir_failed_wayland_connection, ir_override_infer,
                     NULL);
    Ir_ReportProblem(ir_failed_wayland_connection, ir_override_infer,
                     NULL);
    Ir_ReportProblem(ir_failed_wayland_connection, ir_override_infer,
                     NULL);

    return 0;
}
