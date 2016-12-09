; ModuleID = 'bicg.cl'
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; Function Attrs: nounwind ssp uwtable
define void @bicgKernel1(float* nocapture readonly %A, float* nocapture readonly %p, float* nocapture %q, i32 %nx, i32 %ny, i32 %cX, i32 %cY) #0 {
  %1 = icmp sgt i32 %cX, 0
  br i1 %1, label %.lr.ph3, label %._crit_edge

.lr.ph3:                                          ; preds = %0
  %2 = icmp sgt i32 %ny, 0
  %3 = add i32 %ny, -1
  %4 = add i32 %cX, -1
  br label %5

; <label>:5                                       ; preds = %.loopexit, %.lr.ph3
  %x.02 = phi i32 [ 0, %.lr.ph3 ], [ %44, %.loopexit ]
  %6 = tail call i32 (i32, ...)* bitcast (i32 (...)* @get_group_id to i32 (i32, ...)*)(i32 0) #2
  %7 = mul nsw i32 %6, %cX
  %8 = add nsw i32 %7, %x.02
  %9 = tail call i32 (i32, ...)* bitcast (i32 (...)* @get_local_size to i32 (i32, ...)*)(i32 0) #2
  %10 = mul nsw i32 %8, %9
  %11 = tail call i32 (i32, ...)* bitcast (i32 (...)* @get_local_id to i32 (i32, ...)*)(i32 0) #2
  %12 = add nsw i32 %10, %11
  %13 = icmp slt i32 %12, %nx
  br i1 %13, label %14, label %.loopexit

; <label>:14                                      ; preds = %5
  %15 = sext i32 %12 to i64
  %16 = getelementptr inbounds float* %q, i64 %15
  store float 0.000000e+00, float* %16, align 4, !tbaa !9
  br i1 %2, label %.lr.ph, label %.loopexit

.lr.ph:                                           ; preds = %14
  %17 = mul nsw i32 %12, %ny
  %18 = sext i32 %17 to i64
  %xtraiter = and i32 %ny, 1
  %lcmp.mod = icmp ne i32 %xtraiter, 0
  br i1 %lcmp.mod, label %19, label %.lr.ph.split

; <label>:19                                      ; preds = %.lr.ph
  %20 = add nsw i64 0, %18
  %21 = getelementptr inbounds float* %A, i64 %20
  %22 = load float* %21, align 4, !tbaa !9
  %23 = getelementptr inbounds float* %p, i64 0
  %24 = load float* %23, align 4, !tbaa !9
  %25 = fmul float %22, %24
  %26 = fadd float 0.000000e+00, %25
  store float %26, float* %16, align 4, !tbaa !9
  %indvars.iv.next.prol = add nuw nsw i64 0, 1
  %lftr.wideiv.prol = trunc i64 0 to i32
  %exitcond.prol = icmp eq i32 %lftr.wideiv.prol, %3
  br label %.lr.ph.split

.lr.ph.split:                                     ; preds = %19, %.lr.ph
  %.unr = phi float [ 0.000000e+00, %.lr.ph ], [ %26, %19 ]
  %indvars.iv.unr = phi i64 [ 0, %.lr.ph ], [ %indvars.iv.next.prol, %19 ]
  %27 = icmp ult i32 %3, 1
  br i1 %27, label %.loopexit.loopexit, label %.lr.ph.split.split

.lr.ph.split.split:                               ; preds = %.lr.ph.split
  br label %28

; <label>:28                                      ; preds = %28, %.lr.ph.split.split
  %29 = phi float [ %.unr, %.lr.ph.split.split ], [ %43, %28 ]
  %indvars.iv = phi i64 [ %indvars.iv.unr, %.lr.ph.split.split ], [ %indvars.iv.next.1, %28 ]
  %30 = add nsw i64 %indvars.iv, %18
  %31 = getelementptr inbounds float* %A, i64 %30
  %32 = load float* %31, align 4, !tbaa !9
  %33 = getelementptr inbounds float* %p, i64 %indvars.iv
  %34 = load float* %33, align 4, !tbaa !9
  %35 = fmul float %32, %34
  %36 = fadd float %29, %35
  store float %36, float* %16, align 4, !tbaa !9
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv to i32
  %37 = add nsw i64 %indvars.iv.next, %18
  %38 = getelementptr inbounds float* %A, i64 %37
  %39 = load float* %38, align 4, !tbaa !9
  %40 = getelementptr inbounds float* %p, i64 %indvars.iv.next
  %41 = load float* %40, align 4, !tbaa !9
  %42 = fmul float %39, %41
  %43 = fadd float %36, %42
  store float %43, float* %16, align 4, !tbaa !9
  %indvars.iv.next.1 = add nuw nsw i64 %indvars.iv.next, 1
  %lftr.wideiv.1 = trunc i64 %indvars.iv.next to i32
  %exitcond.1 = icmp eq i32 %lftr.wideiv.1, %3
  br i1 %exitcond.1, label %.loopexit.loopexit.unr-lcssa, label %28

