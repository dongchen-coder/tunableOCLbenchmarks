#include <stdlib.h>
#include <math.h>
#include <iostream>
using namespace std;

#define M 2048
#define N 2048

#define FLOAT_N 3214212.01
#define EPS 0.005

#define DIM_LOCAL_WORK_GROUP_KERNEL_1_X 16
#define DIM_LOCAL_WORK_GROUP_KERNEL_1_Y 1
#define DIM_LOCAL_WORK_GROUP_KERNEL_2_X 16
#define DIM_LOCAL_WORK_GROUP_KERNEL_2_Y 1
#define DIM_LOCAL_WORK_GROUP_KERNEL_3_X 16
#define DIM_LOCAL_WORK_GROUP_KERNEL_3_Y 1
#define DIM_LOCAL_WORK_GROUP_KERNEL_4_X 16
#define DIM_LOCAL_WORK_GROUP_KERNEL_4_Y 1

#define DATA_OFFSET 0
#define MEAN_OFFSET M * N
#define STDDEV_OFFSET M * N + M
#define SYMMAT_OFFSET M * N + M + M

void correlation_kernel1_GXYW(float *data, float *mean, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

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
								for (int i = 0; i < N; i++) {
									mean[j] += data[i * M + j];
									(*access)(DATA_OFFSET + i * M + j, wy * widx + wx);
									(*access)(MEAN_OFFSET + j, wy * widx + wx);
								}
								mean[j] /= (float)FLOAT_N;
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

void correlation_kernel2_GXYW(float *data, float *mean, float *stddev, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

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
								stddev[j] = 0.0;
								(*access)(STDDEV_OFFSET + j, wy * widx + wx);
								for (int i = 0; i < N; i++) {
									stddev[j] += (data[i * M + j] - mean[j]) * (data[i * M + j] - mean[j]);
									(*access)(DATA_OFFSET + i * M + j, wy * widx + wx);
									(*access)(MEAN_OFFSET + j, wy * widx + wx);
									(*access)(DATA_OFFSET + i * M + j, wy * widx + wx);
									(*access)(MEAN_OFFSET + j, wy * widx + wx);
									(*access)(STDDEV_OFFSET + j, wy * widx + wx);
								}
								stddev[j] /= FLOAT_N;
								(*access)(STDDEV_OFFSET + j, wy * widx + wx);
								stddev[j] =  sqrt(stddev[j]);
								(*access)(STDDEV_OFFSET + j, wy * widx + wx);
								(*access)(STDDEV_OFFSET + j, wy * widx + wx);
								(*access)(STDDEV_OFFSET + j, wy * widx + wx);
								if(stddev[j] <= EPS) {
									stddev[j] = 1.0;
									(*access)(STDDEV_OFFSET + j, wy * widx + wx);
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

void correlation_kernel3_GXYW(float *mean, float *stddev, float *data, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

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
									data[i * M + j] /= (sqrt(FLOAT_N) * stddev[j]);
									(*access)(MEAN_OFFSET + j, wy * widx + wx);
									(*access)(DATA_OFFSET + i * M + j, wy * widx + wx);
									(*access)(STDDEV_OFFSET + j, wy * widx + wx);
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

void correlation_kernel4_GXYW(float *data, float *symmat, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

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

							if (j1 < (M-1)) {
								symmat[j1 * M + j1] = 1.0;
								(*access)(SYMMAT_OFFSET + j1 * M + j1, wy * widx + wx);
								for (int j2 = (j1 + 1); j2 < M; j2++) {
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


void correlation_cpu(float *data, float *mean, float *stddev, float *symmat) {

	for (int j = 0; j < M; j++) {
		mean[j] = 0.0;
		for (int i = 0; i < N; i++) {
			mean[j] += data[i * M + j];
		}
		mean[j] /= (float)FLOAT_N;
	}

	// Determine standard deviations of column vectors of data matrix. 
	for (int j = 0; j < M; j++) {
		stddev[j] = 0.0;
		for (int i = 0; i < N; i++) {
			stddev[j] += (data[i * M + j] - mean[j]) * (data[i * M + j] - mean[j]);
		}
		
		stddev[j] /= FLOAT_N;
		stddev[j] = sqrt(stddev[j]);
		stddev[j] = stddev[j] <= EPS ? 1.0 : stddev[j];
	}

	// Center and reduce the column vectors. 
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			data[i * M + j] -= mean[j];
			data[i * M + j] /= (sqrt(FLOAT_N)*stddev[j]) ;
		}
	}

	// Calculate the m * m correlation matrix. 
	for (int j1 = 0; j1 < M-1; j1++) {
		symmat[j1 * M + j1] = 1.0;
		for (int j2 = j1+1; j2 < M; j2++) {
			symmat[j1 * M + j2] = 0.0;

			for (int i = 0; i < N; i++) {
				symmat[j1 * M + j2] += (data[i * M + j1] * data[i * M + j2]);
			}

			symmat[j2 * M + j1] = symmat[j1 * M + j2];
		}
	}
 
	symmat[(M-1) * M + M-1] = 1.0;

	return;
}

void init_data(float *data) {
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
	   		data[i * N + j] = ((float) i*j)/ M;	
	   	}
	}
	return;
}

void verify_kernel1(float *mean, float *mean_ref) {
	
	for (int i = 0; i < M; i++) {
		if (mean[i] != mean_ref[i]) {
			cout << "Error in kernel1" << endl;
		}
	}
	
	return;
}

void verify_kernel2(float *stddev, float *stddev_ref) {
    
	for (int i = 0; i < M; i++) {
		if (stddev[i] != stddev_ref[i]) {
			cout << "Error in kernel2" << endl;
		}
	}
	   
    return;
}

void verify_kernel3(float *data, float *data_ref) {
    
    for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			if (data[i * N + j] != data_ref[i * N + j]) {
				cout << "Error in kernel3" << endl;
			}
		}
	}
	
    return;
}

void verify_kernel4(float *symmat, float *symmat_ref) {
 
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < M; j++) {
			if (symmat[i * M + j] != symmat_ref[i * M + j]) {
				cout << "Error in kernel4" << endl;
			}
		}
	}	   
    
    return;
}


