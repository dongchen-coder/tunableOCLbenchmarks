
#for PROG in 3dConvolution_mr 2dConvolution_mr 3mm_mr atax_mr bicg_mr correlation_mr covariance_mr gemm_mr gemver_mr gesummv_mr mvt_mr syrk_mr


for PROG in atax_mr
do 
	for ITER in {3..5}
	do
		for KID in {0..3}
		do
			./bin/${PROG} 1 1 ${KID} >> ./result/${PROG}_${KID}_rand_${ITER}.txt &
		done
	done
done

