#ifndef ROUTE_H
#define ROUTE_H

#include <iostream>
#include <vector>

enum Move : unsigned char
{
  EAST = 0,
  SOUTH = 1,
  WEST = 2,
  NORTH = 3
};

using Route = std::vector<Move>;

std::ostream &operator<<(std::ostream &stream, Move move);
std::ostream &operator<<(std::ostream &stream, const Route &route);

#endif
