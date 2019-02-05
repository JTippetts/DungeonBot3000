#pragma once
#include <vector>
#include <chrono>
#include <random>

class Maze2
{
public:
    enum Directions
    {
        DirNorth=1,
        DirWest=2,
        DirEast=4,
        DirSouth=8
    };
    Maze2();

    void Init(int cellwidth, int cellheight);
    void ClearAllWalls();
    void SetAllWalls();

    bool GetCellWall(int x, int y, Directions dir);
    void SetCellWall(int x, int y, Directions dir);
    void ClearCellWall(int x, int y, Directions dir);

    unsigned char GetCellPattern(int x, int y);
    unsigned char GetNodePattern(int x, int y);

    void DepthFirstMaze(int sx, int sy);

	int GetCellWidth(){return nodearraywidth_-1;}
	int GetCellHeight(){return nodearrayheight_-1;}

protected:
    std::vector<unsigned char> nodes_;
    int nodearraywidth_, nodearrayheight_;

    void RecurseDepthFirst(int x, int y, std::vector<unsigned char> &visited);

    std::mt19937 gen_;
};
