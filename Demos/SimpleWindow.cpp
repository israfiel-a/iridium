#include <Logging.hpp>

int main()
{
    Iridium::Logging::RaiseError(Iridium::Logging::failed_null_assertion);
    Iridium::Logging::SuppressErrors();
    Iridium::Logging::RaiseError(Iridium::Logging::failed_null_assertion,
                                 Iridium::Logging::panic);
    Iridium::Logging::RaiseError(Iridium::Logging::failed_null_assertion);
    Iridium::Logging::SuppressErrors();
}
