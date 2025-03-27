#ifndef DIFFTRACE_H_
#define DIFFTRACE_H_

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <queue>

class store_trace_t
{
  public:
    uint64_t paddr;
    uint64_t data;
    uint8_t mask;

    store_trace_t(uint64_t paddr, uint64_t data, uint8_t mask) : paddr(paddr), data(data), mask(mask)
    {
        do_align();
    }
    store_trace_t(uint64_t paddr, uint64_t data, int len) : paddr(paddr), data(data), mask((1U << len) - 1)
    {
        if (len != 8)
        {
            this->data &= (1UL << (len * 8)) - 1UL;
        }
        do_align();
    };

  private:
    void do_align()
    {
        uint64_t offset = paddr % 8UL;
        if (offset)
        {
            int len = std::log2((long)mask + 1);
            paddr = paddr - offset;
            data &= (1UL << (len * 8)) - 1UL;
            data <<= offset << 3;
            mask <<= offset;
        }
    }
};

class diff_trace_t
{
  private:
    std::queue<store_trace_t> store_trace;

  public:
    int dut_store_commit(uint64_t *addr, uint64_t *data, uint8_t *mask)
    {
        if (store_trace.empty())
        {
            printf("Store commit error: the store trace is empty.\n");
            return -1;
        }

        store_trace_t ref = store_trace.front();
        store_trace_t dut{*addr, *data, *mask};
        if (ref.paddr != dut.paddr || ref.data != dut.data || ref.mask != dut.mask)
        {
            *addr = ref.paddr;
            *data = ref.data;
            *mask = ref.mask;
            return -1;
        }

        store_trace.pop();
        return 0;
    }
};

#endif