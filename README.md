# sim8086

sim8086 is an Intel 8086 instruction decoder, disassembler, and partial CPU simulator written in C++20.

## Current support

Decoding and execution have different levels of support. The instruction table is in [InstructionTable.inl](sim8086/sim8086/src/InstructionTable.inl), and execution dispatch is in [Sim8086.cpp](sim8086/sim8086/src/Sim8086.cpp).

| Instruction group | Decoding / disassembly | Execution |
| --- | --- | --- |
| Data movement | `MOV`, `XCHG` | `MOV` with general-purpose registers, immediates, and memory |
| Arithmetic | `ADD`, `ADC`, `SUB`, `SBB`, `CMP`, `INC`, `DEC`, `NEG` | `ADD`, `ADC`, `SUB`, `SBB`, `CMP` |
| Stack | `PUSH`, `POP` | Not implemented |
| I/O | `IN`, `OUT` | Not implemented |
| Jumps | `JMP` and conditional jumps | Relative `JMP`, `JZ`, `JNZ`, `JG`, `JGE`, `JL`, `JNG` |
| Loops | `LOOP`, `LOOPZ`, `LOOPNZ`, `JCXZ` | `LOOP`, `LOOPZ` |

Support is partial: an instruction's presence in the table does not guarantee that every encoding or operand form is implemented. The table also contains `RET` entries, but return support is incomplete and there is no execution handler.

Arithmetic execution supports word and byte operands, including high/low byte registers, and computes Overflow, Sign, Zero, and Carry flags. `ADC` and `SBB` consume the incoming Carry flag; `CMP` updates flags without writing the subtraction result.

Memory operands use an `EAType` enum and displacement shared by decoding, disassembly, and execution. Execution handles eight register-based addressing forms (`BX+SI`, `BX+DI`, `BP+SI`, `BP+DI`, `SI`, `DI`, `BP`, `BX`) plus direct addressing, with 8-/16-bit displacement decoding. Existing memory execution tests cover direct addressing, `BX+SI`, and `BX+DI`; they do not exhaustively validate every form.

### Current limitations

- Effective-address calculation always selects `DS`, including BP-based forms; default `SS` selection and segment overrides are not implemented.
- Segment-register `MOV` is not decoded or executed.
- Indirect and far/inter-segment jumps are not fully supported by execution.
- Arithmetic flag handling is incomplete; Auxiliary Carry and Parity are not computed.
- Decoded instructions without execution handlers are skipped rather than reported as unsupported.
- Instructions such as `MUL`, `DIV`, `LEA`, `XLAT`, `INT`, and `CALL` remain unimplemented.
- CLI argument validation, output handling, and configurable tracing/memory dumps still need work.

## Repository layout

This is a nested CMake project. There is no `CMakeLists.txt` at the repository root.

- [sim8086/CMakeLists.txt](sim8086/CMakeLists.txt) — top-level CMake project
- [sim8086/sim8086/CMakeLists.txt](sim8086/sim8086/CMakeLists.txt) — executable targets and test registration
- [sim8086/sim8086/src](sim8086/sim8086/src) — implementation and instruction table
- [sim8086/sim8086/tests](sim8086/sim8086/tests) — unit tests, assembly/binary fixtures, and disassembly round-trip scripts

## Build

Requires CMake 3.10 or newer and a C++20 compiler. From the repository root:

```bash
cmake -S sim8086 -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j2
```

With a single-configuration generator, the executables are:

- `build/sim8086/sim8086` — simulator CLI
- `build/sim8086/sim_tests` — unit test runner

## Run the simulator

Use an explicit mode, input binary, and output filename:

```text
sim8086 -d <input.bin> <output.asm>
sim8086 -e <input.bin> <output.asm>
```

The current parser recognizes `-e` for execution and treats other mode strings as disassembly. Always supply all three arguments and use an output path distinct from the input: the output file is opened for writing before the input is loaded.

From the repository root:

```bash
# Disassemble to a file
./build/sim8086/sim8086 -d ./sim8086/sim8086/tests/decodeTests_JMP/test_jmp.bin ./disassembly.asm

# Execute (see the memory-dump prerequisite below)
./build/sim8086/sim8086 -e ./sim8086/sim8086/tests/test_mov_exe.bin ./execution.asm
```

Disassembly writes a NASM-style listing with a `bits 16` header. Relative jump targets use `$` expressions relative to the start of the instruction, rather than generated labels.

Execution tracing calls are currently commented out, so execution does not print per-instruction register or flag traces. The output assembly file is still required by the current CLI, but contains only the header in execution mode.

**Before executing on another checkout:** update `MemoryFile` in [IO.cpp](sim8086/sim8086/src/IO.cpp) to a writable destination and rebuild. Execution writes a 1 MiB memory dump there; the current value is a hard-coded developer path ending in `tests/memory_out.data`. Failed dump-file creation is not handled safely yet.

## Test

Run the unit test executable directly to see individual results:

```bash
./build/sim8086/sim_tests
```

CTest is registered in the subproject:

```bash
ctest --test-dir build/sim8086 --verbose
```

The runner currently returns zero even when assertions fail. Inspect the printed `FAILED` lines and suite totals; CTest's exit status alone does not establish that the assertions passed.

[TestMain.cpp](sim8086/sim8086/tests/TestMain.cpp) runs 99 tests across 16 suites: decoding; `MOV`, `ADD`, `ADC`, `SUB`, `SBB`, and `CMP`; `JNZ`, `JZ`, `JMP`, `JG`, `JGE`, `JL`, and `JNG`; and `LOOP`/`LOOPZ`.

Coverage includes register/byte operations, arithmetic flags, selected memory operands, and taken/not-taken branches. Dedicated immediate and memory execution tests for `ADD`/`ADC`, comprehensive effective-address cases, and complete segmentation behavior remain gaps.

The `decodeTests_*` directories also contain NASM-based disassembly round-trip scripts. These are separate from the CTest unit-test target; check [globals.sh](sim8086/sim8086/tests/globals.sh) for the simulator path before running them.

## Generate sample binaries

NASM is needed to assemble fixtures or run the disassembly round-trip scripts, but is not required to build the C++ targets.

From the repository root:

```bash
nasm ./sim8086/sim8086/tests/decodeTests_JMP/test_jmp.asm -o ./sim8086/sim8086/tests/decodeTests_JMP/test_jmp.bin
```

The repository already includes sample `.asm` and `.bin` files under [tests](sim8086/sim8086/tests).

## Notes

This project is useful for exploring 8086 instruction encoding, disassembling raw machine code, and experimenting with a partial execution engine. It is not a complete 8086 emulator.

Contributions are not accepted at this time. However, anyone is welcome to fork this repository and use it as they please under the terms of the repository's existing license.

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
