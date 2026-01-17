# sim-8086

Intel 8086 instruction-set simulator used to follow along with Casey Muratori's "Computer Enhance" series.

## Overview

This small simulator implements core 8086 functionality to experiment with real assembly examples and to follow the lessons in the Computer Enhance series. It is intended as a learning tool and reference implementation; longer-term the project may be evolved into a production-quality 8086 emulator after completing the series.

## Status

- Minimal instruction decoding and execution implemented.
- Includes a few sample programs under `sim8086/res` for testing.

## Prerequisites

- CMake 3.15+
- A C++17-capable compiler (Visual Studio on Windows, or clang/gcc on other platforms)
- Ninja (optional, recommended for faster command-line builds)

## Building (Windows example)

Open a Developer Command Prompt or PowerShell and run:

```powershell
mkdir build
cd build
cmake .. -G "Ninja"    # or omit -G to use default Visual Studio generator
cmake --build . --config Release
```

After a successful build the executable will be located in the build output folder (for example `build\\sim8086.exe` or inside the Visual Studio `Debug`/`Release` subfolder).

## Running a sample program

There is a sample assembly file in `sim8086/res/single_reg_mov.asm`.

Run the simulator and point it at the sample file, for example:

```powershell
.\\path\\to\\built\\sim8086.exe ..\\sim8086\\res\\single_reg_mov.asm
```

Adjust the path to the built executable according to your generator and configuration.

## Project structure

- `sim8086/src/` — simulator sources (`main.cpp`, headers, decoder)
- `sim8086/res/` — sample assembly programs used for testing and demos
- `CMakeLists.txt` — top-level build configuration

See the source entry point at [sim8086/src/main.cpp](sim8086/src/main.cpp).

## Development notes

- The decoder and instruction implementations live under `sim8086/src` and `sim8086/` headers. Add small, focused test ASM files to `sim8086/res` when experimenting with a new instruction.
- Keep changes small and document any deviations from the series to make it easy to follow along.

## Contribution

Contributions are not welcome at this time. The project is being developed privately and will remain closed to external pull requests until further notice.

## Attribution

This repository is intended to follow Casey Muratori's Computer Enhance series; refer to that series for lesson guidance and sequencing.


