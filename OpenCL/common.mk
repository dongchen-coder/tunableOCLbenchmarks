#OpenCL_SDK=/global/homes/s/sgrauerg/NVIDIA_GPU_Computing_SDK
#INCLUDE=-I${OpenCL_SDK}/OpenCL/common/inc -I${PATH_TO_UTILS}
#LIBPATH=-L${OpenCL_SDK}/OpenCL/common/lib -L${OpenCL_SDK}/shared/lib
INCLUDE=-I${PATH_TO_UTILS}
LIB=-framework OpenCL
all:
	gcc -O3 ${INCLUDE} ${LIBPATH} ${LIB} ${CFILES} -o ${EXECUTABLE}

clean:
	rm -f *~ *.exe *.txt
