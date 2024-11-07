#include "VoxMap.h"
#include "Errors.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <chrono>

static const std::string world[] = {
    "flatland",      // 0
    "labrat",        // 1
    "pyramid",       // 2
    "stairs",        // 3
    "stairs2",       // 4
    "bouncy-castle", // 5
    "goo-ball",      // 6
    "tower-block",   // 7
    "no-route"};     // 8

static const Point source[] = {
    Point(11, 0, 1),
    Point(37, 27, 1),
    Point(7, 7, 4),
    Point(7, 5, 6),
    Point(11, 6, 1),
    Point(6, 6, 2),
    Point(37, 44, 67),
    Point(9, 49, 22), //(71, 4, 22),
    Point(9, 3, 1)};

static const Point destination[] = {
    Point(0, 11, 1),
    Point(21, 35, 1),
    Point(1, 1, 1),
    Point(0, 0, 1),
    Point(0, 0, 6),
    Point(0, 5, 1),
    Point(36, 54, 19),
    Point(7, 6, 36), //(32, 23, 51),
    Point(7, 10, 3)};

int main()
{
    std::chrono::steady_clock::time_point start, end;
    std::chrono::duration<double> elapsed_seconds;

    start = std::chrono::steady_clock::now();

    end = std::chrono::steady_clock::now();
    elapsed_seconds = end - start;
    std::cout << "nothing time: " << std::setw(8) << std::right << elapsed_seconds.count() * 1000000 << " μs\n";

    for (unsigned char i = 0; i < 9; i++)
    {
        std::ifstream stream("data/" + world[i] + ".vox");
        std::cout << "\n"
                  << std::setw(15) << std::left << world[i];
        if (stream.fail())
            return 1;

        start = std::chrono::steady_clock::now(); //!
        VoxMap map(stream);
        end = std::chrono::steady_clock::now(); //!
        elapsed_seconds = end - start;
        std::cout << "parse time: " << std::setw(8) << std::right << short(elapsed_seconds.count() * 1000000) << " μs  "
                  << source[i] << "->" << destination[i] << "\n";

        try
        {
            start = std::chrono::steady_clock::now();
            Route route = map.route(source[i], destination[i]);
            end = std::chrono::steady_clock::now();
            elapsed_seconds = end - start;
            std::cout << "            pathfind time: " << std::setw(8) << std::right << short(elapsed_seconds.count() * 1000000) << " μs\n";
            std::cout << route << '\n';
        }
        catch (const InvalidPoint &err)
        {
            std::cout << "Invalid point: " << err.point() << '\n';
        }
        catch (const NoRoute &err)
        {
            end = std::chrono::steady_clock::now();
            elapsed_seconds = end - start;
            std::cout << "            pathfind time: " << std::setw(8) << std::right << short(elapsed_seconds.count() * 1000000) << " μs\n";
            std::cout << "No route\n";
        }

        stream.close();
    }

    std::ifstream stream("scripts/skywalk.vox");
    std::cout << "\n"
              << std::setw(15) << std::left << "skywalk";
    if (stream.fail())
        return 1;

    start = std::chrono::steady_clock::now(); //!
    VoxMap map(stream);
    end = std::chrono::steady_clock::now(); //!
    elapsed_seconds = end - start;
    std::cout << "parse time: " << std::setw(8) << std::right << short(elapsed_seconds.count() * 1000000) << " μs  "
              << Point(126, 94, 1) << "->" << Point(1, 1, 93) << "\n";

    try
    {
        start = std::chrono::steady_clock::now();
        Route route = map.route(Point(126, 94, 1), Point(1, 1, 93));
        end = std::chrono::steady_clock::now();
        elapsed_seconds = end - start;
        std::cout << "            pathfind time: " << std::setw(8) << std::right << short(elapsed_seconds.count() * 1000000) << " μs\n";
        std::cout << route << '\n';
    }
    catch (const InvalidPoint &err)
    {
        std::cout << "Invalid point: " << err.point() << '\n';
    }
    catch (const NoRoute &err)
    {
        end = std::chrono::steady_clock::now();
        elapsed_seconds = end - start;
        std::cout << "            pathfind time: " << std::setw(8) << std::right << short(elapsed_seconds.count() * 1000000) << " μs\n";
        std::cout << "No route\n";
    }

    stream.close();

    return 0;
}
