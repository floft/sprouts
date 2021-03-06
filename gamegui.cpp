#include "headers/gamegui.h"

GameGUI::GameGUI(SDL_Surface* screen, TTF_Font* font)
    :GameAI(), screen(screen), font(font), playerMode(PvP), state(Blank),
    nodeRadius(5), lineThick(1), player1(true), error(false)
{
    textCol.r = 255;
    textCol.g = 255;
    textCol.b = 255;
}

void GameGUI::init(Mode mode, int count, int radius, int thick)
{
    playerMode = mode;
    nodeRadius = radius;
    lineThick = thick;
    player1 = true;
    error = false;
    if (nodes.size() == 0)
    {
        double theta = 0;

        // Create count nodes
        if (count > 0)
        {
            // Place first node in center
            insertNode(center());

            // Place the rest of the initial nodes along an ellipse at the center
            // of the screen
            for (int i = 1; i < count; i++)
            {
                insertNode(Coord(
                    screen->w/3 * cos(theta) + screen->w/2,
                    screen->h/3 * sin(theta) + screen->h/2));

                theta += 2*3.14/count;
            }
        }
    }
    updateAreas();
    redraw();
}

void GameGUI::redraw(bool lck)
{
    if (lck)
        lock();

    // Blank screen
    SDL_FillRect(screen, NULL, 0);

    // Draw nodes
    for (int i = 0; i < nodes.size(); i++)
    {
        Uint32 color = 0;
        int connections = nodes[i]->conCount();
        if (connections == 0)
            color = 0xFFFFFFFF;
        else if (connections == 1)
            color = 0x069DD6FF;
        else if (connections == 2)
            color = 0xFE0208FF;
        else if (connections == 3)
            color = 0x000000FF;
        circle(nodes[i]->getLoci(), nodeRadius, color);
    }

    // Draw lines
    for (int i = 0; i < lines.size(); i++)
        for (int j = 1; j < lines[i]->size(); j++)
            line((*lines[i])[j-1], (*lines[i])[j], lineCol);

    // Draw temporary line
    for (int i = 1; i < currentLine.size(); i++)
        line(currentLine[i-1], currentLine[i], (player1)?player1Col:player2Col);

    if (lck)
        unlock();
}

// Lock the screen so we can access it
void GameGUI::lock()
{
    SDL_LockSurface(screen);
}

// Unlock and show it (double buffering)
void GameGUI::unlock()
{
    SDL_UnlockSurface(screen);

    // This really should not be here since this function is to unlock the
    // screen and not display text. However, it was easy to put it here and it
    // works.
    if (error)
        displayError("Error: Please connect at 180 degrees.");

    SDL_Flip(screen);
}

void GameGUI::cancel()
{
    currentLine.clear();
    state = Blank;
    redraw();
}

