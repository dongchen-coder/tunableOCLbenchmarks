



__kernel void printOrder(__global float *A, __global float *B, __global float*C) {

	if (get_local_id(0) == 0) {	
		//Potential way to get work group to EU mapping by reading register SR0
		//int EUID;
		//__asm__("mov %eax, %SR0" : "=r"(EUID));
		printf("work group %d\n", get_group_id(0));
	}

	int i = get_global_id(0);
	C[i] = A[i] + B[i];

	if (get_local_id(0) == 0) {
    printf("work group %d\n", get_group_id(0));
  }

}
