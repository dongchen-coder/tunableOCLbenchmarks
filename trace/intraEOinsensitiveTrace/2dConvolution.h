
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define NI 4096
#define NJ 4096

#define DIM_LOCAL_WORK_GROUP_X 32
#define DIM_LOCAL_WORK_GROUP_Y 8

#define A_OFFSET 0
#define B_OFFSET NI * NJ

void convolution2d_kernel_GXYW(float *A, float *B, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

	/* iterate over work group */
	for (int wy = 0; wy < widy; wy++) {
		for (int wx = 0; wx < widx; wx++) {
			/* iterate over work item*/
			for (int ly = 0; ly < lidy; ly++) {
				for (int warp = 0; warp < lidx/16; warp++) {
					for (int w = 0; w < 16; w++) {
						int lx = warp * 16 + w;

						for (int y = 0; y < cY; y++) {
							for (int x = 0; x < cX; x++) {

								int j = (wx * cX + x) * lidx + lx;
								int i = (wy * cY + y) * lidy + ly;

								float c11, c12, c13, c21, c22, c23, c31, c32, c33;
								c11 = +0.2;  c21 = +0.5;  c31 = -0.8;
								c12 = -0.3;  c22 = +0.6;  c32 = -0.9;
								c13 = +0.4;  c23 = +0.7;  c33 = +0.10;
								if ((i < (NI-1)) && (j < (NJ - 1)) && (i > 0) && (j > 0)) {
									B[i * NJ + j] =  c11 * A[(i - 1) * NJ + (j - 1)]
											+ c21 * A[(i - 1) * NJ + (j + 0)]
											+ c31 * A[(i - 1) * NJ + (j + 1)]
											+ c12 * A[(i + 0) * NJ + (j - 1)]
											+ c22 * A[(i + 0) * NJ + (j + 0)]
											+ c32 * A[(i + 0) * NJ + (j + 1)]
											+ c13 * A[(i + 1) * NJ + (j - 1)]
											+ c23 * A[(i + 1) * NJ + (j + 0)]
											+ c33 * A[(i + 1) * NJ + (j + 1)];
									(*access)(A_OFFSET + (i - 1) * NJ + (j - 1), wy * widx + wx);
									(*access)(A_OFFSET + (i - 1) * NJ + (j + 0), wy * widx + wx);
									(*access)(A_OFFSET + (i - 1) * NJ + (j + 1), wy * widx + wx);
									(*access)(A_OFFSET + (i + 0) * NJ + (j - 1), wy * widx + wx);
									(*access)(A_OFFSET + (i + 0) * NJ + (j + 0), wy * widx + wx);
									(*access)(A_OFFSET + (i + 0) * NJ + (j + 1), wy * widx + wx);
									(*access)(A_OFFSET + (i + 1) * NJ + (j - 1), wy * widx + wx);
									(*access)(A_OFFSET + (i + 1) * NJ + (j + 0), wy * widx + wx);
									(*access)(A_OFFSET + (i + 1) * NJ + (j + 1), wy * widx + wx);
									(*access)(B_OFFSET + i * NJ + j, wy * widx + wx);
								}
							}
						}
					}
				}
			}
		}
	}

	return;
}

void init_data(float * A) {

	for (int i = 0; i < NI; ++i) {
		for (int j = 0; j < NJ; ++j) {
			A[i * NJ + j] = (float) (i * NJ + j);
		}
	}

	return;
}

void convolution2d_cpu(float *A, float *B) {

	float c11, c12, c13, c21, c22, c23, c31, c32, c33;

	c11 = +0.2;  c21 = +0.5;  c31 = -0.8;
	c12 = -0.3;  c22 = +0.6;  c32 = -0.9;
	c13 = +0.4;  c23 = +0.7;  c33 = +0.10;


	for (int i = 1; i < NI - 1; i++) {
		for (int j = 1; j < NJ - 1; j++) {
			B[i * NJ + j] = c11 * A[(i - 1) * NJ + (j - 1)]
				+  c12 * A[(i + 0) * NJ + (j - 1)]
				+  c13 * A[(i + 1) * NJ + (j - 1)]
				+  c21 * A[(i - 1) * NJ + (j + 0)]
				+  c22 * A[(i + 0) * NJ + (j + 0)]
				+  c23 * A[(i + 1) * NJ + (j + 0)]
				+  c31 * A[(i - 1) * NJ + (j + 1)]
				+  c32 * A[(i + 0) * NJ + (j + 1)]
				+  c33 * A[(i + 1) * NJ + (j + 1)];
		}
	}

	return;
}

void verify_kernel(float *B, float *B_ref) {

	for (int i = 1; i < NI - 1; i++) {
		for (int j = 1; j < NJ - 1; j++) {
			if (B[i * NJ + j] != B_ref[i * NJ + j]) {
				printf("Error in kernel, %d, %d, %f, %f\n", i, j, B[i * NJ + j], B_ref[i * NJ + j]);
				return;
			}
		}
	}

	return;
}

int convolution2d_main(void (*access)(uint64_t addr, uint64_t wgid), void(*reset)(void), void(*calculate)(void)) {

	float *A;
	float *B;
	float *B_ref;
	
	A = (float *) malloc(NI * NJ * sizeof(float));
	B = (float *) malloc(NI * NJ * sizeof(float));
	B_ref = (float *) malloc(NI * NJ * sizeof(float));

	init_data(A);
	convolution2d_cpu(A, B_ref);

	int lidx = DIM_LOCAL_WORK_GROUP_X;
	int lidy = DIM_LOCAL_WORK_GROUP_Y;
	int gidx = (int)ceil(((float)NI) / ((float)lidx)) * lidx;
	int gidy = (int)ceil(((float)NJ) / ((float)lidy)) * lidy;
	int coalescingMax[2];
	coalescingMax[0] = gidx / lidx;
	coalescingMax[1] = gidy / lidy;

	for (int cX = 1; cX <= coalescingMax[0]; cX = 2*cX) {
		for (int cY = 1; cY <= coalescingMax[1]; cY = 2*cY) {

			(*reset)();

			int globalWorkSizeC[2];
			globalWorkSizeC[0] = gidx / cX;
			globalWorkSizeC[1] = gidy / cY;

			printf("global work size %d, %d local work size %d, %d\n", globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy);
			convolution2d_kernel_GXYW(A, B, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel(B, B_ref);

			(*calculate)();

		}
	}



	return 0;
}

