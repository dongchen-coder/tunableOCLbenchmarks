#include <stdlib.h>
#include <math.h>
#include <iostream>
using namespace std;

#define N 4096

#define DIM_LOCAL_WORK_GROUP_X 16
#define DIM_LOCAL_WORK_GROUP_Y 1

#define A_OFFSET 0
#define X1_OFFSET N * N
#define X2_OFFSET N * N + N
#define Y1_OFFSET N * N + 2 * N
#define Y2_OFFSET N * N + 3 * N

void mvt_kernel1_GXYW(float *A, float *x1, float *y1, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

	/* iterate over work group */
	for (int wy = 0; wy < widy; wy++) {
		for (int wx = 0; wx < widx; wx++) {
			/* iterate over work item*/
			for (int ly = 0; ly < lidy; ly++) {
				for (int warp = 0; warp < lidx/16; warp++) {
					for (int w = 0; w < 16; w++) {
						int lx = warp * 16 + w;

						for (int x = 0; x < cX; x++) {

							int i = (wx * cX + x) * lidx + lx;

							if (i < N) {
								//x1[i] = 0;
								//(*access)(X1_OFFSET + i, wy * widx + wx);

								for (int j = 0; j < N; j++) {
									x1[i] += A[i * N + j] * y1[j];
									(*access)(A_OFFSET + i * N + j, wy * widx + wx);
									(*access)(Y1_OFFSET + j, wy * widx + wx);
									(*access)(X1_OFFSET + i, wy * widx + wx);

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

void mvt_kernel2_GXYW(float *A, float *x2, float *y2, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

	/* iterate over work group */
	for (int wy = 0; wy < widy; wy++) {
		for (int wx = 0; wx < widx; wx++) {
			/* iterate over work item*/
			for (int ly = 0; ly < lidy; ly++) {
				for (int warp = 0; warp < lidx/16; warp++) {
					for (int w = 0; w < 16; w++) {
						int lx = warp * 16 + w;

						for (int x = 0; x < cX; x++) {

							int i = (wx * cX + x) * lidx + lx;

							if (i < N) {
								//x2[i] = 0;
								//(*access)(X2_OFFSET + i, wy * widx + wx);
								for (int j = 0; j < N; j++) {
									x2[i] += A[j * N + i] * y2[j];	
									(*access)(A_OFFSET + i * N + j, wy * widx + wx);
									(*access)(Y2_OFFSET + j, wy * widx + wx);
									(*access)(X2_OFFSET + i, wy * widx + wx);
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

void init_data(float *A, float *x1, float *x2, float *y1, float *y2) {
	
	for (int i = 0; i < N; i++) {
		x1[i] = ((float) i) / N;
		x2[i] = ((float) i + 1) / N;
		y1[i] = ((float) i + 3) / N;
		y2[i] = ((float) i + 4) / N;
		for (int j = 0; j < N; j++) {
			A[i * N + j] = ((float) i*j) / N;
		}
	}

	return;
}

void mvt_cpu(float *A, float *x1, float *x2, float *y1, float *y2) {
	
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			x1[i] = x1[i] + A[i * N + j] * y1[j];
		}
	}
	
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			x2[i] = x2[i] + A[j * N + i] * y2[j];
		}
	}	
	
	return;
}

void verify_kernel1(float *x, float *x_ref) {

	for (int i = 0; i < N; i++) {
		if (x[i] != x_ref[i]) {
			cout << "Error in kernel1" << endl;
			return;
		}
	}

	return;
}

void verify_kernel2(float *x, float *x_ref) {

	for (int i = 0; i < N; i++) {
		if (x[i] != x_ref[i]) {
			cout << "Error in kernel2" << endl;
			return;
		}
	}

	return;
}


int mvt_main(void (*access)(uint64_t addr, uint64_t wgid), void(*reset)(void), void(*calculate)(void)) {
	
	float *A;
	float *x1;
	float *x2;
	float *y1;
	float *y2;
	float *x1_ref;
	float *x2_ref;

	A = (float *) malloc(N * N * sizeof(float));
	x1 = (float *) malloc(N * sizeof(float));
	x2 = (float *) malloc(N * sizeof(float));
	y1 = (float *) malloc(N * sizeof(float));
	y2 = (float *) malloc(N * sizeof(float));
	x1_ref = (float *) malloc(N * sizeof(float));
	x2_ref = (float *) malloc(N * sizeof(float));


	init_data(A, x1_ref, x2_ref, y1, y2);
	mvt_cpu(A, x1_ref, x2_ref, y1, y2);	

	int lidx = DIM_LOCAL_WORK_GROUP_X;
	int lidy = 1;
	int gidx = (int)ceil(((float)N) / ((float)lidx)) * lidx;
	int gidy = 1;
	int coalescingMax[2];
	coalescingMax[0] = gidx / lidx;
	coalescingMax[1] = gidy / lidy;

	for (int cX = 1; cX <= coalescingMax[0]; cX = 2*cX) {
		for (int cY = 1; cY <= coalescingMax[1]; cY = 2*cY) {

			(*reset)();

			init_data(A, x1, x2, y1, y2);

			int globalWorkSizeC[2];
			globalWorkSizeC[0] = (gidx / cX) / lidx;
			globalWorkSizeC[1] = (gidy / cY) / lidy;

			cout << "global work size " << globalWorkSizeC[0] << " " << globalWorkSizeC[1] << " local work size " << lidx << " " << lidy << endl;

			mvt_kernel1_GXYW(A, x1, y1, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel1(x1, x1_ref);

			(*calculate)();
		}
	}

	gidx = (int)ceil(((float)N) / ((float)lidx)) * lidx;
	gidy = 1;
	coalescingMax[0] = gidx / lidx;
	coalescingMax[1] = gidy / lidy;

	for (int cX = 1; cX <= coalescingMax[0]; cX = 2*cX) {
		for (int cY = 1; cY <= coalescingMax[1]; cY = 2*cY) {

			(*reset)();

			int globalWorkSizeC[2];
			globalWorkSizeC[0] = (gidx / cX) / lidx;
			globalWorkSizeC[1] = (gidy / cY) / lidy;

			cout << "global work size " << globalWorkSizeC[0] << " " << globalWorkSizeC[1] << " local work size " << lidx << " " << lidy << endl;
			
			mvt_kernel2_GXYW(A, x2, y2, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel2(x2, x2_ref);
	
			(*calculate)();

		}
	}


	return 0;
}


