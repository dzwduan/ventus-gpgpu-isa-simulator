VENTUS_PREFIX := ${VENTUS_INSTALL_PREFIX}
DRIVER_DIR := $(VENTUS_PREFIX)/../../ventus-gpgpu-isa-simulator/gpgpu-testcase/driver
BUILD_DIR := $(DRIVER_DIR)/build

CLANG := $(VENTUS_PREFIX)/bin/clang
LD := $(VENTUS_PREFIX)/bin/ld.lld
OBJDUMP := $(VENTUS_PREFIX)/bin/llvm-objdump

CFLAGS := -fno-inline -O3 -cl-std=CL2.0 -target riscv32 -mcpu=ventus-gpgpu

.PHONY: test
test: test.riscv
	@echo "[INFO] Copying and running in $(BUILD_DIR)..."
	rm -rf $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)
	cp test.riscv $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake $(DRIVER_DIR)
	cd $(BUILD_DIR) && make
	cd $(BUILD_DIR) && ./spike_test
	$(OBJDUMP) -d --mattr=+v,+zfinx test.riscv > test.dump
	@echo "[DONE] All done. Output: test.riscv, test.dump"

.PHONY: compile
compile: test.o

.PHONY: from_asm
from_asm: test.riscv
	@echo "[INFO] Copying and running in $(BUILD_DIR)..."
	rm -rf $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)
	cp test.riscv $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake $(DRIVER_DIR)
	cd $(BUILD_DIR) && make
	cd $(BUILD_DIR) && ./spike_test
	$(OBJDUMP) -d --mattr=+v,+zfinx test.riscv > test.dump
	@echo "[DONE] Done from_asm. Output: test.riscv, test.dump"

test.ll: test.cl
	$(CLANG) -S $(CFLAGS) -emit-llvm -o $@ $<

test.s: test.ll
	$(CLANG) -S $(CFLAGS) -o $@ $<

test.o: test.s
	$(CLANG) -c $(CFLAGS) -o $@ $<

test.riscv: test.o
	$(LD) -o $@ \
		-T $(VENTUS_PREFIX)/../utils/ldscripts/ventus/elf32lriscv.ld \
		test.o \
		$(VENTUS_PREFIX)/lib/crt0.o \
		$(VENTUS_PREFIX)/lib/riscv32clc.o \
		-L $(VENTUS_PREFIX)/lib -lworkitem \
		--gc-sections --init test

.PHONY: clean
clean:
	rm -rf test.ll test.s test.o test.riscv test.dump
	rm -rf $(BUILD_DIR)
	@echo "[CLEAN] All intermediate files and build dir removed."