State GameGUI::click(Coord location)
{
    Node* selected = selectedNode(location);
    bool validFinish; //Used to keep track of whether or not the line can be drawn.
    error = false; //Initialize as false every click

    // Don't do anything if it's dead
    if (selected && selected->dead())
        return state;

    // Used to determine if we can go straight to the second node if we clicked
    // one or if we have to draw another line to get there.
    Coord direct, selectedLoci;
    if (selected)
    {
        direct = straighten(location, selected->getLoci());
        selectedLoci = selected->getLoci(); //Location of node stored as a coord.
    }

    // Clicked on node to end, make sure this is at least the second line or
    // that the line is perfectly straight between nodes
    if (selected && state == NodeClicked && //If node is selected, a line has already been drawn, and if the node doesn't have 3 connections.
        // Either there's more than one line going from one node to another or the two nodes are perfectly in line horizontally or vertically
        (currentLine.size() > 1 || (currentLine.size() == 1 && (direct.x == currentLine.back().x || direct.y == currentLine.back().y))))
    {
        validFinish=false; //Reset validFinish
        if (validLine(currentLine.back(),
                      straighten(currentLine.back(), selected->getLoci()),true))//Does extending previous line cross any lines.
        {
           //Is the line coming vertically into node?
            if (vertical(currentLine.back(), selectedLoci))
            {
                //If Vertical, does the line intersect another line.
                if(validLine(Coord(selectedLoci.x,currentLine.back().y),
                             selectedLoci,true))
                {
                    if (currentLine.front() == selected->getLoci() && (selected->conCount()==0) && (!vertical(currentLine.front(), currentLine[1])))
                        error = true;
                        //If connecting back to the same node and it is coming at 90 degrees, don't allow

                    // !^ = both true or both false
                    else if (((selected->conCount()==1) && (selectedLoci == currentLine.front()))|| //If 1 existing connection and it is connecting to itself
                            (!(selected->openRight() ^ selected->openLeft()))) //Ensures that line is at 180
                    {
                        validFinish=true; //If not, line becomes a valid move.
                        if(currentLine.size() > 1 && vertical(currentLine[currentLine.size()-2],currentLine.back())) //If last line coming in is vertical as well
                            currentLine.pop_back(); //delete last point
                        currentLine.back().x= selectedLoci.x; //Change the x value to the one of the node so that it will correct and make a straight line

                        // Blocks correction of last coordinate to the end coordinate
                        if (currentLine.back() == selectedLoci)
                            currentLine.pop_back();

                        // Blocks correction of first coordinate from becoming the first node Coord
                        if(currentLine.back()==currentLine.front()&&currentLine.size()>1) //Don't know how to explain.
                            currentLine.pop_back();
                    }
                    else
                        error = true;
                }
            }
            else
            {
                //If Horizontal, does the line intersect another line?
                if(validLine(Coord(currentLine.back().x,selectedLoci.y),
                             selectedLoci,true))
                {
                    if (currentLine.front() == selected->getLoci() && (selected->conCount()==0) && (vertical(currentLine.front(), currentLine[1])))
                        error = true;
                        //If connecting back to the same node and it is coming at 90 degrees, don't allow

                    else if (((selected->conCount()==1) && (selectedLoci == currentLine.front()))|| //If 1 existing connection and it is connecting to itself
                            (!(selected->openUp() ^ selected->openDown()))) //Ensures that line is at 180
                    {
                        validFinish=true; //If not, line becomes a valid move.
                        if(currentLine.size() > 1 && !vertical(currentLine[currentLine.size()-2],currentLine.back())) //If last line coming in is horizontal as well, delete last point.
                           currentLine.pop_back(); //It isn't necessary and it will create diagonal lines.
                        currentLine.back().y = selectedLoci.y; //Change the y value to the one of the node so that it will correct and make a straight line

                        if (currentLine.back() == selectedLoci)
                            currentLine.pop_back();

                        if(currentLine.back()==currentLine.front()&&currentLine.size()>1)
                            currentLine.pop_back();
                    }
                    else
                        error = true;
                }
            }

            //Calculate location of node to be added. Verify that in our correction it didn't
            //go through any other lines.
            if(validFinish==true && validLine(currentLine.back(), selectedLoci, true))
            {
                currentLine.push_back(selectedLoci); //Push the final node onto the vector.
                Coord middle = findMiddle();

                //cout << "Middle: " << middle << " Line: " << currentLine << endl;
                doMove(currentLine, middle);
                player1 = !player1; //Change players

                currentLine.clear();
                state = Blank;

                if (gameEnded())
                    state = GameEnd;

                if (playerMode == PvAI)
                {
                    //cout << "AI playing." << endl;

                    // Computer plays.
                    if(aiTurn())
                        player1 = !player1;
                }
            }
        }
    }

    // Clicked on node to start, make sure this is the first node
    else if (selected && currentLine.size() == 0 && state != GameEnd)
    {
        currentLine.push_back(selectedLoci);
        state = NodeClicked;
    }

    // Clicked to place a line
    else if (state == NodeClicked)
    {
        if (currentLine.size()==1) //If first line, ensure 180.
        {
            selected = findNode(currentLine.front()); // Finds which node was used to start currentLine

            if (selected)
            {
                Coord straightened = firststraighten(currentLine.back(), location,
                    selected->openUp(),    selected->openDown(),
                    selected->openRight(), selected->openLeft());

                if (validLine(currentLine.back(), straightened,false))
                    currentLine.push_back(straightened);
            }
        }
        else
        {
            combineLines(location);
        }
    }

    redraw();

    return state;
}

void GameGUI::cursor(Coord location)
{
    Node* selected = NULL;

    if (state == NodeClicked)
    {
        lock();
        redraw(false);

        if (currentLine.size() == 1) //If it is the first line drawn out of node
        {
            // Which node was used to start currentLine
            selected = findNode(currentLine.front());

            if (selected)
                line(selected->getLoci(), firststraighten(selected->getLoci(), location,
                    selected->openUp(),    selected->openDown(),
                    selected->openRight(), selected->openLeft()), (player1)?player1Col:player2Col);
        }
        else
            line(currentLine.back(), straighten(currentLine.back(), location), (player1)?player1Col:player2Col);

        unlock();
    }


}

