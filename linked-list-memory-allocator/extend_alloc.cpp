#include "alloc.h"
#include <cstdint>
#include <new>

namespace alloc
{
    struct block_header
    {
        std::size_t size;
        block_header* back;
    };

    block_header* root = nullptr;
    block_header* last = nullptr;

    inline block_header* next_of(block_header* header)
    {
        return (block_header*)((uint8_t*)header + sizeof(block_header) + (header->size & ~1));
    }

    void* malloc(std::size_t size)
    {
        size = (size + 7) & ~7;
        if (!size)
            return nullptr;

        if(root == nullptr)
        {
            root = last = (block_header*)detail::start();
            auto s = detail::extend((void*) root, size + sizeof(block_header));
            auto r = new (root) block_header { s - sizeof(block_header), nullptr };
            return (void*)++r;
        }

        block_header* hdr = root;
        while (hdr <= last)
        {
            // if the least significant bit == 0 : alloc
            //                              == 1 : free
            if ((hdr->size & 1) && (hdr->size & ~1) >= size)
            {
                std::size_t bsize = hdr->size & ~1;
                if (bsize - size > sizeof(block_header))
                {
                    hdr->size = size;
                    auto* next_block = new (next_of(hdr)) block_header{(bsize - size - sizeof(block_header)) | 1, hdr};

                    if(hdr == last)
                        last = next_block;
                    else
                        next_of(next_block)->back = next_block;
                }

                hdr->size &= ~1;
                return (void*)++hdr;
            }
            hdr = next_of(hdr);
        }

        // extend memory and try to do stuff???
        std::size_t n = detail::extend((void*)next_of(last), size + 0x100) & ~7;
        if((last->size & 1) == 0)
        {
            auto u = new (next_of(last)) block_header { size & ~1, last };
            auto l = new (next_of(u)) block_header { (n - size - 2 * sizeof(block_header)) | 1, u };
            last = l;
            return (void*)++u;
        }
        else
        {
            std::size_t bsize = ((last->size & ~1) + n) - sizeof(block_header) - size;
            last->size = size;
            auto* l = new (next_of(last)) block_header{bsize | 1, last};
            last->size = size;
            auto* u = last;
            last = l;
            return (void*)++u;
        }
    }

    void* aligned_malloc(std::size_t size, std::size_t align)
    {
        void* buffer = malloc(size + align - 1);
        // TODO: implement
        return buffer;
    }

    void* realloc(void* buf, std::size_t size)
    {
        block_header* hdr = (block_header*)buf - 1;
        if (hdr->size > size)
            return buf;

        char* src = (char*)buf;
        char* target = (char*)malloc(size + 16);
        for (std::size_t i = 0; i < hdr->size; i++)
            target[i] = src[i];
        return target;
    }

    void free(void* buffer)
    {
        if (buffer == nullptr)
            return;

        std::size_t* type = (std::size_t*)buffer - 1;
        block_header* hdr = (block_header*)buffer - 1;

        if (*type & 2)
            hdr = (block_header*)(*type);

        bool is_prev = hdr->back ? hdr->back->size & 1 : false;
        bool is_next = hdr != last ? next_of(hdr)->size & 1 : false;
        block_header* prev = hdr->back;
        block_header* next = next_of(hdr);

        if (is_prev && is_next)
        {
            prev->size = (prev->size & ~1) + (hdr->size & ~1) + (next->size & ~1) + 2 * sizeof(block_header);
            prev->size |= 1;

            // update the back pointer of the block after next
            if (next_of(next) <= last)
                next_of(next)->back = prev;

            if(next == last)
                last = prev;
        }
        else if (is_prev)
        {
            prev->size = (prev->size & ~1) + (hdr->size & ~1) + sizeof(block_header);
            prev->size |= 1;

            // same logic as before
            if (next <= last)
                next->back = prev;

            if(hdr == last)
                last = prev;
        }
        else if (is_next)
        {
            hdr->size = (hdr->size & ~1) + (next->size & ~1) + sizeof(block_header);
            hdr->size |= 1;

            // same logic as before
            if (next_of(next) <= last)
                next_of(next)->back = hdr;

            if(next == last)
                last = hdr;
        }
        else
        {
            hdr->size |= 1;
        }
    }
} // namespace alloc
