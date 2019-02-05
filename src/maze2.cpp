#include "maze2.h"
#include <algorithm>

Maze2::Maze2() : nodearraywidth_(0), nodearrayheight_(0)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	gen_.seed(seed);
}

void Maze2::Init(int cellwidth, int cellheight)
{
    nodearraywidth_=cellwidth+1;
    nodearrayheight_=cellheight+1;
    nodes_.resize(nodearraywidth_ * nodearrayheight_);
}

void Maze2::ClearAllWalls()
{
    for(int c=0; c<(int)nodes_.size(); ++c) nodes_[c]=0;
}

void Maze2::SetAllWalls()
{
    for(int c=0; c<(int)nodes_.size(); ++c) nodes_[c]=12;
}

bool Maze2::GetCellWall(int x, int y, Directions dir)
{
    switch(dir)
    {
    case DirNorth: // Read East edge of node (x,y)
        {
            int idx=y*nodearraywidth_+x;
            if(idx>=(int)nodes_.size()) return true;
            return ((nodes_[idx] & DirEast) != 0);
        } break;
    case DirWest: // Read South edge of node (x,y)
        {
            int idx=y*nodearraywidth_+x;
            if(idx>=(int)nodes_.size()) return true;
            return ((nodes_[idx] & DirSouth) != 0);
        } break;
    case DirEast: // Read South edge of node (x+1, y)
        {
            int idx=y*nodearraywidth_+x+1;
            if(idx>=(int)nodes_.size()) return true;
            return ((nodes_[idx] & DirSouth) != 0);
        } break;
    case DirSouth: // Read East edge of node (x, y+1)
        {
            int idx=(y+1)*nodearraywidth_+x;
            if(idx>=(int)nodes_.size()) return true;
            return ((nodes_[idx] & DirEast) != 0);
        } break;
    };
    return true;
}

void Maze2::SetCellWall(int x, int y, Directions dir)
{
    if(x<0 || y<0 || x>=nodearraywidth_ || y>=nodearrayheight_ || nodes_.size()==0) return;
    switch(dir)
    {
    case DirNorth: // Set East edge of node (x,y),
       {
            int idx=y*nodearraywidth_+x;
            if(idx<(int)nodes_.size()) nodes_[idx]=nodes_[idx] | DirEast;
       } break;
    case DirWest: // Set South edge of node (x,y)
       {
            int idx=y*nodearraywidth_+x;
            if(idx<(int)nodes_.size()) nodes_[idx]=nodes_[idx] | DirSouth;
       } break;
    case DirEast: // Set South edge of node (x+1,y)
       {
            int idx=y*nodearraywidth_+x+1;
            if(idx<(int)nodes_.size()) nodes_[idx]=nodes_[idx] | DirSouth;
       } break;
    case DirSouth: // Set East edge of node (x,y+1)
       {
            int idx=(y+1)*nodearraywidth_+x;
            if(idx<(int)nodes_.size()) nodes_[idx]=nodes_[idx] | DirEast;
       } break;
    };
}

void Maze2::ClearCellWall(int x, int y, Directions dir)
{
    if(x<0 || y<0 || x>=nodearraywidth_ || y>=nodearrayheight_ || nodes_.size()==0) return;
    switch(dir)
    {
    case DirNorth: // Clear East edge of node (x,y),
       {
            int idx=y*nodearraywidth_+x;
            if(idx<(int)nodes_.size()) nodes_[idx]=nodes_[idx] & ~DirEast;
       } break;
    case DirWest: // Clear South edge of node (x,y)
       {
            int idx=y*nodearraywidth_+x;
            if(idx<(int)nodes_.size()) nodes_[idx]=nodes_[idx] & ~DirSouth;
       } break;
    case DirEast: // Clear South edge of node (x+1,y)
       {
            int idx=y*nodearraywidth_+x+1;
            if(idx<(int)nodes_.size()) nodes_[idx]=nodes_[idx] & ~DirSouth;
       } break;
    case DirSouth: // Clear East edge of node (x,y+1)
       {
            int idx=(y+1)*nodearraywidth_+x;
            if(idx<(int)nodes_.size()) nodes_[idx]=nodes_[idx] & ~DirEast;
       } break;
    };
}

unsigned char Maze2::GetCellPattern(int x, int y)
{
    if(x<0 || y<0 || x>=nodearraywidth_ || y>=nodearrayheight_ || nodes_.size()==0) return 15;

    unsigned char pattern=0;
    if(GetCellWall(x,y,DirNorth)) pattern |= 1;
    if(GetCellWall(x,y,DirWest)) pattern |= 2;
    if(GetCellWall(x,y,DirEast)) pattern |= 4;
    if(GetCellWall(x,y,DirSouth)) pattern |= 8;

    return pattern;
}

unsigned char Maze2::GetNodePattern(int x, int y)
{
    if(x<0 || y<0 || x>=nodearraywidth_ || y>=nodearrayheight_ || nodes_.size()==0) return 15;

    int idx=y*nodearraywidth_+x;
    unsigned char pattern=0;
    if(nodes_[idx] & DirNorth) pattern |= 1;
    if(nodes_[idx] & DirWest) pattern |= 2;
    if(nodes_[idx] & DirEast) pattern |= 4;
    if(nodes_[idx] & DirSouth) pattern |= 8;

    return pattern;
}

void Maze2::RecurseDepthFirst(int x, int y, std::vector<unsigned char> &visited)
{
    // Set this cell as visited
    int idx = y*(nodearraywidth_-1)+x;
    if(idx<(int)visited.size()) visited[idx]=1;

    // Check each neighbor, in random order
    Directions dirs[4]={DirNorth, DirSouth, DirEast, DirWest};
    std::shuffle(dirs, dirs+4, gen_);

    for(int c=0; c<4; ++c)
    {
        Directions d=dirs[c];
        int cx=x, cy=y;
        switch(d)
        {
            case DirNorth: cx=x; cy=y-1; break;
            case DirSouth: cx=x; cy=y+1; break;
            case DirEast: cx=x+1; cy=y; break;
            case DirWest: cx=x-1; cy=y; break;
        };
        if(cx>=0 && cy>=0 && cx<nodearraywidth_-1 && cy<nodearrayheight_-1)
        {
            int idx=cy*(nodearraywidth_-1)+cx;
            if(visited[idx]==0)
            {
                ClearCellWall(x,y,d);
                RecurseDepthFirst(cx,cy,visited);
            }
        }
    }
}

void Maze2::DepthFirstMaze(int sx, int sy)
{
    if(nodearraywidth_ <=0 || nodearrayheight_<=0)
    {
        return;
    }
    if(sx<0 || sy<0 || sx>=nodearraywidth_-1 || sy>=nodearrayheight_-1) return;

    std::vector<unsigned char> visited;
    visited.resize((nodearraywidth_-1) * (nodearrayheight_-1));
    for(unsigned int c=0; c<visited.size(); ++c) visited[c]=0;

    SetAllWalls();
    RecurseDepthFirst(sx,sy,visited);
}
