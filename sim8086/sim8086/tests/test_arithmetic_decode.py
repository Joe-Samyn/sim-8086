#!/usr/bin/env python3

from test_utils import *
from constants import *

import unittest
import subprocess

class TestArithmeticDecode(unittest.TestCase):

    def test_add_imm_to_reg_mem(self):
        # Arrange 
        testFile = f'{TEST_BINARIES_DIR}/add_imm_to_reg_mem.bin'

        # Act
        runSim8086(SIMULATOR_PATH, testFile)

        assembleFile(SIM_OUTPUT_ASM, SIM_OUTPUT_BIN)

        resultFileBin = readFileIntoByteArr(SIM_OUTPUT_BIN)
        testFileBin = readFileIntoByteArr(testFile)

        # Assert
        self.assertEqual(resultFileBin, testFileBin)

    def test_adc_imm_to_reg_mem(self):
        # Arrange
        testFile = f'{TEST_BINARIES_DIR}/adc_imm_to_reg_mem.bin'

        # Act
        runSim8086(SIMULATOR_PATH, testFile)

        assembleFile(SIM_OUTPUT_ASM, SIM_OUTPUT_BIN)

        resultFileBin = readFileIntoByteArr(SIM_OUTPUT_BIN)
        testFileBin = readFileIntoByteArr(testFile)

        # Assert
        self.assertEqual(resultFileBin, testFileBin)

    def test_add_reg_mem_w_reg(self):
         # Arrange
        testFile = f'{TEST_BINARIES_DIR}/add_reg_mem_w_reg.bin'

        # Act
        runSim8086(SIMULATOR_PATH, testFile)

        assembleFile(SIM_OUTPUT_ASM, SIM_OUTPUT_BIN)

        resultFileBin = readFileIntoByteArr(SIM_OUTPUT_BIN)
        testFileBin = readFileIntoByteArr(testFile)

        # Assert
        self.assertEqual(resultFileBin, testFileBin)

    def test_add_imm_to_acc(self):
        # Arrange
        testFile = f'{TEST_BINARIES_DIR}/add_imm_to_acc.bin'

        # Act
        runSim8086(SIMULATOR_PATH, testFile)

        assembleFile(SIM_OUTPUT_ASM, SIM_OUTPUT_BIN)

        resultFileBin = readFileIntoByteArr(SIM_OUTPUT_BIN)
        testFileBin = readFileIntoByteArr(testFile)

        # Assert
        self.assertEqual(resultFileBin, testFileBin)

