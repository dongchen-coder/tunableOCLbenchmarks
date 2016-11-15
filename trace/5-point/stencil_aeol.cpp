#include "../utilities/aeol.h"


void stencil(int * a, int * b, unsigned int dim_size, int isize, int jsize, int gxsize, int gysize)
{
	unsigned int size = dim_size + 2;
      
	for (int gidX = 0; gidX < gxsize; gidX++) {
		for (int gidY = 0; gidY < gysize; gidY++) {
			unsigned int gidx = gidX * isize + 1;
			unsigned int gidy = gidY * jsize + 1;
			for (int i = 0; i < isize; i++) {
				for (int j = 0; j < jsize; j++) {
					b[(gidx + i) * size + gidy + j] = a[(gidx+i) * size + gidy + j]
								+ a[(gidx+i) * size + gidy + 1 + j]
								+ a[(gidx+i) * size + gidy - 1 + j]
								+ a[(gidx +i- 1) * size + gidy + j]
								+ a[(gidx +i+ 1) * size + gidy + j];
				}
			}
		}
	}
	return;
}

void stencil_trace_GXYW(int *a, int *b, unsigned int dim_size, int gxsize, int gysize, int isize, int jsize) {
	unsigned int size = dim_size + 2;
	int wgsizeX = gxsize / 32;
	int wgsizeY = gysize / 32;
	for (int gidY = 0; gidY < wgsizeY; gidY++) {
		for (int gidX = 0; gidX < wgsizeX; gidX++) {
			for (int lidY = 0; lidY < 32; lidY++) {
				int warpN = 32 / 16;
				for (int warp = 0; warp < warpN; warp++) {
					unsigned int gidy = (gidY*32+lidY) * jsize + 1;
					for (int i = 0; i < isize; i++) {
						for (int j = 0; j < jsize; j++) {
							for (int w = 0; w < 16; w++) {
								int lidX = 16 * warp + w;
								unsigned int gidx = (gidX*32+lidX) * isize + 1;
								//rtTmpAccess((gidx+i) * size + gidy + j);
								access((gidx+i) * size + gidy + j, gidY * wgsizeX + gidX);
							}
							for (int w = 0; w < 16; w++) {
								int lidX = 16 * warp + w;
								unsigned int gidx = (gidX*32+lidX) * isize + 1;
								//rtTmpAccess((gidx+i) * size + gidy + 1 + j);
								access((gidx+i) * size + gidy + 1 + j, gidY * wgsizeX + gidX);
							}
							for (int w = 0; w < 16; w++) {
								int lidX = 16 * warp + w;
								unsigned int gidx = (gidX*32+lidX) * isize + 1;
								//rtTmpAccess((gidx+i) * size + gidy - 1 + j);
								access((gidx+i) * size + gidy - 1 + j, gidY * wgsizeX + gidX);
							}
							for (int w = 0; w < 16; w++) {
								int lidX = 16 * warp + w;
								unsigned int gidx = (gidX*32+lidX) * isize + 1;
								//rtTmpAccess((gidx +i- 1) * size + gidy + j);
								access((gidx +i- 1) * size + gidy + j, gidY * wgsizeX + gidX);
							}
							for (int w = 0; w < 16; w++) {
								int lidX = 16 * warp + w;
								unsigned int gidx = (gidX*32+lidX) * isize + 1;
								//rtTmpAccess((gidx +i+ 1) * size + gidy + j);
								access((gidx +i+ 1) * size + gidy + j, gidY * wgsizeX + gidX);
							}
							for (int w = 0; w < 16; w++) {
								int lidX = 16 * warp + w;
								unsigned int gidx = (gidX*32+lidX) * isize + 1;
								//rtTmpAccess((gidx + i) * size + gidy + j + 1026*1026);
								access((gidx + i) * size + gidy + j + 1026*1026, gidY * wgsizeX + gidX);
								b[(gidx + i) * size + gidy + j] = a[(gidx+i) * size + gidy + j] +
											a[(gidx+i) * size + gidy + 1 + j] + a[(gidx+i) * size + gidy - 1 + j] +
											a[(gidx +i- 1) * size + gidy + j] + a[(gidx +i+ 1) * size + gidy + j];
							}
						}
					}
				}
			}
		}
	}
	return;
}

