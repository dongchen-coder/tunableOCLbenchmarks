
for PROG in 3dConvolution_mr 2dConvolution_mr 3mm_mr atax_mr bicg_mr correlation_mr covariance_mr gemm_mr gemver_mr gesummv_mr mvt_mr syrk_mr
do 
	for ITER in {4..6}
	do
		./bin/${PROG} 1 1 >> ./result/${PROG}_rand_${ITER}.txt
	done
done


