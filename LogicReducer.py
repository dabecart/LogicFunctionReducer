# **************************************************************************************************
# @file LogicReducer.py
# @brief converts a truth table stored into a text file into a list of minterms. These are fed to 
# the C++ Petrick program so that they get reduced. With this program you will get all the reduced 
# expressions for the columns at the right of the truth table.
#
# @project   Logic Function Reducer
# @version   1.0
# @date      2024-09-15
# @author    @dabecart
#
# @license
# This project is licensed under the MIT License - see the LICENSE file for details.
# **************************************************************************************************

import shlex, os, subprocess
import argparse

# Verifies if the provided filepath is a valid, readable file.
def verifyFile(filepath: str) -> bool:
    # Check if the file exists
    if not os.path.exists(filepath):
        print(f"Error: The file '{filepath}' does not exist.")
        return False

    # Check if the filepath is a file (not a directory)
    if not os.path.isfile(filepath):
        print(f"Error: The path '{filepath}' is not a file.")
        return False

    # Check if the file is readable
    if not os.access(filepath, os.R_OK):
        print(f"Error: The file '{filepath}' is not readable.")
        return False

    return True

def fromRowToNumbers(input: str) -> list[int]:
    ret = []
    if 'x' in input:
        ret.extend(fromRowToNumbers(input.replace('x', '0', 1)))
        ret.extend(fromRowToNumbers(input.replace('x', '1', 1)))
    else:
        input = input.replace(' ', '')
        ret.append(int(input, 2))
    return ret

def executeCommand(cmd: str, cwd: str):
    commandArgs = shlex.split(cmd)
    # So that the windowed application doesn't open a terminal to run the code on Windows (nt).
    # Taken from here:
    # https://code.activestate.com/recipes/409002-launching-a-subprocess-without-a-console-window/
    
    if os.name == 'nt':
        startupInfo = subprocess.STARTUPINFO()
        startupInfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        
        runResult = subprocess.run(commandArgs,
                                stdout   = subprocess.PIPE, 
                                stderr   = subprocess.PIPE,
                                cwd      = cwd,
                                startupinfo = startupInfo)
    else:
        runResult = subprocess.run(commandArgs,
                                stdout   = subprocess.PIPE, 
                                stderr   = subprocess.PIPE,
                                cwd      = cwd)

    # Taken from here: 
    # https://stackoverflow.com/questions/24849998/how-to-catch-exception-output-from-python-subprocess-check-output
    if runResult.stderr:
        raise subprocess.CalledProcessError(
            returncode = runResult.returncode,
            cmd = runResult.args,
            stderr = runResult.stderr
        )
    
    print(runResult.stdout.decode('utf-8'))

def main():
    parser = argparse.ArgumentParser(
        prog="LogicReducer.py",
        description="Process the truth table from a file and reduce to its maximally reduced\n"
                    "algebraic expression. Inputs are labeled a to z, outputs Q0 to Q9, from left\n"
                    "to right.",
        epilog="By @dabecart, 2024.")
    parser.add_argument('-v', '--verbose', action='store_true', 
                        help='Verbose mode displays more info on the process.')
    parser.add_argument('-c', '--colored', action='store_true', 
                        help='Negated terms shown in red, non-negated in green.')
    parser.add_argument('file', type=str, 
                        help='The path to the text file containing the truth table.')
    
    args = parser.parse_args()
    if not verifyFile(args.file):
        exit(-1)

    # Store all the functions.
    minterms: list[list[int]]   = []
    dnc: list[list[int]]        = []

    cwd = os.path.dirname(os.path.abspath(__file__))

    with open(args.file) as f:
        lines = f.readlines()

        inputCount = len(lines[0].split('|')[0].strip().split(" "))
        outFuncCount = len(lines[0].split('|')[1].strip().split(" "))
        minterms    = [[] for i in range(outFuncCount)]
        dnc         = [[] for i in range(outFuncCount)]

        for line in lines:
            line = line.split("|")

            inputs: list[int] = fromRowToNumbers(line[0].strip())
            

            outputs: list[str] = line[1].strip().split(' ')
            for index, out in enumerate(outputs):
                if out == '1':
                    minterms[index].extend(inputs)
                elif out == 'x':
                    dnc[index].extend(inputs)

        for index, (m, d) in enumerate(zip(minterms, dnc)):
            print(f"Q{index}: {m}")
            print(f"DNC{index}: {d}")
            
            mstr = str(m).replace(" ", "")
            dstr = str(d).replace(" ", "")
            optionalArgs = ""
            if args.verbose: optionalArgs += "-v "
            if args.colored: optionalArgs += "-c"

            if os.name == 'nt':
                executeCommand(f"petrick.exe {optionalArgs} {inputCount} {mstr} {dstr}", cwd)
            else:
                executeCommand(f"./petrick {optionalArgs} {inputCount} {mstr} {dstr}", cwd)

if __name__ == "__main__":
    main()