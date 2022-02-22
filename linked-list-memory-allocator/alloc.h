#ifndef __LLMALLOC_H__
#define __LLMALLOC_H__
#include <cstddef>

namespace alloc
{
    size_t add_region(void*, size_t);
    void remove_region(void*);
    void* malloc(size_t);
    void* realloc(void*, size_t);
    void free(void*);
}


#endif
