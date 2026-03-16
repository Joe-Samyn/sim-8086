
def readFileIntoByteArr(file) -> bytearray:
    '''Read a binary file into a byte array. 

    This is used to convert output from the 8086 simulator and pre-assembled
    binaries into a byte array that can be easily compared in Python. 
    
    Parameters
    -----------
    file: (string)
        The path to the binary file to be read.
        
    Returns
    --------
    The contents of the file as a byte array. '''
    with open(file, mode="rb") as file:
            fileBinary = file.read()
            byteArr = bytearray(fileBinary)
            return byteArr