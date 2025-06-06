#pragma OPENCL EXTENSION cl_khr_fp64 : enable

void foo(__private int *f,__private int t);

__kernel void test(__global int *out, __global int *in)
{
    int tid = get_global_id(0);
    __private int tmp;
    foo(&tmp, in[tid]);
    out[tid] = tmp;
}

void foo(__private int *f, __private int t) {
    if (t > 0)
        *f = 1;
    else
        *f = -1;
}
