# B-Tree (2-3-4 Tree) Assignment

## Implementation Details
* **Duplicate Handling:** Duplicates are allowed and handled safely by treating them as greater keys during the traversal, naturally shifting them to the right.
* **STL Containers:** No C++ STL containers (like std::vector or std::queue) were used. The level-order traversal utilizes static array-based queues to meet constraints.

## Concept Questions
**1. Why is a 2-3-4 Tree a special case of B-Tree?**
A 2-3-4 tree is simply a B-Tree of order 4 (minimum degree t = 2). This means every internal node can hold between 1 and 3 keys, and have between 2 and 4 children.

**2. Maximum number of children?**
The maximum number of children for any node in a 2-3-4 tree is 4.

**3. Explain split operation.**
When a node becomes full (contains 3 keys), it must split before a new key can be inserted. It splits into two nodes (each taking 1 key), and the middle key is pushed up to the parent node to maintain balance.

**4. Complexity of insertion?**
The time complexity for insertion is O(log n), as the height of a balanced 2-3-4 tree is logarithmic relative to the number of keys.

## System Recovery Questions
**1. Why is saving only keys not enough?**
Because a B-Tree's shape relies heavily on its internal structure. If we only save keys, we lose the topology of the tree (which keys were grouped together in which nodes) and would have to re-insert them, which might result in a different tree shape depending on the insertion order.

**2. How do you distinguish node types in serialization?**
Before saving a node's keys to the snapshot file, I wrote a boolean flag (`1` for leaf, `0` for internal node) followed by the number of keys. When restoring, the program reads this flag to know exactly how to reconstruct the child pointers.