.loopexit.loopexit.unr-lcssa:                     ; preds = %28
  br label %.loopexit.loopexit

.loopexit.loopexit:                               ; preds = %.lr.ph.split, %.loopexit.loopexit.unr-lcssa
  br label %.loopexit

.loopexit:                                        ; preds = %.loopexit.loopexit, %14, %5
  %44 = add nuw nsw i32 %x.02, 1
  %exitcond4 = icmp eq i32 %x.02, %4
  br i1 %exitcond4, label %._crit_edge.loopexit, label %5

._crit_edge.loopexit:                             ; preds = %.loopexit
  br label %._crit_edge

._crit_edge:                                      ; preds = %._crit_edge.loopexit, %0
  ret void
}

declare i32 @get_group_id(...) #1

declare i32 @get_local_size(...) #1

declare i32 @get_local_id(...) #1

; Function Attrs: nounwind ssp uwtable
define void @bicgKernel2(float* nocapture readonly %A, float* nocapture readonly %r, float* nocapture %s, i32 %nx, i32 %ny, i32 %cX, i32 %cY) #0 {
  %1 = icmp sgt i32 %cX, 0
  br i1 %1, label %.lr.ph3, label %._crit_edge

.lr.ph3:                                          ; preds = %0
  %2 = icmp sgt i32 %nx, 0
  %3 = sext i32 %ny to i64
  %4 = add i32 %nx, -1
  %5 = add i32 %cX, -1
  br label %6

; <label>:6                                       ; preds = %.loopexit, %.lr.ph3
  %x.02 = phi i32 [ 0, %.lr.ph3 ], [ %44, %.loopexit ]
  %7 = tail call i32 (i32, ...)* bitcast (i32 (...)* @get_group_id to i32 (i32, ...)*)(i32 0) #2
  %8 = mul nsw i32 %7, %cX
  %9 = add nsw i32 %8, %x.02
  %10 = tail call i32 (i32, ...)* bitcast (i32 (...)* @get_local_size to i32 (i32, ...)*)(i32 0) #2
  %11 = mul nsw i32 %9, %10
  %12 = tail call i32 (i32, ...)* bitcast (i32 (...)* @get_local_id to i32 (i32, ...)*)(i32 0) #2
  %13 = add nsw i32 %11, %12
  %14 = icmp slt i32 %13, %ny
  br i1 %14, label %15, label %.loopexit

; <label>:15                                      ; preds = %6
  %16 = sext i32 %13 to i64
  %17 = getelementptr inbounds float* %s, i64 %16
  store float 0.000000e+00, float* %17, align 4, !tbaa !9
  br i1 %2, label %.lr.ph.preheader, label %.loopexit

.lr.ph.preheader:                                 ; preds = %15
  %xtraiter = and i32 %nx, 1
  %lcmp.mod = icmp ne i32 %xtraiter, 0
  br i1 %lcmp.mod, label %.lr.ph.prol, label %.lr.ph.preheader.split

.lr.ph.prol:                                      ; preds = %.lr.ph.preheader
  %18 = mul nsw i64 0, %3
  %19 = add nsw i64 %18, %16
  %20 = getelementptr inbounds float* %A, i64 %19
  %21 = load float* %20, align 4, !tbaa !9
  %22 = getelementptr inbounds float* %r, i64 0
  %23 = load float* %22, align 4, !tbaa !9
  %24 = fmul float %21, %23
  %25 = fadd float 0.000000e+00, %24
  store float %25, float* %17, align 4, !tbaa !9
  %indvars.iv.next.prol = add nuw nsw i64 0, 1
  %lftr.wideiv.prol = trunc i64 0 to i32
  %exitcond.prol = icmp eq i32 %lftr.wideiv.prol, %4
  br label %.lr.ph.preheader.split

