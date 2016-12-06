#include <stdlib.h>
#include <math.h>
#include <iostream>
using namespace std;

#define NI 256
#define NJ 256
#define NK 256

#define DIM_LOCAL_WORK_GROUP_X 16
#define DIM_LOCAL_WORK_GROUP_Y 1

#define A_OFFSET 0
#define B_OFFSET NI * NJ * NK

void convolution3d_kernel_GXYW(float *A, float *B, int i, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

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

								int k = (wx * cX + x) * lidx + lx;
								int j = (wy * cY + y) * lidy + ly;

								float c11, c12, c13, c21, c22, c23, c31, c32, c33;
								c11 = +2;  c21 = +5;  c31 = -8;
								c12 = -3;  c22 = +6;  c32 = -9;
								c13 = +4;  c23 = +7;  c33 = +10;
	
								// Do the operation
								if ((i < (NI - 1)) && (j < (NJ - 1)) &&  (k < (NK - 1)) && (i > 0) && (j > 0) && (k > 0)) {
									B[i*(NK * NJ) + j*NK + k] = c11 * A[(i - 1)*(NK * NJ) + (j - 1)*NK + (k - 1)]
											+   c13 * A[(i + 1)*(NK * NJ) + (j - 1)*NK + (k - 1)]
											+   c21 * A[(i - 1)*(NK * NJ) + (j - 1)*NK + (k - 1)]
											+   c23 * A[(i + 1)*(NK * NJ) + (j - 1)*NK + (k - 1)]
											+   c31 * A[(i - 1)*(NK * NJ) + (j - 1)*NK + (k - 1)]
											+   c33 * A[(i + 1)*(NK * NJ) + (j - 1)*NK + (k - 1)]
											+   c12 * A[(i + 0)*(NK * NJ) + (j - 1)*NK + (k + 0)]
											+   c22 * A[(i + 0)*(NK * NJ) + (j + 0)*NK + (k + 0)]
											+   c32 * A[(i + 0)*(NK * NJ) + (j + 1)*NK + (k + 0)]
											+   c11 * A[(i - 1)*(NK * NJ) + (j - 1)*NK + (k + 1)]
											+   c13 * A[(i + 1)*(NK * NJ) + (j - 1)*NK + (k + 1)]
											+   c21 * A[(i - 1)*(NK * NJ) + (j + 0)*NK + (k + 1)]
											+   c23 * A[(i + 1)*(NK * NJ) + (j + 0)*NK + (k + 1)]
											+   c31 * A[(i - 1)*(NK * NJ) + (j + 1)*NK + (k + 1)]
											+   c33 * A[(i + 1)*(NK * NJ) + (j + 1)*NK + (k + 1)];
									(*access)(A_OFFSET + (i - 1)*(NK * NJ) + (j - 1)*NK + (k - 1), wy * widx + wx);
									(*access)(A_OFFSET + (i + 1)*(NK * NJ) + (j - 1)*NK + (k - 1), wy * widx + wx);
									(*access)(A_OFFSET + (i - 1)*(NK * NJ) + (j - 1)*NK + (k - 1), wy * widx + wx);
                                    (*access)(A_OFFSET + (i + 1)*(NK * NJ) + (j - 1)*NK + (k - 1), wy * widx + wx);
                                    (*access)(A_OFFSET + (i - 1)*(NK * NJ) + (j - 1)*NK + (k - 1), wy * widx + wx);
                                    (*access)(A_OFFSET + (i + 1)*(NK * NJ) + (j - 1)*NK + (k - 1), wy * widx + wx);
                                    (*access)(A_OFFSET + (i + 0)*(NK * NJ) + (j - 1)*NK + (k + 0), wy * widx + wx);
                                    (*access)(A_OFFSET + (i + 0)*(NK * NJ) + (j + 0)*NK + (k + 0), wy * widx + wx);
                                    (*access)(A_OFFSET + (i + 0)*(NK * NJ) + (j + 1)*NK + (k + 0), wy * widx + wx);
                                    (*access)(A_OFFSET + (i - 1)*(NK * NJ) + (j - 1)*NK + (k + 1), wy * widx + wx);
                                    (*access)(A_OFFSET + (i + 1)*(NK * NJ) + (j - 1)*NK + (k + 1), wy * widx + wx);
                                    (*access)(A_OFFSET + (i - 1)*(NK * NJ) + (j + 0)*NK + (k + 1), wy * widx + wx);
                                    (*access)(A_OFFSET + (i + 1)*(NK * NJ) + (j + 0)*NK + (k + 1), wy * widx + wx);
                                    (*access)(A_OFFSET + (i - 1)*(NK * NJ) + (j + 1)*NK + (k + 1), wy * widx + wx);
                                    (*access)(A_OFFSET + (i + 1)*(NK * NJ) + (j + 1)*NK + (k + 1), wy * widx + wx);
									(*access)(B_OFFSET + i * (NK * NJ) + j * NK + k, wy * widx + wx);
								} else {
									B[i*(NK * NJ) + j*NK + k] = 0;
									(*access)(B_OFFSET + i * (NK * NJ) + j * NK + k, wy * widx + wx);
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


void init_data(float *A) {
	
	for (int i = 0; i < NI; ++i) {
		for (int j = 0; j < NJ; ++j) {
			for (int k = 0; k < NK; ++k) {
				A[i * NJ * NK + j * NK + k] = i % 12 + 2 * (j % 7) + 3 * (k % 13);
			}
		}
	}

	return;
}

void convolution3d_cpu(float *A, float *B) {

	float c11, c12, c13, c21, c22, c23, c31, c32, c33;

	c11 = +2;  c21 = +5;  c31 = -8;
	c12 = -3;  c22 = +6;  c32 = -9;
	c13 = +4;  c23 = +7;  c33 = +10;

	for (int i = 1; i < NI - 1; i++) {
		for (int j = 1; j < NJ - 1; j++) {
			for (int k = 1; k < NK -1; k++) {
				B[i * NJ * NK + j * NK + k] = c11 * A[(i - 1) * NJ * NK + (j - 1) * NK + (k - 1)]
					+   c13 * A[(i + 1) * NJ * NK + (j - 1) * NK + (k - 1)]
					+   c21 * A[(i - 1) * NJ * NK + (j - 1) * NK + (k - 1)]
					+   c23 * A[(i + 1) * NJ * NK + (j - 1) * NK + (k - 1)]
					+   c31 * A[(i - 1) * NJ * NK + (j - 1) * NK + (k - 1)]
					+   c33 * A[(i + 1) * NJ * NK + (j - 1) * NK + (k - 1)]
					+   c12 * A[(i + 0) * NJ * NK + (j - 1) * NK + (k + 0)]
					+   c22 * A[(i + 0) * NJ * NK + (j + 0) * NK + (k + 0)]
					+   c32 * A[(i + 0) * NJ * NK + (j + 1) * NK + (k + 0)]
					+   c11 * A[(i - 1) * NJ * NK + (j - 1) * NK + (k + 1)]
					+   c13 * A[(i + 1) * NJ * NK + (j - 1) * NK + (k + 1)]
					+   c21 * A[(i - 1) * NJ * NK + (j + 0) * NK + (k + 1)]
					+   c23 * A[(i + 1) * NJ * NK + (j + 0) * NK + (k + 1)]
					+   c31 * A[(i - 1) * NJ * NK + (j + 1) * NK + (k + 1)]
					+   c33 * A[(i + 1) * NJ * NK + (j + 1) * NK + (k + 1)];
			}
		}
	}

	return;
}

void verify_kernel(float *B, float *B_ref) {

	for (int i = 1; i < NI - 1; i++) {
		for (int j = 1; j < NJ - 1; j++) {
			for (int k = 1; k < NK - 1; k++) {
				if ((B[i * NJ * NK + j * NK + k] - B_ref[i * NJ * NK + j * NK + k]) / B_ref[i * NJ * NK + j * NK + k] > 1.05) {
					cout << "Error in kernel" << endl;
					return;
				}
			}
		}
	}

	return;
}

int convolution3d_main(void (*access)(uint64_t addr, uint64_t wgid), void(*reset)(void), void(*calculate)(void)) {

	float *A;
	float *B;
	float *B_ref;

	A = (float *)malloc(NI * NJ * NK * sizeof(float));
	B = (float *)malloc(NI * NJ * NK * sizeof(float));
	B_ref = (float *)malloc(NI * NJ * NK * sizeof(float));

	init_data(A);
	convolution3d_cpu(A, B_ref);

	int lidx = DIM_LOCAL_WORK_GROUP_X;
	int lidy = DIM_LOCAL_WORK_GROUP_Y;
	int gidx = (int)ceil(((float)NK) / ((float)lidx)) * lidx;
	int gidy = (int)ceil(((float)NJ) / ((float)lidy)) * lidy;
	int coalescingMax[2];
	coalescingMax[0] = gidx / lidx;
	coalescingMax[1] = gidy / lidy;

	for (int cX = 1; cX <= coalescingMax[0]; cX = 2*cX) {
		for (int cY = 1; cY <= coalescingMax[1]; cY = 2*cY) {

			(*reset)();

			int globalWorkSizeC[2];
			globalWorkSizeC[0] = (gidx / cX) / lidx;
			globalWorkSizeC[1] = (gidy / cY) / lidy;

			cout << "global work size " << globalWorkSizeC[0] << " " << globalWorkSizeC[1] << " local work size " << lidx << " " << lidy << endl;
			for (int i = 1; i < NI - 1; i++) {
				convolution3d_kernel_GXYW(A, B, i, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);
			}

			verify_kernel(B, B_ref);
	
			(*calculate)();

		}
	}

	return 0;
}
