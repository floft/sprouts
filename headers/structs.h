/*
 * This is the definitions file for miscellaneous structs including:
 *   Coord
 *   Connection
 *   Line
 */
#ifndef H_structs
#define H_structs

#include <vector>
#include <iostream>

using namespace std;

class Node;

struct Coord
{
    int x;
    int y;

    Coord() :x(0), y(0) { }
    Coord(int x, int y) :x(x), y(y) { }

    friend ostream& operator<<(ostream&,const Coord&);
    bool operator==(const Coord& c) const { return x==c.x && y==c.y; }
};

typedef vector<Coord> Line;

struct Connection
{
    Line* line;
    Node* dest;
    bool exists() const { return (line&&dest); }

    Connection() :line(NULL), dest(NULL) { }
    Connection(Line* l, Node* d) :line(l), dest(d) { }
    Connection(const Connection& c) :line(c.line), dest(c.dest) { }
    
    friend ostream& operator<<(ostream&,const Connection&);
};

ostream& operator<<(ostream&,const Coord&);
ostream& operator<<(ostream&,const Line&);

#endif
