/*
*   TONEVERDO list (wish list)
*   added a function to check validity of current game data state
*/

#ifndef H_Game
#define H_Game

#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include "node.h"
#include "structs.h"

using namespace std;

// Used for std::find since our vectors contain pointers
template<class T> class PointerFind
{
    public:
        const T& item;

        PointerFind(const T& item) :item(item) { }

        bool operator()(const T* search) const
        {
            return item == *search;
        }
};

// We'll throw this when trying to use connectable() with outdated areasets
class AreasOutdated { };

// Thrown in doMove() if the center point isn't on the line
class InvalidMiddle
{
    const int count;
    const Coord middle;

    public:
        InvalidMiddle(int count, Coord middle) :count(count), middle(middle) { }
        friend ostream& operator<<(ostream& os, const InvalidMiddle&);
};

// Thrown in doMove() if the line doesn't end with two nodes (or runs more than
// two)
class InvalidNode { };

// Thrown when two nodes aren't connectable when put into doMove()
class NotConnectable { };

class Game
{
    // Vectors of addresses since addresses of an element in a vector will
    // change as it grows
    private:
        bool updated;
        int moveCount;
        vector<Area*> areas;
    protected:
        vector<Areaset*> areasets;  // now protected instead of private for use in currentAreas() function
        vector<Node*> nodes;
        vector<Line*> lines;
    public:
        Game();

        // Copy constructor and assignment
        Game(const Game&);
        Game& operator=(const Game&);

        // This is the function you'll use a LOT. Set extraChecks to true if
        // you want the A-Checker to verify two nodes should be connectable.
        // This is useful when line-crossing code doesn't work, but it's twice
        // as slow, which is noticeable near the end of a 10+ node game.
        void doMove(const Line&, Coord middle, bool extraChecks = false);

        void updateAreas(); //will call node.walk in its process
        int moves() const; // Returns how many times doMove has been called
        bool connectable(const Node&,const Node&) const;
        bool isInArea(const Area&,Coord) const;
        bool gameEnded() const; // Can any nodes be connected still?
        // Needed for initializing nodes on the screen
        Node& insertNode(Coord, Connection = Connection(), Connection = Connection());
        Node* findNode(Coord) const; // Find node exactly at Coord, NULL if not found

        // Make this private? Only thing that uses it is the test suite?
        Line& insertLine(const Line&);

        // Used for debugging
        friend ostream& operator<<(ostream&, const Game&);
    private:
        void cleanup();
        void copy(const Game&);
        void clearAreas(); // empty areas/areasets and delete items pointed to
        void deleteLastNode(); // Undo last add, used in doMove
    public:
        virtual ~Game();
};

ostream& operator<<(ostream&, const Game&);

#endif
