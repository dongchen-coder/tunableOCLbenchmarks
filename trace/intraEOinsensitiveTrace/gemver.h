#include <stdlib.h>
#include <math.h>
#include <iostream>
using namespace std;

#define nN 4096
#define DIM_LOCAL_WORK_GROUP_KERNEL_1_X 16
#define DIM_LOCAL_WORK_GROUP_KERNEL_1_Y 1
#define DIM_LOCAL_WORK_GROUP_KERNEL_2_X 16
#define DIM_LOCAL_WORK_GROUP_KERNEL_2_Y 1
#define DIM_LOCAL_WORK_GROUP_KERNEL_3_X 16
#define DIM_LOCAL_WORK_GROUP_KERNEL_3_Y 1

#define A_OFFSET 0
#define U1_OFFSET nN * nN
#define V1_OFFSET nN * nN + nN
#define U2_OFFSET nN * nN + 2 * nN
#define V2_OFFSET nN * nN + 3 * nN
#define X_OFFSET nN * nN + 4 * nN
#define Y_OFFSET nN * nN + 5 * nN
#define Z_OFFSET nN * nN + 6 * nN
#define W_OFFSET nN * nN + 7 * nN


void gemver_kernel1_GXYW(float *A, float *U1, float *V1, float *U2, float *V2, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {
	
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

								if ((i < nN) && (j < nN)) {
									A[i * nN + j] += U1[i] * V1[j] + U2[i] * V2[j];
									(*access)(U1_OFFSET + i, wy * widx + wx);
									(*access)(V1_OFFSET + j, wy * widx + wx);
									(*access)(U2_OFFSET + i, wy * widx + wx);
                                    (*access)(V2_OFFSET + j, wy * widx + wx);
									(*access)(A_OFFSET + i * nN + j, wy * widx + wx);
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

void gemver_kernel2_GXYW(float beta, float *A, float *X, float *Y, float *Z, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

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
							
							//if (i == 0) {
							//	cout << "wy widy ly lidy " << wy << " " << widy << " " << ly << " " << lidy << endl;
							//	cout << "wx cX x lidx lx " << wx << " "<< cX << " " << x << " " << lidx << " " << lx << endl;
							//	cout << i << endl; 
							//}

							if (i < nN) {
								X[i] = 0;
								(*access)(X_OFFSET + i, wy * widx + wx);
								int j;
								for(j = 0; j < nN; j++) {
									X[i] += beta * A[j * nN + i] * Y[j];
									(*access)(A_OFFSET + j * nN + i, wy * widx + wx);
									(*access)(Y_OFFSET + j, wy * widx + wx);
									(*access)(X_OFFSET + i, wy * widx + wx);
								}
								X[i] += Z[i];
								(*access)(Z_OFFSET + i, wy * widx + wx);
								(*access)(X_OFFSET + i, wy * widx + wx);
							}
						}
					}
				}
			}
		}
	}

	return;
}

