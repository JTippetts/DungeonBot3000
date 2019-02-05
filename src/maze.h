#pragma once
#include <chrono>
#include <random>
#include <vector>

struct MazeGeneratorNode
{
    unsigned char edges;

	MazeGeneratorNode() : edges(0){}

    void setEdge(unsigned char e){edges |= e;};
    void clearEdge(unsigned char e){edges &= ~e;};
    void clear(){edges=0;};
    void set(){edges=15;};
    bool getEdge(unsigned char e){return ((edges & e) != 0);};
    bool isBlocked(){return (edges != 0);};
};

struct SNodeListElement
{
    unsigned int x, y;

    SNodeListElement(unsigned int X, unsigned int Y) : x(X), y(Y) {};
};

enum EEdgeMasks
{
    E_North=0x01,
    E_West=0x02,
    E_East=0x04,
    E_South=0x08,
    E_Visited=0x10
};

class MazeGenerator
{
    public:
        MazeGenerator(unsigned int w, unsigned int h);
        ~MazeGenerator(){};

        void init(unsigned int cw, unsigned int ch);

        unsigned int getCellWidth(){return cell_width;};
        unsigned int getCellHeight(){return cell_height;};
        unsigned int getNodeWidth(){return node_width;};
        unsigned int getNodeHeight(){return node_height;};

        // Node-based functions
        void setEdgeNode(unsigned int x, unsigned int y, unsigned char cm_edge);
        void clearEdgeNode(unsigned int x, unsigned int y, unsigned char cm_edge);
        bool isBlockedNode(unsigned int x, unsigned int y);
        bool getEdgeNode(unsigned int x, unsigned int y, unsigned char cm_edge);

        // Cell-based functions
        void setEdgeCell(unsigned int x, unsigned int y, unsigned char cm_edge);
        void clearEdgeCell(unsigned int x, unsigned int y, unsigned char cm_edge);
        bool getEdgeCell(unsigned x, unsigned y, unsigned char cm_edge);
        unsigned char getEdgePattern(unsigned int x, unsigned int y);

        // Spanning-tree based generation base functionality
        // Used by algorithms that generate based on some sort of
        // recursive or tree-based randomized search algorithm
        void setVisited(unsigned int x, unsigned int y);
        bool getVisited(unsigned int x, unsigned int y);
        void clearVisited(unsigned int x, unsigned int y);
        void setBackDirection(unsigned int x, unsigned int y, unsigned char dir);
        unsigned char getBackDirection(unsigned int x, unsigned int y);
        void clearBackDirection(unsigned int x, unsigned int y);


        // General functions
        void setAllEdges();
        void clearAllEdges();
        void setBorderEdges();

        void clearAllCells();  // Clear back and visited for all cells

        void clear(){clearAllEdges(); clearAllCells(); setBorderEdges();};


        //  Generation functions
        void generateWallGrowthMaze(unsigned int min_wall, unsigned int max_wall);
        void generateDepthFirstMaze(unsigned int sx, unsigned int sy);

    private:
        //MazeGeneratorNode *nodes;
		std::vector<MazeGeneratorNode> nodes;
        //unsigned char *cells;
		std::vector<unsigned char> cells;
        unsigned int cell_width, cell_height; // dimensions in cells
        unsigned int node_width, node_height; // dims in nodes

        void buildNodeList(std::vector<SNodeListElement> &list);
        void shuffleNodeList(std::vector<SNodeListElement> &list);
        unsigned char randomDir();
        void buildMazeWall(unsigned int x, unsigned int y, unsigned char cm_dir, unsigned int len);
        void recurseDepthFirst(unsigned int x, unsigned int y);
		int rollRange(int low, int high);

		std::mt19937 gen_;
};

