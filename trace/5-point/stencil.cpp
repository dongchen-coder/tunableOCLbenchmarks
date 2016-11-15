//#include<iostream>
//#include<map>
#include "wb.h"
//using namespace std;

std::map<int, int> rtTmp;
std::map<int, int> fat;
std::map<int, int> lat;
unsigned long long refT = 0;

void rtTmpAccess(int addr) {
	refT ++;
	if (fat.find(addr) == fat.end()) {
		fat[addr] = refT;
		lat[addr] = refT;
	} else {
		if (rtTmp.find(refT - lat[addr]) == rtTmp.end()) {
			rtTmp[refT - lat[addr]] = 1;
		} else {
			rtTmp[refT - lat[addr]] ++;
		}
		lat[addr] = refT;
	}
	return;
}

void dumpRtTmp() {
	for (std::map<int, int>::iterator it = rtTmp.begin(), eit = rtTmp.end(); it != eit; ++it) {
		cout << it->first << "	" << it->second << endl;
	}

	rtTmp.clear();
	fat.clear();
	lat.clear();

	return;
}



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

void stencil_trace_XY( int * a, int * b, unsigned int dim_size, int gxsize, int gysize, int isize, int jsize)
{

      unsigned int size = dim_size + 2;

      for (int gidX = 0; gidX < gxsize; gidX++) {
        for (int gidY = 0; gidY < gysize; gidY++) {
          unsigned int gidx = gidX * isize + 1;
          unsigned int gidy = gidY * jsize + 1;
          for (int i = 0; i < isize; i++) {
            for (int j = 0; j < jsize; j++) {
							
							rtTmpAccess((gidx+i) * size + gidy + j);
							rtTmpAccess((gidx+i) * size + gidy + 1 + j);
							rtTmpAccess((gidx+i) * size + gidy - 1 + j);
							rtTmpAccess((gidx +i- 1) * size + gidy + j);
							rtTmpAccess((gidx +i+ 1) * size + gidy + j);
							rtTmpAccess((gidx + i) * size + gidy + j + 1026*1026);
							/*
							Access((gidx+i) * size + gidy + j, 'R');
							Access((gidx+i) * size + gidy + 1 + j,'R');
							Access((gidx+i) * size + gidy - 1 + j,'R');
							Access((gidx +i- 1) * size + gidy + j,'R');
							Access((gidx +i+ 1) * size + gidy + j,'R');
							Access((gidx + i) * size + gidy + j + 1026*1026,'W');
							*/
						}
          }
        }
      }
      return;
}

void stencil_trace_YX( int * a, int * b, unsigned int dim_size, int gxsize, int gysize, int isize, int jsize)
{


      unsigned int size = dim_size + 2;

			for (int gidY = 0; gidY < gysize; gidY++) {
      	for (int gidX = 0; gidX < gxsize; gidX++) {
          unsigned int gidx = gidX * isize + 1;
          unsigned int gidy = gidY * jsize + 1;
          for (int i = 0; i < isize; i++) {
            for (int j = 0; j < jsize; j++) {
              Access((gidx+i) * size + gidy + j, 'R');
              Access((gidx+i) * size + gidy + 1 + j,'R');
              Access((gidx+i) * size + gidy - 1 + j,'R');
              Access((gidx +i- 1) * size + gidy + j,'R');
              Access((gidx +i+ 1) * size + gidy + j,'R');
              Access((gidx + i) * size + gidy + j + 1026*1026,'W');
            }
          }
        }
      }
      return;
}

