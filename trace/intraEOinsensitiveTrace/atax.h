#include <stdlib.h>
#include <math.h>
#include <iostream>
using namespace std;

#define NX 4096
#define NY 4096
#define DIM_LOCAL_WORK_GROUP_X 16
#define DIM_LOCAL_WORK_GROUP_Y 1

void atax_kernel1_GXYW(float *A, float *x, float *tmp, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

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
							if (i < NX) {
								tmp[i] = 0;
								(*access)(NX * NY + NY + i, wy*widx+wx);
								int j;
								for(j=0; j < NY; j++)
								{
									tmp[i] += A[i * NY + j] * x[j];
									(*access)(i * NY + j, wy*widx+wx);
									(*access)(NX * NY + j, wy*widx+wx);
									(*access)(NX * NY + NY + i, wy*widx+wx);
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

void atax_kernel2_GXYW(float *A, float *tmp, float *y, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

	/* iterate over work group */
	for (int wy = 0; wy < widy; wy++) {
		for (int wx = 0; wx < widx; wx++) {
			/* iterate over work item*/
			for (int ly = 0; ly < lidy; ly++) {
				for (int warp = 0; warp < lidx/16; warp++) {
					for (int w = 0; w < 16; w++) {
						int lx = warp * 16 + w;

						for (int cx = 0; cx < cX; cx++) {
							int j = (wx * cX + cx) * lidx + lx;
							if (j < NY) {
								y[j] = 0;
								(*access)(NX * NY + NY + NX + j, wy*widx+wx);
								int i;
								for(i=0; i < NX; i++) {
									y[j] += A[i * NY + j] * tmp[i];
									(*access)(i * NY + j, wy*widx+wx);
									(*access)(NX * NY + NY + i, wy*widx+wx);
									(*access)(NX * NY + NY + NX + j, wy*widx+wx);
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

void init_data(float *A, float *x) {
	for (int i = 0; i < NX; i++) {
		x[i] = i * M_PI;
		for (int j = 0; j < NY; j++)
		{
			A[i * NY + j] = ((float) i*j) / NX;
		}
	}
	return;
}

void atax_cpu(float *A, float *x, float *tmp, float *y) {
	
	for (int i= 0; i < NY; i++) {
		y[i] = 0;
	}

	for (int i = 0; i < NX; i++) {
		tmp[i] = 0;
		for (int j = 0; j < NY; j++)
		{
			tmp[i] = tmp[i] + A[i * NY + j] * x[j];
		}
		
		for (int j = 0; j < NY; j++)
		{
			y[j] = y[j] + A[i * NY + j] * tmp[i];
		}
	}

	return;
}

void verify_kernel1(float *tmp, float *tmp_ref) {
	
	for (int i = 0; i < NX; i++) {
		if (tmp[i] != tmp_ref[i]) {
			cout << "kernel1 error" << endl;
			return;
		}
	}
	
	return;
}

void verify_kernel2(float *y, float *y_ref) {

	for (int i = 0; i < NY; i++) {
		if (y[i] != y_ref[i]) {
			cout << "kernel2 error" << endl;
			return;
		}
	}
	return;
}

int atax_main(void (*access)(uint64_t addr, uint64_t wgid), void(*reset)(void), void(*calculate)(void)) {

	float *A;
	float *x;
	float *y;
	float *tmp;
	float *tmp_ref;
	float *y_ref;

	A = (float *) malloc(NX * NY * sizeof(float));
	x = (float *) malloc(NY * sizeof(float));
	y = (float *) malloc(NY * sizeof(float));
	tmp = (float *) malloc(NX * sizeof(float));
	
	tmp_ref = (float *) malloc(NX * sizeof(float));
	y_ref = (float *) malloc(NY * sizeof(float));
	
	init_data(A, x);
	atax_cpu(A, x, tmp_ref, y_ref);
	
	int lidx = DIM_LOCAL_WORK_GROUP_X;
	int lidy = 1;
	int gidx = (int)ceil(((float)NX) / ((float)lidx)) * lidx;
	int gidy = 1;
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
			atax_kernel1_GXYW(A, x, tmp, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel1(tmp, tmp_ref);

			(*calculate)();

		}
	}

	gidx = (int)ceil(((float)NY) / ((float)lidx)) * lidx;
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
			atax_kernel2_GXYW(A, tmp, y, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel2(y, y_ref);

			(*calculate)();

		}
	}

	return 0;
}
