#include <vector>
//#include
#ifndef H_Node
#define H_Node
using namespace std;
class Node
{
protected:
    Coord loci;
    Areaset areaset[3];
    Connection connections[3];
    bool open[4];
public:
    Node();
    void walk(vector<area> &areas); //Append new cirucits/areas to this vector


};
#endif
