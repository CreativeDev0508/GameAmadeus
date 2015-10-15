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

pair<int, int> getNextPos(DIR iDir)
{
	auto aOutput = gMyPos;
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

bool detectColision(DIR iDir)
{
	pair<int, int> aNextPos = getNextPos(iDir);
	return gMap[aNextPos.first][aNextPos.second] != 0;
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
	map<DIR, string> dirToString = {{DOWN,"DOWN"}, {UP,"UP"}, {RIGHT,"RIGHT"}, {LEFT,"LEFT"}};
	

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
        
        int nbTurns = 0;
        while(detectColision(aDirection) && nbTurns < 4)
        {
        	cerr  << "Collision detected, turning: (" << dirToString[aDirection] << ", (" << gMyPos.first << "," << gMyPos.second << ")" << ", (" << getNextPos(aDirection).first << "," <<getNextPos(aDirection).second << ")" << endl;
        	aDirection = static_cast<DIR>((aDirection+1)%4) ;
        	nbTurns++;
		}

		if (nbTurns > 3)
			cout << "DEPLOY";
		else
        	cout << dirToString[aDirection] << endl;
    }
}
