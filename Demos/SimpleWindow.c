#include <Errors.h>
#include <Iridium.h>
#include <Logging.h>

int main(void)
{
    Ir_SetLogOutputS("./log");
    Ir_ReportProblem(ir_unexpected_param, ir_override_infer, "helloooo");

    return 0;
}
