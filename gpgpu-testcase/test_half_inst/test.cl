__kernel void test(__global ushort* a, __global ushort* b, __global ushort* c) {
    int gid = get_global_id(0);
    ushort vx = b[0];
    c[gid] = a[gid] * vx;
}
