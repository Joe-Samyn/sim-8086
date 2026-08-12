# sim8086

sim8086 is a small 8086 instruction decoder, disassembler, and (partial) simulator written in modern C++. It loads a binary program image, walks the instruction stream, and either prints a readable assembly-style listing or executes the instructions against an in-memory CPU/register model.

## What it does

The project currently focuses on decoding and disassembly of a subset of 8086 instructions, including:

- Data movement: `MOV`
- Arithmetic: `ADD`, `ADC`, `SUB`, `SBB`, `CMP`, `DEC`, `NEG`, `INC`
- Stack: `PUSH`, `POP`
- Control flow: `JMP` and common conditional jumps such as `JZ`, `JNZ`, `JGE`, `JNG`, `JA`, `JNA`, `JO`, `JNO`, `JS`, `JPE`, and related variants

The implementation is driven by an instruction table in [InstructionTable.inl](sim8086/sim8086/src/InstructionTable.inl), and the main entry point is [Main.cpp](sim8086/sim8086/src/Main.cpp).

> Disassembly covers the instructions listed above. Execution is newer and narrower: right now only `MOV` (immediate-to-register and register-to-register, including 8-bit high/low register halves) actually updates CPU state. Every other decoded instruction is still execution-stub-only.

## Current support status

### Implemented / supported today

- [x] Disassembly: `MOV`, `ADD`, `ADC`, `SUB`, `SBB`, `CMP`, `INC`, `DEC`, `NEG`, `PUSH`, `POP`, `JMP`, and common conditional jumps (`JZ`, `JNZ`, `JGE`, `JNG`, `JA`, `JNA`, `JO`, `JNO`, `JS`, `JPE`, and related variants)
- [x] Execution: `MOV` immediate-to-register
- [x] Execution: `MOV` register-to-register, including 8-bit high/low byte register access (e.g. `BH`, `BL`)
- [x] CLI `-e` flag to run a loaded program instead of disassembling it, with before/after register-state dumps per instruction

### Planned / not yet fully supported

- [ ] Execution support for instructions beyond `MOV` (`ADD`, `SUB`, `CMP`, `PUSH`/`POP`, jumps, etc.)
- [ ] Execution of memory-operand `MOV`s (effective-address calculation is decoded but not yet read/written during execution)
- [ ] Additional 8086 instructions such as `MUL`, `DIV`, `XCHG`, `LEA`, `XLAT`, `INT`, `CALL`, `RET`, `LOOP`, and `LOOPE`/`LOOPNE`
- [ ] Full coverage for all memory addressing forms and edge-case encodings
- [ ] More complete handling of segment register, far-jump, and inter-segment behaviors

## Repository layout

This is a nested CMake project (two `sim8086` directories deep) — worth knowing before the build section below:

- [sim8086/CMakeLists.txt](sim8086/CMakeLists.txt) — top-level CMake project file (there is no `CMakeLists.txt` at the repo root)
- [sim8086/sim8086/CMakeLists.txt](sim8086/sim8086/CMakeLists.txt) — subproject build rules and test registration
- [sim8086/sim8086/src](sim8086/sim8086/src) — implementation and instruction table
- [sim8086/sim8086/tests](sim8086/sim8086/tests) — sample assembly and binary fixtures plus the unit test entry point

## Build

This project uses CMake. The top-level `CMakeLists.txt` lives in `sim8086/`, not the repo root, so point `-S` there:

```bash
cmake -S sim8086 -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j2
```

The verified build output in this workspace produces:

- [build/sim8086/sim8086](build/sim8086/sim8086) — the simulator CLI
- [build/sim8086/sim_tests](build/sim8086/sim_tests) — the unit test binary

## Run the simulator

The CLI expects a binary file as input, and an optional `-e` flag to execute instead of disassemble:

```bash
# Disassemble (default)
./build/sim8086/sim8086 ./sim8086/sim8086/tests/test_jmp.bin

# Execute
./build/sim8086/sim8086 -e ./sim8086/sim8086/tests/test_mov_exe.bin
```

Disassembly produces a listing such as:

```asm
	JMP $+2
	MOV BX, CX
	JMP $+2
	MOV CX, DX
	JMP $-8
```

Jump targets are printed using the `$` syntax (offset relative to the start of the jump instruction itself) rather than as generated labels.

Execution prints the register state before and after each instruction as it runs, e.g.:

```
Register State
AX   0x0000
BX   0x0000
...

MOV BX, 1024
BX <-- 0x0400

Register State
AX   0x0000
BX   0x0400
...
```

## Test

The project registers a CTest target for the simulator test executable.

Run the tests with:

```bash
ctest --test-dir build/sim8086 --output-on-failure
```

> Note: the unit tests in [test_main.cpp](sim8086/sim8086/tests/test_main.cpp) are currently commented out pending a fix, so this reports 1/1 passed vacuously — the test binary isn't yet asserting anything.

## Generate sample binaries

If you want to create your own binaries for testing, you can assemble `.asm` files with NASM. For example:

```bash
cd sim8086/sim8086/tests
nasm test_jmp.asm -o test_jmp.bin
```

The repository already contains sample `.asm` and `.bin` files under [sim8086/sim8086/tests](sim8086/sim8086/tests).

## Notes

This project is best viewed as a decoder/disassembler with an emerging execution engine. It is useful for exploring 8086 instruction encoding, generating readable assembly listings from raw machine code bytes, and (for `MOV`) tracing register-level program state.

Contributions are not accepted at this time. However, anyone is welcome to fork this repository and use it as they please under the terms of the repository’s existing license.

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
