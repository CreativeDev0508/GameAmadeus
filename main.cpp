#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <utility>

using namespace std;

#define MAX_TURNS 250
#define MARGIN 20

const int kEnemyPredictVal = -5;
const int maxX = 30;
const int maxY = 15;


typedef vector<vector<int> > MAP;
MAP gMap(30, vector<int> (15, 0));
pair<int, int> gMyPos;
vector<pair<int, int>> lastEnemyPos(4, make_pair(-1,-1));

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

void cleanMapPredictions(){
	for(int i=0;i<maxX; i++)
		for(int j=0;j<maxY; j++)
			if(gMap[i][j] == kEnemyPredictVal)
				gMap[i][j] = 0;
}

bool validEnemy(int iX, int iY){
	return (iX != -1 && iY != -1);
}

pair<int, int> predictEnemyMove(int iID, pair<int, int> iCurrentPos){
	pair<int, int> nextMove{0,0};
	DIR nextDir;

	if(iCurrentPos.first - lastEnemyPos[iID].first)
		nextDir = (iCurrentPos.first - lastEnemyPos[iID].first >= 1 ) ? RIGHT:LEFT;
	else
		nextDir = (iCurrentPos.second - lastEnemyPos[iID].second >= 1 ) ? DOWN:UP;

	nextMove = getNextPos(nextDir, iCurrentPos);
	return nextMove;
}


// in how many turn will we loose
int computeMaxTurnBeforeDead(DIR iDir, const MAP& iMap)
{
	if (detectColision(iDir, gMyPos, iMap))
		return 0;
	bool dead = false;
	auto aPredMap = iMap;
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

	DIR aDirection = RIGHT;
	bool firstTurn = true;

    // game loop
    while (1) {
    	int helperBots;
    	cin >> helperBots; cin.ignore();

		cleanMapPredictions();

        // read player trails
        for (int i = 0; i < playerCount; i++) {
            int x;
            int y;
            cin >> x >> y; cin.ignore();
            if (!validEnemy(x, y))
            	continue;

            gMap[x][y] = i+1;

            if(myId == i)
            {
            	gMyPos = make_pair(x, y);
			}
            // try to predict next enemy position
            else if (validEnemy(lastEnemyPos[i].first, lastEnemyPos[i].second))
            {
            	pair<int, int> enemyPos = make_pair(x,y);
            	if (gMap[predictEnemyMove(i, enemyPos).first][predictEnemyMove(i, enemyPos).second] == 0)
            		gMap[predictEnemyMove(i, enemyPos).first][predictEnemyMove(i, enemyPos).second]= kEnemyPredictVal;
            } else if (firstTurn)
            {
            	if (gMap[getNextPos(UP, make_pair(x, y)).first][getNextPos(UP, make_pair(x, y)).second] == 0)
            		gMap[getNextPos(UP, make_pair(x, y)).first][getNextPos(UP, make_pair(x, y)).second] = kEnemyPredictVal;
            	if (gMap[getNextPos(DOWN, make_pair(x, y)).first][getNextPos(UP, make_pair(x, y)).second] == 0)
            		gMap[getNextPos(DOWN, make_pair(x, y)).first][getNextPos(UP, make_pair(x, y)).second] = kEnemyPredictVal;
            	if (gMap[getNextPos(LEFT, make_pair(x, y)).first][getNextPos(UP, make_pair(x, y)).second] == 0)
            		gMap[getNextPos(LEFT, make_pair(x, y)).first][getNextPos(UP, make_pair(x, y)).second] = kEnemyPredictVal;
            	if (gMap[getNextPos(RIGHT, make_pair(x, y)).first][getNextPos(UP, make_pair(x, y)).second] == 0)
            		gMap[getNextPos(RIGHT, make_pair(x, y)).first][getNextPos(UP, make_pair(x, y)).second] = kEnemyPredictVal;
            	
			}
            lastEnemyPos[i]=make_pair(x, y);
        }

        // deploy helper bots
        int removalCount;
        cin >> removalCount; cin.ignore();
        for (int i = 0; i < removalCount; i++) {
            int removeX;
            int removeY;
            cin >> removeX >> removeY; cin.ignore();
            if (gMyPos != make_pair(removeX, removeY))
            	gMap[removeX][removeY] = 0;
        }

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        
        //bool dead = false;
        //aDirection = computeDirection(aDirection, gMyPos, dead, gMap);
        
        // if blocked
        auto next = getNextPos(aDirection, gMyPos);
        int deployPredict = 0;
        if (helperBots>0 && gMap[next.first][next.second] != 0 && gMap[next.first][next.second] != kEnemyPredictVal 
			&& find(lastEnemyPos.begin(), lastEnemyPos.end(), next)==lastEnemyPos.end()) 
        {
        	// try if deploy helps
        	auto tempMap = gMap;
        	tempMap[next.first][next.second] = 0;
        	deployPredict = computeMaxTurnBeforeDead(aDirection, tempMap);
        	cerr << "Direction " << "DEPLOY" << " will be loosing in " << deployPredict << " turns !" << endl;
		}
        
        map<DIR, int> aMaxTurnMap = {{aDirection,0}, {static_cast<DIR>((aDirection+1)%4),0}, {static_cast<DIR>((aDirection+2)%4),0}, {static_cast<DIR>((aDirection+3)%4),0}};
        for(auto& aDirMap: aMaxTurnMap)
        {
        	aDirMap.second = computeMaxTurnBeforeDead(aDirMap.first, gMap);
        	cerr << "Direction " << dirToString[aDirMap.first] << " will be loosing in " << aDirMap.second << " turns !" << endl;
		}
		
		int maxValue = 0;      
        for(auto& aDirMap: aMaxTurnMap)
        {
			if(maxValue <  aDirMap.second)
			{
				aDirection = aDirMap.first;
				maxValue = aDirMap.second;
			}	
        }
        
        if ((maxValue == 0 && helperBots>0) || deployPredict > maxValue + MARGIN)
			cout << "DEPLOY" << endl;
		else
        	cout << dirToString[aDirection] << endl;
        	
        firstTurn = false;
    }
}
