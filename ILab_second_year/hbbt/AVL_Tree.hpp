//
// Created by denis on 04.11.19.
//

#include "AVL_Tree.h"

using namespace hbbt;

/*AVL_Tree_Node methods implementation*/

template < class T >
AVL_Tree_Node< T >::AVL_Tree_Node(const_ref_type key, node_p parent_node) : key_(key), parent_(parent_node) {}

template < class T >
AVL_Tree_Node< T >::AVL_Tree_Node(const_ref_type key) : key_(key) {}

//---------------------------------------------------------------------------------------//

template <
        class T,
        class Alloc,
        class Compare
         >
AVL_Tree< T, Alloc, Compare >::AVL_Tree(const T &key) {
    root__ = n_traits::allocate(allocator_, 1);
    //the case when operating system hasn't managed with allocating the memory --- return with error message
    if (!root__) {
        ERROR_MESSAGE
        return;
    }

    n_traits::construct(allocator_, root__, key);
}

template <
        class T,
        class Alloc,
        class Compare
         >
AVL_Tree< T, Alloc, Compare >::AVL_Tree(tree_const_ref another_tree) {
    root__ = Copy(another_tree.root__);
}

template <
        class T,
        class Alloc,
        class Compare
         >
AVL_Tree< T, Alloc, Compare > &AVL_Tree< T, Alloc, Compare >::operator=(tree_const_ref another_node) &{
    if (&another_node == this)
        return *this;

    destroy_tree_with_right_rotations(root__);
    root__ = Copy(another_node.root__);

    return *this;
}

template <
        class T,
        class Alloc,
        class Compare
         >
AVL_Tree< T, Alloc, Compare >::AVL_Tree(rvalue_ref_type tempr_tree) noexcept {
    root__ = tempr_tree.root__;
    tempr_tree.root__ = nullptr;
}

template <
        class T,
        class Alloc,
        class Compare
         >
AVL_Tree< T, Alloc, Compare > &AVL_Tree< T, Alloc, Compare >::operator=(rvalue_ref_type tempr_tree) & noexcept {
    destroy_tree_with_right_rotations(root__);
    root__ = tempr_tree.root__;
    tempr_tree.root__ = nullptr;
}

template <
        class T,
        class Alloc,
        class Compare
         >
AVL_Tree< T, Alloc, Compare >::~AVL_Tree() {
    destroy_tree_with_right_rotations(root__);
}

/*AVL_Tree methods implementation and it is an iterative one due to clearing
 * in with a recursive one will be to stack memory costly (it will create a lot of stack frames and as the compiler
 * perform TCO only if the function perform returning in the case of returning with function call, so it won't be called
 *
 *
 * This algorithm require only one extra queue of pointers*/
template <
        class T,
        class Alloc,
        class Compare
         >
void AVL_Tree< T, Alloc, Compare >::destroy_tree(node_p node) {
    if (!node)
        return;

    std::cout << __PRETTY_FUNCTION__ << "has worked\n";


}


//This function start print right from the root__ node
template <
        class T,
        class Alloc,
        class Compare
         >
void AVL_Tree< T, Alloc, Compare >::show_tree() const {
    if (!root__)
        return;

    std::string file_path(FILE_EXTENSION);

    //request for name of file to print the tree
    std::string name_file;
    std::getline(std::cin, name_file);

    //make the full path
    file_path.insert(POS_TO_INSERT, name_file);

    std::ofstream file_dump;
    file_dump.open(file_path);
    if (!file_dump.is_open()) {
        std::cout << "ERROR in opening file with name : " << file_path << std::endl;
        std::cout << "The operation of dumping the tree cannot be performed" << std::endl;
        return;
    }

    file_dump << "digraph " << name_file << " {\n";
    file_dump << "size=\"2.5\"\n";

    file_dump << "node[shape=circle];\n";

    std::stack< const_node_p > helper;
    const_node_p node = root__;

    for (;;) {
        if (node) {
            helper.push(node);
            node = node->left_ch_;
            continue;
        }
        if (!helper.empty()) {
            const_node_p p = helper.top();
            helper.pop();
            if (p->parent_)
                file_dump << p->parent_->key_ << "->" << p->key_ << "\n";
            node = p->right_ch_;
        } else break;
    }


    file_dump << "}";

    file_dump.close();

}

template <
        class T,
        class Alloc,
        class Compare
         >
void AVL_Tree< T, Alloc, Compare >::destroy_tree_with_right_rotations(node_p node) {
    if (!node)
        return;
    for (;;) {
        if (node->left_ch_) {
            right_rotation(node);
            node = root__;
            continue;
        }
        node_p save = node->right_ch_;
        if (save)
            save->parent_ = nullptr;
        n_traits::destroy(allocator_, node);
        n_traits::deallocate(allocator_, node, 1);
        if (!save)
            return;
        node = save;
    }
}

