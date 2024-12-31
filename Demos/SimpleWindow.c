#include <Errors.h>
#include <Iridium.h>

int main(void)
{
    Ir_CatchProblems("main");

    Ir_SetProblemFatality(ir_all_problems);
    Ir_ReportProblem(ir_failed_wayland_connection, ir_override_warning,
                     "hello world");
    Ir_ReleaseProblems("main");
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
