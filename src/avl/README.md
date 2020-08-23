# avl

Implementation of generic AVL tree. Generic in the sense of arbitrary data and arbitrary comparison and deallocation functions. Great alternative of the `Set` C data structure for the major jobs, except for obtaining inorder successor and predecessor (which takes `O(log(n))`, while Set (double-linked list) takes `O(1)`).

## Comparison with Python set

The set class was found to be the closest to avl in native Python. Therefore, some benchmarks were run to compare the most basic operations:

### Insertion (add/avlInsert)

![Imgur](https://i.imgur.com/jjolBLi.png)

### Querying (contains/avlSearch)

![Imgur](https://i.imgur.com/ZA4egZS.png)

### Iterating (iterate/avlTraverse)

![Imgur](https://i.imgur.com/m1KrbdQ.png)
