#include <iostream>
#include "AVL_Tree.hpp"
#include "TrackingAllocator.hpp"

/*Notes : The memory managing class operates with overload operator new which won't throw an exception
 * in case of not managing with allocating memory conversely it returns nullptr, so function for copying avl trees and some
 * other function dealing with allocating space for tree will process the situations when the operator new function returns nullptr
 * sometimes where necessary(copy trees) the function for deleting the allocated part of the tree will be called*/
int main() {


    hbbt::AVL_Tree< int, TrackingAllocator< AVL_Tree_Node< int>> > first_try;


    for (int i = 0; i < 50; ++i)
        first_try.push(i);


    hbbt::AVL_Tree< int, TrackingAllocator< AVL_Tree_Node< int>> > second_try = first_try;

    first_try.show_tree();

    second_try.show_tree();

    return 0;
}