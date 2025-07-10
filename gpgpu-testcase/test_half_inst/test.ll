; ModuleID = 'test.cl'
source_filename = "test.cl"
target datalayout = "e-m:e-p:32:32-i64:64-n32-S128-A5-G1"
target triple = "riscv32"

; Function Attrs: convergent mustprogress nofree noinline norecurse nounwind willreturn memory(argmem: readwrite) vscale_range(1,2048)
define dso_local ventus_kernel void @test(ptr addrspace(1) nocapture noundef readonly align 2 %0, ptr addrspace(1) nocapture noundef readonly align 2 %1, ptr addrspace(1) nocapture noundef writeonly align 2 %2) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 {
  %4 = call i32 @_Z13get_global_idj(i32 noundef 0) #2
  %5 = getelementptr inbounds i16, ptr addrspace(1) %0, i32 %4
  %6 = load i16, ptr addrspace(1) %5, align 2, !tbaa !9
  %7 = getelementptr inbounds i16, ptr addrspace(1) %1, i32 %4
  %8 = load i16, ptr addrspace(1) %7, align 2, !tbaa !9
  %9 = sub i16 %6, %8
  %10 = getelementptr inbounds i16, ptr addrspace(1) %2, i32 %4
  store i16 %9, ptr addrspace(1) %10, align 2, !tbaa !9
  ret void
}

; Function Attrs: convergent mustprogress nofree nounwind willreturn memory(none)
declare dso_local i32 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #1

attributes #0 = { convergent mustprogress nofree noinline norecurse nounwind willreturn memory(argmem: readwrite) vscale_range(1,2048) "disable-tail-calls"="true" "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="ventus-gpgpu" "target-features"="+32bit,+a,+m,+relax,+zdinx,+zfinx,+zhinx,+zve32f,+zve32x,+zvl32b,-64bit,-save-restore" "uniform-work-group-size"="false" }
attributes #1 = { convergent mustprogress nofree nounwind willreturn memory(none) "disable-tail-calls"="true" "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="ventus-gpgpu" "target-features"="+32bit,+a,+m,+relax,+zdinx,+zfinx,+zhinx,+zve32f,+zve32x,+zvl32b,-64bit,-save-restore" }
attributes #2 = { convergent nounwind willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2}
!opencl.ocl.version = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"ilp32"}
!2 = !{i32 1, !"SmallDataLimit", i32 8}
!3 = !{i32 2, i32 0}
!4 = !{!"clang version 16.0.0 (https://github.com/THU-DSP-LAB/llvm-project.git fbabe9474711354ca53ba7ee71cb9a991bc85a15)"}
!5 = !{i32 1, i32 1, i32 1}
!6 = !{!"none", !"none", !"none"}
!7 = !{!"short*", !"short*", !"short*"}
!8 = !{!"", !"", !""}
!9 = !{!10, !10, i64 0}
!10 = !{!"short", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
