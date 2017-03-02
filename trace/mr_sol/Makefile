# the compiler: gcc for C program, define as g++ for C++
CC = g++

# compiler flags:
#  -g	adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -O3 -fopenmp
INC = ../../utilities/hrtimer_x86.c

SOURCE = src/
BIN = bin/

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

$(CON3D): $(SOURCE)$(CON3D).c
	$(CC) $(CFLAGS) $(INC) -o $(BIN)$(CON3D) $(SOURCE)$(CON3D).c

$(CON2D): $(SOURCE)$(CON2D).c
	$(CC) $(CFLAGS) $(INC) -o $(BIN)$(CON2D) $(SOURCE)$(CON2D).c

$(3MM): $(SOURCE)$(3MM).c
	$(CC) $(CFLAGS) $(INC) -o $(BIN)$(3MM) $(SOURCE)$(3MM).c

$(ATAX): $(SOURCE)$(ATAX).c
	$(CC) $(CFLAGS) $(INC) -o $(BIN)$(ATAX) $(SOURCE)$(ATAX).c

$(BICG): $(SOURCE)$(BICG).c
	$(CC) $(CFLAGS) $(INC) -o $(BIN)$(BICG) $(SOURCE)$(BICG).c

$(CORRELATION): $(SOURCE)$(CORRELATION).c
	$(CC) $(CFLAGS) $(INC) -o $(BIN)$(CORRELATION) $(SOURCE)$(CORRELATION).c

$(COVARIANCE): $(SOURCE)$(COVARIANCE).c
	$(CC) $(CFLAGS) $(INC) -o $(BIN)$(COVARIANCE) $(SOURCE)$(COVARIANCE).c

$(GEMM): $(SOURCE)$(GEMM).c
	$(CC) $(CFLAGS) $(INC) -o $(BIN)$(GEMM) $(SOURCE)$(GEMM).c

$(GEMVER): $(SOURCE)$(GEMVER).c
	$(CC) $(CFLAGS) $(INC) -o $(BIN)$(GEMVER) $(SOURCE)$(GEMVER).c

$(GESUMMV): $(SOURCE)$(GESUMMV).c
	$(CC) $(CFLAGS) $(INC) -o $(BIN)$(GESUMMV) $(SOURCE)$(GESUMMV).c

$(MVT): $(SOURCE)$(MVT).c
	$(CC) $(CFLAGS) $(INC) -o $(BIN)$(MVT) $(SOURCE)$(MVT).c

$(SYRK): $(SOURCE)$(SYRK).c
	$(CC) $(CFLAGS) $(INC) -o $(BIN)$(SYRK) $(SOURCE)$(SYRK).c

clean:
	$(RM) $(BIN)$(CON3D)
	$(RM) $(BIN)$(CON2D)
	$(RM) $(BIN)$(3MM)
	$(RM) $(BIN)$(ATAX)
	$(RM) $(BIN)$(BICG)
	$(RM) $(BIN)$(CORRELATION)
	$(RM) $(BIN)$(COVARIANCE)
	$(RM) $(BIN)$(GEMM)
	$(RM) $(BIN)$(GEMVER)
	$(RM) $(BIN)$(GESUMMV)
	$(RM) $(BIN)$(MVT)
	$(RM) $(BIN)$(SYRK)

