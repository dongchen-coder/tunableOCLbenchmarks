#include <stdlib.h>
#include <math.h>
#include <iostream>
using namespace std;

#define nN 4096

#define DIM_LOCAL_WORK_GROUP_X 256
#define DIM_LOCAL_WORK_GROUP_Y 1

#define A_OFFSET 0
#define B_OFFSET nN * nN
#define X_OFFSET nN * nN + nN
#define Y_OFFSET nN * nN + 2 * nN
#define TMP_OFFSET nN * nN + 3 * nN

void gesummv_kernel(float alpha, float beta, float *A, float *B, float *x, float *tmp, float *y, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

	/* iterate over work group */
	for (int wy = 0; wy < widy; wy++) {
		for (int wx = 0; wx < widx; wx++) {
			/* iterate over work item*/
			for (int ly = 0; ly < lidy; ly++) {
				for (int warp = 0; warp < lidx/16; warp++) {
					for (int w = 0; w < 16; w++) {
						int lx = warp * 16 + w;

						for (int cx = 0; cx < cX; cx++) {

							int i = (wx * cX + cx) * lidx + lx;

							if (i < nN) {
								tmp[i] = 0;
								y[i] = 0;
								
								(*access)(TMP_OFFSET + i, wy*widx+wx);
								(*access)(Y_OFFSET + i, wy*widx+wx);
								
								for(int j = 0; j < nN; j++) {
									tmp[i] += A[i * nN + j] * x[j];
									y[i] += B[i * nN + j] * x[j];
									(*access)(A_OFFSET + i * nN + j, wy*widx+wx);
									(*access)(X_OFFSET + j, wy*widx+wx);
									(*access)(TMP_OFFSET + i, wy*widx+wx);
									(*access)(B_OFFSET + i * nN + j, wy*widx+wx);
									(*access)(X_OFFSET + j, wy*widx+wx);
									(*access)(Y_OFFSET + i, wy*widx+wx);
								}
								y[i] = alpha * tmp[i] + beta * y[i];
								(*access)(TMP_OFFSET + i, wy*widx+wx);
								(*access)(Y_OFFSET + i, wy*widx+wx);
								(*access)(Y_OFFSET + i, wy*widx+wx);
							}
						}
					}
				}
			}
		}
	}

	return;
}

void init_data(float *alpha, float *beta, float *A, float *B, float *x) {

	*alpha = 43532;
	*beta = 12313;

	for (int i = 0; i < nN; i++) {
		x[i] = ((float) i) / nN;

		for (int j = 0; j < nN; j++) {
			A[i * nN + j] = ((float) i*j) / nN;
			B[i * nN + j] = ((float) i*j) / nN;
		}
	}	

	return;
}

void gesummv_cpu(float alpha, float beta, float *A, float *B, float *tmp, float *x, float *y) {

	for (int i = 0; i < nN; i++) {
		tmp[i] = 0;
		y[i] = 0;
		for (int j = 0; j < nN; j++)
		{
			tmp[i] = A[i * nN + j] * x[j] + tmp[i];
			y[i] = B[i * nN + j] * x[j] + y[i];
		}
		
		y[i] = alpha * tmp[i] + beta * y[i];
	}

	return;
}

void verify(float *y, float * y_ref) {

	for (int i = 0; i < nN; i++) {
		if (y[i] != y_ref[i]) {
			cout << "Error in kernel" << endl;
			return;
		}
	}

	return;
}

int gesummv_main(void (*access)(uint64_t addr, uint64_t wgid), void(*reset)(void), void(*calculate)(void), void(*dump)(void),int cX, int cY) {
	
	float alpha;
	float beta;

	float *A;
	float *B;
	float *tmp;
	float *x;
	float *y;
	float *y_ref;

	A = (float *) malloc( nN * nN * sizeof(float));
	B = (float *) malloc( nN * nN * sizeof(float));
	tmp = (float *) malloc( nN * sizeof(float));
	x = (float *) malloc( nN * sizeof(float));
	y = (float *) malloc( nN * sizeof(float));
	y_ref = (float *) malloc( nN * sizeof(float));

	init_data(&alpha, &beta, A, B, x);

	gesummv_cpu(alpha, beta, A, B, tmp, x, y_ref);

	int lidx = DIM_LOCAL_WORK_GROUP_X;
	int lidy = 1;
	int gidx = (int)ceil(((float) nN) / ((float)lidx)) * lidx;
	int gidy = 1;
	int coalescingMax[2];
	coalescingMax[0] = gidx / lidx;
	coalescingMax[1] = gidy / lidy;
	
	if (cX <= coalescingMax[0] && cY <= coalescingMax[1]) {
			init_data(&alpha, &beta, A, B, x);
			(*reset)();

			int globalWorkSizeC[2];
			globalWorkSizeC[0] = (gidx / cX) / lidx;
			globalWorkSizeC[1] = (gidy / cY) / lidy;

			cout << "global work size " << globalWorkSizeC[0] << " " << globalWorkSizeC[1] << " local work size " << lidx << " " << lidy << endl;

			gesummv_kernel(alpha, beta, A, B, x, tmp, y, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify(y, y_ref);
			
			(*calculate)();
			(*dump)();
	} else {
		cout << "No such config:" << cX << " " << cY << " " << coalescingMax[0] << " " << coalescingMax[1] << endl;
	}

	return 0;
}


