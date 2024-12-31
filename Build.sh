# Tiny build script for the Iridium game engine.
# Provides options for debugging, build mode, and more.
#!/bin/bash

if printf '%s\0' "$@" | grep -Fxqz -- '--help'; then
    echo Build script for the Iridium game engine.
    echo "Usage: ./Build.sh [options]"
    echo "Options:"
    echo "      --help:             Display help and exit."
    echo "      --type=[value]:     Specify the type of build to do."
    echo "                          Possible values: Debug, Release"
    echo "      --debugger=[value]: Specify the debugger to use."
    echo "                          Possible values: Valgrind, GDB"
    echo "      --static:           Build the library statically."
    echo "      --no-demo:          Do not build engine demos."
    echo "      --no-docs:          Do not build engine documentation."
    echo "      --verbose:          Show CMake output."
    echo "      --no-example:       Do not run the SimpleWindow example."
    exit 0
fi

echo Building the Iridium engine.
# Make sure we're in the Iridium directory.
script_directory=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $script_directory
echo Working directory: $script_directory

build_type=Release
if printf '%s\0' "$@" | grep -Fxqz -- '--type=Debug'; then
    build_type=Debug
fi
echo Building in $build_type mode.

# Flag for CMake so we don't add both Valgrind and ASAN to the same
# binary. They don't play nice together.
no_sanitize=false

debugger=None
if printf '%s\0' "$@" | grep -Fxqz -- '--debugger=Valgrind'; then
    debugger=Valgrind
    no_sanitize=true
elif printf '%s\0' "$@" | grep -Fxqz -- '--debugger=GDB'; then
    debugger=GDB
fi

if [ "$debugger" != "None" ]; then
    echo Debugging with $debugger.
fi

build_shared=true
if printf '%s\0' "$@" | grep -Fxqz -- '--static'; then
    build_shared=false
fi
echo Building dynamic library: $build_shared.

build_demos=true
if printf '%s\0' "$@" | grep -Fxqz -- '--no-demos'; then
    build_demos=false
fi
echo Building demos: $build_demos.

build_docs=true
if printf '%s\0' "$@" | grep -Fxqz -- '--no-docs'; then
    build_docs=false
fi
echo Building documentation: $build_docs.

verbose_output=false
if printf '%s\0' "$@" | grep -Fxqz -- '--verbose'; then
    verbose_output=true
    echo Enabling verbose output.
fi

if [ $verbose_output == "false" ]; then
    cmake -B build -DCMAKE_BUILD_TYPE=$build_type -DBUILD_SHARED_LIBS=$build_shared \
        -DIRIDIUM_BUILD_DEMOS=$build_demos -DIRIDIUM_NO_SANITIZE=$no_sanitize       \
        -DIRIDIUM_BUILD_DOCS=$build_docs > /dev/null || exit 255
else
    cmake -B build -DCMAKE_BUILD_TYPE=$build_type -DBUILD_SHARED_LIBS=$build_shared \
        -DIRIDIUM_BUILD_DEMOS=$build_demos -DIRIDIUM_NO_SANITIZE=$no_sanitize       \
        -DIRIDIUM_BUILD_DOCS=$build_docs || exit 255
fi

# Enter the build directory so we can actually build the project.
cd build

if [ $verbose_output == "false" ]; then
    make > /dev/null || exit 255
else
    make || exit 255
fi

if printf '%s\0' "$@" | grep -Fxqz -- '--no-example'; then
    exit 0
fi

# Enter the demo directory if we are to run SimpleWindow.
cd Iridium/Demos
if [ "$debugger" == "Valgrind" ]; then
    valgrind --tool=memcheck --leak-check=yes --track-origins=yes \
        --trace-children=yes ./SimpleWindow
elif [ "$debugger" == "GDB" ]; then
    gdb --annotate=3 SimpleWindow
else
    ./SimpleWindow
fi
