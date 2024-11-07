#include "Route.h"

std::ostream &operator<<(std::ostream &stream, Move move)
{
  char c;

  switch (move)
  {
  case Move::EAST:
    c = 'e';
    break;
  case Move::SOUTH:
    c = 's';
    break;
  case Move::WEST:
    c = 'w';
    break;
  default:
    c = 'n';
  }

  return stream << c;
}

std::ostream &operator<<(std::ostream &stream, const Route &route)
{
  for (Move move : route)
  {
    stream << move;
  }

  return stream;
}
