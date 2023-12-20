/**
 * Ref: https://github.com/itsjohncs/minmaxheap-cpp
 * @file MinMaxHeap.hpp
 * @author John Sullivan (jsull003 at ucr.edu)
 * @date December 28, 2010
 *
 * @brief Definition and implementation for class @ref MinMaxHeap.
 *
 * @see Paper Introducing the Min-Max Heap (http://www.cs.otago.ac.nz/staffpriv/mike/Papers/MinMaxHeaps/MinMaxHeaps.pdf)
 * @see Alternative Implementation (http://www.coldbrains.com/code/code/C++/Data_Structures/Min-Max_Heap/MinMaxHeap.C.html,
 *                                  http://www.coldbrains.com/code/code/C++/Data_Structures/Min-Max_Heap/MinMaxHeap.H.html)
 **/

#ifndef GSTORELIMITK_QUERY_TOPKSTRUCTURE_MINMAXHEAP_H_
#define GSTORELIMITK_QUERY_TOPKSTRUCTURE_MINMAXHEAP_H_

#include <vector>
#include <stdexcept>
#include <iostream>

namespace minmax
{

/**
 * @brief Returns the log base 2 of a number @b zvalue.
 **/
inline unsigned int log2(unsigned int zvalue)
{
  // log2(0) is undefined so error
  if (zvalue == 0) throw std::domain_error("Log base 2 of 0 is undefined.");

  unsigned int result = 0;

  // Loop until zvalue equals 0
  while (zvalue)
  {
    zvalue >>= 1;
    ++result;
  }

  return result - 1;
}

/**
 * @brief An implementation of the Min-Max Heap.
 *
 * @tparam T         The type of element stored in the heap.
 * @tparam Container The type fo container used to store the heap. Must support
 *                   random access and be stl compliant.
 * @tparam Compare   The comparison function class that will be used to
 *                   determine the ordering of the heap.
 **/
template<class T, class Container = std::vector<T>, class Compare = std::less<T> >
class MinMaxHeap // Root is on Max level
{
  /**
   * @brief The container that is used to store the heap.
   **/
  Container heap_;

  /**
   * @brief The comparison object used for comparisons.
   **/
  Compare compare_;

  /**
   * @brief Returns the index of the parent of the node specified by
   *        @c zindex.
   **/
  static inline unsigned int parent(unsigned int zindex)
  {
    return (zindex - 1) / 2;
  }

  /**
   * @brief Returns the index of the left child of the node specified by
   *        @c zindex.
   **/
  static inline unsigned int leftChild(unsigned int zindex)
  {
    return 2 * zindex + 1;
  }

  /**
   * @brief Returns the index of the right child of the node specified by
   *        @c zindex.
   **/
  static inline unsigned int rightChild(unsigned int zindex)
  {
    return 2 * zindex + 2;
  }

  /**
   * @brief Returns @c true if the node specified by @c zindex is on a
   *        @e min-level.
   **/
  static inline bool isOnMinLevel(unsigned int zindex)
  {
    return log2(zindex + 1) % 2 == 1;
  }

  /**
   * @brief Returns @c true if the node specified by @c zindex is on a
   *        @e max-level.
   **/
  static inline bool isOnMaxLevel(unsigned int zindex)
  {
    return !isOnMinLevel(zindex);
  }

  /**
   * @brief Performs a sift-up, trickle-up, or bubble-up operation on the node
   *        specified by @c zindex without checking to see if the node is
   *        on the right @e track (min or max).
   *
   * @note This is a helper function for @c trickleUp(). For those not
   *       familiar with this convention, that is what the underscore after
   *       the name signifies.
   *
   * @tparam MaxLevel Set to @c true if and only if @c zindex is on a max
   *         level.
   **/
  template<bool MaxLevel>
  void trickleUp_(unsigned int zindex)
  {
    // Can't bring the root any farther up
    if (zindex == 0) return;

    // Find the parent of the passed node first
    unsigned int zindex_grandparent = parent(zindex);

    // If there is no grandparent, return
    if (zindex_grandparent == 0) return;

    // Find the grandparent
    zindex_grandparent = parent(zindex_grandparent);

    // Check to see if we should swap with the grandparent
    if (compare_(heap_[zindex], heap_[zindex_grandparent]) ^ MaxLevel)
    {
      std::swap(heap_[zindex_grandparent], heap_[zindex]);
      trickleUp_<MaxLevel>(zindex_grandparent);
    }
  }

