import os
import sys

cudaDir = "/localdisk/dchenTmp/scheduling/tunableOCLbenchmarks/CUDA_remapping"
cudaBinDir = "/localdisk/dchenTmp/scheduling/tunableOCLbenchmarks/bin"
cudaResultDir = "/localdisk/dchenTmp/scheduling/tunableOCLbenchmarks/result"

remappingModeDir = "/localdisk/dchenTmp/scheduling/tunableOCLbenchmarks/utilities"

os.system("setenv PATH /localdisk/cuda/cuda-8.0/bin:$PATH")

os.system("rm " + cudaResultDir + "/*")

for t_x_mapping in [0, 2, 5, 6]: #range(9):
	for t_y_mapping in [0, 2, 5, 6]: #range(9):
		for b_x_mapping in [0, 2, 5, 6]: #range(9):
			for b_y_mapping in [0, 2, 5, 6]: #range(9):

				t_x_mode = "#define THREADXMODE " + str(t_x_mapping) 
				os.system("echo \'" + t_x_mode + "\' > " + remappingModeDir +"/remapping_mode.h")
				t_y_mode = "#define THREADYMODE " + str(t_y_mapping)
				os.system("echo \'" + t_y_mode + "\' >> " + remappingModeDir + "/remapping_mode.h")
				b_x_mode = "#define BLOCKXMODE " + str(b_x_mapping)
				os.system("echo \'" + b_x_mode + "\' >> " + remappingModeDir + "/remapping_mode.h")
				b_y_mode = "#define BLOCKYMODE " + str(b_y_mapping)
				os.system("echo \'" + b_y_mode + "\' >> " + remappingModeDir + "/remapping_mode.h")
	
				for root, subFolders, files in os.walk(cudaDir):
					for folder in subFolders:
						os.system("cd " + os.path.join(root, folder) + " && make && mv " + folder + ".exe " + cudaBinDir  + " \n")
			
				mappingMode = str(t_x_mapping) + "_" + str(t_y_mapping) + "_" + str(b_x_mapping) + "_" + str(b_y_mapping) + ".txt"
				for n in range(3):
					for app in os.listdir(cudaBinDir):
						os.system("nvprof " + cudaBinDir + "/" + app + " &>> " + cudaResultDir + "/" + app + "_result" + mappingMode)

