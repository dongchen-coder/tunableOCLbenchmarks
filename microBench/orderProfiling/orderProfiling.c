#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define OPENCL_DEVICE_SELECTION CL_DEVICE_TYPE_GPU
#define MAX_SOURCE_SIZE (0x100000)

#define DIM_LOCAL_WORK_GROUP_X 256
#define DIM_LOCAL_WORK_GROUP_Y 1

#define DIM_GLOBAL_WORK_SIZE_X 2048
#define DIM_GLOBAL_WORK_SIZE_Y 1

char str_temp[1024];

cl_platform_id platform_id;
cl_device_id device_id;
cl_uint num_devices;
cl_uint num_platforms;
cl_int errcode;
cl_context clGPUContext;
cl_command_queue clCommandQue;
cl_kernel clKernel;
cl_program clProgram;
cl_mem a_mem_obj;
cl_mem b_mem_obj;
cl_mem c_mem_obj;

FILE *fp;
char *source_str;
size_t source_size;



void read_cl_file()
{ 
  // Load the kernel source code into the array source_str
  fp = fopen("orderProfiling.cl", "r");
  if (!fp) {
    fprintf(stderr, "Failed to load kernel.\n");
    exit(1);
  }
  source_str = (char*)malloc(MAX_SOURCE_SIZE);
  source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
  fclose( fp );
}

void cl_initialization() {
 
  // Get platform and device information
  errcode = clGetPlatformIDs(1, &platform_id, &num_platforms);
  if(errcode == CL_SUCCESS) printf("number of platforms is %d\n",num_platforms);

  errcode = clGetPlatformInfo(platform_id,CL_PLATFORM_NAME, sizeof(str_temp), str_temp,NULL);
  if(errcode == CL_SUCCESS) printf("platform name is %s\n",str_temp);

  errcode = clGetPlatformInfo(platform_id, CL_PLATFORM_VERSION, sizeof(str_temp), str_temp,NULL);
  if(errcode == CL_SUCCESS) printf("platform version is %s\n",str_temp);

  errcode = clGetDeviceIDs( platform_id, OPENCL_DEVICE_SELECTION, 1, &device_id, &num_devices);
  if(errcode == CL_SUCCESS) printf("device id is %d\n",device_id);

  errcode = clGetDeviceInfo(device_id,CL_DEVICE_NAME, sizeof(str_temp), str_temp,NULL);
  if(errcode == CL_SUCCESS) printf("device name is %s\n",str_temp);
 
  // Create an OpenCL context
  clGPUContext = clCreateContext( NULL, 1, &device_id, NULL, NULL, &errcode);
  if(errcode != CL_SUCCESS) printf("Error in creating context\n");

  //Create a command-queue
  clCommandQue = clCreateCommandQueue(clGPUContext, device_id, 0, &errcode);
  if(errcode != CL_SUCCESS) printf("Error in creating command queue\n");
}

void cl_mem_init(float * A, float * B, float * C)
{
  a_mem_obj = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, DIM_GLOBAL_WORK_SIZE_X * sizeof(float), NULL, &errcode);
  b_mem_obj = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, DIM_GLOBAL_WORK_SIZE_X * sizeof(float), NULL, &errcode);
	c_mem_obj = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, DIM_GLOBAL_WORK_SIZE_X * sizeof(float), NULL, &errcode);

  if(errcode != CL_SUCCESS) printf("Error in creating buffers\n");

  errcode = clEnqueueWriteBuffer(clCommandQue, a_mem_obj, CL_TRUE, 0, DIM_GLOBAL_WORK_SIZE_X * sizeof(float), A, 0, NULL, NULL);
  errcode = clEnqueueWriteBuffer(clCommandQue, b_mem_obj, CL_TRUE, 0, DIM_GLOBAL_WORK_SIZE_X * sizeof(float), B, 0, NULL, NULL);
	errcode = clEnqueueWriteBuffer(clCommandQue, c_mem_obj, CL_TRUE, 0, DIM_GLOBAL_WORK_SIZE_X * sizeof(float), C, 0, NULL, NULL);
  if(errcode != CL_SUCCESS)printf("Error in writing buffers\n");
}

void cl_load_prog()
{ 
  // Create a program from the kernel source
  clProgram = clCreateProgramWithSource(clGPUContext, 1, (const char **)&source_str, (const size_t *)&source_size, &errcode);
  
  if(errcode != CL_SUCCESS) printf("Error in creating program\n");
 
  // Build the program
  errcode = clBuildProgram(clProgram, 1, &device_id, NULL, NULL, NULL);
  if(errcode != CL_SUCCESS) printf("Error in building program\n");
  
	if (errcode == CL_BUILD_PROGRAM_FAILURE) {
    // Determine the size of the log
    size_t log_size;
    clGetProgramBuildInfo(clProgram, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    // Allocate memory for the log
    char *log = (char *) malloc(log_size);
    // Get the log
    clGetProgramBuildInfo(clProgram, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
    // Print the log
    printf("%s\n", log);
  }
 
  // Create the OpenCL kernel
  clKernel = clCreateKernel(clProgram, "printOrder", &errcode);
  if(errcode != CL_SUCCESS) printf("Error in creating kernel\n");
  clFinish(clCommandQue);
}

void cl_launch_kernel()
{ 
  size_t localWorkSize[2], globalWorkSize[2];
  localWorkSize[0] = DIM_LOCAL_WORK_GROUP_X;
  localWorkSize[1] = DIM_LOCAL_WORK_GROUP_Y;
  globalWorkSize[0] = DIM_GLOBAL_WORK_SIZE_X;
  globalWorkSize[1] = DIM_GLOBAL_WORK_SIZE_Y;
  
  // Set the arguments of the kernel
	errcode =  clSetKernelArg(clKernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
  errcode |= clSetKernelArg(clKernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
	errcode |= clSetKernelArg(clKernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);	
	if(errcode != CL_SUCCESS) printf("Error in seting arguments\n");

  // Execute the OpenCL kernel
  errcode = clEnqueueNDRangeKernel(clCommandQue, clKernel, 2, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
  if(errcode != CL_SUCCESS) printf("Error in launching kernel\n");
  clFinish(clCommandQue);
}

void cl_clean_up()
{ 
  // Clean up
  errcode = clFlush(clCommandQue);
  errcode = clFinish(clCommandQue);
  errcode = clReleaseKernel(clKernel);
  errcode = clReleaseProgram(clProgram);
  //errcode = clReleaseMemObject(b_mem_obj);
  errcode = clReleaseCommandQueue(clCommandQue);
  errcode = clReleaseContext(clGPUContext);
  if(errcode != CL_SUCCESS) printf("Error in cleanup\n");
}

int main(int argc, char *argv[]) {

	float *A = malloc(DIM_GLOBAL_WORK_SIZE_X * sizeof(float));
	float *B = malloc(DIM_GLOBAL_WORK_SIZE_X * sizeof(float));
	float *C = malloc(DIM_GLOBAL_WORK_SIZE_X * sizeof(float));

	for (int i = 0; i < DIM_GLOBAL_WORK_SIZE_X; i++) {
		A[i] = i;
		B[i] = i;
		C[i] = 0;
	}

	read_cl_file();
	cl_initialization();
	cl_mem_init(A, B, C);
	cl_load_prog();

	cl_launch_kernel();

	errcode = clEnqueueReadBuffer(clCommandQue, c_mem_obj, CL_TRUE, 0, DIM_GLOBAL_WORK_SIZE_X * sizeof(float), C, 0, NULL, NULL);

	cl_clean_up();

	return 0;
}
