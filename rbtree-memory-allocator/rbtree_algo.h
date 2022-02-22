//
// Created by flowey on 12/28/21.
//

#ifndef __RBTREE_ALGO_H__
#define __RBTREE_ALGO_H__

#include "types.h"
#include <cassert>
#include <utility>
#include <new>
#include <cstddef>

namespace alloc
{
    namespace detail
    {
        static inline size_flags_t get(free_node* ptr)
        {
            if (ptr)
                return ptr->size_flags;
            return { BLACK, FREE, 0 };
        }

        constexpr static inline void set(free_node* ptr, _color_flags_t flags)
        {
            if (ptr)
                ptr->size_flags = flags;
        }

        constexpr static inline void rol(free_node* node, free_node*& root)
        {
            free_node* right = node->right;
            node->right = right->left;
            if (right->left)
                right->left->parent = node;

            right->parent = node->parent;

            if (node->parent)
            {
                if (node == node->parent->left)
                    node->parent->left = right;
                else
                    node->parent->right = right;
            }
            else
                root = right;
            right->left = node;
            node->parent = right;
        }

        constexpr static inline void ror(free_node* node, free_node*& root)
        {
            free_node* left = node->left;
            node->left = left->right;
            if (left->right)
                left->right->parent = node;

            left->parent = node->parent;

            if (node->parent)
            {
                if (node == node->parent->right)
                    node->parent->right = left;
                else
                    node->parent->left = left;
            }
            else
                root = left;
            left->right = node;
            node->parent = left;
        }

        constexpr static inline free_node* grandparent(free_node* node)
        {
            return node->parent->parent;
        }

        static inline void rbtree_fix_insert(free_node* node, free_node*& root)
        {
            free_node* uncle;

            while (node != root && get(node->parent) == RED)
            {
                if (node->parent == grandparent(node)->left)
                {
                    uncle = grandparent(node)->right;

                    if (get(uncle) == RED)
                    {
                        set(node->parent, BLACK);
                        set(uncle, BLACK);
                        set(grandparent(node), RED);
                        node = grandparent(node);
                    }
                    else
                    {
                        if (node == node->parent->right)
                        {
                            node = node->parent;
                            rol(node, root);
                        }
                        set(node->parent, BLACK);
                        set(grandparent(node), RED);
                        ror(grandparent(node), root);
                    }
                }
                else
                {
                    uncle = grandparent(node)->left;
                    if (get(uncle) == RED)
                    {
                        set(node->parent, BLACK);
                        set(uncle, BLACK);
                        set(grandparent(node), RED);
                        node = grandparent(node);
                    }
                    else
                    {
                        if (node == node->parent->left)
                        {
                            node = node->parent;
                            ror(node, root);
                        }
                        set(node->parent, BLACK);
                        set(grandparent(node), RED);
                        rol(grandparent(node), root);
                    }
                }
            }

            set(root, BLACK);
        }

        static inline void rbtree_fix_delete(free_node* child, free_node* parent, free_node* root)
        {
            free_node* sibling;
            bool flag = true;

            if (parent->right == child)
                sibling = parent->left;
            else
                sibling = parent->right;

            while (flag)
            {
                if (!parent)
                {
                    return;
                }

                if (get(sibling) == RED)
                {
                    set(parent, RED);
                    set(sibling, BLACK);
                    if (parent->right == child)
                        ror(parent, root);
                    else
                        rol(parent, root);

                    if (parent->right == child)
                        sibling = parent->left;
                    else
                        sibling = parent->right;
                }

                if (get(parent) == BLACK && get(sibling) == BLACK && get(sibling->left) == BLACK && get(sibling->right) == BLACK)
                {
                    if (sibling)
                        set(sibling, RED);

                    child = parent;
                    parent = parent->parent;
                    if (parent->right == child)
                        sibling = parent->left;
                    else
                        sibling = parent->right;
                }
                else
                    flag = false;
            }

            if (get(parent) == RED && get(sibling) == BLACK && get(sibling->left) == BLACK && get(sibling->right) == BLACK)
            {
                if (sibling)
                    set(sibling, RED);
                set(parent, BLACK);
                return;
            }
            if (parent->right == child && get(sibling) == BLACK && get(sibling->right) == RED && get(sibling->left) == BLACK)
            {
                set(sibling, RED);
                set(sibling->right, BLACK);
                rol(sibling, root);
                if (parent->right == child)
                    sibling = parent->left;
                else
                    sibling = parent->right;
            }
            else if (parent->left == child && get(sibling) == BLACK && get(sibling->left) == RED && get(sibling->right) == BLACK)
            {
                set(sibling, RED);
                set(sibling->left, BLACK);
                ror(sibling, root);
                if (parent->right == child)
                    sibling = parent->left;
                else
                    sibling = parent->right;
            }

            if (sibling)
                sibling->size_flags.copy_flags(parent->size_flags);

            set(parent, BLACK);

            if (parent->right == child)
            {
                set(sibling->left, BLACK);
                ror(parent, root);
            }
            else
            {
                set(sibling->right, BLACK);
                rol(parent, root);
            }
        }

