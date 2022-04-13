#include "linked-list-memory-allocator/alloc.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>
#include "linked-list-memory-allocator/extend_alloc.cpp"
constexpr auto BUF = 100000;

void* buf = (void*)new uint64_t[BUF];
using namespace alloc;
void dump()
{
    // std::cout << "Region:\n";
    block_header* b = (block_header*)(buf);
    while (b <= last)
    {
        if (next_of(b) <= last && next_of(b)->back != b)
        {
            std::cerr << "LINK LIST ERROR!";
            exit(-1);
        }
        if (b->size > 10000000)
        {
            std::cerr << "ILLEGAL SIZE! OVERFLOW!";
            exit(-1);
        }
        std::cout << "  " << b << ": size = " << b->size << " | next = " << next_of(b) << " | back = " << b->back << '\n';
        b = next_of(b);
    }
}

void* alloc::detail::start()
{
    return buf;
}

std::size_t alloc::detail::extend(void* current, std::size_t min)
{
    return min;
}

void rngcase()
{
    std::vector<void*> ptrs;

    for (int i = 0; i < 1000; i++)
    {
        std::cout << "Running iteration: " << i + 1 << '\n';
        int val = rand() % 100;
        for (int j = 0; j < val; j++)
        {
            std::cout << "-- malloc test: " << j + 1 << '\n';
            ptrs.push_back(alloc::malloc(rand() % 13));
            dump();
        }

        std::shuffle(ptrs.begin(), ptrs.end(), std::default_random_engine(rand()));

        val = rand() % 50;
        val = std::min(val, (int)ptrs.size());

        for (int j = 0; j < val; j++)
        {
            std::cout << "-- free test: " << j + 1 << '\n';
            alloc::free(ptrs.back());
            dump();
            ptrs.pop_back();
        }

        int j = 1;
        while (ptrs.size() > 1000)
        {
            std::cout << "-- free2 test: " << j << '\n';
            alloc::free(ptrs.back());
            dump();
            ptrs.pop_back();
            j++;
        }
    }

    for (auto i : ptrs)
        alloc::free(i);

}

void foo()
{
    void* a = alloc::malloc(100);
    dump();
    std::cout << '\n';
    void* b = alloc::malloc(10);
    dump();
    std::cout << '\n';
    alloc::free(a);
    dump();
    std::cout << '\n';
    alloc::malloc(10);
    dump();
}

int main(int argc, char** argv)
{
    for (int i = 0; i < 100000; i++)
    {
        srand(i);
        rngcase();
    }
}
