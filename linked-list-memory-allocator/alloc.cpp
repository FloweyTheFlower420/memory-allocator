#include "alloc.h"
#include <cstdint>
#include <new>

namespace alloc
{
    struct megablock_header
    {
        megablock_header* next;
        megablock_header* back;
    };

    struct block_header
    {
        std::size_t size;
        block_header* next;
        block_header* back;
    };

    static megablock_header* root = nullptr;

    std::size_t add_region(void* buf, std::size_t size)
    {
        size &= ~7;

        auto* block = new (buf) megablock_header{nullptr, nullptr};
        new (block + 1) block_header{(size - sizeof(megablock_header) - sizeof(block_header)) | 1, nullptr, nullptr};
        if (root == nullptr)
        {
            root = (megablock_header*)buf;
            return size;
        }

        block->next = root->next;
        root->next = block;

        return size;
    }

    void remove_region(void* buffer)
    {
        megablock_header* header = (megablock_header*)buffer;
        if (header == root)
        {
            if (header->next)
                header->next->back = nullptr;
            root = header->next;
        }
        else
        {
            if (header->back)
                header->back->next = header->next;
            if (header->next)
                header->next->back = header->back;
        }
    }

    void* malloc(std::size_t size)
    {
        if (!size)
            return nullptr;

        size = (size + 7) & ~7;

        auto* current_megablock = root;
        while (current_megablock)
        {
            block_header* hdr = (block_header*)(current_megablock + 1);
            while (hdr)
            {
                // if the least significant bit == 0 : alloc
                //                              == 1 : free
                if ((hdr->size & 1) && (hdr->size & ~1) >= size)
                {
                    std::size_t bsize = hdr->size & ~1;
                    if (bsize - size >= sizeof(block_header))
                    {
                        auto* next_block = new ((uint8_t*)hdr + sizeof(block_header) + size)
                            block_header{(hdr->size - size - sizeof(block_header)), hdr->next, hdr};

                        hdr->size = size;
                        if (hdr->next)
                            hdr->next->back = next_block;

                        hdr->next = next_block;
                    }

                    hdr->size &= ~1;
                    return (void*)++hdr;
                }
                hdr = hdr->next;
            }

            current_megablock = root->next;
        }
        return nullptr;
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
        for (int i = 0; i < hdr->size; i++)
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
        bool is_next = hdr->next ? hdr->next->size & 1 : false;
        block_header* prev = hdr->back;
        block_header* next = hdr->next;

        if (is_prev && is_next)
        {
            prev->next = next->next;
            prev->size = (prev->size & ~1) + (hdr->size & ~1) + (next->size & ~1) + 2 * sizeof(block_header);
            prev->size |= 1;

            // update the back pointer of the block after next
            if (next->next)
                next->next->back = prev;
        }
        else if (is_prev)
        {
            prev->next = next;
            prev->size = (prev->size & ~1) + (hdr->size & ~1) + sizeof(block_header);
            prev->size |= 1;

            // same logic as before
            if (next)
                next->back = prev;
        }
        else if (is_next)
        {
            hdr->next = next->next;
            hdr->size = (hdr->size & ~1) + (next->size & ~1) + sizeof(block_header);
            hdr->size |= 1;

            // same logic as before
            if (next->next)
                next->next->back = hdr;
        }
        else
        {
            hdr->size |= 1;
        }
    }
} // namespace alloc
