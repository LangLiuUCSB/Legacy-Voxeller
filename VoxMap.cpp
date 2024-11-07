#include "VoxMap.h"
#include "Errors.h"

#define out std::cout <<
#define nl << std::endl

VoxMap::VoxMap(std::istream &stream)
{
  stream >> xLim >> yLim >> zLim;
  map_area = xLim * yLim;
  char schem[map_area]; // index = x + xLim * y

  map_volume = map_area * zLim;
  graph = new Node *[map_volume]; // index = x + xLim * (y + yLim * z)
  std::memset(graph, 0, map_volume * sizeof(Node *));

  std::string xLine;
  unsigned short x_quad;
  size_t area_north = 0;
  //! z=0 parse
  for (unsigned short y = 0; y < yLim; y++)
  {
    stream >> xLine;
    x_quad = 0;
    for (char ch : xLine)
    {
      char hex = hexToDec(ch);
      char bit_place = 0b1000;
      for (size_t i = area_north; i < area_north + 4; i++)
      {
        if ((hex & bit_place)) //? when curr is full
          schem[i] = 0b100;    //* track full in schem[]
        else                   //? when curr is non void
          schem[i] = 0b111;
        bit_place >>= 1;
      }
      x_quad += 4;
      area_north += 4;
    }
  }
  //! default parse
  size_t graphIndex;
  size_t volume_below = map_area;
  for (unsigned short z = 1; z < zLim; z++)
  {
    area_north = 0;
    for (unsigned short y = 0; y < yLim; y++)
    {
      stream >> xLine;
      x_quad = 0;
      for (char ch : xLine)
      {
        char hex = hexToDec(ch);
        char bit_place = 0b1000;
        for (size_t i = area_north; i < area_north + 4; i++)
        {
          if ((hex & bit_place))     //? when curr is full
            schem[i] = 0b100;        //* track full in schem[]
          else if (schem[i] ^ 0b111) //? when curr is empty AND prev is non void
          {
            schem[i] >>= 1;    //* update non void in schem[]
            if (schem[i] >> 1) //? when prev is full (valid for new Node)
            {
              Node *newNode = new Node(x_quad + i - area_north, y, z); //* create new Node
              //! WEST
              if (x_quad + i - area_north && !(schem[i - 1] & 0b100)) //? when west exists and is Node
              {
                graphIndex = i - 1 + volume_below;
                while (!graph[graphIndex])                       //? while west is nullptr
                  graphIndex -= map_area;                        //* fall
                newNode->next[Move::WEST] = graph[graphIndex];   //* new Node reaches west Node
                if (schem[i - 1])                                //? when west relation is symmetric
                  graph[graphIndex]->next[Move::EAST] = newNode; //* west Node reaches new Node
              }
              //! NORTH
              if (y && !(schem[i - xLim] & 0b100)) //? when north exists and is Node
              {
                graphIndex = i - xLim + volume_below;
                while (!graph[graphIndex])                        //? while north is nullptr
                  graphIndex -= map_area;                         //* fall
                newNode->next[Move::NORTH] = graph[graphIndex];   //* new Node reaches north Node
                if (schem[i - xLim])                              //? when north relation is symmetric
                  graph[graphIndex]->next[Move::SOUTH] = newNode; //* north Node reaches new Node
              }
              graph[i + volume_below] = newNode; //* add newNode in graph[]
            }
            else if (schem[i]) //? when prev is new Node
            {
              //! WEST
              if (x_quad + i - area_north && !(schem[i - 1] ^ 0b10)) //? when west exists and is new Node
              {
                graph[i - map_area + volume_below]->next[Move::WEST] = graph[i - 1 + volume_below]; //* old Node reaches west Node
                graph[i - 1 + volume_below]->next[Move::EAST] = graph[i - map_area + volume_below]; //* west Node reaches old Node
              }
              //! NORTH
              if (y && !(schem[i - xLim] ^ 0b10)) //? when north exists and is new Node
              {
                graph[i - map_area + volume_below]->next[Move::NORTH] = graph[i - xLim + volume_below]; //* old Node reaches north Node
                graph[i - xLim + volume_below]->next[Move::SOUTH] = graph[i - map_area + volume_below]; //* north Node reaches old Node
              }
            }
            else //? when prev is old Node
            {
              //! WEST
              if (x_quad + i - area_north && !(schem[i - 1] ^ 0b10)) //? when west exists and is new Node
              {
                graphIndex = i - map_area - map_area + volume_below;
                while (!graph[graphIndex])                                         //? while curr is nullptr
                  graphIndex -= map_area;                                          //* fall
                graph[i - 1 + volume_below]->next[Move::EAST] = graph[graphIndex]; //* west Node reaches old Node
              }
              //! NORTH
              if (y && !(schem[i - xLim] ^ 0b10)) //? when north exists and is new Node
              {
                graphIndex = i - 2 * map_area + volume_below;
                while (!graph[graphIndex])                                             //? while curr is nullptr
                  graphIndex -= map_area;                                              //* fall
                graph[i - xLim + volume_below]->next[Move::SOUTH] = graph[graphIndex]; //* north Node reaches old Node
              }
            }
          }
          bit_place >>= 1;
        }
        x_quad += 4;
        area_north += 4;
      }
    }
    volume_below += map_area;
  }
  frontiers = new OpenSet(map_volume >> 3);
}

VoxMap::~VoxMap()
{
  for (size_t i = 0; i < map_volume; i++)
    if (graph[i])
      delete graph[i];
  delete[] graph;
  delete frontiers;
}

Route VoxMap::route(Point src, Point dst)
{
  if (!inBounds(src.x, src.y, src.z) || !(srcNode = getNode(src.x, src.y, src.z)))
    throw InvalidPoint(src);

  const short xd = dst.x;
  const short yd = dst.y;
  const short zd = dst.z;
  if (!inBounds(xd, yd, zd) || !(dstNode = getNode(xd, yd, zd)))
    throw InvalidPoint(dst);

  Route path;
  if (srcNode == dstNode)
    return path;

  srcNode->visit = currVisit;
  dstNode->cost = 0;

  auto setCost = [xd, yd, zd](Node *n)
  { n->cost = abs(xd - n->x) + abs(yd - n->y) + abs(zd - n->z); }; // Manhattan Distance from Destination

  setCost(srcNode);

  //! first iteration
  for (Move direction : cardinal_directions)
  {
    adjNode = srcNode->next[direction];
    if (adjNode)
    {
      adjNode->visit = currVisit;
      setCost(adjNode);
      frontiers->push(adjNode);
      adjNode->prev = srcNode;
      adjNode->move = direction;
    }
  }
  //! first iteration
  while (!frontiers->empty())
  {
    currNode = frontiers->pop();
    if (currNode == dstNode)
    {
      frontiers->clear();
      ++currVisit;
      while (currNode != srcNode)
      {
        path.push_back(currNode->move);
        currNode = currNode->prev;
      }
      std::reverse(path.begin(), path.end());
      return path;
    }
    for (Move direction : cardinal_directions)
    {
      adjNode = currNode->next[direction];
      if (adjNode && adjNode->visit != currVisit)
      {
        adjNode->visit = currVisit;
        setCost(adjNode);
        frontiers->push(adjNode);
        adjNode->prev = currNode;
        adjNode->move = direction;
      }
    }
  }
  frontiers->clear();
  ++currVisit;
  throw NoRoute(src, dst);
}
