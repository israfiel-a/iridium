## Iridium
Iridium is a C++20-based game engine. It is built specifically to fit the needs of the Leto project. This project is under the AGPLv3, meaning you are free to do whatever you wish with it and its source code--with exceptions outlined in [the license](./LICENSE.md).

### Dependencies
Iridium is built to be as dependency-light as physically possible. As of right now, the only thing you need is to be running in a supported desktop environment with the C++ standard library installed on your system. However, Iridium requires the [CMake toolchain](https://cmake.org/) to build properly, but pre-compiled binaries do not have that extra caveat.

### Compatibility
Iridium is built from the ground up to rely on as few platform-specific piecees of code as possible. Because of this, it is compatible with a wide range of systems, including [Wayland](https://wayland.freedesktop.org/) Linux and Windows 10 or newer (although Windows 7 is *possible*, it will take some work). However, support for more niche systems is never a bad thing, and I'm fully open to talking about something like [Haiku](https://www.haiku-os.org/).

> Note: Because it is so early in the development process, the above section is a lie. Iridium only supports Wayland Linux.