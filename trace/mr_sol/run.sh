
for X in 1 2 4 8 16 32 64 256
do
	for Y in 1 2 4 8 16 32 64 256
	do
		./bin/3dConvolution_mr $X $Y >> ./res_3d_2048_32_8_rand/rand_2048_32_8_${X}_${Y}.txt &
	done
	wait
done

'''
for ITER in {1..5}
do
	./3dConvolution_mr 1 1 >> ./res/rand_256_16_4_rand${ITER}.txt &
done

'''

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
