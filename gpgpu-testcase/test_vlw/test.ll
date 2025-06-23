; ModuleID = 'test.cl'
source_filename = "test.cl"
target datalayout = "e-m:e-p:32:32-i64:64-n32-S128-A5-G1"
target triple = "riscv32"

; Function Attrs: convergent mustprogress nofree noinline norecurse nounwind willreturn memory(argmem: readwrite) vscale_range(1,2048)
define dso_local ventus_kernel void @test(ptr addrspace(1) nocapture noundef writeonly align 4 %0, ptr addrspace(1) nocapture noundef readonly align 4 %1) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 {
  %3 = alloca i32, align 4, addrspace(5)
  %4 = call i32 @_Z13get_global_idj(i32 noundef 0) #4
  call void @llvm.lifetime.start.p5(i64 4, ptr addrspace(5) %3) #5
  %5 = getelementptr inbounds i32, ptr addrspace(1) %1, i32 %4
  %6 = load i32, ptr addrspace(1) %5, align 4, !tbaa !9
  store i32 %6, ptr addrspace(5) %3, align 4, !tbaa !9
  %7 = addrspacecast ptr addrspace(1) %0 to ptr
  call void @foo(ptr noundef %7, ptr poison, i32 noundef %4, ptr addrspace(5) noundef %3)
  call void @llvm.lifetime.end.p5(i64 4, ptr addrspace(5) %3) #5
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p5(i64 immarg, ptr addrspace(5) nocapture) #1

; Function Attrs: convergent mustprogress nofree nounwind willreturn memory(none)
declare dso_local i32 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #2

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn memory(argmem: readwrite) vscale_range(1,2048)
define dso_local void @foo(ptr nocapture noundef writeonly %0, ptr nocapture readnone %1, i32 noundef %2, ptr addrspace(5) nocapture noundef readonly %3) local_unnamed_addr #3 {
  %5 = load i32, ptr addrspace(5) %3, align 4, !tbaa !9
  %6 = icmp slt i32 %5, 1
  %7 = select i1 %6, i32 -1, i32 1
  %8 = getelementptr inbounds i32, ptr %0, i32 %2
  store i32 %7, ptr %8, align 4
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p5(i64 immarg, ptr addrspace(5) nocapture) #1

attributes #0 = { convergent mustprogress nofree noinline norecurse nounwind willreturn memory(argmem: readwrite) vscale_range(1,2048) "disable-tail-calls"="true" "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="ventus-gpgpu" "target-features"="+32bit,+a,+m,+relax,+zdinx,+zfinx,+zhinx,+zve32f,+zve32x,+zvl32b,-64bit,-save-restore" "uniform-work-group-size"="false" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { convergent mustprogress nofree nounwind willreturn memory(none) "disable-tail-calls"="true" "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="ventus-gpgpu" "target-features"="+32bit,+a,+m,+relax,+zdinx,+zfinx,+zhinx,+zve32f,+zve32x,+zvl32b,-64bit,-save-restore" }
attributes #3 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(argmem: readwrite) vscale_range(1,2048) "disable-tail-calls"="true" "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="ventus-gpgpu" "target-features"="+32bit,+a,+m,+relax,+zdinx,+zfinx,+zhinx,+zve32f,+zve32x,+zvl32b,-64bit,-save-restore" }
attributes #4 = { convergent nounwind willreturn memory(none) }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!opencl.ocl.version = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"ilp32"}
!2 = !{i32 1, !"SmallDataLimit", i32 8}
!3 = !{i32 2, i32 0}
!4 = !{!"clang version 16.0.0 (https://github.com/THU-DSP-LAB/llvm-project.git 69dd6693295c79a68395dc27356adff6928130ba)"}
!5 = !{i32 1, i32 1}
!6 = !{!"none", !"none"}
!7 = !{!"int*", !"int*"}
!8 = !{!"", !""}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
