import sys
import os
#import matplotlib.pyplot.plot as plt

def timeToFloat(t):
	if ("ms" in t):
		return float(t.replace("ms","")) * 0.001
	if ("us" in t):
		return float(t.replace("us","")) * 0.000001
	if ("s" in t):
		return float(t.replace("s",""))
	print t
	return t

path = "/localdisk/dchenTmp/scheduling/tunableOCLbenchmarks/result_stencil-1d"

#benches = ["2mm", "3mm", "atax", "bicg", "doitgen", "gemm", "gemver", "gesummv", "syr2k", "syrk", "gramschmidt", "lu", "correlation", "covariance", "adi"]

benches = ["stencil-1d"]

# "fdtd-2d"
# "convolution-2d", "convolution-3d"
# "jacobi-1d-imper"
# "jacobi-2d-imper"

kernels = {"2mm" : ["mm2_kernel1", "mm2_kernel2"], "3mm" : ["mm3_kernel1", "mm3_kernel2", "mm3_kernel3"], "atax" : ["atax_kernel1", "atax_kernel2"], "bicg" : ["bicg_kernel1", "bicg_kernel2"], "doitgen" : ["doitgen_kernel1", "doitgen_kernel2"], "gemm" : ["gemm_kernel"], "gemver" : ["gemver_kernel1", "gemver_kernel2", "gemver_kernel3"], "gesummv" : ["gesummv_kernel"], "mvt" : ["mvt_kernel1", "mvt_kernel2"],"syr2k" : ["syr2k_kernel"], "syrk" : ["syrk_kernel"], "gramschmidt" : ["gramschmidt_kernel1", "gramschmidt_kernel2", "gramschmidt_kernel3"], "lu" : ["lu_kernel1", "lu_kernel2"], "correlation" : ["mean_kernel", "std_kernel", "reduce_kernel", "corr_kernel"], "covariance" : ["mean_kernel", "reduce_kernel", "covar_kernel"], "adi" : ["adi_kernel1", "adi_kernel2", "adi_kernel3", "adi_kernel4", "adi_kernel5", "adi_kernel6"], "convolution-2d" : ["convolution2D_kernel"], "convolution-3d" : ["convolution3D_kernel"], "fdtd-2d" : ["fdtd_step1_kernel", "fdtd_step2_kernel", "fdtd_step3_kernel"], "jacobi-1d-imper" : ["runJacobiCUDA_kernel1", "runJacobiCUDA_kernel2"], "jacobi-2d-imper" : ["runJacobiCUDA_kernel1", "runJacobiCUDA_kernel2"], "stencil-1d" : ["stencil_1D"]}



for name in benches:
	for kernel in kernels[name]:
		times = []
		variant = []

		default = 0
		var_def = 0

		files = os.listdir(path)
		files.sort()

		for file in files:
			if (name in file):
				f = open(path + "/" + file, "r")
				tmp = 0
				cnt = 0
				for line in f:
					if (kernel in line):
						cnt += 1
						tmp += timeToFloat(line.split()[1])
				avg = tmp / cnt
				times.append(avg)
				f.close()				

				if (file == name+"_result0_0_0_0.txt"):
					default = avg

				var = 0
				f = open(path + "/" + file, "r")
				for line in f:
					if (kernel in line):
						var = max(abs(timeToFloat(line.split()[1]) - avg), var)
				variant.append(var)			

				if (file == name+"_result0_0_0_0.txt"):
					var_def = var

		minTime = min(times)		

		

		#for file in os.listdir(path):
		for i in range(14):
			file = "stencil-1d_result" + str(i) + "_0_0_0.txt"
			if (name in file):
				f = open(path + "/" + file, "r")
				tmp = 0
				cnt = 0
				for line in f:
					if (kernel in line):
						cnt += 1
						tmp += timeToFloat(line.split()[1])
				avg = tmp / cnt
				times.append(avg)
				f.close()

				print file, avg		

				if (avg == minTime):
					print "min_config: " + file

		#print times
		print kernel, "%.4g" % min(times), "%.4g" % max(times), "%.2f%%" % (max(variant) / min(times) * 100) , "%.2f%%" % ( max(times) / min(times) * 100 - 100) 
		print default / min(times)
		#plt.plot(times)
#plt.show()



#print file.replace(name + ".exe_result", "").replace(".txt","").split("_")
			
