# sim8086

sim8086 is a small 8086 instruction decoder and disassembler written in modern C++. It loads a binary program image, walks the instruction stream, and prints a readable assembly-style listing for supported opcodes.

## What it does

The project currently focuses on decoding and disassembly of a subset of 8086 instructions, including:

- Data movement: `MOV`
- Arithmetic: `ADD`, `ADC`, `SUB`, `SBB`, `CMP`, `DEC`, `NEG`, `INC`
- Stack: `PUSH`, `POP`
- Control flow: `JMP` and common conditional jumps such as `JZ`, `JNZ`, `JGE`, `JNG`, `JA`, `JNA`, `JO`, `JNO`, `JS`, `JPE`, and related variants

The implementation is driven by an instruction table in [sim8086/src/InstructionTable.inl](sim8086/src/InstructionTable.inl), and the main entry point is [sim8086/src/Main.cpp](sim8086/src/Main.cpp).

> The current codebase is primarily a decoder/disassembler. The execution path is still a work in progress, and the `Execute` routine remains a stub. Full instruction support for disassembly is still being expanded.

## Current support status

### Implemented / supported today

- [x] `MOV`
- [x] `ADD`
- [x] `ADC`
- [x] `SUB`
- [x] `SBB`
- [x] `CMP`
- [x] `INC`
- [x] `DEC`
- [x] `NEG`
- [x] `PUSH`
- [x] `POP`
- [x] `JMP`
- [x] Common conditional jumps such as `JZ`, `JNZ`, `JGE`, `JNG`, `JA`, `JNA`, `JO`, `JNO`, `JS`, `JPE`, and related variants

### Planned / not yet fully supported

- [ ] Additional 8086 instructions such as `MUL`, `DIV`, `XCHG`, `LEA`, `XLAT`, `INT`, `CALL`, `RET`, `LOOP`, and `LOOPE`/`LOOPNE`
- [ ] Full coverage for all memory addressing forms and edge-case encodings
- [ ] More complete handling of segment register, far-jump, and inter-segment behaviors
- [ ] Execution-stage support and runtime emulation, beyond disassembly

## Repository layout

- [CMakeLists.txt](CMakeLists.txt) — top-level CMake project file
- [sim8086/CMakeLists.txt](sim8086/CMakeLists.txt) — subproject build rules and test registration
- [sim8086/src](sim8086/src) — implementation and instruction table
- [sim8086/tests](sim8086/tests) — sample assembly and binary fixtures plus the unit test entry point

## Build

This project uses CMake.

From the repository root, configure and build it with:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j2
```

The verified build output in this workspace produces:

- [build/sim8086/sim8086](build/sim8086/sim8086)
- [build/sim8086/sim_tests](build/sim8086/sim_tests)

## Run the disassembler

The simulator expects a binary file as input:

```bash
./build/sim8086/sim8086 ./sim8086/tests/test_jmp.bin
```

Example output includes a disassembled listing such as:

```asm
L3:
	JMP L1
	ADD CX, 18
L1:
	MOV BX, CX
	JMP L2
```

## Test

The project registers a CTest target for the simulator test executable.

Run the tests with:

```bash
ctest --test-dir build/sim8086 --output-on-failure
```

In the current workspace, this reported:

- 1 test run
- 1 passed
- 0 failed

## Generate sample binaries

If you want to create your own binaries for testing, you can assemble `.asm` files with NASM. For example:

```bash
cd sim8086/tests
nasm test_jmp.asm -o test_jmp.bin
```

The repository already contains sample `.asm` and `.bin` files under [sim8086/tests](sim8086/tests).

## Notes

This project is best viewed as a decoder/disassembler prototype. It is useful for exploring 8086 instruction encoding and for generating readable assembly listings from raw machine code bytes.

Contributions are not accepted at this time. However, anyone is welcome to fork this repository and use it as they please under the terms of the repository’s existing license.

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
