#include "maze.h"
#include <algorithm>

MazeGenerator::MazeGenerator(unsigned int w, unsigned int h)
{
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	gen_.seed(seed);

	init(w,h);
}

void MazeGenerator::buildNodeList(std::vector<SNodeListElement> &list)
{
    // Populate a vector with (x,y) pairs for every node in the maze

    unsigned int x,y;
    list.clear();
    list.reserve(node_width*node_height);

    for(x=0; x<node_width; ++x)
	{
        for(y=0; y<node_height; ++y)
		{
            list.push_back(SNodeListElement(x,y));
        }
    }

    shuffleNodeList(list);
    return;
}

void MazeGenerator::shuffleNodeList(std::vector<SNodeListElement> &list)
{
   std::shuffle (list.begin(), list.end(), gen_);

   //std::random_shuffle(list.begin(), list.end(), g_randTarget);
}

int MazeGenerator::rollRange(int low, int high)
{
	std::uniform_int_distribution<int> dist(low,high);
	return dist(gen_);
}



void MazeGenerator::init(unsigned int cw, unsigned int ch)
{
   if(cw==0 || ch==0) return;

    cell_width=cw; cell_height=ch;
    node_width=cw+1; node_height=ch+1;

    //nodes=new MazeGeneratorNode[node_width * node_height];
    //cells=new unsigned char[cell_width*cell_height];
	nodes.resize(node_width * node_height);
	cells.resize(cell_width * cell_height);
    clearAllEdges();
    clearAllCells();
}

// Node-based functions


// setEdgeNode-- Connect an edge starting at x,y and extending one edge length in the
// direction given by cm_edge.
void MazeGenerator::setEdgeNode(unsigned int x, unsigned int y, unsigned char cm_edge)
{
    if(x>=node_width || y>=node_height) return;

    nodes[y*node_width+x].setEdge(cm_edge);

    // Now, set other node
    unsigned char e;
    unsigned int nx, ny;
    if(cm_edge==E_North)
    {
        e=E_South;
        nx=x;
        ny=y-1;
    }
    else if(cm_edge==E_South)
    {
        e=E_North;
        nx=x;
        ny=y+1;
    }
    else if(cm_edge==E_East)
    {
        e=E_West;
        nx=x+1;
        ny=y;
    }
    else if(cm_edge==E_West)
    {
        e=E_East;
        nx=x-1;
        ny=y;
    }
    else return;

    if(nx>=node_width || ny>=node_height) return;

    nodes[ny*node_width+nx].setEdge(e);

}


// clearEdgeNode -- Remove an edge starting at x,y in direction cm_edge
void MazeGenerator::clearEdgeNode(unsigned int x, unsigned int y, unsigned char cm_edge)
{
    if(x>=node_width || y>=node_height) return;


    nodes[y*node_width+x].clearEdge(cm_edge);

    // Now, clear other node
    unsigned char e;
    unsigned int nx, ny;
    if(cm_edge==E_North)
    {
        e=E_South;
        nx=x;
        ny=y-1;
    }
    else if(cm_edge==E_South)
    {
        e=E_North;
        nx=x;
        ny=y+1;
    }
    else if(cm_edge==E_East)
    {
        e=E_West;
        nx=x+1;
        ny=y;
    }
    else if(cm_edge==E_West)
    {
        e=E_East;
        nx=x-1;
        ny=y;
    }
    else return;

    if(nx>=node_width || ny>=node_height) return;

    nodes[ny*node_width+nx].clearEdge(e);
}



// isBlockedNode -- Query a node to see if any edges connect to it;
// this constitutes a 'blocking' condition for the wall-growth algorithm
bool MazeGenerator::isBlockedNode(unsigned int x, unsigned int y)
{
    if(x>=node_width || y>=node_height) return false;

    return (nodes[y*node_width+x].isBlocked());
}


// getEdgeNode -- Determine if a node is connected to an edge extending in the given direction
bool MazeGenerator::getEdgeNode(unsigned int x, unsigned int y, unsigned char cm_edge)
{
    if(x>=node_width || y>=node_height) return false;

    return (nodes[y*node_width+x].getEdge(cm_edge));
}

// Cell-based functions

// setEdgeCell -- Set the edge of the given cell in the given direction
void MazeGenerator::setEdgeCell(unsigned int x, unsigned int y, unsigned char cm_edge)
{
    if(x>=cell_width || y>=cell_height) return;

    unsigned int nx, ny;
    unsigned char e;

    // Calculate one endpoint of edge
    if(cm_edge==E_North)
    {
        nx=x;
        ny=y;
        e=E_East;
    }
    else if(cm_edge==E_South)
    {
        nx=x;
        ny=y+1;
        e=E_East;
    }
    else if(cm_edge==E_East)
    {
        nx=x+1;
        ny=y;
        e=E_South;
    }
    else if(cm_edge==E_West)
    {
        nx=x;
        ny=y;
        e=E_South;
    }
    else return;

    setEdgeNode(nx, ny, e);
}