template <
        class T,
        class Alloc,
        class Compare
         >
AVL_Tree_Node< T > *AVL_Tree< T, Alloc, Compare >::Copy(const_node_p start_node) {
    if (!start_node)
        return nullptr;

    /*Make a new pointer as a future root to the new tree*/
    node_p head = n_traits::allocate(allocator_, 1);
    if (!head) {
        ERROR_MESSAGE
        return nullptr;
    }
    n_traits::construct(allocator_, head, start_node->key_, start_node->parent_);

    /*Two running pointers in new tree and in old respectevely*/
    node_p new_cur = head;
    const_node_p old_cur = start_node;

    for (;;) {
        if (old_cur->left_ch_) {
            /*if the operating system haven't managed with allocating the memory for a new node
             * than in this function of copying the tree we will delete the part of the tree and return nullptr
             * as a result of copying*/
            new_cur->left_ch_ = n_traits::allocate(allocator_, 1);
            if (!new_cur->left_ch_) {
                ERROR_MESSAGE
                destroy_tree(head);
                return nullptr;
            }
            n_traits::construct(allocator_, new_cur->left_ch_, old_cur->left_ch_->key_, new_cur);
            new_cur = new_cur->left_ch_;
            old_cur = old_cur->left_ch_;
            continue;
        }

        /*This loop deal with the right children of each node*/
        for (;;) {
            /*work with the right copying of the each node*/
            if (old_cur->right_ch_) {
                new_cur->right_ch_ = n_traits::allocate(allocator_, 1);
                if (!new_cur->right_ch_) {
                    ERROR_MESSAGE
                    destroy_tree(head);
                    return nullptr;

                }
                n_traits::construct(allocator_, new_cur->right_ch_, old_cur->right_ch_->key_, new_cur);
                new_cur = new_cur->right_ch_;
                old_cur = old_cur->right_ch_;
                break;
            }

            for (;;) {
                if (old_cur->parent_) {
                    const_node_p save = old_cur;
                    old_cur = old_cur->parent_;
                    new_cur = new_cur->parent_;
                    if (save == old_cur->left_ch_)
                        break;
                    continue;
                } else return head;
            }
        }
    }


}

template <
        class T,
        class Alloc,
        class Compare
         >
bool AVL_Tree< T, Alloc, Compare >::delete_node(const_ref_type key, node_p node) {
    if (!node)
        return false;

    unsigned char flag = 0;
    node_p cur_n = find(key, node, flag);
    if (flag == LEFT || flag == RIGHT)
        return false;
    else {

    }


}

template <
        class T,
        class Alloc,
        class Compare
         >
AVL_Tree_Node< T > *AVL_Tree< T, Alloc, Compare >::insert(const T &key, node_p start) {

    unsigned char flag = 0;
    node_p cur_n = find(key, start, flag);
    if (flag == EXIST)
        return cur_n;

    node_p inserted_node = n_traits::allocate(allocator_, 1);
    if (!inserted_node) {
        ERROR_MESSAGE
        return nullptr;
    }
    n_traits::construct(allocator_, inserted_node, key, cur_n);

    if (flag == NOT_EXIST) {
        root__ = inserted_node;
        return root__;
    } else if (flag == RIGHT)
        cur_n->right_ch_ = inserted_node;
    else
        cur_n->left_ch_ = inserted_node;


    /*This is a invariant loop aka retracing operation
     * we must make sure that each node has invariant balanced factor
     * otherwise to perform rotation to make it balanced*/

    retracing(cur_n);

    return inserted_node;
}

template <
        class T,
        class Alloc,
        class Compare
         >
void AVL_Tree< T, Alloc, Compare >::retracing(node_p cur_n) {
    while (cur_n) {
        fixheight(cur_n);
        short bf = bfactor(cur_n);
        //check the invariant of an avl_tree
        assert(std::abs(bf) <= 2);

        if (bf == 0)
            break;

        if (std::abs(bf) <= 1) {
            cur_n = cur_n->parent_;
            continue;
        }

        if (bf == 2) {
            short left_bf = bfactor(cur_n->left_ch_);
            assert(std::abs(left_bf) <= 1);
            if (left_bf == -1)
                cur_n = left_right_rotation(cur_n->left_ch_);
            else cur_n = right_rotation(cur_n);
        } else {
            short right_bf = bfactor(cur_n->right_ch_);
            assert(std::abs(right_bf) <= 1);
            if (right_bf == 1)
                cur_n = right_left_rotation(cur_n->right_ch_);
            else cur_n = left_rotation(cur_n);
        }


    }
}