bool GameGUI::vertical(Coord last, Coord point) const
{
    //Returns true is point is between pi/4 & 3pi/4 || 5pi/4 && 7pi/4
    //in respect to the last last coord.
    if (((point.y<=(last.y+(last.x-point.x)))&&
         (point.y<=(last.y-(last.x-point.x))))||
        ((point.y>=(last.y+(last.x-point.x)))&&
         (point.y>=(last.y-(last.x-point.x)))))
         return true;
    else
        return false;
}

Coord GameGUI::firststraighten(Coord node, Coord location, bool up, bool down, bool right, bool left) const
{
    int buffer = 10;

    //If two lines are coming out of a node, the new line is restricted to a 90 degree move
    if (!right&&!left)
        return Coord(node.x,location.y);
    else if (!up&&!down)
        return Coord(location.x,node.y);

    //If a line already exists out of a node, new line is limited to the opposite dirrection
    else if (up&&!down)
    {
        if (location.y < node.y)
            return Coord(node.x,location.y-buffer);
        else
            return Coord(node.x,node.y-buffer);
    }
    else if(!up&&down)
    {
        if (location.y > node.y)
            return Coord(node.x,location.y+buffer);
        else
            return Coord(node.x,node.y+buffer);
    }
    else if (!right&&left)
    {
        if (location.x < node.x)
            return Coord(location.x-buffer,node.y);
        else
            return Coord(node.x-buffer,node.y);
    }
    else if (right&&!left)
    {
        if (location.x > node.x)
            return Coord(location.x+buffer,node.y);
        else
            return Coord(node.x+buffer,node.y);
    }

    //If there are no connections, line is not limited
    else
        return straighten(node, location);
}

void GameGUI::combineLines(Coord location)
{
    //Combine last two lines if they go in the same direction. This is necessary to prevent error in the straightening functinon.
    if (validLine(currentLine.back(),straighten(currentLine.back(), location),false))
    {
        if (vertical(currentLine.back(),straighten(currentLine.back(), location)) && vertical(currentLine[currentLine.size()-2], currentLine.back())) //If last line and line to add are both vertical
        {
            currentLine.back() = Coord(currentLine.back().x, location.y);// last coord is changed to the extended line.
        }
            //currentLine.push_back(straighten(currentLine.back(), location));
        else if (!vertical(currentLine.back(),straighten(currentLine.back(), location)) && !vertical(currentLine[currentLine.size()-2], currentLine.back())) //If last line and line to add are both horizontal
            currentLine.back() = Coord(location.x, currentLine.back().y);// last coord is changed to the extended line.
        else
            currentLine.push_back(straighten(currentLine.back(), location));
    }
}

Coord GameGUI::straighten(Coord last, Coord point) const
{
    // Determine to snap vertically or horizontally
    if (vertical(last, point))
    {
        //validLine(coord(last.x, last.y), coord(last.x, point.y))

        //keeps line from backtracking on itself
        if(currentLine.size() > 1 &&
           (((point.y < last.y)&&(currentLine[currentLine.size()-2].y < last.y))||
            ((point.y > last.y)&&(currentLine[currentLine.size()-2].y > last.y))))
            return Coord(point.x,last.y);

        return Coord(last.x, point.y);
    }
    else
    {
        //keeps line from backtracking on itself
        if(currentLine.size() > 1 &&
           (((point.x < last.x)&&(currentLine[currentLine.size()-2].x < last.x))||
            ((point.x > last.x)&&(currentLine[currentLine.size()-2].x > last.x))))
            return Coord(last.x,point.y);

        return Coord(point.x, last.y);
    }
}

Coord GameGUI::center() const
{
    return Coord(screen->w/2, screen->h/2);
}

void GameGUI::line(Coord a, Coord b, Uint32 color)
{
    for (int i = -lineThick; i < lineThick; i++)
        for (int j = -lineThick; j < lineThick; j++)
            lineColor(screen, a.x+i, a.y+j, b.x+i, b.y+j, color);
}

void GameGUI::circle(Coord p, int radius, Uint32 color)
{
    filledCircleColor(screen, p.x, p.y, radius, color);
}

// Select the closest node to the point if within the nodeRadius, otherwise
// return NULL
Node* GameGUI::selectedNode(Coord point) const
{
    int closestIndex = -1;
    double minDist = numeric_limits<double>::infinity();

    for (int i = 0; i < nodes.size(); i++)
    {
        double currentDist = distance(nodes[i]->getLoci(), point);

        if (currentDist < minDist)
        {
            minDist = currentDist;
            closestIndex = i;
        }
    }

    if (closestIndex != -1 && minDist <= nodeRadius)
        return nodes[closestIndex];
    else
        return NULL;
}

