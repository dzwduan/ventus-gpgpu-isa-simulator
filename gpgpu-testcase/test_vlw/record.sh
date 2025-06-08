if ! ${VENTUS_INSTALL_PREFIX}/bin/clang -S -fno-inline -O3 -cl-std=CL2.0 -target riscv32 -mcpu=ventus-gpgpu test.cl -emit-llvm -o test.ll ; then
    exit -1
fi
if ! ${VENTUS_INSTALL_PREFIX}/bin/clang -S -fno-inline -O3 -cl-std=CL2.0 -target riscv32 -mcpu=ventus-gpgpu test.ll -o test.s ; then
    exit -1
fi
if ! ${VENTUS_INSTALL_PREFIX}/bin/clang -c -fno-inline -O3 -cl-std=CL2.0 -target riscv32 -mcpu=ventus-gpgpu test.s -o test.o ; then
    exit -1
fi
${VENTUS_INSTALL_PREFIX}/bin/ld.lld  -o  test.riscv -T ${VENTUS_INSTALL_PREFIX}/../utils/ldscripts/ventus/elf32lriscv.ld test.o ${VENTUS_INSTALL_PREFIX}/lib/crt0.o ${VENTUS_INSTALL_PREFIX}/lib/riscv32clc.o -L ${VENTUS_INSTALL_PREFIX}/lib -lworkitem --gc-sections --init test

# cp test.riscv ${VENTUS_INSTALL_PREFIX}/../../ventus-gpgpu-isa-simulator/gpgpu-testcase/driver/build
# cd ${VENTUS_INSTALL_PREFIX}/../../ventus-gpgpu-isa-simulator/gpgpu-testcase/driver/build
# ./spike_test
# cd -

${VENTUS_INSTALL_PREFIX}/bin/llvm-objdump -d --mattr=+v,+zfinx test.riscv > test.dump
