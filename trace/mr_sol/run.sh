
for X in 1 2 4 8 16 32 64 256
#for X in 1
do
	for Y in 1 2 4 8 16 32 64 256
#	for Y in 1
	do
		for ITER in {1..2}
		do
			./3dConvolution_mr $X $Y >> ./res/rand_256_16_4_${X}_${Y}_${ITER}.txt &
		done
	done
done

'''
for ITER in {1..3}
do 

./3dConvolution_mr >> res/3dConvolution_mr_$ITER.txt &
./2dConvolution_mr >> res/2dConvolution_mr_$ITER.txt &
./3mm_mr >> res/3mm_mr_$ITER.txt &
./atax_mr >> res/atax_mr_$ITER.txt &
./bicg_mr >> res/bicg_mr_$ITER.txt &
./correlation_mr >> res/correlation_mr_$ITER.txt &
./covariance_mr >> res/covariance_mr_$ITER.txt &
./gemm_mr >> res/gemm_mr_$ITER.txt &
./gemver_mr >> res/gemver_mr_$ITER.txt &
./gesummv_mr >> res/gesummv_mr_$ITER.txt &
./mvt_mr >> res/mvt_mr_$ITER.txt &
./syrk_mr >> res/syrk_mr_$ITER.txt &

done
'''
