#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define N 4096

#define DIM_LOCAL_WORK_GROUP_X 256
#define DIM_LOCAL_WORK_GROUP_Y 1

#define A_OFFSET 0
#define B_OFFSET N*N
#define X_OFFSET N*N+N
#define Y_OFFSET N*N+2*N
#define TMP_OFFSET N*N+3*N

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

							if (i < N) {
								tmp[i] = 0;
								y[i] = 0;
								
								(*access)(TMP_OFFSET + i, wy*widx+wx);
								(*access)(Y_OFFSET + i, wy*widx+wx);
								
								for(int j = 0; j < N; j++) {
									tmp[i] += A[i * N + j] * x[j];
									y[i] += B[i * N + j] * x[j];
									(*access)(A_OFFSET + i * N + j, wy*widx+wx);
									(*access)(X_OFFSET + j, wy*widx+wx);
									(*access)(TMP_OFFSET + i, wy*widx+wx);
									(*access)(B_OFFSET + i * N + j, wy*widx+wx);
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

	for (int i = 0; i < N; i++) {
		x[i] = ((float) i) / N;

		for (int j = 0; j < N; j++) {
			A[i * N + j] = ((float) i*j) / N;
			B[i * N + j] = ((float) i*j) / N;
		}
	}	

	return;
}

void gesummv_cpu(float alpha, float beta, float *A, float *B, float *tmp, float *x, float *y) {

	for (int i = 0; i < N; i++) {
		tmp[i] = 0;
		y[i] = 0;
		for (int j = 0; j < N; j++)
		{
			tmp[i] = A[i * N + j] * x[j] + tmp[i];
			y[i] = B[i * N + j] * x[j] + y[i];
		}
		
		y[i] = alpha * tmp[i] + beta * y[i];
	}

	return;
}

void verify(float *y, float * y_ref) {

	for (int i = 0; i < N; i++) {
		if (y[i] != y_ref[i]) {
			printf("Error in kernel\n");
			return;
		}
	}

	return;
}

int gesummv_main(void (*access)(uint64_t addr, uint64_t wgid), void(*reset)(void), void(*calculate)(void)) {
	
	float alpha;
	float beta;

	float *A;
	float *B;
	float *tmp;
	float *x;
	float *y;
	float *y_ref;

	A = (float *) malloc(N * N * sizeof(float));
	B = (float *) malloc(N * N * sizeof(float));
	tmp = (float *) malloc(N * sizeof(float));
	x = (float *) malloc(N * sizeof(float));
	y = (float *) malloc(N * sizeof(float));
	y_ref = (float *) malloc(N * sizeof(float));

	init_data(&alpha, &beta, A, B, x);

	gesummv_cpu(alpha, beta, A, B, tmp, x, y_ref);

	int lidx = DIM_LOCAL_WORK_GROUP_X;
	int lidy = 1;
	int gidx = (int)ceil(((float)N) / ((float)lidx)) * lidx;
	int gidy = 1;
	int coalescingMax[2];
	coalescingMax[0] = gidx / lidx;
	coalescingMax[1] = gidy / lidy;
	
	for (int cX = 1; cX <= coalescingMax[0]; cX = 2*cX) {
		for (int cY = 1; cY <= coalescingMax[1]; cY = 2*cY) {

			init_data(&alpha, &beta, A, B, x);
			(*reset)();

			int globalWorkSizeC[2];
			globalWorkSizeC[0] = gidx / cX;
			globalWorkSizeC[1] = gidy / cY;

			printf("global work size %d, %d local work size %d, %d\n", globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy);
			gesummv_kernel(alpha, beta, A, B, x, tmp, y, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify(y, y_ref);
			
			(*calculate)();
			
		}
	}



	return 0;
}