// clearEdgeCell -- Remove the given edge of the given cell
void MazeGenerator::clearEdgeCell(unsigned int x, unsigned int y, unsigned char cm_edge)
{
    if(x>=cell_width || y>=cell_height) return;

    unsigned int nx, ny;
    unsigned char e;

    // Calculate one endpoint of edge
    if(cm_edge==E_North)
    {
        nx=x;
        ny=y;
        e=E_East;
    }
    else if(cm_edge==E_South)
    {
        nx=x;
        ny=y+1;
        e=E_East;
    }
    else if(cm_edge==E_East)
    {
        nx=x+1;
        ny=y;
        e=E_South;
    }
    else if(cm_edge==E_West)
    {
        nx=x;
        ny=y;
        e=E_South;
    }
    else return;

    clearEdgeNode(nx, ny, e);
}


// getEdgeCell -- Query the given edge of the given cell
bool MazeGenerator::getEdgeCell(unsigned int x, unsigned int y, unsigned char cm_edge)
{
    if(x>=cell_width || y>=cell_height) return false;

    unsigned int nx, ny;
    unsigned char e;

    // Calculate one endpoint of edge
    if(cm_edge==E_North)
    {
        nx=x;
        ny=y;
        e=E_East;
    }
    else if(cm_edge==E_South)
    {
        nx=x;
        ny=y+1;
        e=E_East;
    }
    else if(cm_edge==E_East)
    {
        nx=x+1;
        ny=y;
        e=E_South;
    }
    else if(cm_edge==E_West)
    {
        nx=x;
        ny=y;
        e=E_South;
    }
    else return false;

    return getEdgeNode(nx, ny, e);
}


// getEdgePattern -- Compile an unsigned char value specifying the edge pattern of
// the given cell. Classifies a cell based on it's open and closed edges into one of
// sixteen categories.
unsigned char MazeGenerator::getEdgePattern(unsigned int x, unsigned int y)
{
    unsigned char pattern=0;

    if(getEdgeCell(x,y,E_North)) pattern |= E_North;
    if(getEdgeCell(x,y,E_South)) pattern |= E_South;
    if(getEdgeCell(x,y,E_East)) pattern |= E_East;
    if(getEdgeCell(x,y,E_West)) pattern |= E_West;

    return pattern;
}


// General functions

// setAllEdges -- Connect all edges between nodes in the entire maze
void MazeGenerator::setAllEdges()
{


    unsigned int c;
    for(c=0; c<=node_width*node_height; ++c)
	{
        nodes[c].set();
    }
}

// clearAllEdges -- Clear all edges between nodes in the entire maze
void MazeGenerator::clearAllEdges()
{


    unsigned int c;
    for(c=0; c<=node_width*node_height; ++c)
	{
        nodes[c].clear();
    }
}


// setBorderEdges -- Set the edges on the outside border of the maze
void MazeGenerator::setBorderEdges()
{


    int c;

    // Set top and bottom edge
    for(c=0; c<cell_height; ++c)
	{
        setEdgeCell(c,0,E_North);
        setEdgeCell(c,cell_height-1,E_South);
    }

    // Set left and right edge
    for(c=0; c<cell_width; ++c)
	{
        setEdgeCell(0,c,E_West);
        setEdgeCell(cell_width-1,c,E_East);
    }
}


// setVisited -- Mark the given cell has 'visited'. Used by the depth-first generation algo
void MazeGenerator::setVisited(unsigned int x, unsigned int y)
{
    if(x>=cell_width || y>=cell_height) return;

    cells[y*cell_width+x] |= E_Visited;
}

// getVisited -- Check to see if the given cell has been visited already
bool MazeGenerator::getVisited(unsigned int x, unsigned int y)
{
    if(x>=cell_width || y>=cell_height ) return false;

    return (cells[y*cell_width+x] & E_Visited);
}

// clearVisited -- Clear the visited status of a cell
void MazeGenerator::clearVisited(unsigned int x, unsigned int y)
{
    if(x>=cell_width || y>=cell_height ) return;

    cells[y*cell_width+x] &= ~E_Visited;
}

void MazeGenerator::setBackDirection(unsigned int x, unsigned int y, unsigned char dir)
{
    if(x>=cell_width || y>=cell_height ) return;

    cells[y*cell_width+x] |= dir;
}

