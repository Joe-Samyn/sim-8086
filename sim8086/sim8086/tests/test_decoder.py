#!/usr/bin/env python3

# TODO: Plan out how tests are going to work via steps

import subprocess
import sys
from pathlib import Path

def validateSimOutput(testFile, simOutput):
	pass

def run8086Simulator(testFile):
	pass

def assembleFile(testFile) -> str:
	''' Assembles the test file using NASM 
	
	Parameters
	----------
		testFile: str
			The test Intel 8086 assembly file to assemble into a binary. 
	
	Returns
	-------
		(str): The path to the assembled file 
	'''
	pass

if __name__ == "__main__":
	print("Running decoder tests..\n", flush=True)

	# Check to see if we have proper arguments for running tests
	# Need 2 args: sim8086 exe and test file
	if len(sys.argv) < 2:
		print("Provide path to sim8086 executable.", flush=True)

	# Load test files
	current_dir = Path(__file__).parent 
	mov_test_file_path = f"{current_dir}\mov_tst"

	# Run sim8086 program
	decoder_path = sys.argv[1]
	decoder_run_result = subprocess.run([decoder_path, mov_test_file_path])