void stencil_trace_GXY( int * a, int * b, unsigned int dim_size, int gxsize, int gysize, int isize, int jsize)
{
      unsigned int size = dim_size + 2;

			int wgsizeX = gxsize / 32;
			int wgsizeY = gysize / 32;

      for (int gidX = 0; gidX < wgsizeX; gidX++) {
        for (int gidY = 0; gidY < wgsizeY; gidY++) {
					for (int lidX = 0; lidX < 32; lidX++) {
						for (int lidY = 0; lidY < 32; lidY++) {
          		unsigned int gidx = (gidX*32+lidX) * isize + 1;
          		unsigned int gidy = (gidY*32+lidY) * jsize + 1;
          		for (int i = 0; i < isize; i++) {
         	   		for (int j = 0; j < jsize; j++) {
              		Access((gidx+i) * size + gidy + j, 'R');
              		Access((gidx+i) * size + gidy + 1 + j,'R');
              		Access((gidx+i) * size + gidy - 1 + j,'R');
              		Access((gidx +i- 1) * size + gidy + j,'R');
              		Access((gidx +i+ 1) * size + gidy + j,'R');
              		Access((gidx + i) * size + gidy + j + 1026*1026,'W');
            		}
							}
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
									Access((gidx+i) * size + gidy + j, 'R');
								}
								for (int w = 0; w < 16; w++) {
          				int lidX = 16 * warp + w;
                  unsigned int gidx = (gidX*32+lidX) * isize + 1;
									//rtTmpAccess((gidx+i) * size + gidy + 1 + j);
          				Access((gidx+i) * size + gidy + 1 + j, 'R');
								}
								for (int w = 0; w < 16; w++) {
                  int lidX = 16 * warp + w;
                  unsigned int gidx = (gidX*32+lidX) * isize + 1;
									//rtTmpAccess((gidx+i) * size + gidy - 1 + j);
          				Access((gidx+i) * size + gidy - 1 + j, 'R');
								}
								for (int w = 0; w < 16; w++) {
                  int lidX = 16 * warp + w;
                  unsigned int gidx = (gidX*32+lidX) * isize + 1;
									//rtTmpAccess((gidx +i- 1) * size + gidy + j);
									Access((gidx +i- 1) * size + gidy + j, 'R');
								}
								for (int w = 0; w < 16; w++) {
                  int lidX = 16 * warp + w;
                  unsigned int gidx = (gidX*32+lidX) * isize + 1;
									//rtTmpAccess((gidx +i+ 1) * size + gidy + j);
          				Access((gidx +i+ 1) * size + gidy + j, 'R');
								}
								for (int w = 0; w < 16; w++) {
                  int lidX = 16 * warp + w;
                  unsigned int gidx = (gidX*32+lidX) * isize + 1;
									//rtTmpAccess((gidx + i) * size + gidy + j + 1026*1026);
									Access((gidx + i) * size + gidy + j + 1026*1026, 'W');
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
            	Access((gidx+i) * size + gidy + j, 'R');
						}
					}
					for (int lidY = 0; lidY < 32; lidY++) {
            for (int lidX = 0; lidX < 32; lidX++) {
              unsigned int gidx = (gidX*32+lidX) * isize + 1;
              unsigned int gidy = (gidY*32+lidY) * jsize + 1;
							//rtTmpAccess((gidx+i) * size + gidy + 1 + j);
							Access((gidx+i) * size + gidy + 1 + j, 'R');
						}
					}
					for (int lidY = 0; lidY < 32; lidY++) {
            for (int lidX = 0; lidX < 32; lidX++) {
              unsigned int gidx = (gidX*32+lidX) * isize + 1;
              unsigned int gidy = (gidY*32+lidY) * jsize + 1;
            	//rtTmpAccess((gidx+i) * size + gidy - 1 + j);
							Access((gidx+i) * size + gidy - 1 + j, 'R');
						}
					}
					for (int lidY = 0; lidY < 32; lidY++) {
            for (int lidX = 0; lidX < 32; lidX++) {
              unsigned int gidx = (gidX*32+lidX) * isize + 1;
              unsigned int gidy = (gidY*32+lidY) * jsize + 1;
            	//rtTmpAccess((gidx +i- 1) * size + gidy + j);
							Access((gidx +i- 1) * size + gidy + j, 'R');
						}
					}
					for (int lidY = 0; lidY < 32; lidY++) {
            for (int lidX = 0; lidX < 32; lidX++) {
              unsigned int gidx = (gidX*32+lidX) * isize + 1;
              unsigned int gidy = (gidY*32+lidY) * jsize + 1;
            	//rtTmpAccess((gidx +i+ 1) * size + gidy + j);
							Access((gidx +i+ 1) * size + gidy + j, 'R');
						}
					}
					for (int lidY = 0; lidY < 32; lidY++) {
            for (int lidX = 0; lidX < 32; lidX++) {
              unsigned int gidx = (gidX*32+lidX) * isize + 1;
              unsigned int gidy = (gidY*32+lidY) * jsize + 1;
              //rtTmpAccess((gidx + i) * size + gidy + j + 1026*1026);
							Access((gidx + i) * size + gidy + j + 1026*1026, 'W');
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
			reset();
 			stencil_trace_GXYW_RR(a, b, 1024, i, j, 1024/i, 1024/j);		
			RTtoFP();
			double mr = FPtoMR(32768);
			cout << mr << endl;
			//dumpFP();	
			

			//dumpRtTmp();					
			/*if (varify(b, b_r)) {
				cout << "Success" << endl;
			} else {
				cout << "Failed" << endl;
			}*/
		}
	}

	return 0;
}