  /**
   * @brief Performs a sift-up, trickle-up, or bubble-up operation on the node
   *        specified by @c zindex.
   *
   * The operation is carried out similarily to the equivalant operation in a
   * standard heap.
   *
   * This function simply places the node on a min or max level as needed,
   * then calls @c trickleUp_() acoordingly.
   **/
  void trickleUp(unsigned int zindex)
  {
    // Can't bring the root any farther up
    if (zindex == 0) return;

    // Find the parent of the passed node
    unsigned int zindex_parent = parent(zindex);

    if (isOnMinLevel(zindex))
    {
      // Check to see if we should swap with the parent
      if (compare_(heap_[zindex_parent], heap_[zindex]))
      {
        std::swap(heap_[zindex_parent], heap_[zindex]);
        trickleUp_<true>(zindex_parent);
      }
      else
      {
        trickleUp_<false>(zindex);
      }
    }
    else
    {
      // Check to see if we should swap with the parent
      if (compare_(heap_[zindex], heap_[zindex_parent]))
      {
        std::swap(heap_[zindex_parent], heap_[zindex]);
        trickleUp_<false>(zindex_parent);
      }
      else
      {
        trickleUp_<true>(zindex);
      }
    }
  }

  /**
   * @brief Performs a sift-down, trickle-down, or bubble-down operation on
   *        the node specified by @c zindex without checking to see if the
   *        node is on the right @e track (min or max).
   *
   * @note This is a helper function for @c trickleDown(). For those not
   *       familiar with this convention, that is what the underscore after
   *       the name signifies.
   *
   * @tparam MaxLevel Set to @c true if and only if @c zindex is on a max
   *         level.
   *
   * @exception std::invalid_argument Thrown when no element as @c zindex exists.
   **/
  template<bool MaxLevel>
  void trickleDown_(unsigned int zindex)
  {
    /* In the following comments, substitute the word "less" with the word
     * "more" and the word "smallest" with the word "greatest" when MaxLevel
     * equals true. */

    // Ensure the element exists.
    if (zindex >= heap_.size())
      throw std::invalid_argument("Element specified by zindex does not "
                                  "exist");

    /* This will hold the index of the smallest node among the children,
     * grandchildren of the current node, and the current node itself. */
    unsigned int smallestNode = zindex;

    /* Get the left child, all other children and grandchildren can be found
     * from this value. */
    unsigned int left = leftChild(zindex);

    // Check the left and right child
    if (left < heap_.size() && (compare_(heap_[left], heap_[smallestNode]) ^ MaxLevel))
      smallestNode = left;
    if (left + 1 < heap_.size() && (compare_(heap_[left + 1], heap_[smallestNode]) ^ MaxLevel))
      smallestNode = left + 1;

    /* Check the grandchildren which are guarenteed to be in consecutive
     * positions in memory. */
    unsigned int leftGrandchild = leftChild(left);
    for (unsigned int i = 0; i < 4 && leftGrandchild + i < heap_.size(); ++i)
      if (compare_(heap_[leftGrandchild + i], heap_[smallestNode]) ^ MaxLevel)
        smallestNode = leftGrandchild + i;

    // The current node was the smallest node, don't do anything.
    if (zindex == smallestNode) return;

    // Swap the current node with the smallest node
    std::swap(heap_[zindex], heap_[smallestNode]);

    // If the smallest node was a grandchild...
    if (smallestNode - left > 1)
    {
      // If the smallest node's parent is bigger than it, swap them
      if (compare_(heap_[parent(smallestNode)], heap_[smallestNode]) ^ MaxLevel)
        std::swap(heap_[parent(smallestNode)], heap_[smallestNode]);

      trickleDown_<MaxLevel>(smallestNode);
    }
  }

  /**
   * @brief Performs a sift-down, trickle-down, or bubble-down operation on
   *        the node specified by @c zindex.
   *
   * This operation is carried out similarily to the equivalent operation in a
   * standard heap.
   **/
  void trickleDown(unsigned int zindex)
  {
    if (isOnMinLevel(zindex))
      trickleDown_<false>(zindex);
    else
      trickleDown_<true>(zindex);
  }

