#include <stdlib.h>
#include <math.h>
#include <iostream>
using namespace std;

#define NX 4096
#define NY 4096
#define DIM_LOCAL_WORK_GROUP_X 16
#define DIM_LOCAL_WORK_GROUP_Y 1

#define A_OFFSET 0
#define S_OFFSET NX*NY
#define Q_OFFSET NX*NY+NY
#define P_OFFSET NX*NY+NY+NX
#define R_OFFSET NX*NY+NY+NX+NY

void bicg_kernel1_GXYW(float *A, float *p, float *q, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {
	
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

							if (i < NX) {
								q[i] = 0.0;
								(*access)(Q_OFFSET + i, wy*widx+wx);

								for(int j=0; j < NY; j++) {
									q[i] += A[i * NY + j] * p[j];
									(*access)(A_OFFSET + i * NY + j, wy*widx+wx);
									(*access)(P_OFFSET + j, wy*widx+wx);
									(*access)(Q_OFFSET + i, wy*widx+wx);
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

void bicg_kernel2_GXYW(float *A, float *r, float *s, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {
	
	/* iterate over work group */
	for (int wy = 0; wy < widy; wy++) {
		for (int wx = 0; wx < widx; wx++) {
			/* iterate over work item*/
			for (int ly = 0; ly < lidy; ly++) {
				for (int warp = 0; warp < lidx/16; warp++) {
					for (int w = 0; w < 16; w++) {
						int lx = warp * 16 + w;

						for (int x = 0; x < cX; x++) {
							int j = (wx * cX + x) * lidx + lx;
							if (j < NY) {
								s[j] = 0.0;
								(*access)(S_OFFSET + j, wy*widx+wx);

								for(int i = 0; i < NX; i++) {
									s[j] += A[i * NY + j] * r[i];
									(*access)(A_OFFSET + i * NY + j, wy*widx+wx);
									(*access)(R_OFFSET + i, wy*widx+wx);
									(*access)(S_OFFSET + j, wy*widx+wx);
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


void init_data(float *A, float *p, float *r) {

	for (int i = 0; i < NY; i++)
	{
		p[i] = i * M_PI;
	}

	for (int i = 0; i < NX; i++)
	{
		r[i] = i * M_PI;

		for (int j = 0; j < NY; j++)
		{
			A[i * NY + j] = ((float) i*j) / NX;
		}
	}

	return;
}

void bicg_cpu(float *A, float *p, float *r, float *s, float *q) {
	
	for (int i = 0; i < NY; i++)
	{
		s[i] = 0.0;
	}

	for (int i = 0; i < NX; i++)
	{
		q[i] = 0.0;
		for (int j = 0; j < NY; j++)
		{
			s[j] = s[j] + r[i] * A[i * NY + j];
			q[i] = q[i] + A[i * NY + j] * p[j];
		}
	}
	
	return;
}

void verify_kernel1(float *q, float *q_ref) {
	
	for (int i = 0; i < NX; i++) {
		if (q[i] != q_ref[i]) {
			cout << "Error in kernel1" << endl;
			return;
		}
	}
	return;
}

void verify_kernel2(float *s, float *s_ref) {

	for (int i = 0; i < NY; i++) {	
		if (s[i] != s_ref[i]) {
			cout << "Error in kernel2" << endl;
			return;
		}
	}
	return;
}

int bicg_main(void (*access)(uint64_t addr, uint64_t wgid), void(*reset)(void), void(*calculate)(void), void (*dump)(void), int cX, int cY) {

	float *A;
	float *s;
	float *q;
	float *p;
	float *r;
	float *s_ref;
	float *q_ref;

	A = (float *) malloc(NX * NY * sizeof(float));
	s = (float *) malloc(NY * sizeof(float));
	q = (float *) malloc(NX * sizeof(float));
	p = (float *) malloc(NY * sizeof(float));
	r = (float *) malloc(NX * sizeof(float));
	s_ref = (float *) malloc(NY * sizeof(float));
	q_ref = (float *) malloc(NX * sizeof(float));

	init_data(A, p, r);
	bicg_cpu(A, p, r, s_ref, q_ref);

	int lidx = DIM_LOCAL_WORK_GROUP_X;
	int lidy = DIM_LOCAL_WORK_GROUP_Y;
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

			bicg_kernel1_GXYW(A, p, q, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel1(q, q_ref);

			(*calculate)();

			(*dump)();

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
			bicg_kernel2_GXYW(A, r, s, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel2(s, s_ref);

			(*calculate)();

			(*dump)();
		
		}
	}

	return 0;	
}
