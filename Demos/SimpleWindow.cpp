#include <Files.hpp>
#include <Ignition.hpp>

int main()
{
    Iridium::File opened_file("cat.jpeg");
    opened_file.Hexdump();
}
