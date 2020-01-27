# set

Basic mathematical structure that guarantees the uniqueness of an entity in the whole. The comparison between two items in the set can be extended beyond the virtual memory address to a custom function that may, if provided.

## Implementation notice

It is implemented as an ordered double-linked list. If no comparison custom function is provided, finding an item in a set will heuristically outperform iterating in an equivalent unordered double-linked list.