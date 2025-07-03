; ModuleID = 'test.cl'
source_filename = "test.cl"
target datalayout = "e-m:e-p:32:32-i64:64-n32-S128-A5-G1"
target triple = "riscv32"

; Function Attrs: convergent noinline norecurse nounwind vscale_range(1,2048)
define dso_local ventus_kernel void @test_vadd64(ptr addrspace(1) nocapture noundef readnone align 2 %0, ptr addrspace(1) nocapture noundef readnone align 2 %1, ptr addrspace(1) nocapture noundef readnone align 2 %2) local_unnamed_addr #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 {
  call void asm sideeffect ".word 0x0005a0fb  \0A\09.word 0x0006217b  \0A\09", ""() #1, !srcloc !9
  call void asm sideeffect ".word $0\0A\09.word $1\0A\09", "i,i"(i32 3147860, i32 262656) #1, !srcloc !10
  call void asm sideeffect ".word 0x0035207b  \0A\09", ""() #1, !srcloc !11
  ret void
}

attributes #0 = { convergent noinline norecurse nounwind vscale_range(1,2048) "disable-tail-calls"="true" "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="ventus-gpgpu" "target-features"="+32bit,+a,+m,+relax,+zdinx,+zfinx,+zhinx,+zve32f,+zve32x,+zvl32b,-64bit,-save-restore" "uniform-work-group-size"="false" }
attributes #1 = { convergent nounwind }

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
!7 = !{!"half*", !"half*", !"half*"}
!8 = !{!"", !"", !""}
!9 = !{i64 284, i64 305, i64 360}
!10 = !{i64 1080, i64 1091, i64 1116}
!11 = !{i64 1212, i64 1233}
