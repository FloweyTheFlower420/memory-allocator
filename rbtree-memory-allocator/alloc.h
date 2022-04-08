#ifndef __RBTREE_ALLOC_H__
#define __RBTREE_ALLOC_H__
#include "rbtree_algo.h"
#include <cstddef>
#include <cstring>

namespace alloc
{
    using memcpy_t = void* (*)(void*, void*, size_t);

    template <memcpy_t memcpy_impl> class memory_region
    {
        free_node* root_node;
        common_node* last;
        constexpr public : memory_region(void* ptr, size_t size)
        {
            root_node = (free_node*)ptr;
            new (ptr) free_node;
            root_node->size_flags = size - sizeof(free_node);
            last = static_cast<common_node*>(ptr_add(ptr, size));
        }

        void* malloc(size_t size)
        {
            // Memory Allocation Algorithm
            // The algorithm should work as follows:
            // Given the parameters size,
            // 0. find the smallest node larger or equal to (>=) size
            // 1. check if the node exists (== nullptr). If it does not, the memory allocation has failed
            // 2. remove the node from the red-black tree
            // 3. from here, a few possible cases occur:
            //    a) there is not enough space to put a new free header
            //       in this case, extend the allocated space to the entire block
            //    b) there is enough room for another free node
            //       in this case, place [allocated header][allocated buffer][free block]
            //       then, re-insert the free block back into the red-black tree

            // align size to 8 byte
            // this could be unnecessary
            size = size > sizeof(free_node) ? size : sizeof(free_node);
            size = size & (~7);

            // 0.
            free_node* node = upper_bound(root_node, size);

            // 1.
            if (node == nullptr)
                return nullptr;

            // 2.
            remove(node, root_node);

            void* back = node->back;
            size_t current_size = node->size_flags;

            // obtain the size that shall remain for the free block
            // if this size is less than to that of sizeof(free_node), that means we are forced to extend the memory
            // block if not, we can place a header with a size of free_block_size
            size_t free_block_size = node->size_flags - sizeof(free_node) - size;

            // there is not enough room, extend allowed space
            if (free_block_size < sizeof(free_node))
            {
                // initalize used header in place of the free header
                alloced_node* curr = new (node) alloced_node;

                // update back pointer, which is lost in init, then update size
                curr->back = back;

                // size of blocks includes the headers
                curr->size_flags = current_size;

                // calculate end of alloced_node struct (start of usable buffer)
                return ptr_add(curr, sizeof(alloced_node));
            }
            else
            {
                // get pointer to the start of the next free block
                // [allocated header][buffer returned][free block]
                // ^                 ^                ^
                // node    +sizeof(alloced_header)  +size

                // initalize free header after used header + alloced space
                free_node* next = new (ptr_add(node, sizeof(alloced_node) + size)) free_node;

                // update next->next -> back pointer, size, and next -> back pointer
                if (!is_next_last(node, last))
                    next_of(node)->back = (void*)next;

                // we must compute the sizes of the blocks
                // the size of the allocated block would the size + sizeof(alloced_node)
                // the size of the free block would therefore be current_size - size of allocated block
                size_t allocated_block_size = sizeof(alloced_node) + size;

                next->back = (void*)node;
                next->size_flags = node->size_flags - allocated_block_size;

                // initialize used header in place of the free header
                alloced_node* curr = new (node) alloced_node;

                // update back pointer, which is lost in init, then update size
                curr->back = back;
                curr->size_flags = allocated_block_size;

                // re-insert the free node right after current allocated node
                insert(next, root_node);

                // calculate end of alloced_node struct (start of usable buffer)
                return ptr_add(curr, sizeof(alloced_node));
            }

            // achievement get: how did we get here?
        }

        void free(void* buf)
        {
            // Memory Free Algorithm
            // note: the minimum size of an allocated block is sizeof(free_node) alloc.cpp:30
            //
            // The algorithm should work as follows:
            // 0. compute the allocated node start, which is trivial to calculate
            // 1. find the back and next nodes
            //    from there, there are several cases:
            //    a) both nodes are allocated
            //       this is the simple case. Just preserve the back pointer and size
            //       and initialize a free node in place, then do a red-back tree insertion.
            //    b) the back node is allocated, but the next node is free
            //       take the sum of the current size and the previous size
            //       obtain a pointer to the next node
            //       point the back pointer of the next node to the previous node
            //       remove the previous node from the red-black tree, set the size, and re-insert
            //    c) the back node is free, but the next node is allocated
            //       take the sum of the current size and the next size
            //       point the next next node to this
            //    d) both are free

            // compute offset of block
            alloced_node* used = (alloced_node*)((char*)buf - sizeof(alloced_node));

            void* prev = used->back;
            void* next = (void*)((char*)buf + used->size_flags);

            bool is_next_free;
            bool is_prev_free;

            if (is_next_last(used, last))
                is_next_free = false;
            else
                is_next_free = ((size_flags_t*)next)->get_type();

            if (prev)
                is_prev_free = ((size_flags_t*)prev)->get_type();
            else
                is_prev_free = false;

            // both are free
            if (is_next_free && is_prev_free)
            {
                free_node* next_node = (free_node*)next;
                free_node* prev_node = (free_node*)prev;
                // remove both from the tree
                remove(next_node, root_node);
                remove(prev_node, root_node);
                // re-size prev_node
                prev_node->size_flags = prev_node->size_flags + used->size_flags + // sizeof current node and header
                                        next_node->size_flags;                     // sizeof next node and header

                insert(prev_node, root_node);

                // next next node should point to this
                // back->back is still valid
                next_of(next_node)->back = prev;
            }
            else if (is_prev_free)
            {
                free_node* prev_node = (free_node*)prev;
                remove(prev_node, root_node);
                // resize back node
                prev_node->size_flags = prev_node->size_flags + used->size_flags;
                insert(prev_node, root_node);
                // next node should point to this
                // back->back is still valid
                ((common_node*)next)->back = prev;
            }
            else if (is_next_free)
            {
                // this case is a bit annoying
                free_node* next_node = (free_node*)next;

                // keep copy of back ptr
                void* back = used->back;

                remove(next_node, root_node);
                size_t buf_size = next_node->size_flags + used->size_flags;
                free_node* curr = new (used) free_node;
                curr->size_flags = buf_size;
                curr->back = back;
                insert(curr, root_node);
            }
            else
            {
                size_t size = used->size_flags;
                assert(size >= sizeof(free_node));

                // placement new myself
                free_node* p = new (used) free_node;
                p->size_flags = size;
                insert(p, root_node);
            }
        }
        void* realloc(void* buf, size_t new_size)
        {
            alloced_node* used = (alloced_node*)((char*)buf - sizeof(alloced_node));
            if (used->size_flags >= new_size)
                return buf;

            void* new_buf = malloc(new_size);
            memcpy_impl(new_buf, buf, used->size_flags);
            free(buf);
            return new_buf;
        }
    };

} // namespace alloc

#endif
