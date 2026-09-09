import os
import subprocess

build_path = 'build'
os.makedirs(build_path, exist_ok=True)
os.chdir(build_path)
subprocess.run(["cmake", "-G MinGW Makefiles", '..'], check=True)
subprocess.run(["cmake", "--build", "."], check=True)