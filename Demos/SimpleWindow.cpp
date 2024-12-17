#include <Ignition.hpp>
#include <iostream>

int main()
{
    std::cout << Iridium::Engine::Get().GetVersion().Stringify()
              << std::endl;
}