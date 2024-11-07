#ifndef VOXMAP_H
#define VOXMAP_H

#include <istream>
#include <cstring>
#include <algorithm>

#include "Point.h"
#include "Route.h"

struct Node
{
  short x, y, z;
  unsigned short cost;
  Node *next[4] = {nullptr, nullptr, nullptr, nullptr};
  Node *prev;
  Move move;
  unsigned char visit = 0;
  Node(short x, short y, short z) : x(x), y(y), z(z) {}
};

struct CompareNode
{
  bool operator()(const Node &n1, const Node &n2) { return n1.cost > n2.cost; }
};

class OpenSet
{
  size_t size;
  size_t maxSize;
  Node **heap;

  void heapifyUp(size_t index)
  {
    while (index > 0)
    {
      size_t parentIndex = (index - 1) / 2;
      if (heap[index]->cost >= heap[parentIndex]->cost)
        break;
      std::swap(heap[index], heap[parentIndex]);
      index = parentIndex;
    }
  }
  void heapifyDown(size_t index)
  {
    size_t smallest = index;
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;
    if (left < size && heap[left]->cost < heap[smallest]->cost)
      smallest = left;
    if (right < size && heap[right]->cost < heap[smallest]->cost)
      smallest = right;
    if (smallest != index)
    {
      std::swap(heap[index], heap[smallest]);
      heapifyDown(smallest);
    }
  }

public:
  OpenSet(size_t maxSize) : size(0), maxSize(maxSize), heap(new Node *[maxSize]) {}
  ~OpenSet() { delete[] heap; }

  void push(Node *node)
  {
    if (size < maxSize)
    {
      heap[size] = node;
      heapifyUp(size);
      size++;
    }
    else if (node->cost < heap[0]->cost)
    {
      heap[0] = node;
      heapifyDown(0);
    }
  }
  Node *pop()
  {
    if (size > 0)
    {
      Node *popped_node = heap[0];
      heap[0] = heap[size - 1];
      size--;
      heapifyDown(0);
      return popped_node;
    }
    return nullptr;
  }
  bool empty() const { return size == 0; }
  void clear() { size = 0; }
};

class VoxMap
{
  // Member Variables
  unsigned short xLim, yLim, zLim;
  size_t map_area, map_volume;
  Node **graph;
  OpenSet *frontiers;
  Node *srcNode, *dstNode, *currNode, *adjNode;
  static constexpr Move cardinal_directions[4] = {Move::EAST, Move::SOUTH, Move::WEST, Move::NORTH};
  unsigned char currVisit = 1;

  // Helper Functions
  inline char hexToDec(char hex) const { return (hex <= '9') ? hex - '0' : hex - 'W'; }
  Node *getNode(short x, short y, short z) const { return graph[x + xLim * (y + yLim * z)]; };
  inline bool inBounds(short x, short y, short z) { return x < xLim && y < yLim && z < zLim; }

public:
  VoxMap(std::istream &stream);
  ~VoxMap();

  Route route(Point src, Point dst);

  void printMap(Point src, Point dst) const;
};

#endif