.lr.ph.preheader.split:                           ; preds = %.lr.ph.prol, %.lr.ph.preheader
  %.unr = phi float [ 0.000000e+00, %.lr.ph.preheader ], [ %25, %.lr.ph.prol ]
  %indvars.iv.unr = phi i64 [ 0, %.lr.ph.preheader ], [ %indvars.iv.next.prol, %.lr.ph.prol ]
  %26 = icmp ult i32 %4, 1
  br i1 %26, label %.loopexit.loopexit, label %.lr.ph.preheader.split.split

.lr.ph.preheader.split.split:                     ; preds = %.lr.ph.preheader.split
  br label %.lr.ph

.lr.ph:                                           ; preds = %.lr.ph, %.lr.ph.preheader.split.split
  %27 = phi float [ %.unr, %.lr.ph.preheader.split.split ], [ %43, %.lr.ph ]
  %indvars.iv = phi i64 [ %indvars.iv.unr, %.lr.ph.preheader.split.split ], [ %indvars.iv.next.1, %.lr.ph ]
  %28 = mul nsw i64 %indvars.iv, %3
  %29 = add nsw i64 %28, %16
  %30 = getelementptr inbounds float* %A, i64 %29
  %31 = load float* %30, align 4, !tbaa !9
  %32 = getelementptr inbounds float* %r, i64 %indvars.iv
  %33 = load float* %32, align 4, !tbaa !9
  %34 = fmul float %31, %33
  %35 = fadd float %27, %34
  store float %35, float* %17, align 4, !tbaa !9
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv to i32
  %36 = mul nsw i64 %indvars.iv.next, %3
  %37 = add nsw i64 %36, %16
  %38 = getelementptr inbounds float* %A, i64 %37
  %39 = load float* %38, align 4, !tbaa !9
  %40 = getelementptr inbounds float* %r, i64 %indvars.iv.next
  %41 = load float* %40, align 4, !tbaa !9
  %42 = fmul float %39, %41
  %43 = fadd float %35, %42
  store float %43, float* %17, align 4, !tbaa !9
  %indvars.iv.next.1 = add nuw nsw i64 %indvars.iv.next, 1
  %lftr.wideiv.1 = trunc i64 %indvars.iv.next to i32
  %exitcond.1 = icmp eq i32 %lftr.wideiv.1, %4
  br i1 %exitcond.1, label %.loopexit.loopexit.unr-lcssa, label %.lr.ph

.loopexit.loopexit.unr-lcssa:                     ; preds = %.lr.ph
  br label %.loopexit.loopexit

.loopexit.loopexit:                               ; preds = %.lr.ph.preheader.split, %.loopexit.loopexit.unr-lcssa
  br label %.loopexit

.loopexit:                                        ; preds = %.loopexit.loopexit, %15, %6
  %44 = add nuw nsw i32 %x.02, 1
  %exitcond4 = icmp eq i32 %x.02, %5
  br i1 %exitcond4, label %._crit_edge.loopexit, label %6

._crit_edge.loopexit:                             ; preds = %.loopexit
  br label %._crit_edge

._crit_edge:                                      ; preds = %._crit_edge.loopexit, %0
  ret void
}

attributes #0 = { nounwind ssp uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+ssse3,+cx16,+sse,+sse2,+sse3" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+ssse3,+cx16,+sse,+sse2,+sse3" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!opencl.kernels = !{!0, !6}
!llvm.module.flags = !{!7}
!llvm.ident = !{!8}

!0 = !{void (float*, float*, float*, i32, i32, i32, i32)* @bicgKernel1, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"DATA_TYPE*", !"DATA_TYPE*", !"DATA_TYPE*", !"int", !"int", !"int", !"int"}
!4 = !{!"kernel_arg_base_type", !"float*", !"float*", !"float*", !"int", !"int", !"int", !"int"}
!5 = !{!"kernel_arg_type_qual", !"", !"", !"", !"", !"", !"", !""}
!6 = !{void (float*, float*, float*, i32, i32, i32, i32)* @bicgKernel2, !1, !2, !3, !4, !5}
!7 = !{i32 1, !"PIC Level", i32 2}
!8 = !{!"Apple LLVM version 7.0.0 (clang-700.0.72)"}
!9 = !{!10, !10, i64 0}
!10 = !{!"float", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
