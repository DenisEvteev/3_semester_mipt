//
// Created by denis on 04.11.19.
//

#ifndef BST_AVL_TREE_HPP
#define BST_AVL_TREE_HPP

#include <functional>
#include <stack>
#include <iostream>
#include <cassert>
#include <memory>
#include <fstream>
#include <string>
/*I hesitated a lot about whether to implement iterative or recursive algorithm for finding, insertion
 * and deleting nodes in avl_tree, but finally I've found the information that a recursive method is slower
 * and take more space, so I've come to the desicion to implement an iterative method of avl opetrations*/


//namespace [high balanced binary tree === hbbt]

namespace hbbt {


#define ERROR_MESSAGE std::cout << "operator new haven't managed with allocation memory for the node\n";\
    std::cout << "line in " << __LINE__ << std::endl;\
    std::cout << "function is " << __PRETTY_FUNCTION__ << std::endl;\


#define FILE_EXTENSION "trees_with_dot/.dot"
#define POS_TO_INSERT 15

    enum {
        EMPTY,
        NOT_EXIST,
        LEFT,
        RIGHT,
        EXIST
    };

    template < class Key >
    struct AVL_Tree_Node {
        using node_p = AVL_Tree_Node *;
        using value_type = Key;
        using u_type = unsigned char;


        Key key_;
        node_p left_ch_ = nullptr;
        node_p right_ch_ = nullptr;
        node_p parent_ = nullptr;

        /*The height of the node is the maximum number in the depth with the root__ node*/
        u_type height_ = 1;


        AVL_Tree_Node(value_type key, node_p parent_node);

        ~AVL_Tree_Node() = default;

    };

    //______helper functions_________//
    template < typename Key >
    void fixheight(AVL_Tree_Node< Key > *node) {
        if (!node)
            return;

        unsigned char hl = height(node->left_ch_);
        unsigned char hr = height(node->right_ch_);

        node->height_ = (hl > hr ? hl : hr) + 1;
    }

    template < typename Key >
    inline unsigned char height(AVL_Tree_Node< Key > *node) {
        return node ? node->height_ : EMPTY;
    }

    template < typename Key >
    /*bfactor can be negative so the return value will be of type char*/
    inline short bfactor(AVL_Tree_Node< Key > *node) {
        if (!node)
            return EMPTY;
        return static_cast<short>(height(node->left_ch_) - height(node->right_ch_));
    }
    //_______________________________________//

    template <
            class Key,
            class Alloc,
            class Compare = std::less< Key>>
    class AVL_Tree {
        using node_p = AVL_Tree_Node< Key > *;
        using const_node_p = const AVL_Tree_Node< Key > *;
        using alloc = Alloc;
        using n_traits = std::allocator_traits< alloc >;


    public:

        /*User interface*/
        bool exist(const Key &key) const;

        void push(const Key &key);

        void remove(const Key &key);

        /*This function use an inorder iterative traverse algorithm
         * the iterative algorithm much more efficient, less complex, convenience for maintaining
         * reliability and general, so I implement only iterative algorithm when dealing with tree operations
         *
         * allow this algorith to be without any initial arguments of pointers to a node due to it will print all the
         * tree starting with the root__ so it has no sense to implement separatelly
         * interface function and backand function
         *
         * initially call of this function will request the file name string where to print
         * the result*/
        void show_tree() const;


        ~AVL_Tree();
        /*---------------*/

    private:

        node_p root__ = nullptr;
        alloc allocator_;
        Compare compr_;

        /*This function will work in copy constructor and assignment operator
         * Note : this function is implemented in a quite different way than it was in my previous
         * project for BSPTree. THis function will process the exceptional situations where we need to destroy
         * the part of tree and free the allocated memory
         * Also it's implemented with a bit different algorithm
         *
         *
         * This function is iterative and it doesn't use any extra struct of data
         * due to in my realization of avl tree exist a pointer to the parent node so we will easily
         * retrace back from the left bottom to copy the right subtrees of each node*/
        node_p Copy(const_node_p start_node);

        /*Functions of rotations the most important and pivotal in the algorithm*/

        node_p left_rotation(node_p node);

        node_p right_rotation(node_p node);

        /* This function take a pointer to the left child of the current node
         *
         *
         * This function describe the type of rotation when we perform
         * 1) left_rotation for the left child of the current node
         * 2) right_rotation for the current node
         *
         * This implementing of a double rotation as a single function will work more efficiently as i'll say further*/


        node_p left_right_rotation(node_p node);

        /*This function take a pointer to the right child of current node
         *
         * This is a type of rotation (aka double rotation) for avl tree when we perform
         * 1) right_rotation for the right child of current node
         * 2) left_rotation for the current node
         *
         * I've decided to implement this type of rotation in a single function
         * due to this operation must perform faster than using two separate function for both right child and
         * current node rotation --- it is obvious
         *
         * this function will work at the constant time*/
        node_p right_left_rotation(node_p node);

        /*This function will determine whether the element in the tree or not
         * 1) flag == 1  --- not exist
         * 2) flag == 2  --- the left subtree
         * 3) flag == 3  --- the right subtree
         * 4) flag == 4  --- the node is in the tree*/

        /*These function make the right parent nodes in cases of right, left rotations
         * and in case of double rotations respectevely*/

        inline void Make_Parent_1(node_p node, node_p head) {
            /*nullptr as a parent can have only the root__ node so else branch represent such a case*/
            if (node->parent_) {
                if (node->parent_->right_ch_ == node)
                    node->parent_->right_ch_ = head;
                else node->parent_->left_ch_ = head;
            } else root__ = head;
        }

        inline void Make_Parent_2(node_p node, node_p head) {
            /*the same notice as in the Make_Parent_2*/
            if (node->parent_->parent_) {
                if (node->parent_->parent_->left_ch_ == node->parent_)
                    node->parent_->parent_->left_ch_ = head;
                else node->parent_->parent_->right_ch_ = head;
            } else root__ = head;
        }
        //------------------------------------------------------------------------------//

        node_p find(const Key &key, node_p node, unsigned char &flag) const;

        void retracing(node_p cur_n);

        /*This is an iterative function
       * 1) For insertion object of type Key must have operator==
       * 2) compr_ it's a functor which have operator(). It returns true if
       * the first value is smaller otherwise false*/
        node_p insert(const Key &key, node_p start);

        void destroy_tree(node_p node);

        void destroy_tree_with_right_rotations(node_p node);

        bool delete_node(const Key &key, node_p node);


    };
}


#endif //BST_AVL_TREE_HPP
