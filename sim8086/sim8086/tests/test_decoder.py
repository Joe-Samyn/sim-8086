#!/usr/bin/env python3

import subprocess
import sys
from pathlib import Path
import filecmp

if __name__ == "__main__":
	print("Running decoder tests..\n", flush=True)

	# Load test files
	current_dir = Path(__file__).parent 
	test_file_asm = f"{current_dir}/asm/mov_tst.asm"
	test_file_bin = f"{current_dir}/mov_tst.out"

	# Assemble test file 
	subprocess.run(['nasm', test_file_asm, '-o', test_file_bin], check=True)


	# Run sim8086 program
	decoder_path = sys.argv[1]
	decoder_run_result = subprocess.run([decoder_path, test_file_bin], check=True, capture_output=True)

	# Get sim8086 output file and assemble it
	result_file = f'{current_dir}/result.asm'
	result_assembled = f'{current_dir}/result.out'
	subprocess.run(['nasm', result_file, '-o', result_assembled], check=True)

	# Compare sim8086 output to expected output
	assert filecmp.cmp(result_assembled, test_file_bin, shallow=False) == True