double GameGUI::distance(Coord a, Coord b) const
{
    return sqrt(pow(1.0*a.x-b.x,2)+pow(1.0*a.y-b.y,2));
}

// Determine if the line from the coordinates start to end would cross any line
// segments in the passed in Line.
bool GameGUI::validSingleLine(const Line& line, Coord start, Coord end,int lineSize, bool node) const
{
    const int startX = start.x;
    const int startY = start.y;
    const int endX = end.x;
    const int endY = end.y;

    for (int j = 1; j < lineSize; j++)
    {
        const int A2 = line[j-1].x;
        const int B2 = line[j-1].y;
        const int A3 = line[j].x;
        const int B3 = line[j].y;

        //verticle line being drawn
        if (endX == startX)
        {
            //pre existing horizontal
            if(A2 != A3)
            {
                //determines existing line
                if(A2 > A3)
                {
                    //checks if our new line crosses the horizontal line on the x axis
                    if((startX > A3)&&(startX < A2))
                    {
                        //determines our current line
                        if(startY > endY)
                        {
                            if(!node){//checks if our current line corsses on the y axis
                                if((B2 <= startY)&&(B2 >= endY))
                                    return false;
                            }
                            else
                                if((B2 < startY)&&(B2 > endY))
                                    return false;
                        }
                        else
                            if(!node){
                                if((B2 >= startY)&&(B2 <= endY)) //only difference from above is direction line was drawn
                                    return false;
                            }
                            else
                                if((B2 > startY)&&(B2 < endY)) //only difference from above is direction line was drawn
                                    return false;

                    }
                }
                else
                    if((startX < A3)&&(startX > A2)) //only difference from above is direction line was drawn
                    {
                        if(startY > endY)
                        {
                            if(!node){
                                if((B2 <= startY)&&(B2 >= endY))
                                    return false;
                            }
                            else
                                if((B2 < startY)&&(B2 > endY))
                                    return false;
                        }
                        else
                            if(!node){
                                if((B2 >= startY)&&(B2 <= endY))
                                    return false;
                            }
                            else
                                if((B2 > startY)&&(B2 < endY))
                                    return false;

                    }
            }
            else
                if(startX == A2) //check for line verticle line being drawn against other vertcle lines
                {
                    if(B2 > B3) //defines the pre existing line
                    {
                        if(((startY > B3)&&(startY <B2))||((endY > B3)&&(endY < B2))) //checks if our line shares similar y values intersect
                            return false;
                        if(startY < endY)
                        {
                            if((startY < B3)&&(endY > B3)) //also needed to check if y intercepts intersect
                                return false;
                        }
                        else
                            if((startY > B2)&&(endY < B2))//same as above comment only our current line is oriented in the other direction
                                return false;
                    }
                    else//same as above only for existing line in other direction
                    {
                        if(((startY > B2)&&(startY < B3))||((endY > B2)&&(endY < B3)))
                            return false;
                        if(startY < endY)
                        {
                            if((startY < B2)&&(endY > B2))
                                return false;
                        }
                        else
                            if((startY > B3)&&(endY < B3))
                                return false;
                    }
                }

        }
        else//same as above only for horizontal lines not horrizontal lines
        {
            if(B2 != B3)
            {
                if(B2 > B3)
                {
                    if((startY < B2)&&(startY > B3))
                    {
                        if(startX > endX)
                        {
                            if(!node){
                                if((A2 <= startX)&&(A2 >= endX))
                                    return false;
                            }
                            else
                                if((A2 < startX)&&(A2 > endX))
                                    return false;
                        }
                        else
                            if(!node){
                                if((A2 >= startX)&&(A2 <= endX))
                                    return false;
                            }
                            else
                                if((A2 > startX)&&(A2 < endX))
                                    return false;
                    }
                }
                else
                    if((startY > B2)&&(startY < B3))
                    {
                        if(startX > endX)
                        {
                            if(!node){
                                if((A2 <= startX)&&(A2 >= endX))
                                    return false;
                            }
                            else
                                if((A2 < startX)&&(A2 > endX))
                                    return false;

                        }
                        else
                            if(!node){
                                if((A2 >= startX)&&(A2 <= endX))
                                    return false;
                            }
                            else
                                if((A2 > startX)&&(A2 < endX))
                                    return false;

                    }
            }
            else
                if(startY == B2)
                {
                    if(A2 > A3)
                    {
                        if(((startX > A3)&&(startX < A2))||((endX > A3)&&(endX < A2)))
                            return false;
                        if(startX < endX)
                        {
                            if((startX < A3)&&(endX > A3))
                                return false;
                        }
                        else
                            if((startX > A2)&&(endX < A2))
                                return false;
                    }
                    else
                    {
                        if(((startX > A2)&&(startX < A3))||((endX > A2)&&(endX < A3)))
                            return false;
                        if(startX < endX)
                        {
                            if((startX < A2)&&(endX > A2))
                                return false;
                        }
                        else
                            if((startX > A3)&&(endX < A3))
                                return false;
                    }
                }
        }
    }

    return true;
}


