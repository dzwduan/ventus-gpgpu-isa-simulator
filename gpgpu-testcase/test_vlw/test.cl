#pragma OPENCL EXTENSION cl_khr_fp64 : enable

void foo(__local int *f, __private int t);

__kernel void test(__global int *out, __global int *in, __local int *shared_buf)
{
    int tid = get_global_id(0);
    foo(&shared_buf[tid], in[tid]);  
    out[tid] = shared_buf[tid];      
}

void foo(__local int *f, __private int t) {
    if (t > 0)
        *f = 1;
    else
        *f = -1;
}
