#include <stdlib.h>
#include <math.h>
#include <iostream>
using namespace std;

# define NI 512
# define NJ 512
# define NK 512
# define NL 512
# define NM 512
#define DIM_LOCAL_WORK_GROUP_X 16
#define DIM_LOCAL_WORK_GROUP_Y 1

void threeMMkernel1_GXYW(float *A, float *B, float *E, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {
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

								if ((i < NI) && (j < NJ))
								{
									int k;
									E[i*NJ + j] = 0;
									
									(*access)(512*512*4+i*NJ + j, wy*widx+wx);
	
									for(k=0; k < NK; k++)
									{
										E[i * NJ + j] += A[i * NK + k] * B[k * NJ + j];
										(*access)(512*512*4+i * NJ + j, wy*widx+wx);
										(*access)(i * NK + k, wy*widx+wx);
										(*access)(512*512 + k * NJ + j, wy*widx+wx);
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

void threeMMkernel2_GXYW(float *C, float *D, float *F, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {
	
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
				
								if ((i < NJ) && (j < NL))
								{
									int k;
									F[i*NL + j] = 0;
		
									(*access)(512*512*5+i*NL + j, wy*widx+wx);

									for(k=0; k < NM; k++)
									{
										F[i * NL + j] += C[i * NM + k] * D[k * NL +j];
										(*access)(512*512*5+i*NL + j, wy*widx+wx);
										(*access)(512*512*2+i * NM + k, wy*widx+wx);
										(*access)(512*512*3+k * NL +j, wy*widx+wx);
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

void threeMMkernel3_GXYW(float *E, float *F, float *G, int widx, int widy, int lidx, int lidy, int cX, int cY, void (*access)(uint64_t addr, uint64_t wgid)) {

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

								if ((i < NI) && (j < NL))
								{
									int k;
									G[i*NL + j] = 0;

									(*access)(512*512*6+i*NL+j, wy*widx+wx);

									for(k=0; k < NJ; k++)
									{
										G[i * NL + j] += E[i * NJ + k] * F[k * NL + j];
										(*access)(512*512*4 + i * NJ + k, wy*widx+wx);
										(*access)(512*512*5 + k * NL + j, wy*widx+wx);
										(*access)(512*512*6 + i * NL + j, wy*widx+wx);
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


void threeMMkernel1_cpu(float *A, float *B, float *E) {

	for (int i = 0; i < NI; i++)
	{
		for (int j = 0; j < NJ; j++)
		{
			E[i*NJ+j] = 0;
			for (int k = 0; k < NK; ++k)
			{
				E[i*NJ+j] += A[i*NK+k] * B[k*NJ+j];
			}
		}
	}

	return;
}

void threeMMkernel2_cpu(float *C, float *D, float *F) {

	/* F := C*D */
	for (int i = 0; i < NJ; i++)
	{
		for (int j = 0; j < NL; j++)
		{
			F[i*NL+j] = 0;
			for (int k = 0; k < NM; ++k)
			{
				F[i*NL+j] += C[i*NM+k] * D[k*NL+j];
			}
		}
	}

	return;
}

void threeMMkernel3_cpu(float *E, float *F, float *G) {
	/* G := E*F */
	for (int i = 0; i < NI; i++)
	{
		for (int j = 0; j < NL; j++)
		{
			G[i*NL+j] = 0;
			for (int k = 0; k < NJ; ++k)
			{
				G[i*NL+j] += E[i*NJ+k] * F[k*NL+j];
			}
		}
	}

	return;
}

void init_data(float *A, float *B, float *C, float *D) {
	
	int i, j;

	for (i = 0; i < NI; i++)
	{
		for (j = 0; j < NK; j++)
		{
			A[i*NK+j] = ((float) i*j) / NI;
		}
	}
  
	for (i = 0; i < NK; i++)
	{
		for (j = 0; j < NJ; j++)
		{
			B[i*NJ+j] = ((float) i*(j+1)) / NJ;
		}
	}
  
	for (i = 0; i < NJ; i++)
	{
		for (j = 0; j < NM; j++)
		{
			C[i*NM+j] = ((float) i*(j+3)) / NL;
		}
	}
  
	for (i = 0; i < NM; i++)
	{
		for (j = 0; j < NL; j++)
		{
			D[i*NL+j] = ((float) i*(j+2)) / NK;
		}
	}

	return;
}

bool verify(float *G, float *G_ref) {

	for (int i = 0; i < NI; i++) {
		for (int j = 0; j < NJ; j++) {
			if (G[i * NJ + j] != G_ref[i * NJ + j]) {
				return false;
			}
		}
	}

	return true;
}
	

int threeMM_main(void (*access)(uint64_t addr, uint64_t wgid), void(*reset)(void), void(*calculate)(void), void(*dump)(void), int cX, int cY) {

	float *A = (float *)malloc(sizeof(float) * NI * NK);
	float *B = (float *)malloc(sizeof(float) * NK * NJ);
	float *C = (float *)malloc(sizeof(float) * NJ * NM);
	float *D = (float *)malloc(sizeof(float) * NM * NL);
	float *E = (float *)malloc(sizeof(float) * NI * NJ);
	float *F = (float *)malloc(sizeof(float) * NJ * NL);
	float *G = (float *)malloc(sizeof(float) * NI * NL);

	float *E_ref = (float *)malloc(sizeof(float) * NJ * NM);
	float *F_ref = (float *)malloc(sizeof(float) * NJ * NL);
	float *G_ref = (float *)malloc(sizeof(float) * NI * NL);

	init_data(A, B, C, D);
	
	threeMMkernel1_cpu(A, B, E_ref);	
	threeMMkernel2_cpu(C, D, F_ref);
	threeMMkernel3_cpu(E_ref, F_ref, G_ref);
	
	/* kernel 1 */
	int lidx = DIM_LOCAL_WORK_GROUP_X;
	int lidy = DIM_LOCAL_WORK_GROUP_Y;
	int gidx = (int)ceil(((float)NJ) / ((float)lidx)) * lidx;
	int gidy = (int)ceil(((float)NI) / ((float)lidy)) * lidy;
	int coalescingMax[2];
	coalescingMax[0] = gidx / lidx;
	coalescingMax[1] = gidy / lidy;

	for (int cX = 1; cX <= coalescingMax[0]; cX = 2*cX) {
		for (int cY = 1; cY <= coalescingMax[1]; cY = 2*cY) {
	
			(*reset)();

			int globalWorkSizeC[2];
			globalWorkSizeC[0] = (gidx / cX) / lidx;
			globalWorkSizeC[1] = (gidy / cY) / lidy;
			
			printf("global work size %d, %d local work size %d, %d\n", globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy);		
			threeMMkernel1_GXYW(A, B, E, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);			
			
			verify(E, E_ref);

			(*calculate)();			

			(*dump)();

		}
	}

	gidx = (size_t)ceil(((float)NL) / ((float)lidx)) * lidx;
	gidy = (size_t)ceil(((float)NJ) / ((float)lidy)) * lidy;
	coalescingMax[0] = gidx / lidx;
    coalescingMax[1] = gidy / lidy;
	
	for (int cX = 1; cX <= coalescingMax[0]; cX = 2*cX) {
        for (int cY = 1; cY <= coalescingMax[1]; cY = 2*cY) {

			(*reset)();

            int globalWorkSizeC[2];
            globalWorkSizeC[0] = (gidx / cX) / lidx;
            globalWorkSizeC[1] = (gidy / cY) / lidy;
	
			printf("global work size %d, %d local work size %d, %d\n", globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy);
			threeMMkernel2_GXYW(C, D, F, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);
     
			verify(F, F_ref);
	 	
			(*calculate)();

			(*dump)();

		}
    }

	gidx = (size_t)ceil(((float)NL) / ((float)lidx)) * lidx;
    gidy = (size_t)ceil(((float)NI) / ((float)lidy)) * lidy;
    coalescingMax[0] = gidx / lidx;
    coalescingMax[1] = gidy / gidy;
	
	for (int cX = 1; cX <= coalescingMax[0]; cX = 2*cX) {
        for (int cY = 1; cY <= coalescingMax[1]; cY = 2*cY) {

			(*reset)();
	
            int globalWorkSizeC[2];
            globalWorkSizeC[0] = (gidx / cX) / lidx;
            globalWorkSizeC[1] = (gidy / cY) / lidy;

			printf("global work size %d, %d local work size %d, %d\n", globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy);
			threeMMkernel3_GXYW(E, F, G, globalWorkSizeC[0], globalWorkSizeC[1], lidx, lidy, cX, cY, access);	

			verify(G, G_ref);
			
			(*calculate)();

			(*dump)();

		}
	}

	return 0;
}
