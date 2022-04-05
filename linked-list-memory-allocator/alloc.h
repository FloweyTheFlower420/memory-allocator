#ifndef __LLMALLOC_H__
#define __LLMALLOC_H__
#include <cstddef>

namespace alloc
{
    std::size_t add_region(void*, std::size_t);
    void remove_region(void*);
    void* malloc(std::size_t);
    void* realloc(void*, std::size_t);
    void* aligned_malloc(std::size_t, std::size_t);
    void free(void*);
} // namespace alloc

#endif
