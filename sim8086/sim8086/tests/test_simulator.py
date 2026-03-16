#!/usr/bin/env python3

import filecmp
import subprocess
import sys
import unittest
from pathlib import Path


class TestDecoder(unittest.TestCase):
    def test_mov_mem_to_acc(self):
        # Arrange test files
        simulatorPath = "/Users/joey/Projects/sim-8086/sim8086/out/build/macos-debug/sim8086/Debug/sim8086"
        currentDir = Path(__file__).parent
        testFile = f"{currentDir}/mem_to_acc.bin"
        resultFile = f"{currentDir}/result.asm"

        # Execute the simulator
        subprocess.run([simulatorPath, testFile], check=True)

        # Read the test binary files into a byte array
        with open(testFile, mode="rb") as file:
            testFileBinary = file.read()
            testFileBinArr = bytearray(testFileBinary)

        # Assembly result file and read into binary array
        resultBinary = f"{currentDir}/result.bin"
        subprocess.run(["nasm", resultFile, "-o", resultBinary])
        with open(resultBinary, mode="rb") as file:
            resultFileBinary = file.read()
            resultFileBinArr = bytearray(resultFileBinary)

        # Compare binary files and assert they are equal
        self.assertEqual(testFileBinArr, resultFileBinArr)


if __name__ == "__main__":
    unittest.main()
