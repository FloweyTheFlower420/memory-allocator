#include "alloc.h"
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <random>
#include <iostream>

constexpr auto BUF = 100000;

struct megablock_header
{
     megablock_header* next;
     megablock_header* back;
};

struct block_header
{
    size_t size;
    block_header* next;
    block_header* back;
};

void dump(void* buf)
{
    megablock_header* a = (megablock_header*)buf;
    while(a)
    {
        //std::cout << "Region:\n";
        block_header* b = (block_header*)(a + 1);
        while(b)
        {
            if(b->next && b->next->back != b)
            {
                std::cerr << "LINK LIST ERROR!";
                exit(-1);
            }
            if(b->size > 10000000)
            {
                std::cerr << "ILLEGAL SIZE! OVERFLOW!";
                exit(-1);
            }
            //std::cout << "  " << b << ": size = " << b->size << " | next = " << b->next << " | back = " << b->back << '\n';
            b = b->next;
        }
        a = a->next;
    }
}

void rngcase()
{
    void* buf = (void*)new uint64_t[BUF];
    alloc::add_region(buf, sizeof(uint64_t) * BUF); 
    std::vector<void*> ptrs;
    
    for(int i = 0; i < 1000; i++)
    {
        std::cout << "Running iteration: " << i + 1 << '\n';
        int val = rand() % 100;
        for(int j = 0; j < val; j++)
        {
            std::cout << "-- malloc test: " << j + 1 << '\n';
            ptrs.push_back(alloc::malloc(rand() % 13));
            dump(buf);
        }

        std::shuffle(ptrs.begin(), ptrs.end(), std::default_random_engine(rand()));

        val = rand() % 50;
        val = std::min(val, (int)ptrs.size());

        for(int j = 0; j < val; j++)
        {
            std::cout << "-- free test: " << j + 1 << '\n';
            alloc::free(ptrs.back());
            dump(buf);
            ptrs.pop_back();
        }

        int j = 1;
        while(ptrs.size() > 1000)
        {
            std::cout << "-- free2 test: " << j << '\n';
            alloc::free(ptrs.back());
            dump(buf);
            ptrs.pop_back();
            j++;
        }
    }

    for(auto i : ptrs)
        alloc::free(i);

    alloc::remove_region(buf);
    delete[] (uint64_t*)buf;

}

void foo()
{
    void* buf = (void*)new char[BUF];
    alloc::add_region(buf, BUF); 
    void* a = alloc::malloc(10);
    dump(buf);
    void* b = alloc::malloc(10);
    dump(buf);
    void* c = alloc::malloc(10);
    dump(buf);
    alloc::free(a);
    dump(buf);
    alloc::free(b);
    dump(buf);
    alloc::free(c);
    dump(buf);
}

int main(int argc, char** argv)
{
    for(int i = 0; i < 100000; i++)
    {
        srand(i);
        rngcase();
    } 
}