int correlation_main(void (*access)(uint64_t addr, uint64_t wgid), void(*reset)(void), void(*calculate)(void)) {
	
	float *data;
	float *mean;
	float *stddev;
	float *symmat;

	float *data_ref;
	float *mean_ref;
	float *stddev_ref;
	float *symmat_ref;
	

	data = (float *) malloc(M * N * sizeof(float));
	mean = (float *) malloc(M * sizeof(float));	
	stddev = (float *) malloc(M * sizeof(float));
	symmat = (float *) malloc(M * M * sizeof(float));

	data_ref = (float *) malloc(M * N * sizeof(float));
	mean_ref = (float *) malloc(M * sizeof(float));
	stddev_ref = (float *) malloc(M * sizeof(float));
	symmat_ref = (float *) malloc(M * M * sizeof(float));

	init_data(data_ref);
	correlation_cpu(data_ref, mean_ref, stddev_ref, symmat_ref);
	init_data(data);

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
			correlation_kernel1_GXYW(data, mean, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel1(mean, mean_ref);

			(*calculate)();	
		}
	}

	lidx = DIM_LOCAL_WORK_GROUP_KERNEL_2_X;
    lidy = DIM_LOCAL_WORK_GROUP_KERNEL_2_Y;
    gidx = (int)ceil(((float)M) / ((float)lidx)) * lidx;
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
	
			correlation_kernel2_GXYW(data, mean_ref, stddev, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel2(stddev, stddev_ref);

			(*calculate)();
		}
	}
		
	lidx = DIM_LOCAL_WORK_GROUP_KERNEL_3_X;
    lidy = DIM_LOCAL_WORK_GROUP_KERNEL_3_Y;
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
	
			correlation_kernel3_GXYW(mean_ref, stddev_ref, data, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);

			verify_kernel3(data, data_ref);

			(*calculate)();
		}
	}
			
	lidx = DIM_LOCAL_WORK_GROUP_KERNEL_4_X;
    lidy = DIM_LOCAL_WORK_GROUP_KERNEL_4_Y;
    gidx = (int)ceil(((float)M) / ((float)lidx)) * lidx;
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

			correlation_kernel4_GXYW(data_ref, symmat, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);
			
			verify_kernel4(data, data_ref);

			(*calculate)();
		}
	}
	return 0;
}
