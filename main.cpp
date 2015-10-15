#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <utility>

using namespace std;

#define MAX_TURNS 10

typedef vector<vector<int> > MAP;
vector<vector<int> > gMap(30, vector<int> (15, 0));
pair<int, int> gMyPos;

enum DIR : int {
	UP = 0,
	RIGHT,
	DOWN,
	LEFT
};


map<DIR, string> dirToString = {{DOWN,"DOWN"}, {UP,"UP"}, {RIGHT,"RIGHT"}, {LEFT,"LEFT"}};
	

pair<int, int> getNextPos(DIR iDir, const pair<int, int> &iCurrentPos)
{
	auto aOutput = iCurrentPos;
	switch(iDir)
	{
		case UP:
			aOutput.second = (aOutput.second - 1 +15)%15;
			break;
		case DOWN:
			aOutput.second = (aOutput.second +1)%15;
			break;
		case LEFT:
			aOutput.first = (aOutput.first - 1 +30 )%30;
			break;
		case RIGHT:
			aOutput.first = (aOutput.first +1)%30;
			break;
	}
	return aOutput;
}

bool detectColision(DIR iDir, const pair<int, int> &iCurrentPos, const MAP& iMap)
{
	pair<int, int> aNextPos = getNextPos(iDir, iCurrentPos);
	return iMap[aNextPos.first][aNextPos.second] != 0;
}

DIR computeDirection(DIR iDir, const pair<int, int> &iCurrentPos, bool&dead, const MAP& iMap)
{
    int nbTurns = 0;
    DIR aDir = iDir, forbiddenDir = static_cast<DIR>((iDir+2)%4);
    while(detectColision(aDir, iCurrentPos, iMap) && nbTurns < 3)
    {
     	cerr  << "Collision detected, turning: (" << dirToString[aDir] << ", (" << iCurrentPos.first << "," << iCurrentPos.second << ")" << ", (" << getNextPos(aDir, iCurrentPos).first << "," <<getNextPos(aDir, iCurrentPos).second << ")" << endl;
        aDir = static_cast<DIR>((aDir+1)%4) ;
        if(aDir == forbiddenDir)
        {
        	aDir = static_cast<DIR>((aDir+1)%4);
		}
        nbTurns++;
	}
	dead = nbTurns == 3;
    return aDir;
}

// in how many turn will we loose
int computeMaxTurnBeforeDead(DIR iDir)
{
	if (detectColision(iDir, gMyPos, gMap))
		return 0;
	bool dead = false;
	auto aPredMap = gMap;
	auto aPredPos = gMyPos;
	int aTurns = 0;
	while(!dead && aTurns < MAX_TURNS)
	{
    	iDir = computeDirection(iDir, aPredPos, dead, aPredMap);
    	
    	aPredPos = getNextPos(iDir, aPredPos);
    	aPredMap[aPredPos.first][aPredPos.second] = 10;
    	aTurns++;
    }
    return aTurns;
}

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main()
{
    int playerCount;
    cin >> playerCount; cin.ignore();
    int myId;
    cin >> myId; cin.ignore();

	DIR aDirection = DOWN;

    // game loop
    while (1) {
        int helperBots;
        cin >> helperBots; cin.ignore();
        for (int i = 0; i < playerCount; i++) {
            int x;
            int y;
            cin >> x >> y; cin.ignore();
            if (x==-1 || y==-1)
            	continue;
            gMap[x][y] = i+1;
            if(myId == i)
            {
            	gMyPos = make_pair(x, y);
			}
        }
        int removalCount;
        cin >> removalCount; cin.ignore();
        for (int i = 0; i < removalCount; i++) {
            int removeX;
            int removeY;
            cin >> removeX >> removeY; cin.ignore();
            gMap[removeX][removeY] = 0;
        }

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        
        bool dead = false;
        aDirection = computeDirection(aDirection, gMyPos, dead, gMap);
        
        map<DIR, int> aMaxTurnMap = {{DOWN,0}, {UP,0}, {RIGHT,0}, {LEFT,0}};
        for(auto& aDirMap: aMaxTurnMap)
        {
        	aDirMap.second = computeMaxTurnBeforeDead(aDirMap.first);
        	cerr << "Direction " << dirToString[aDirMap.first] << " will be loosing in " << aDirMap.second << " turns !" << endl;
		}
        cerr << "Loosing in " << computeMaxTurnBeforeDead(aDirection) << " turns !" << endl;
        
        if (dead)
			cout << "DEPLOY" << endl;
		else
        	cout << dirToString[aDirection] << endl;
    }
}
