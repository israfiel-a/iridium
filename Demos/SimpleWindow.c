#include <Iridium.h>
#include <Logging.h>

int main(void)
{
    ir_loggable_t log_object = {.severity = success,
                                .title = "Log Test",
                                .description = "logloglog",
                                .context = "hello context"};
    Ir_Log(&log_object);

    return 0;
}
