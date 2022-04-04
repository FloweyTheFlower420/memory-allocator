#linked - list - memory - allocator
Basic linked list based memory allocator.It scans the entire list and finds the first block that fits.##API 
```c++
    // initalize buffer
    alloc::add_region(buf, size);
// malloc some data
void* ptr = alloc::malloc(100);
// free the pointer
alloc::free(ptr);
// remove buffer
alloc::remove_region(buf);
```