template <
        class T,
        class Alloc,
        class Compare
         >
AVL_Tree_Node< T > *AVL_Tree< T, Alloc, Compare >::find(const T &key, node_p node, unsigned char &flag) const {
    if (!node) {
        flag = NOT_EXIST;
        return nullptr;
    }

    node_p cur_n = node;

    while (cur_n) {
        if (key == cur_n->key_) {
            flag = EXIST;
            return cur_n;
        } else if (compr_(key, cur_n->key_)) {
            if (!cur_n->left_ch_) {
                flag = LEFT;
                break;
            }
            cur_n = cur_n->left_ch_;
        } else {
            if (!cur_n->right_ch_) {
                /*In this case the returned pointer always not a nullptr -- it's an important information
                 * similarly in the case of the left subtree*/
                flag = RIGHT;
                break;
            }
            cur_n = cur_n->right_ch_;
        }
    }

    return cur_n;
}

template <
        class T,
        class Alloc,
        class Compare
         >
AVL_Tree_Node< T > *AVL_Tree< T, Alloc, Compare >::left_rotation(node_p node) {
    assert(node && node->right_ch_);

    node_p head = node->right_ch_;
    node->right_ch_ = head->left_ch_;
    if (node->right_ch_)
        node->right_ch_->parent_ = node;

    head->left_ch_ = node;

    Make_Parent_1(node, head);

    head->parent_ = node->parent_;

    node->parent_ = head;

    fixheight(node);
    fixheight(head);
    return head->parent_;
}

template <
        class T,
        class Alloc,
        class Compare
         >
AVL_Tree_Node< T > *AVL_Tree< T, Alloc, Compare >::right_rotation(node_p node) {
    assert(node && node->left_ch_);

    node_p head = node->left_ch_;
    node->left_ch_ = head->right_ch_;

    if (node->left_ch_)
        node->left_ch_->parent_ = node;


    Make_Parent_1(node, head);

    head->parent_ = node->parent_;
    head->right_ch_ = node;
    node->parent_ = head;

    fixheight(node);
    fixheight(head);

    return head->parent_;
}

template <
        class T,
        class Alloc,
        class Compare
         >
AVL_Tree_Node< T > *AVL_Tree< T, Alloc, Compare >::right_left_rotation(node_p node) {
    assert(node && node->left_ch_);
    node_p head = node->left_ch_;
    node->left_ch_ = head->right_ch_;

    if (head->right_ch_)
        head->right_ch_->parent_ = node;


    Make_Parent_2(node, head);

    head->parent_ = node->parent_->parent_;


    node->parent_->right_ch_ = head->left_ch_;
    if (head->left_ch_)
        head->left_ch_->parent_ = node->parent_;

    head->left_ch_ = node->parent_;
    head->right_ch_ = node;
    node->parent_ = head;
    head->left_ch_->parent_ = head;

    fixheight(node);
    fixheight(head->left_ch_);
    fixheight(head);

    return head->parent_;

}

template <
        class T,
        class Alloc,
        class Compare
         >
AVL_Tree_Node< T > *AVL_Tree< T, Alloc, Compare >::left_right_rotation(node_p node) {
    assert(node && node->right_ch_);

    node_p head = node->right_ch_;
    node->right_ch_ = head->left_ch_;

    if (node->right_ch_)
        node->right_ch_->parent_ = node;

    Make_Parent_2(node, head);

    head->parent_ = node->parent_->parent_;
    node->parent_->left_ch_ = head->right_ch_;

    if (head->right_ch_)
        head->right_ch_->parent_ = node->parent_;

    head->left_ch_ = node;
    head->right_ch_ = node->parent_;

    node->parent_ = head;
    head->right_ch_->parent_ = head;


    fixheight(node);
    fixheight(head->right_ch_);
    fixheight(head);

    return head->parent_;

}

//________________User interface methods____________________________//
template <
        class Key,
        class Alloc,
        class Compare
         >
bool hbbt::AVL_Tree< Key, Alloc, Compare >::exist(const Key &key) const {
    unsigned char flag = 0;

    find(key, root__, flag);

    return (flag == EXIST);
}

template <
        class Key,
        class Alloc,
        class Compare
         >
void hbbt::AVL_Tree< Key, Alloc, Compare >::push(const Key &key) {
    node_p new_node = insert(key, root__);
    assert(new_node);
}

template <
        class Key,
        class Alloc,
        class Compare
         >
void hbbt::AVL_Tree< Key, Alloc, Compare >::remove(const Key &key) {

}
//-----------------------------------------------------------------//