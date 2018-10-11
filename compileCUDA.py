import os
import sys

cudaDir = "/localdisk/dchenTmp/scheduling/tunableOCLbenchmarks/CUDA"

os.system("setenv PATH=/localdisk/cuda/cuda-8.0/bin:$PATH")

for root, subFolders, files in os.walk(cudaDir):
	for folder in subFolders:
		os.system("cd " + os.path.join(root, folder) + "\n make")

