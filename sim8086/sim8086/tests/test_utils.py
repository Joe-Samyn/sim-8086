
import subprocess

def readFileIntoByteArr(file) -> bytearray:
    """Read a binary file into a byte array.

    This is used to convert output from the 8086 simulator and pre-assembled
    binaries into a byte array that can be easily compared in Python.

    Parameters
    ----------
    file : str
        The path to the binary file to be read.

    Returns
    -------
    bytearray
        The contents of the file as a byte array.
    """
    with open(file, mode="rb") as file:
            fileBinary = file.read()
            byteArr = bytearray(fileBinary)
            return byteArr
    

def assembleFile(asmFile, outFile):
    """Assemble an ASM file into a binary file using NASM.

    This is used to assemble the output from the simulator into
    a binary file for validating correctness.

    Parameters
    ----------
    asmFile : str
        The assembly file to assemble with NASM.
    outFile : str
        The name and destination of the output file. This is passed to the `-o` flag
        in nasm to specify the output.
    """
    subprocess.run(['nasm', asmFile, '-o', outFile], check=True)

      
def runSim8086(simPath, testFile):
    """Run the Intel 8086 simulator specified by simPath on the test file.

    Parameters
    ----------
    simPath : str
        File path to the 8086 simulator executable.
    testFile : str
        The binary test file to run in the simulator. This file must be an assembled Intel 8086 program.
    """
    subprocess.run([simPath, testFile], check=True)