        static inline void change_parent(free_node* parent, free_node* old_node, free_node* new_node, free_node* root)
        {
            if (!parent)
            {
                if (root == old_node)
                    root = new_node;
                return;
            }
            if (parent->left == old_node)
                parent->left = new_node;
            if (parent->right == old_node)
                parent->right = new_node;
        }

        static inline void change_child(free_node* child, free_node* old_node, free_node* new_node)
        {
            if (child && child->parent == old_node)
                child->parent = new_node;
        }
    }

    static inline void insert(free_node* node, free_node*& root)
    {
        using namespace detail;

        free_node* curr = root;
        free_node* prev = nullptr;

        while (curr)
        {
            prev = curr;

            if (node->size_flags < curr->size_flags)
                curr = curr->left;
            else
                curr = curr->right;
        }

        node->parent = prev;
        node->left = node->right = nullptr;
        node->size_flags = RED;

        if (prev)
        {
            if (node->size_flags < prev->size_flags)
                prev->left = node;
            else
                prev->right = node;
        }
        else
            root = node;

        rbtree_fix_insert(node, root);
    }

    static inline bool remove(free_node* node, free_node*& root)
    {
        if (!root)
        {
            return false;
        }
        if (node == root)
        {
            root = nullptr;
            return true;
        }
        using namespace detail;

        free_node* child;
        if (node->left && node->right)
        {
            free_node* right = node->right;

            while (right->left)
                right = right->left;

            std::swap(node->size_flags, right->size_flags);

            change_parent(node->parent, node, right, root);
            if (node->right != right)
                change_parent(right->parent, right, node, root);

            change_child(right->left, right, node);
            change_child(right->left, right, node);
            change_child(right->right, right, node);
            change_child(right->right, right, node);
            change_child(node->left, node, right);

            if (node->right != right)
                change_child(node->right, node, right);
            if (node->right == right)
            {
                node->right = node;
                right->parent = right;
            }

            std::swap(node->parent, right->parent);
            std::swap(node->left, right->left);
            std::swap(node->right, right->right);
        }

        if (node->left)
            child = node->left;
        else
            child = node->right;

        change_parent(node->parent, node, child, root);
        change_child(child, node, node->parent);

        if (get(node) == RED);
        else if (get(child) == RED)
        {
            if (child)
                set(child, BLACK);
        }
        else
            rbtree_fix_delete(child, node->parent, root);

        node->parent = nullptr;
        node->left = nullptr;
        node->right = nullptr;

        set(node, BLACK);
        return true;
    }

    static inline free_node* upper_bound(free_node* root, size_t size)
    {
        free_node* node = root;
        free_node* tmp = nullptr;
        node = root;

        while (node)
        {
            if (node->size_flags == size)
                return node;
            if (node->size_flags < size)
                node = node->right;
            else
            {
                tmp = node;
                node = node->left;
            }
        }
        return tmp;
    }

    template <typename T>
    constexpr void* ptr_add(T* ptr, size_t n)
    {
        return (void*)((uint8_t*)ptr + n);
    }

    template <typename T>
    constexpr common_node* next_of(T* node)
    {
        return (common_node*)ptr_add(node, node->size_flags + sizeof(T));
    }

    template <typename T>
    constexpr bool is_next_last(T* node, common_node* last_ptr)
    {
        return (void*)next_of(node) == last_ptr;
    }
}; // namespace alloc


#endif //__RBTREE_ALGO_H__
