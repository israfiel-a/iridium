#include <Logging.hpp>

namespace Iridium::Logging
{
    Interface::Interface(std::ostream &output_stream)
        : output(output_stream)
    {
        if (Closed()) return;
        output.flush();
    }
}