  /**
   * @brief Finds the smallest element in the Min-Max Heap and return its
   *        index.
   *
   * @exception std::underflow_error
   **/
  unsigned int findMinIndex() const
  {
    // There are four cases
    switch (heap_.size())
    {
      case 0:
        // The heap is empty so throw an error
        throw std::underflow_error("No min element exists because "
                                   "there are no elements in the "
                                   "heap.");
      case 1:
        // There is only one element in the heap, return that element
        return 0;
      case 2:
        // There are two elements in the heap, the child must be the min
        return 1;
      default:
        /* There are three or more elements in the heap, return the
         * smallest child */
        return compare_(heap_[1], heap_[2]) ? 1 : 2;
    }
  }

  /**
   * @brief Deletes the element at the given index in the heap while
   *        preserving the Min-Max heap property.
   *
   * @exception std::underflow_error
   **/
  void deleteElement(unsigned int zindex)
  {
    // Ensure the element exists
    if (zindex >= (unsigned int)heap_.size())
      throw std::underflow_error("Cannot delete specified element from "
                                 "the heap because it does not exist.");

    // If we're deleting the last element in the heap, just delete it
    if (zindex == heap_.size() - 1)
    {
      heap_.pop_back();
      return;
    }

    // Replace the element with the last element in the heap
    std::swap(heap_[zindex], heap_[heap_.size() - 1]);

    // Delete the last element in the heap
    heap_.pop_back();

    /* Let the element trickle down so that the min-max heap property is
     * preserved */
    trickleDown(zindex);
  }

 public:
  MinMaxHeap() { }

  MinMaxHeap(Container zcontainer, Compare zcompare = Compare())
      : heap_(zcontainer), compare_(zcompare) { }

  /**
   * @brief Returns true if and only if the heap is empty.
   **/
  bool empty() const
  {
    return heap_.size() == 0;
  }

  /**
   * @brief Returns the number of elements in the heap.
   **/
  unsigned int size() const
  {
    return (unsigned int)heap_.size();
  }

  /**
   * @brief Adds an element with the given value onto the heap.
   **/
  void push(const T & zvalue)
  {
    // Push the value onto the end of the heap
    heap_.push_back(zvalue);

    // Reorder the heap so that the min-max heap property holds true
    trickleUp(heap_.size() - 1);
  }


  /**
   * @brief Adds an element with the given value onto the heap.
   **/
  void push(T && move_value)
  {
    // Push the value onto the end of the heap
    heap_.push_back(std::move(move_value));

    // Reorder the heap so that the min-max heap property holds true
    trickleUp(heap_.size() - 1);
  }

  /**
   * @brief Returns the element with the greatest value in the heap.
   *
   * @exception std::underflow_error
   **/
  const T & findMax() const
  {
    // If the heap is empty throw an error
    if (empty())
      throw std::underflow_error("No max element exists because there "
                                 "are no elements in the heap.");

    return heap_[0];
  }

  /**
   * @brief Returns the element with the least value in the heap.
   *
   * @exception std::underflow_error
   **/
  const T & findMin() const
  {
    // findMinIndex() will throgh an underflow_error if no min exists
    return heap_[findMinIndex()];
  }

  /**
   * @brief Removes the element with the greatest value from the heap and
   *        returns its value.
   *
   * @exception std::underflow_error
   **/
  T popMax()
  {
    // If the heap is empty throw an error
    if (heap_.size() == 0)
      throw std::underflow_error("No max element exists because there "
                                 "are no elements in the heap.");

    // Save the max value
    T temp = heap_[0];

    deleteElement(0);

    return temp;
  }

  /**
   * @brief Convenience function that calls popMax().
   **/
  T pop()
  {
    return popMax();
  }

  /**
   * @brief Removes the element with the least value from the heap and returns
   *        its value.
   *
   * @exception std::underflow_error
   **/
  T popMin()
  {
    // If the heap is empty throw an error
    if (heap_.size() == 0)
      throw std::underflow_error("No max element exists because there "
                                 "are no elements in the heap.");

    // Save the min's index
    unsigned int smallest = findMinIndex();

    // Save the min value
    T temp = heap_[smallest];

    deleteElement(smallest);

    return temp;
  }

  /**
   * @brief Outputs the heap, as layed out in memory, into the given output
   *        stream.
   **/
  void printRaw(std::ostream & zout = std::cout) const
  {
    zout << "{";
    if (empty())
      zout << "Heap is Empty";
    else
      for (unsigned int i = 0; i < heap_.size(); ++i)
        zout << heap_[i] << (i != heap_.size() - 1 ? ", " : "");
    zout << "}";

    zout << std::endl;
  }
};

} // namespace minmax

#endif //GSTORELIMITK_QUERY_TOPKSTRUCTURE_MINMAXHEAP_H_