#include <Errors.hpp>
#include <iostream>

int main()
{
    Iridium::Errors::Push(Iridium::Errors::failed_null_assertion);
    std::cout << Iridium::Errors::Pull().Stringify() << std::endl;
}
