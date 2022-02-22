# rbtree-memory-allocator
Memory allocator based of off red-black tree to search to a optimal block. Standalone implementation, so it can be used for kernel development.
## API 
```c++
// initalize buffer
alloc::set_malloc_space(buf, size);
// malloc some data
void* ptr = alloc::malloc(100);
// free the pointer
alloc::free(ptr);
```
