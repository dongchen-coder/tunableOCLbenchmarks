#include <stdlib.h>
#include <math.h>
#include <iostream>
using namespace std;

#define nN 4096

#define DIM_LOCAL_WORK_GROUP_X 32
#define DIM_LOCAL_WORK_GROUP_Y 8

#define A_OFFSET 0
#define X1_OFFSET nN * nN
#define X2_OFFSET nN * nN + nN
#define Y1_OFFSET nN * nN + 2 * nN
#define Y2_OFFSET nN * nN + 3 * nN

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

							if (i < nN) {
								//x1[i] = 0;
								//(*access)(X1_OFFSET + i, wy * widx + wx);

								for (int j = 0; j < nN; j++) {
									x1[i] += A[i * nN + j] * y1[j];
									(*access)(A_OFFSET + i * nN + j, wy * widx + wx);
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

							if (i < nN) {
								//x2[i] = 0;
								//(*access)(X2_OFFSET + i, wy * widx + wx);
								for (int j = 0; j < nN; j++) {
									x2[i] += A[j * nN + i] * y2[j];	
									(*access)(A_OFFSET + i * nN + j, wy * widx + wx);
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
	
	for (int i = 0; i < nN; i++) {
		x1[i] = ((float) i) / nN;
		x2[i] = ((float) i + 1) / nN;
		y1[i] = ((float) i + 3) / nN;
		y2[i] = ((float) i + 4) / nN;
		for (int j = 0; j < nN; j++) {
			A[i * nN + j] = ((float) i*j) / nN;
		}
	}

	return;
}

void mvt_cpu(float *A, float *x1, float *x2, float *y1, float *y2) {
	
	for (int i = 0; i < nN; i++) {
		for (int j = 0; j < nN; j++) {
			x1[i] = x1[i] + A[i * nN + j] * y1[j];
		}
	}
	
	for (int i = 0; i < nN; i++) {
		for (int j = 0; j < nN; j++) {
			x2[i] = x2[i] + A[j * nN + i] * y2[j];
		}
	}	
	
	return;
}

void verify_kernel1(float *x, float *x_ref) {

	for (int i = 0; i < nN; i++) {
		if (x[i] != x_ref[i]) {
			cout << "Error in kernel1" << endl;
			return;
		}
	}

	return;
}

void verify_kernel2(float *x, float *x_ref) {

	for (int i = 0; i < nN; i++) {
		if (x[i] != x_ref[i]) {
			cout << "Error in kernel2" << endl;
			return;
		}
	}

	return;
}


int mvt_main(void (*access)(uint64_t addr, uint64_t wgid), void(*reset)(void), void(*calculate)(void), void(*dump)(void), int cX, int cY, int kID) {
	
	float *A;
	float *x1;
	float *x2;
	float *y1;
	float *y2;
	float *x1_ref;
	float *x2_ref;

	A = (float *) malloc( nN * nN * sizeof(float));
	x1 = (float *) malloc( nN * sizeof(float));
	x2 = (float *) malloc( nN * sizeof(float));
	y1 = (float *) malloc( nN * sizeof(float));
	y2 = (float *) malloc( nN * sizeof(float));
	x1_ref = (float *) malloc( nN * sizeof(float));
	x2_ref = (float *) malloc( nN * sizeof(float));


	init_data(A, x1_ref, x2_ref, y1, y2);
	mvt_cpu(A, x1_ref, x2_ref, y1, y2);	

	int lidx = DIM_LOCAL_WORK_GROUP_X;
	int lidy = 1;
	int gidx = (int)ceil(((float) nN) / ((float)lidx)) * lidx;
	int gidy = 1;
	int coalescingMax[2];
	coalescingMax[0] = gidx / lidx;
	coalescingMax[1] = gidy / lidy;

	if (kID == 0) {
		if (cX <= coalescingMax[0] && cY <= coalescingMax[1]) {
			(*reset)();

			init_data(A, x1, x2, y1, y2);

			int globalWorkSizeC[2];
			globalWorkSizeC[0] = (gidx / cX) / lidx;
			globalWorkSizeC[1] = (gidy / cY) / lidy;

			cout << "global work size " << globalWorkSizeC[0] << " " << globalWorkSizeC[1] << " local work size " << lidx << " " << lidy << endl;

			mvt_kernel1_GXYW(A, x1, y1, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel1(x1, x1_ref);

			(*calculate)();

			(*dump)();
		} else {
			cout << "No such config:" << cX << " " << cY << " " << coalescingMax[0] << " " << coalescingMax[1] << endl;
		}
	}

	gidx = (int)ceil(((float) nN) / ((float)lidx)) * lidx;
	gidy = 1;
	coalescingMax[0] = gidx / lidx;
	coalescingMax[1] = gidy / lidy;

	if (kID == 1) {
		if (cX <= coalescingMax[0] && cY <= coalescingMax[1]) {
			(*reset)();

			init_data(A, x1, x2, y1, y2);

			int globalWorkSizeC[2];
			globalWorkSizeC[0] = (gidx / cX) / lidx;
			globalWorkSizeC[1] = (gidy / cY) / lidy;

			cout << "global work size " << globalWorkSizeC[0] << " " << globalWorkSizeC[1] << " local work size " << lidx << " " << lidy << endl;
			
			mvt_kernel2_GXYW(A, x2, y2, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel2(x2, x2_ref);
	
			(*calculate)();
			(*dump)();
		} else {
			cout << "No such config:" << cX << " " << cY << " " << coalescingMax[0] << " " << coalescingMax[1] << endl;
		}
	}

	return 0;
}