void stencil_trace_GXYW_RR(int *a, int *b, unsigned int dim_size, int gxsize, int gysize, int isize, int jsize) {

	unsigned int size = dim_size + 2;
	int wgsizeX = gxsize / 32;
	int wgsizeY = gysize / 32;
	for (int gidY = 0; gidY < wgsizeY; gidY++) {
		for (int gidX = 0; gidX < wgsizeX; gidX++) {
			for (int i = 0; i < isize; i++) {
				for (int j = 0; j < jsize; j++) {
					for (int lidY = 0; lidY < 32; lidY++) {
						for (int lidX = 0; lidX < 32; lidX++) {
							unsigned int gidx = (gidX*32+lidX) * isize + 1;
							unsigned int gidy = (gidY*32+lidY) * jsize + 1;
							//rtTmpAccess((gidx+i) * size + gidy + j);
							access((gidx+i) * size + gidy + j, gidY * wgsizeX + gidX);
						}
					}
					for (int lidY = 0; lidY < 32; lidY++) {
						for (int lidX = 0; lidX < 32; lidX++) {
							unsigned int gidx = (gidX*32+lidX) * isize + 1;
							unsigned int gidy = (gidY*32+lidY) * jsize + 1;
							//rtTmpAccess((gidx+i) * size + gidy + 1 + j);
							access((gidx+i) * size + gidy + 1 + j, gidY * wgsizeX + gidX);
						}
					}
					for (int lidY = 0; lidY < 32; lidY++) {
						for (int lidX = 0; lidX < 32; lidX++) {
							unsigned int gidx = (gidX*32+lidX) * isize + 1;
							unsigned int gidy = (gidY*32+lidY) * jsize + 1;
							//rtTmpAccess((gidx+i) * size + gidy - 1 + j);
							access((gidx+i) * size + gidy - 1 + j, gidY * wgsizeX + gidX);
						}
					}
					for (int lidY = 0; lidY < 32; lidY++) {
						for (int lidX = 0; lidX < 32; lidX++) {
							unsigned int gidx = (gidX*32+lidX) * isize + 1;
							unsigned int gidy = (gidY*32+lidY) * jsize + 1;
							//rtTmpAccess((gidx +i- 1) * size + gidy + j);
							access((gidx +i- 1) * size + gidy + j, gidY * wgsizeX + gidX);
						}
					}
					for (int lidY = 0; lidY < 32; lidY++) {
						for (int lidX = 0; lidX < 32; lidX++) {
							unsigned int gidx = (gidX*32+lidX) * isize + 1;
							unsigned int gidy = (gidY*32+lidY) * jsize + 1;
							//rtTmpAccess((gidx +i+ 1) * size + gidy + j);
							access((gidx +i+ 1) * size + gidy + j, gidY * wgsizeX + gidX);
						}
					}
					for (int lidY = 0; lidY < 32; lidY++) {
						for (int lidX = 0; lidX < 32; lidX++) {
							unsigned int gidx = (gidX*32+lidX) * isize + 1;
							unsigned int gidy = (gidY*32+lidY) * jsize + 1;
							//rtTmpAccess((gidx + i) * size + gidy + j + 1026*1026);
							access((gidx + i) * size + gidy + j + 1026*1026, gidY * wgsizeX + gidX);
							b[(gidx + i) * size + gidy + j] = a[(gidx+i) * size + gidy + j] +
										a[(gidx+i) * size + gidy + 1 + j] + a[(gidx+i) * size + gidy - 1 + j] +
										a[(gidx +i- 1) * size + gidy + j] + a[(gidx +i+ 1) * size + gidy + j];
						}
					}
				}
			}
		}	
	}
	return;
}

bool varify(int * b, int * a) {
	for (int i = 1; i < 1025; i++) {
		for (int j = 1; j < 1025; j++) {
			//if (b[i * 1026 + j] != a[i*1026+j] + a[i*1026+j + 1] + a[i*1026+j - 1] + a[(i-1)*1026+j] + a[(i+1)*1026+j]) {
			//	return false;
			//}
			if (a[i * 1026 + j] != b[i * 1026 + j]) {
				return false;
			}
		}
	}
	return true;
}

void testCase () {
	access(1, 0);
	access(2, 0);
	access(3, 0);
	access(1, 0);
	access(2, 1);
	access(3, 1);
	access(1, 1);
	access(1, 1);
	access(2, 2);
	access(3, 2);
	access(2, 2);
	access(3, 2);
	return;
}

int main() {
	int* a = (int*)malloc(1026*1026*sizeof(int));
	int* b = (int*)malloc(1026*1026*sizeof(int));
	int* b_r = (int*) malloc(1026*1026*sizeof(int));

	for (int i = 0; i < 1026 * 1026; i++) {
		a[i] = i % 256;
	} 	


	stencil(a, b_r, 1024, 1024, 1024, 1, 1);
	
	for (int i = 32; i <= 1024; i = i * 2) {
		for (int j = 32; j <= 1024; j = j * 2) {
			cout << i << " " << j << " --------" << endl;
			stencil_trace_GXYW(a, b, 1024, i, j, 1024/i, 1024/j);		
			
			calculateRTDistribution();

			//aeol();
			//dumpD();
			//dumpf();
			//dumpe();
			//dumpInterRT();			

			reset();
		}
	}

	/*
	testCase();
	aeol();
	dumpD();
	dumpL();
	dumpf();
	dumpe();
	dumpInterRT();  	
	reset();
	*/
	return 0;
}





