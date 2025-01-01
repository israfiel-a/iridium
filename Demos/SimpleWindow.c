#include <Errors.h>
#include <Iridium.h>

int main(void)
{
    Ir_SetMaxProblems(5);
    Ir_ReportProblem(ir_failed_allocation, ir_override_error, nullptr);
    Ir_ReportProblem(ir_failed_allocation, ir_override_error, nullptr);
    Ir_ReportProblem(ir_failed_allocation, ir_override_error, nullptr);
    Ir_ReportProblem(ir_failed_allocation, ir_override_error, nullptr);
    Ir_ReportProblem(ir_failed_allocation, ir_override_error, nullptr);
    Ir_ReportProblem(ir_failed_allocation, ir_override_error, nullptr);
    Ir_ReportProblem(ir_failed_allocation, ir_override_error, nullptr);
    Ir_ReportProblem(ir_failed_allocation, ir_override_error, nullptr);
    Ir_ReportProblem(ir_failed_allocation, ir_override_error, nullptr);
    Ir_ReportProblem(ir_failed_allocation, ir_override_error, nullptr);
    Ir_ReportProblem(ir_failed_allocation, ir_override_error, nullptr);
    ir_problem_t problem;
    Ir_PullProblem(0, &problem);
    return 0;
}
