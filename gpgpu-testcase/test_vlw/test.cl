#pragma OPENCL EXTENSION cl_khr_fp64 : enable
void foo(int *out, int *in, int tid, __private int *num);

__kernel void test(__global int *out, __global int *in)
{
    int tid = get_global_id(0);
    int num=in[tid];
    foo(out, in, tid, &num);
}

void foo(int *out, int *in, int tid, __private int *num) {
    if (*num > 0)
        out[tid] = 1;
    else
        out[tid] = -1;
}