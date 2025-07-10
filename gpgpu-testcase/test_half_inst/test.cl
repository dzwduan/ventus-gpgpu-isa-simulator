__kernel void test(__global short* a, __global short* b, __global short* c) {
    int gid = get_global_id(0);
    c[gid] = a[gid] - b[gid];
}