bool GameGUI::validLine(Coord start, Coord end, bool node) const //send in true if where the click happened was a node or false if it was not a node
{
    if (!validSingleLine(currentLine, start, end,currentLine.size()-1,node))
        return false;

    //code for checking among the line currently being drawn
    for (int i = 0; i < lines.size(); i++)
    {
        const Line& line = *lines[i];

        if (!validSingleLine(line, start, end,line.size(),node))
            return false;
    }

    //for if line trys to end in node but node isn't clicked, minor problem fix
    for (int i = 0; i < nodes.size(); i++)//calls to each node
    {
            if(((end.x > nodes[i]->getLoci().x-nodeRadius)&&(end.x < nodes[i]->getLoci().x+nodeRadius))&&((end.y > nodes[i]->getLoci().y-nodeRadius)&&(end.y < nodes[i]->getLoci().y+nodeRadius)))
                if(!node)
                    return false;
    }


    //check for if line is going directly through node
    if(start.x==end.x)  //vertical line being drawn
    {
        for (int i = 0; i < nodes.size(); i++)//calls to each node
            if((start.x-nodeRadius <= nodes[i]->getLoci().x)&&(nodes[i]->getLoci().x <= start.x+nodeRadius)) //gives a boundary so lines cant pass through any part of circle
            {
                if(start.y < end.y) //defines the line being drawn
                {
                    if((nodes[i]->getLoci().y > start.y)&&(nodes[i]->getLoci().y < end.y)) //checks for if node is between the line on the y values
                        return false;
                }
                else
                    if((nodes[i]->getLoci().y < start.y)&&(nodes[i]->getLoci().y > end.y)) //same as above just for the line being oriented the other direction
                        return false;
            }
    }
    else
        for (int i = 0; i < nodes.size(); i++)
            if((start.y-nodeRadius <= nodes[i]->getLoci().y)&&(nodes[i]->getLoci().y <= start.y+nodeRadius))
            {
                if(start.x < end.x)
                {
                    if((nodes[i]->getLoci().x > start.x)&&(nodes[i]->getLoci().x < end.x))
                        return false;
                }
                else
                    if((nodes[i]->getLoci().x < start.x)&&(nodes[i]->getLoci().x > end.x))
                        return false;
            }

    return true;
}

void GameGUI::displayError(const string& msg)
{
    // Top left
    static SDL_Rect location;
    location.x = 0;
    location.y = 0;

    // Only update top left corner.
    location.w = 130;
    location.h = 20;

    SDL_Surface* error = TTF_RenderText_Blended(font, msg.c_str(), textCol);

    SDL_FillRect(screen, &location, 0);
    SDL_BlitSurface(error, NULL, screen , &location);
    //SDL_Flip(screen);
    SDL_FreeSurface(error);
}

void GameGUI::displayPosition(Coord c)
{
    ostringstream s;
    s << c;

    // Bottom left
    static SDL_Rect origin;
    origin.w = 60; // Approximate width and height
    origin.h = 20;
    origin.x = 0;
    origin.y = screen->h - origin.h;

    // Only update top left corner.

    SDL_Surface* hover = TTF_RenderText_Blended(font, s.str().c_str(), textCol);

    SDL_FillRect(screen, &origin, 0);
    SDL_BlitSurface(hover, NULL, screen , &origin);
    //SDL_Flip(screen);
    SDL_FreeSurface(hover);
}

bool GameGUI::playerTurn() const
{
    return player1;
}

Coord GameGUI::findMiddle() const
{
    int longestIndex = -1;
    double greatestDist = 0;

    for (int i = 1; i < currentLine.size(); i++)
    {
        double currentDist = distance(currentLine[i], currentLine[i-1]);

        if (currentDist > greatestDist)
        {
            greatestDist = currentDist;
            longestIndex = i;
        }
    }

    // Something went wrong, just pick the middle
    if (longestIndex == -1)
        longestIndex = currentLine.size()/2;

    return Coord((currentLine[longestIndex-1].x+currentLine[longestIndex].x)/2,
                 (currentLine[longestIndex-1].y+currentLine[longestIndex].y)/2);
}

GameGUI::~GameGUI()
{

}