void gemver_kernel3_GXYW(float alpha, float *A, float *X, float *W, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

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
								W[i] = 0;
								(*access)(W_OFFSET + i, wy * widx + wx);
								for(int j = 0; j < nN; j++) {
									W[i] += alpha * A[i * nN + j] * X[j];
									(*access)(A_OFFSET + i * nN + j, wy * widx + wx);
									(*access)(X_OFFSET + j, wy * widx + wx);
									(*access)(W_OFFSET + i, wy * widx + wx);
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

void init_data(float *alpha, float *beta, float *A, float *u1, float *v1, float *u2, float *v2, float *w, float *x, float *y, float *z) {
	
	*alpha = 43532;
	*beta = 12313;

	for (int i = 0; i < nN; i++)
	{
		u1[i] = i;
		u2[i] = (i+1)/ nN /2.0;
		v1[i] = (i+1)/ nN /4.0;
		v2[i] = (i+1)/ nN /6.0;
		y[i] = (i+1)/ nN /8.0;
		z[i] = (i+1)/ nN /9.0;
		x[i] = 0.0;
		w[i] = 0.0;

		for (int j = 0; j < nN; j++) {
			A[i * nN + j] = ((float) i*j) / nN;
		}
	}

	return;
}

void gemver_cpu(float alpha, float beta, float *A, float *u1, float *v1, float *u2, float *v2, float *w, float *x, float *y, float *z) {

	for (int i = 0; i < nN; i++) {
		for (int j = 0; j < nN; j++) {
			A[i * nN + j] = A[i * nN + j] + u1[i] * v1[j] + u2[i] * v2[j];
		}
	}

	for (int i = 0; i < nN; i++) {
		for (int j = 0; j < nN; j++) {
			x[i] = x[i] + beta * A[j * nN + i] * y[j];
		}
	}

	for (int i = 0; i < nN; i++) {
		x[i] = x[i] + z[i];
	}

	for (int i = 0; i < nN; i++) {
		for (int j = 0; j < nN; j++) {
			w[i] = w[i] +  alpha * A[i * nN + j] * x[j];
		}
	}

	return;
}

void verify_kernel1(float *A, float *A_ref) {
	
	for (int i = 0; i < nN; i++) {
		for (int j = 0; j < nN; j++) {
			if (A[i * nN + j] != A_ref[i * nN + j]) {
				cout << "Error in kernel 1" << endl;
				return;
			}
		}
	}

	return;
}

void verify_kernel2(float *x, float *x_ref) {

	for (int i = 0; i < nN; i++) {
		if (x[i] != x_ref[i]) {
			cout << "Error in kernel 2" << endl;
			return;
		}
	}
	return;
}

void verify_kernel3(float *w, float *w_ref) {
	
	for (int i = 0; i < nN; i++) {
		if (w[i] != w_ref[i]) {
			cout << "Error in kernel 3" << endl;
			return;
		}
	}
	
	return;
}


int gemver_main(void (*access)(uint64_t addr, uint64_t wgid), void(*reset)(void), void(*calculate)(void), void(*dump)(void), int cX, int cY) {

	float alpha;
	float beta;
	
	float *A;
	float *u1;
	float *v1;
	float *u2;
	float *v2;
	float *w;
	float *x;
	float *y;
	float *z;
	float *A_ref;
	float *x_ref;
	float *w_ref;

	A = (float *) malloc( nN * nN * sizeof(float));
	u1 = (float *) malloc( nN * sizeof(float));
	v1 = (float *) malloc( nN * sizeof(float));
	u2 = (float *) malloc( nN * sizeof(float));
	v2 = (float *) malloc( nN * sizeof(float));
	w = (float *) malloc( nN * sizeof(float));
	x = (float *) malloc( nN * sizeof(float));
	y = (float *) malloc( nN * sizeof(float));
	z = (float *) malloc( nN * sizeof(float));
	A_ref = (float *) malloc( nN * nN * sizeof(float));
	x_ref = (float *) malloc( nN * sizeof(float));
	w_ref = (float *) malloc( nN * sizeof(float));

	init_data(&alpha, &beta, A_ref, u1, v1, u2, v2, w_ref, x_ref, y, z);
	init_data(&alpha, &beta, A, u1, v1, u2, v2, w, x, y, z);

	gemver_cpu(alpha, beta, A_ref, u1, v1, u2, v2, w_ref, x_ref, y, z);

	int lidx = DIM_LOCAL_WORK_GROUP_KERNEL_1_X;
	int lidy = DIM_LOCAL_WORK_GROUP_KERNEL_1_Y;
	int gidx = (int)ceil(((float) nN) / ((float)lidx)) * lidx;
	int gidy = (int)ceil(((float) nN) / ((float)lidy)) * lidy;
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

			init_data(&alpha, &beta, A, u1, v1, u2, v2, w, x, y, z);

			gemver_kernel1_GXYW(A, u1, v1, u2, v2, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel1(A, A_ref);
			(*calculate)();
		}
	}

	lidx = DIM_LOCAL_WORK_GROUP_KERNEL_2_X;
	lidy = DIM_LOCAL_WORK_GROUP_KERNEL_2_Y;
	gidx = (int)ceil(((float) nN) / ((float)lidx)) * lidx;
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
			
			//cout << cX << " " << cY << endl;

			gemver_kernel2_GXYW(beta, A_ref, x, y, z, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel2(x, x_ref);
			(*calculate)();
		}
	}

	lidx = DIM_LOCAL_WORK_GROUP_KERNEL_3_X;
	lidy = DIM_LOCAL_WORK_GROUP_KERNEL_3_Y;
	gidx = (int)ceil(((float) nN) / ((float)lidx)) * lidx;
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

			gemver_kernel3_GXYW(alpha, A_ref, x, w, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel3(w, w_ref);
			(*calculate)();
		}
	}

	return 0;
}



