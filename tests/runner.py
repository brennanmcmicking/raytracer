import subprocess
import os

PROGRAM_COMMAND = "RayTracer" # executable called "RayTracer" or "RayTracer.exe"
# PROGRAM_COMMAND = "python3 RayTracer.py" # python3 on unix-like systems
# PROGRAM_COMMAND = "python RayTracer.py" # python 3 on windows OR python 2.7 on any system

def execute_all_tests(verbose = False):
    out = None
    if not verbose:
        out = subprocess.DEVNULL
    
    for filename in os.listdir("."):
        if filename.endswith(".txt"):
            print(f"RUNNING {filename}")
            subprocess.run([PROGRAM_COMMAND, filename], stdout=out)
            print(f"DONE {filename}")

if __name__ == "__main__":
    execute_all_tests(verbose=False)