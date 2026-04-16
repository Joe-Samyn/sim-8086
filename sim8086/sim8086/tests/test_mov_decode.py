#!/usr/bin/env python3
from constants import *
from test_utils import readFileIntoByteArr

import subprocess
import unittest



class TestDecodingMov(unittest.TestCase):
    def test_mem_to_acc(self):
        '''Test simulator can properly decode memory to accumulator moves.'''
        # Arrange test files
        testFile = f"{TEST_BINARIES_DIR}/mem_to_acc.bin"

        # Execute the simulator
        subprocess.run([SIMULATOR_PATH, testFile], check=True)

        # Read the test binary files into a byte array
        testFileBinArr = readFileIntoByteArr(testFile)

        # Assembly result file and read into binary array
        subprocess.run(["nasm", SIM_OUTPUT_ASM, "-o", SIM_OUTPUT_BIN])
        resultFileBinArr = readFileIntoByteArr(SIM_OUTPUT_BIN)

        # Compare binary files and assert they are equal
        self.assertEqual(testFileBinArr, resultFileBinArr)

    def test_acc_to_mem(self):
        '''Test simulator can properly decode accumulator to memory moves.'''
        # Arrange test files
        testFile = f"{TEST_BINARIES_DIR}/acc_to_mem.bin"

        # Execute the simulator
        subprocess.run([SIMULATOR_PATH, testFile], check=True)

        # Read the test binary files into a byte array
        testFileBinArr = readFileIntoByteArr(testFile)

        # Assembly result file and read into binary array
        subprocess.run(["nasm", SIM_OUTPUT_ASM, "-o", SIM_OUTPUT_BIN])
        resultFileBinArr = readFileIntoByteArr(SIM_OUTPUT_BIN)

        # Compare binary files and assert they are equal
        self.assertEqual(testFileBinArr, resultFileBinArr)
    
    def test_reg_mem_to_from_reg(self):
        '''Test simulator can properly decode register/memory to/from register moves.'''
        # Arrange test files
        testFile = f"{TEST_BINARIES_DIR}/reg_mem_to_from_reg.bin"

        # Execute the simulator
        subprocess.run([SIMULATOR_PATH, testFile], check=True)

        # Read the test binary files into a byte array
        testFileBinArr = readFileIntoByteArr(testFile)

        # Assembly result file and read into binary array
        subprocess.run(["nasm", SIM_OUTPUT_ASM, "-o", SIM_OUTPUT_BIN])
        resultFileBinArr = readFileIntoByteArr(SIM_OUTPUT_BIN)

        # Compare binary files and assert they are equal
        self.assertEqual(testFileBinArr, resultFileBinArr)

    def test_imm_to_reg_mem(self):
        '''Test similator can properly decode immediate to register/memory moves.'''

        # Arrange test files
        testFile = f"{TEST_BINARIES_DIR}/imm_to_reg_mem.bin"

        # Execute the simulator
        subprocess.run([SIMULATOR_PATH, testFile], check=True)

        # Read the test binary files into a byte array
        testFileBinArr = readFileIntoByteArr(testFile)

        # Assembly result file and read into binary array
        subprocess.run(["nasm", SIM_OUTPUT_ASM, "-o", SIM_OUTPUT_BIN])
        resultFileBinArr = readFileIntoByteArr(SIM_OUTPUT_BIN)

        # Compare binary files and assert they are equal
        self.assertEqual(testFileBinArr, resultFileBinArr)

    def test_all_mov_instructions(self):
        '''Test simulator can decode all move instructions aggregated together in a single binary.'''

        '''This is essentially the final verification that the simulator can differentiate and decode
        all move instructions properly from a single binary file.'''
        # Arrange test files
        testFile = f"{TEST_BINARIES_DIR}/mov_tst.bin"

        # Execute the simulator
        subprocess.run([SIMULATOR_PATH, testFile], check=True)

        # Read the test binary files into a byte array
        testFileBinArr = readFileIntoByteArr(testFile)

        # Assembly result file and read into binary array
        subprocess.run(["nasm", SIM_OUTPUT_ASM, "-o", SIM_OUTPUT_BIN])
        resultFileBinArr = readFileIntoByteArr(SIM_OUTPUT_BIN)

        # Compare binary files and assert they are equal
        self.assertEqual(testFileBinArr, resultFileBinArr)
    

if __name__ == "__main__":
    unittest.main()
