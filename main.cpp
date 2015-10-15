#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <utility>

using namespace std;


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

bool detectColision(DIR iDir, const pair<int, int> &iCurrentPos)
{
	pair<int, int> aNextPos = getNextPos(iDir, iCurrentPos);
	return gMap[aNextPos.first][aNextPos.second] != 0;
}

DIR computeDirection(DIR iDir, const pair<int, int> &iCurrentPos, bool&dead)
{
    int nbTurns = 0;
    DIR aDir = iDir;
    while(detectColision(aDir, iCurrentPos) && nbTurns < 4)
    {
     	cerr  << "Collision detected, turning: (" << dirToString[aDir] << ", (" << iCurrentPos.first << "," << iCurrentPos.second << ")" << ", (" << getNextPos(aDir, iCurrentPos).first << "," <<getNextPos(aDir, iCurrentPos).second << ")" << endl;
        aDir = static_cast<DIR>((aDir+1)%4) ;
        nbTurns++;
	}
	dead = nbTurns == 4;
    return aDir;
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
        aDirection = computeDirection(aDirection, gMyPos, dead);
        
        if (dead)
			cout << "DEPLOY" << endl;
		else
        	cout << dirToString[aDirection] << endl;
    }
}
