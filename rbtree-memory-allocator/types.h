//
// Created by flowey on 12/28/21.
//

#ifndef __TYPES_H__
#define __TYPES_H__

#include <cstddef>
#include <cstdint>
#include <compare>
namespace alloc
{
    namespace detail
    {
        struct _color_flags_t
        {
            bool b;
        };

        struct _type_flags_t
        {
            bool b;
        };
    }

    constexpr detail::_color_flags_t RED{ false };
    constexpr detail::_color_flags_t BLACK{ true };

    constexpr detail::_type_flags_t FREE{ true };
    constexpr detail::_type_flags_t USED{ false };

    struct size_flags_t
    {
        size_t val;

        size_flags_t() { val = 0; }

        size_flags_t(const detail::_color_flags_t& color, const detail::_type_flags_t& flags, size_t size)
        {
            val = size;
            *this = color;
            *this = flags;
        }

        // color flags conversion
        inline size_flags_t& operator=(detail::_color_flags_t color)
        {
            val &= ~(size_t)1; // clear the bit where color exists
            val |= color.b ? 1 : 0;
            return *this;
        };

        inline bool operator==(const detail::_color_flags_t& lhs) const { return (val & 1) == lhs.b; }

        // type flags conversion
        inline size_flags_t& operator=(detail::_type_flags_t color)
        {
            val &= ~(size_t)2;
            val |= color.b ? 2 : 0;
            return *this;
        }

        inline constexpr operator uint64_t() const { return val & (~(size_t)3); }

        inline bool operator==(const detail::_type_flags_t& lhs) const { return (val & (size_t)2) == lhs.b; }

        inline std::strong_ordering operator<=>(const size_flags_t& lhs) const
        {
            return (val & (~(size_t)3)) <=> (lhs.val & (~(size_t)3));
        }

        inline size_flags_t& operator=(size_t size)
        {
            val = (val & (size_t)3) | (size & (~(size_t)3));
            return *this;
        }

        // or + and operators for setting individual flags

        inline bool get_color() const { return val & (size_t)1; }
        inline bool get_type() const { return val & (size_t)2; }
        inline void copy_flags(const size_flags_t& rhs)
        {
            val &= ~(size_t)1;
            val |= rhs.get_color() ? 1 : 0;
            val &= ~(size_t)2;
            val |= rhs.get_type() ? 2 : 0;
        }
    };

    struct common_node
    {
        size_flags_t size_flags;
        void* back = nullptr;
    };

    struct free_node
    {
        // in this case, size_flags includes the size of the header
        // this is very important, because the free block itself is free memory
        size_flags_t size_flags = { RED, FREE, 0 };
        void* back = nullptr;
        free_node* left = nullptr;
        free_node* right = nullptr;
        free_node* parent = nullptr;
    };

    struct alloced_node
    {
        size_flags_t size_flags = { RED, USED, 0 };
        void* back = nullptr;
    };
}

#endif //__TYPES_H__
