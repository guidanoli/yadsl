# avl

Implementation of generic AVL tree. Generic in the sense of arbitrary data and arbitrary comparison and deallocation functions. Great alternative of the `Set` data structure for the major jobs, except for obtaining inorder successor and predecessor (which takes `O(log(n))`, while Set (double-linked list) takes `O(1)`).
