#include <stdlib.h>
#include <math.h>
#include <iostream>
using namespace std;

#define M 2048
#define N 2048

#define DIM_LOCAL_WORK_GROUP_KERNEL_1_X 16
#define DIM_LOCAL_WORK_GROUP_KERNEL_1_Y 1
#define DIM_LOCAL_WORK_GROUP_KERNEL_2_X 16
#define DIM_LOCAL_WORK_GROUP_KERNEL_2_Y 1
#define DIM_LOCAL_WORK_GROUP_KERNEL_3_X 16
#define DIM_LOCAL_WORK_GROUP_KERNEL_3_Y 1

#define DATA_OFFSET 0
#define MEAN_OFFSET M * N
#define SYMMAT_OFFSET M * N + M

void covariance_kernel1_GXYW(float *mean, float *data, float float_n, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

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

							if (j < M) {
								mean[j] = 0.0;
								(*access)(MEAN_OFFSET + j, wy * widx + wx);		
								for(int i = 0; i < N; i++) {
									mean[j] += data[i * M + j];
									(*access)(DATA_OFFSET + i * M + j, wy * widx + wx);
									(*access)(MEAN_OFFSET + j, wy * widx + wx);
								}
								mean[j] /= (float)float_n;
								(*access)(MEAN_OFFSET + j, wy * widx + wx);
							}
						}
					}
				}
			}
		}
	}

	return;
}

