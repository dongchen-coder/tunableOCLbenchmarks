#include <stdlib.h>
#include <math.h>
#include <iostream>
using namespace std;

#define NI 1024
#define NJ 1024

#define DIM_LOCAL_WORK_GROUP_X 32
#define DIM_LOCAL_WORK_GROUP_Y 8

#define A_OFFSET 0
#define C_OFFSET NI * NJ

void syrk_kernel_GXYW(float alpha, float beta, float *A, float *C, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {
	
	/* iterate over work group */
	for (int wy = 0; wy < widy; wy++) {
		for (int wx = 0; wx < widx; wx++) {
			/* iterate over work item*/
			for (int ly = 0; ly < lidy; ly++) {
				for (int warp = 0; warp < lidx/16; warp++) {
					for (int w = 0; w < 16; w++) {
						int lx = warp * 16 + w;
	
						for (int cy = 0; cy < cY; cy++) {
							for (int cx = 0; cx < cX; cx++) {	
								int j = (wx * cX + cx) * lidx + lx;
								int i = (wy * cY + cy) * lidy + ly;
								if ((i < NJ) && (j < NJ)) {
									C[i * NJ + j] *= beta;
									(*access)(C_OFFSET + i * NJ + j, wy * widx + wx);
									for(int k=0; k< NI; k++) {
										C[i * NJ + j] += alpha * A[i * NI + k] * A[j * NI + k];
										(*access)(A_OFFSET + i * NI + k, wy * widx + wx);
										(*access)(A_OFFSET + j * NI + k, wy * widx + wx);
										(*access)(C_OFFSET + i * NJ + j, wy * widx + wx);
									}
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

void init_data(float *alpha, float *beta, float *A, float *C) {

	*alpha = 32412;
	*beta = 2123;
	for (int i = 0; i < NI; i++)
	{
		for (int j = 0; j < NJ; j++)
		{
			A[i * NJ + j] = ((float) i*j) / NI;
		}
	}

	for (int i = 0; i < NI; i++)
	{
		for (int j = 0; j < NI; j++)
		{
			C[i * NI + j] = ((float) i*j) / NI;
		}
	}	


	return;
}

void syrk_cpu(float alpha, float beta, float *A, float *C) {

	for (int i = 0; i < NI; i++) {
		for (int j = 0; j < NI; j++) {
			C[i * NI + j] *= beta;
		}
	}
	
	for (int i = 0; i < NI; i++) {
		for (int j = 0; j < NI; j++) {
			for (int k = 0; k < NJ; k++) {
				C[i * NI + j] += alpha * A[i * NJ + k] * A[j * NJ + k];
			}
		}
	}

	return;
}

void verify_syrk_kernel(float *C, float *C_ref) {

	for (int i = 0; i < NI; i++) {
		for (int j = 0; j < NI; j++) {
			if (C[i * NI + j] != C_ref[i * NI + j]) {
				cout << "Error in kernel" << endl;
				return;
			}
		}
	}

	return;
}

int syrk_main(void (*access)(uint64_t addr, uint64_t wgid), void(*reset)(void), void(*calculate)(void), void(*dump)(void), int cX, int cY, int kID) {

	float alpha;
	float beta;
	float *A;
	float *C;
	float *C_ref;

	A = (float *) malloc(NI * NJ * sizeof(float));
	C = (float *) malloc(NI * NI * sizeof(float));
	C_ref = (float *) malloc(NI * NI * sizeof(float));

	init_data(&alpha, &beta, A, C_ref);
	syrk_cpu(alpha, beta, A, C_ref);

	int lidx = DIM_LOCAL_WORK_GROUP_X;
	int lidy = DIM_LOCAL_WORK_GROUP_Y;
	int gidx = (int)ceil(((float)NJ) / ((float)lidx)) * lidx;
	int gidy = (int)ceil(((float)NI) / ((float)lidy)) * lidy;
	int coalescingMax[2];
	coalescingMax[0] = gidx / lidx;
	coalescingMax[1] = gidy / lidy;

	if (kID == 0) {
		if (cX <= coalescingMax[0] && cY <= coalescingMax[1]) {
			init_data(&alpha, &beta, A, C);

			(*reset)();

			int globalWorkSizeC[2];
			globalWorkSizeC[0] = (gidx / cX) / lidx;
			globalWorkSizeC[1] = (gidy / cY) / lidy;

			cout << "global work size " << globalWorkSizeC[0] << " " << globalWorkSizeC[1] << " local work size " << lidx << " " << lidy << endl;		
	
			syrk_kernel_GXYW(alpha, beta, A, C, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_syrk_kernel(C, C_ref);

			(*calculate)();

			(*dump)();
		} else {
			cout << "No such config:" << cX << " " << cY << " " << coalescingMax[0] << " " << coalescingMax[1] << endl;
		}
	}

	return 0;
}





