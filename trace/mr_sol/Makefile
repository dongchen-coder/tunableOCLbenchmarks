# the compiler: gcc for C program, define as g++ for C++
CC = g++

# compiler flags:
#  -g	adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -O3 -fopenmp
INC = ../../utilities/hrtimer_x86.c

# the build target executable:
CON3D = 3dConvolution_mr
CON2D = 2dConvolution_mr
3MM = 3mm_mr
ATAX = atax_mr
BICG = bicg_mr
CORRELATION = correlation_mr
COVARIANCE = covariance_mr
GEMM = gemm_mr
GEMVER = gemver_mr
GESUMMV = gesummv_mr
MVT = mvt_mr
SYRK = syrk_mr

all: $(CON3D) $(CON2D) $(3MM) $(ATAX) $(BICG) $(CORRELATION) $(COVARIANCE) $(GEMM) $(GEMVER) $(GESUMMV) $(MVT) $(SYRK)

$(CON3D): $(CON3D).c
	$(CC) $(CFLAGS) $(INC) -o $(CON3D) $(CON3D).c

$(CON2D): $(CON2D).c
	$(CC) $(CFLAGS) $(INC) -o $(CON2D) $(CON2D).c

$(3MM): $(3MM).c
	$(CC) $(CFLAGS) $(INC) -o $(3MM) $(3MM).c

$(ATAX): $(ATAX).c
	$(CC) $(CFLAGS) $(INC) -o $(ATAX) $(ATAX).c

$(BICG): $(BICG).c
	$(CC) $(CFLAGS) $(INC) -o $(BICG) $(BICG).c

$(CORRELATION): $(CORRELATION).c
	$(CC) $(CFLAGS) $(INC) -o $(CORRELATION) $(CORRELATION).c

$(COVARIANCE): $(COVARIANCE).c
	$(CC) $(CFLAGS) $(INC) -o $(COVARIANCE) $(COVARIANCE).c

$(GEMM): $(GEMM).c
	$(CC) $(CFLAGS) $(INC) -o $(GEMM) $(GEMM).c

$(GEMVER): $(GEMVER).c
	$(CC) $(CFLAGS) $(INC) -o $(GEMVER) $(GEMVER).c

$(GESUMMV): $(GESUMMV).c
	$(CC) $(CFLAGS) $(INC) -o $(GESUMMV) $(GESUMMV).c

$(MVT): $(MVT).c
	$(CC) $(CFLAGS) $(INC) -o $(MVT) $(MVT).c

$(SYRK): $(SYRK).c
	$(CC) $(CFLAGS) $(INC) -o $(SYRK) $(SYRK).c

clean:
	$(RM) $(CON3D)
	$(RM) $(CON2D)
	$(RM) $(3MM)
	$(RM) $(ATAX)
	$(RM) $(BICG)
	$(RM) $(CORRELATION)
	$(RM) $(COVARIANCE)
	$(RM) $(GEMM)
	$(RM) $(GEMVER)
	$(RM) $(GESUMMV)
	$(RM) $(MVT)
	$(RM) $(SYRK)

