

__kernel void printOrder(__global float *A, __global float *B, __global float*C) {

	if (get_local_id(0) == 0) {	
		printf("work group %d\n", get_group_id(0));
	}

	int i = get_global_id(0);
	C[i] = A[i] + B[i];

	if (get_local_id(0) == 0) {
    printf("work group %d\n", get_group_id(0));
  }

}