unsigned char MazeGenerator::getBackDirection(unsigned int x, unsigned int y)
{
    if(x>=cell_width || y>=cell_height ) return 0;

    return cells[y*cell_width+x] & 0x0F;
}

void MazeGenerator::clearBackDirection(unsigned int x, unsigned int y)
{
    if(x>=cell_width || y>=cell_height ) return;

    cells[y*cell_width+x] &= ~0x0F;
}


// clearAllCells -- Clear the visited and other status flags of all cells in the maze
void MazeGenerator::clearAllCells()
{
	for(unsigned int c=0; c<cell_width*cell_height; ++c)
	{
        cells[c]=0;
    }
}


// buildMazeWall -- The workhorse of the wall-growth algorithm. Given a starting location, a
// direction, and a length, attempt to build a wall. Blocked nodes terminate wall building
void MazeGenerator::buildMazeWall(unsigned int x, unsigned int y, unsigned char dir, unsigned int len)
{
    unsigned int wx=x, wy=y;
    unsigned int ox,oy;

    if(isBlockedNode(x,y)) return;
    unsigned int c;
    for(c=0; c<len; ++c)
	{
        ox=wx;
        oy=wy;
        if(dir==1) wy-=1;
        if(dir==2) wx-=1;
        if(dir==4) wx+=1;
        if(dir==8) wy+=1;

        if(isBlockedNode(wx,wy))
        {
            setEdgeNode(ox,oy,dir);
            return;
        }
        else
        {
            setEdgeNode(ox,oy,dir);
        }
    }
}


// randomDir -- Select one of the for random directions
unsigned char MazeGenerator::randomDir()
{
    unsigned int roll=rollRange(1,4);

    switch(roll)
    {
        case 1: return E_North;
        case 2: return E_West;
        case 3: return E_East;
        case 4: return E_South;
		default: return E_North;
    };
}


// generateWallGrowthMaze -- Construct a maze using the wall-growth method.
// The method works by accumulating all node coordinates into a list, shuffling the list, then iterating
// through it. At each location, attempt to draw a wall of randomly determined length and direction.
// Blocked nodes prevent wall building. This algorithm results in a maze that has lots of loops and
// multiple paths between any two given points. The passed parameters min_wall and max_wall
// determine the range of lengths of walls to be drawn. Walls may in reality turn out shorter
// due to hitting a wall. Longer wall lengths result in longer straight passages.
void MazeGenerator::generateWallGrowthMaze(unsigned int min_wall, unsigned int max_wall)
{


    std::vector<SNodeListElement> nlist;
    buildNodeList(nlist);

    unsigned int size=nlist.size();
    if(size==0) return;

    unsigned int c;
    for(c=0; c<size; ++c)
	{
        unsigned char d=randomDir();
        unsigned int len=rollRange(min_wall, max_wall);
        unsigned int x=nlist[c].x;
        unsigned int y=nlist[c].y;
        buildMazeWall(x,y,d,len);
    }
}

// recurseDepthFirst -- The workhorse of the depth-first tree-based generation algorithm
// The function marks the current cell as visited, then shuffles the cell's 4 adjacent neighbors into
// random order and calls itself for each cell in the list that has not already been visited. When
// a cell's neighbor is visited, the edge between the two cells is knocked down, creating a passage.

void MazeGenerator::recurseDepthFirst(unsigned int x, unsigned int y)
{
    // First, mark cell as visited
    setVisited(x,y);

    // Now, check each neighbor. If it hasn't been visited, knock down the
    // wall and recurse on that cell
    unsigned int cx,cy;

    unsigned char dirs[4]={E_North, E_South, E_East, E_West};
    std::shuffle(dirs, dirs+4, gen_);

    for(int c=0; c<4; ++c)
    {
        unsigned char d=dirs[c];
        switch(d)
        {
            case E_North: cx=x; cy=y-1; break;
            case E_South: cx=x; cy=y+1; break;
            case E_East: cx=x+1; cy=y; break;
            case E_West: cx=x-1; cy=y; break;
        }

        if(cx>=0 && cy>=0 && cx<cell_width && cy<cell_height)
		{
            if(!getVisited(cx,cy))
            {
                clearEdgeCell(x,y,d);
                recurseDepthFirst(cx,cy);
            }
        }
    }
}

// generateDepthFirstMaze -- Generates a maze using a randomized depth-first search of the maze
// starting at a random location.
// This algorithm generates a 'perfect' maze, in which there is exactly 1 path between any two randomly
// selected cells.
void MazeGenerator::generateDepthFirstMaze(unsigned int sx, unsigned int sy)
{
    // First, select a random starting cell
	setAllEdges();
    recurseDepthFirst(sx, sy);
}