void covariance_kernel2_GXYW(float *mean, float *data, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

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

								if ((i < N) && (j < M)) {
									data[i * M + j] -= mean[j];
									(*access)(MEAN_OFFSET + j, wy * widx + wx);
									(*access)(DATA_OFFSET + i * M + j, wy * widx + wx);
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

void covariance_kernel3_GXYW(float *data, float *symmat, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

	/* iterate over work group */
	for (int wy = 0; wy < widy; wy++) {
		for (int wx = 0; wx < widx; wx++) {
			/* iterate over work item*/
			for (int ly = 0; ly < lidy; ly++) {
				for (int warp = 0; warp < lidx/16; warp++) {
					for (int w = 0; w < 16; w++) {
						int lx = warp * 16 + w;

						for (int x = 0; x < cX; x++) {
							int j1 = (wx * cX + x) * lidx + lx;

							if (j1 < M) {
								for (int j2 = j1; j2 < M; j2++) {		
									symmat[j1 * M + j2] = 0.0;
									(*access)(SYMMAT_OFFSET + j1 * M + j2, wy * widx + wx);
									for(int i = 0; i < N; i++) {
										symmat[j1 * M + j2] += data[i * M + j1] * data[i * M + j2];
										(*access)(DATA_OFFSET + i * M + j1, wy * widx + wx);
										(*access)(DATA_OFFSET + i * M + j2, wy * widx + wx);
										(*access)(SYMMAT_OFFSET + j1 * M + j2, wy * widx + wx);
									}
									symmat[j2 * M + j1] = symmat[j1 * M + j2];
									(*access)(SYMMAT_OFFSET + j1 * M + j2, wy * widx + wx);
									(*access)(SYMMAT_OFFSET + j2 * M + j1, wy * widx + wx);
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

void covariance_cpu(float *data, float *mean, float *symmat, float float_n) {

	for (int j = 0; j < M; j++) {
		mean[j] = 0.0;
		for (int i = 0; i < N; i++) {
				mean[j] += data[i * M + j];
		}
		mean[j] /= float_n;
	}

  	/* Center the column vectors. */
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			data[i * M + j] -= mean[j];
		}
	}

  	/* Calculate the m * m covariance matrix. */
	for (int j1 = 0; j1 < M; j1++) {
		for (int j2 = j1; j2 < M; j2++) {
			symmat[j1 * M + j2] = 0.0;
			for (int i = 0; i < N; i++) {
				symmat[j1 * M + j2] += data[i * M + j1] * data[i * M + j2];
			}
			symmat[j2 * M + j1] = symmat[j1 * M + j2];
		}
	}

	return;
}


void init_data(float * data) {
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			data[i * N + j] = ((float) i*j) / M;
		}
	}
	return;
}

void verify_kernel1(float *mean, float *mean_ref) {

	for (int i = 0; i < M; i++) {
		if (mean[i] != mean_ref[i]) {
			cout << "Error in Kernel1 " << i << " " << mean[i] << " " << mean_ref[i] << endl;
			return;
		}
	}

	return;
}

void verify_kernel2(float *data, float *data_ref) {

	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			if (data[i * N + j] != data_ref[i * N + j]) {
				cout << "Error in Kernel2" << endl;
				return;
			}
		}
	}

	return;
}

void verify_kernel3(float *symmat, float *symmat_ref) {

	for (int i = 0; i < M; i++) {
		for (int j = 0; j < M; j++) {
			if (symmat[i * M + j] != symmat_ref[i * M + j]) {
				cout << "Error in Kernel 3" << endl;
				return;
			}
		}
	}

	return;
}

int covariance_main(void (*access)(uint64_t addr, uint64_t wgid), void(*reset)(void), void(*calculate)(void)) {

	float float_n= 3214212.01;

	float *data;
	float *symmat;
	float *mean;
	float *data_ref;
	float *symmat_ref;
	float *mean_ref;

	data = (float *) malloc(M * N * sizeof(float));
	symmat = (float *) malloc(M * M * sizeof(float));
	mean = (float *) malloc(M * sizeof(float));

	data_ref = (float *) malloc(M * N * sizeof(float));
	symmat_ref = (float *) malloc(M * M * sizeof(float));
	mean_ref = (float *) malloc(M * sizeof(float));

	init_data(data);
	init_data(data_ref);
	covariance_cpu(data_ref, mean_ref, symmat_ref, float_n);

	int lidx = DIM_LOCAL_WORK_GROUP_KERNEL_1_X;
	int lidy = DIM_LOCAL_WORK_GROUP_KERNEL_1_Y;
	int gidx = (int)ceil(((float)M) / ((float)lidx)) * lidx;
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

			covariance_kernel1_GXYW(mean, data, float_n, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel1(mean, mean_ref);

			(*calculate)();

		}
	}

	lidx = DIM_LOCAL_WORK_GROUP_KERNEL_2_X;
	lidy = DIM_LOCAL_WORK_GROUP_KERNEL_2_Y;
	gidx = (int)ceil(((float)M) / ((float)lidx)) * lidx;
	gidy = (int)ceil(((float)N) / ((float)lidy)) * lidy;
	coalescingMax[0] = gidx / lidx;
	coalescingMax[1] = gidy / lidy;

	for (int cX = 1; cX <= coalescingMax[0]; cX = 2*cX) {
		for (int cY = 1; cY <= coalescingMax[1]; cY = 2*cY) {

			(*reset)();

			init_data(data);

			int globalWorkSizeC[2];
			globalWorkSizeC[0] = (gidx / cX) / lidx;
			globalWorkSizeC[1] = (gidy / cY) / lidy;

			cout << "global work size " << globalWorkSizeC[0] << " " << globalWorkSizeC[1] << " local work size " << lidx << " " << lidy << endl;

			covariance_kernel2_GXYW(mean_ref, data, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel2(data, data_ref);

			(*calculate)();

		}
	}

	lidx = DIM_LOCAL_WORK_GROUP_KERNEL_3_X;
	lidy = DIM_LOCAL_WORK_GROUP_KERNEL_3_Y;
	gidx = (int)ceil(((float)M) / ((float)lidx)) * lidx;
	gidy = 1;
	coalescingMax[0] = gidx / lidx;
	coalescingMax[1] = gidy / lidy;

	for (int cX = 1; cX <= coalescingMax[0]; cX = 2*cX) {
		for (int cY = 1; cY <= coalescingMax[1]; cY = 2*cY) {

			(*reset)();

			init_data(data);

			int globalWorkSizeC[2];
			globalWorkSizeC[0] = (gidx / cX) / lidx;
			globalWorkSizeC[1] = (gidy / cY) / lidy;

			cout << "global work size " << globalWorkSizeC[0] << " " << globalWorkSizeC[1] << " local work size " << lidx << " " << lidy << endl;
	
			covariance_kernel3_GXYW(data_ref, symmat, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);
		
			verify_kernel3(symmat, symmat_ref);

			(*calculate)();
		}
	}

	return 0;
}
