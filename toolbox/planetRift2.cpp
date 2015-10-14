#include <iostream>
#include <string>
#include <sstream> 
#include <vector>
#include <set>
#include <list>
#include <algorithm>
#include <ctime>
#include <map>
#include <ctime>
using namespace std;

typedef enum e_StrategyType {
  NoStrategy,
  Explore,
  Attack,
  Defend,
  Flew
} StrategyType ;

typedef enum e_CommandType {
  NoCommand,
  Move,
  Spawn
} CommandType ;

class Command
{
public:
   Command():_type(NoCommand),_zIdOrg(0),_zIdDest(0), _pods(0), _utility(0), _stratType(NoStrategy) {};
    CommandType _type;
    int _zIdOrg; // from where to move
    int _zIdDest; // to where
    int _pods; // how many pods
    int _utility; // the utility of this Move for prioritizing
    StrategyType _stratType;
};

class SpawnCommand : public Command
{
    public:
    SpawnCommand():Command(),_zId(0) {_type = Spawn;};
    int _zId; // where to spawn
};

class MoveCommand : public Command
{
    public:
    MoveCommand():Command() {_type = Move;};
};

class World
{
    public:
    World(int iPlayerCount,int iMyId,int iZoneCount,int iLinkCount)
    :playerCount(iPlayerCount),
    myId(iMyId),
    zoneCount(iZoneCount),
    linkCount(iLinkCount),
    _totalPltSrc(0),
    _myPlat(0),
    _enemyPlat(0),
    _previousPlat(0),
    platSrc(iZoneCount,0),
    _lastDiscoveredZone(iZoneCount,-1),
    aLinks(iZoneCount, vector<bool>(iZoneCount,false)),
    _linksSet(iZoneCount),
    aNumLinks(iZoneCount,0),
    _currentPodsPerZone(iZoneCount, vector<int>(4,0)),
    _previousPodsPerZone(iZoneCount, vector<int>(4,0)),
    _currentZoneStatus(iZoneCount,-1),
    _previousZoneStatus(iZoneCount,-1),
    _distanceMap(iZoneCount, vector<int>(iZoneCount,-1)),
    _staticUtility(iZoneCount, 0),
    _symetrical(false),
    _isSparse(false),
    _targetedZone(iZoneCount, false),
    _turnCount(0){}
    
    void findGateways();
    void findSubArea(int iStartingGate,const list<int>& iExitGroup, set<int>& iSubArea, vector<int>& ioExploredIds);
    void findLimitZones(); // find the zone at equal distance beetween 2 bases
    void subPropagateStatic(int iStaticPotential,int i, map<int, int>& ioAddPot);
    void buildGlobalStat();
    void computeDistanceMap();
    void computeStaticUtility();
    template<class T> 
    void sortByStaticUtility(list<T> &oList);
    void computeStrategy();
    void sendAllToEnemyBase(list<MoveCommand>& oMoveCommandLst);

    int playerCount; // the amount of players (2 to 4)
    int myId; // my player ID (0, 1, 2 or 3)
    int zoneCount; // the amount of zones on the map
    int linkCount; // the amount of links between all zones
    int _totalPltSrc;
    int _myPlat;
    int _enemyPlat;
    int _previousPlat; // the platinum I spent the turn before.
    vector<int> platSrc;
    vector<int> _lastDiscoveredZone;
    vector<vector<bool> > aLinks;
    vector<set<int> > _linksSet;
    vector<int> aNumLinks;
    vector<vector<int> > _currentPodsPerZone;
    vector<vector<int> > _previousPodsPerZone;
    vector<int> _currentZoneStatus;
    vector<int> _previousZoneStatus;
    vector<vector<int> > _distanceMap;
    list<int> _orderedIdByPlat;
    vector<int> _staticUtility; // LvL2 , LVL1 is platSrc ...
    list<int> _orderedIdByStaticUtility;
    map<int, list<list<int>>> _gateways;
    set<int> _subAreaGates;
    map<int, set<int>> _subAreaSets;
    vector<bool> _targetedZone;
    StrategyType _globalStrategy;
    map<int, StrategyType> _perSubAreaStrategy;
    bool _symetrical;
    bool _isSparse;
    bool _iamLast;
    int _myBase;
    int _enemyBase;
    int _turnCount;
};

void generateSpawnCommand(const list<SpawnCommand>& iSpawnList, ostringstream& oSpawnCommand)
{
    for(auto& iSC: iSpawnList)
    {
        oSpawnCommand << iSC._pods << " " << iSC._zId<< " " ;
    }
}

void generateMoveCommand(const list<MoveCommand>& iMoveList, ostringstream& oMoveCommand)
{
    for(auto& iMC: iMoveList)
    {
        if(iMC._zIdOrg != iMC._zIdDest && iMC._zIdDest >= 0)
        {
            oMoveCommand << iMC._pods << " " << iMC._zIdOrg<< " " << iMC._zIdDest << " ";
        }
    }
}

void computeMoveablePods(World& iW, const list<MoveCommand>& iMoveCommandLst, list<MoveCommand>& oMoveCommandLst)
{
for (int i = 0; i < iW.zoneCount; i++) {
        if(iW._currentPodsPerZone[i][iW.myId] >= 1)
        { // find not moveable with same origin in oMovecommandLst
            int aPodsNotToMove = 0;
            for(auto& aMCit: iMoveCommandLst)
            {
                if(aMCit._zIdOrg == aMCit._zIdDest && iW._currentPodsPerZone[aMCit._zIdOrg][iW.myId] == iW._previousPodsPerZone[aMCit._zIdOrg][iW.myId]) // pods not to move, but weren't attacked ...
                {
                     //we can move them ...
                }
                //else 
                if(aMCit._zIdOrg == i)
                {
                    aPodsNotToMove += aMCit._pods;
                }
            }
            MoveCommand aMC;
            aMC._pods = iW._currentPodsPerZone[i][iW.myId] - aPodsNotToMove;
            aMC._zIdOrg = i;
            aMC._zIdDest = -1; // we know it is moveable
            if(aMC._pods > 0)
            {
                oMoveCommandLst.push_back(aMC);
                //cerr << "zone " << aMC._zIdOrg << " pods " << aMC._pods  << " moveable" << endl;
            }
        }
    }
}

class ZoneInDanger
{
  public:
  ZoneInDanger(): _zId(0), _threatPods(0), _myPods(0) {};
  int _zId;
  int _threatPods;
  int _myPods;
  int _utility; // for prioritization
  list<int> _threatOriginLst;
  int _minThreatUtility;
};

void checkAndBuildDefense(World& iW, int iPlatinum,  list<SpawnCommand>& oSpawnList, list<MoveCommand>& oMoveCommandLst)
{
    // do we have zone with platinum in danger ? and how much threatening pods ?
    list<ZoneInDanger> aDangerLst;
    //cerr << "how many _orderedID : " << iW._orderedIdByPlat.size() << endl;
    for(auto aId: iW._orderedIdByPlat)
    {
        if(iW._currentZoneStatus[aId] == iW.myId && iW.platSrc[aId] > 0)
        {
            //look for enemy pods in surounding area and sum them
            ZoneInDanger aZinD;
            aZinD._zId = aId;
            aZinD._myPods = iW._currentPodsPerZone[aId][iW.myId];
            aZinD._utility = iW.platSrc[aId];
            aZinD._minThreatUtility = 6;
            // threat on my zone : under attack ! -> the max of other player present
            int aCombatZoneMaxThreat = 0;
            for(int i = 0; i < iW.playerCount; ++i)
            {
                if(iW._currentPodsPerZone[aId][i] > 0 && iW._currentPodsPerZone[aId][i] > aCombatZoneMaxThreat && i != iW.myId)
                {
                    aCombatZoneMaxThreat = iW._currentPodsPerZone[aId][i];
                }
            }
            aZinD._threatPods += aCombatZoneMaxThreat;
            
            //cerr << "how many link around : " << iW._linksSet[aId].size() << endl;
            //cerr << "how many link threat : " << aZinD._threatPods << endl;
            for(auto aNextZId: iW._linksSet[aId])
            {
                if(iW._currentZoneStatus[aNextZId] >= 0 && iW._currentZoneStatus[aNextZId] != iW.myId)
                {
                    // check it isn't a fight
                    // how many other are here ?
                    int aPlayerPresent = 0; // if player >= 2 -> it is a fight
                    int aOtherPlayerId = 0;
                    for(int i = 0; i < iW.playerCount; ++i)
                    {
                        if(iW._currentPodsPerZone[aNextZId][i] > 0 && i != iW.myId)
                        {
                            ++aPlayerPresent;
                            aOtherPlayerId = i;
                        }
                    }
                    
                    if(aPlayerPresent == 1) // and it is not me ...
                    {
                        // have to prepare defense
                        aZinD._threatPods += iW._currentPodsPerZone[aNextZId][aOtherPlayerId];
                        aZinD._threatOriginLst.push_back(aNextZId);
                        if(aZinD._minThreatUtility > iW.platSrc[aNextZId])
                        {
                            aZinD._minThreatUtility = iW.platSrc[aNextZId];
                        }
                    }
                }
            } // loop on neighbooring zone
            if(aZinD._threatPods > 0)
            {
                aDangerLst.push_back(aZinD);
                cerr << "zone " << aZinD._zId << " in danger from " << aZinD._threatPods << " pods in " << aZinD._threatOriginLst.size() << " enemy zone(s)" << endl;
            }
        }
    }
    // now analyse what we can do with all those danger
    //cerr << "how many danger : " << aDangerLst.size() << endl;
    if(!aDangerLst.empty())
    {
        // sort by utility and spawn as much as we can
        // TODO try to move other pods not threatened ..
        auto aSortByUtility = [](ZoneInDanger& a, ZoneInDanger& b) {
            if(b._utility < a._utility)
            {
                return true;
            }
            return b._threatPods < a._threatPods;   
            };
            
        aDangerLst.sort(aSortByUtility);
        int anAvailablePods = iPlatinum / 20;
        map<int,int> aStaticDefense;
        for(auto& aZinD: aDangerLst)
        {
            aStaticDefense[aZinD._zId] = 0;
            int neededPods = min(max(aZinD._threatPods - aZinD._myPods,0),4);
            //int neededPods = min(aZinD._threatPods,4); // 4 is the min for defend one tour
            if(neededPods >= 0 && aZinD._myPods > 0)
            {
                // tag the counted already existing pods as DO NOT MOVE
                MoveCommand aMC;
                if(aZinD._threatPods >= aZinD._myPods)
                {
                    aMC._pods = aZinD._myPods; // don't move all needed for defense
                }
                else
                {
                    aMC._pods = min(aZinD._threatPods,4);
                }
                
                // do not move 4 maximum, if you have more to loose than to win
                if(aZinD._minThreatUtility <= aZinD._utility && aZinD._utility > 0)
                {
                    aMC._pods = min(aMC._pods,4);
                    aStaticDefense[aZinD._zId] += aMC._pods;
                    aMC._zIdDest = aZinD._zId;; // if Dest == Org -> Do not Move;
                    aMC._zIdOrg = aZinD._zId;
                    oMoveCommandLst.push_back(aMC);
                    cerr << "Defense : don't move " << aMC._pods << "pods from " <<  aZinD._zId << endl;
                }
                else
                {
                    cerr << "Loose Defense on " <<  aZinD._zId << " cause we have more to win" << endl;
                }
            }
            
            if(anAvailablePods > 0 && neededPods > 0 && aZinD._myPods < 4 && anAvailablePods >= neededPods)
            {
               
                // we will spawn
                SpawnCommand aSC;
                aSC._zId = aZinD._zId; // this is always my zone
                aSC._pods = neededPods;
                aStaticDefense[aZinD._zId] += aSC._pods;
                oSpawnList.push_back(aSC);
                cerr << "Defense : spawn " << aSC._pods << "pods on " <<  aZinD._zId << endl;
                anAvailablePods -= neededPods;
            }
        }
        
        
        list<MoveCommand> aDefenseByMoveList;
        for(auto& aZinD: aDangerLst)
        {
            // static defense is done, try dynamic one
            list<MoveCommand> aMoveableList;
            computeMoveablePods(iW, oMoveCommandLst, aMoveableList);
            
            if(aStaticDefense[aZinD._zId] < aZinD._threatPods)
            {
                int neededPods = min(max(aZinD._threatPods - aStaticDefense[aZinD._zId],0),4);
                if(neededPods > 0)
                {
                    // do I have other pods around ?
                    for (auto& aMC : aMoveableList) {
                        if(neededPods > 0)
                        {
                            for (auto aNextId : iW._linksSet[aZinD._zId]) {
                                if(neededPods > 0)
                                {
                                    if(aMC._zIdOrg == aNextId && aMC._pods > 0)
                                    {
                                        aMC._zIdDest = aZinD._zId;
                                        if(neededPods < aMC._pods)
                                        {
                                            aMC._pods = neededPods;
                                        }
                                        aDefenseByMoveList.push_back(aMC);
                                        cerr << "a defense by move to " << aMC._zIdDest << " with " << aMC._pods << " from " << aMC._zIdOrg << endl;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        
        }
    }
}

void applyAggressiveStrategy(World& iW, int iPlatinum,  list<SpawnCommand>& oSpawnList, list<MoveCommand>& oMoveCommandLst)
{
    // consider not available pods
    list<MoveCommand> aMoveablePods; // prepare all moveable pods
    computeMoveablePods(iW,oMoveCommandLst, aMoveablePods);
    
    // todo recycle spawn on free place
    
    list<int> aMyZoneNextToEnemy;
    list<int> aEnemyZones;
    for (int i = 0; i < iW.zoneCount; i++) {
        if(iW._currentZoneStatus[i] == iW.myId)
        {
            for (auto aNextId : iW._linksSet[i]) {
                if(iW._currentZoneStatus[aNextId] != iW.myId && iW._currentZoneStatus[aNextId] != -1)
                {
                    aMyZoneNextToEnemy.push_back(i);
                    break;
                }
            }
        }
        else if(iW._currentZoneStatus[i] != -1)
        {
            aEnemyZones.push_back(i);
        }
    }
    
    //cerr << "zone next to ennemy:" << aMyZoneNextToEnemy.size() << endl;
    iW.sortByStaticUtility(aMyZoneNextToEnemy);
    
    map<int,int> aThreatPerZone;
    for(auto& aZId : aMyZoneNextToEnemy)
    {
        aThreatPerZone[aZId] = 0;
        for (auto aNextId : iW._linksSet[aZId]) {
            for (int i = 0; i < iW.playerCount; i++) {
                if(i != iW.myId)
                {
                    aThreatPerZone[aZId] += iW._currentPodsPerZone[aNextId][i];
                }
            }
        }
    }
    auto aSortByThreat = [&aThreatPerZone](int a, int b) {
        return aThreatPerZone[b] < aThreatPerZone[a];   
    };
    aMyZoneNextToEnemy.sort(aSortByThreat);
    
    int anAvailablePods = iPlatinum / 20;
    // TODO : don't spawn if we can move ...
    if(anAvailablePods > 0)
    {
        for (auto& aZId : aMyZoneNextToEnemy) {
            // do we already have defense here ?
            
            // how many threat
            
            if(anAvailablePods > 0)
            {
                SpawnCommand aSC;
                aSC._zId = aZId;
                int aPods = max(min(min(anAvailablePods,aThreatPerZone[aZId]),4),1);
                aSC._pods = aPods;
                anAvailablePods -= aPods;
                oSpawnList.push_back(aSC);
                if(anAvailablePods <= 0)
                {
                    break;
                }
            }
        }
    }
        
    // move to freespace or attack
    //list<int> aMyPodCloseToEnemy;
    for (auto& aMCp : aMoveablePods) {
        // find freespace with
        int aMaxPlatFreeSpace = -1;
        int aMaxPlatId = 0;
        for (auto aNextId : iW._linksSet[aMCp._zIdOrg])
        {
            if(iW._currentZoneStatus[aNextId] == -1 && iW.platSrc[aNextId] > aMaxPlatFreeSpace && !iW._targetedZone[aNextId])
            {
                aMaxPlatFreeSpace = iW.platSrc[aNextId];
                aMaxPlatId = aNextId;
            }
        }
        
        if(aMaxPlatFreeSpace == -1)
        {
        for (auto aNextId : iW._linksSet[aMCp._zIdOrg]) 
        {
            if(iW._currentZoneStatus[aNextId] != iW.myId && iW._currentZoneStatus[aNextId] != -1)
            {
                cerr << "move " << aMCp._pods << " pod(s) to attack " << aNextId << endl;
                aMCp._zIdDest = aNextId;
                iW._targetedZone[aNextId] = true;
                oMoveCommandLst.push_back(aMCp);
                break;
            }
        }
        }
        else
        {
            aMCp._zIdDest = aMaxPlatId;
            iW._targetedZone[aMaxPlatId] = true;
            oMoveCommandLst.push_back(aMCp);
        }
    }
    
    // do we still have available Pods to spawn !! -> spawn closer to best enemy zone
    if(anAvailablePods > 0)
    {
        iW.sortByStaticUtility(aEnemyZones);
        for (auto aEnemyZone : aEnemyZones) {
            // find my zone closest to enemy zone
            int aMinDist = iW.zoneCount;
            int aMinId = -1;
            for (int i = 0; i < iW.zoneCount; i++) {
            if(iW._currentZoneStatus[i] == iW.myId)
            {
                if(iW._distanceMap[aEnemyZone][i] < aMinDist)
                {
                    aMinDist = iW._distanceMap[aEnemyZone][i];
                    aMinId = i;
                }
            }
            }
            if(aMinId != -1)
            {
            SpawnCommand aSC;
            aSC._zId = aMinId;
            aSC._pods = 1;
            anAvailablePods -= aSC._pods;
            oSpawnList.push_back(aSC);
            }
            if(anAvailablePods <= 0)
            {
                break;
            }
        }
    }
}

void moveToClosestFreeOrEnemyZone(World& iW, list<MoveCommand>& oMoveCommandLst)
{
    // consider not available pods
    list<MoveCommand> aMoveablePods; // prepare all moveable pods
    computeMoveablePods(iW,oMoveCommandLst, aMoveablePods);
    
    //compute the reached zone
    vector<bool> aTargetedZone(iW.zoneCount, false);
    
    // go first to reachable zone with prio for platinum
    for (auto& aMCit : aMoveablePods) {
        int aPods = aMCit._pods;
        int aPreviousPods = aPods;
        while(aPods > 0)
        {
            int aMaxPlat = 0, aMaxId = -1;
            for(auto aNextZid:iW._linksSet[aMCit._zIdOrg])
            {
                if(iW._currentZoneStatus[aNextZid] != iW.myId && !aTargetedZone[aNextZid] && aMaxPlat <= iW.platSrc[aNextZid])
                {
                    aMaxPlat = iW.platSrc[aNextZid];
                    aMaxId = aNextZid;
                }
            }
            
            if(aMaxId != -1) // set a pod to go to this target
            {
                // set the target    
                aMCit._zIdDest = aMaxId;
                int aSavePods = aMCit._pods;
                aMCit._pods = 1;
                aPods--;
                aSavePods--;
                //aReachedZone[aMCit._zIdDest] = true;
                aTargetedZone[aMaxId] = true;
                oMoveCommandLst.push_back(aMCit);
                aMCit._pods = aSavePods;
                cerr << "pod in " << aMCit._zIdOrg << " short targets " << aMaxId << endl;
            }
            
            if (aPreviousPods == aPods)
            {
                aPods--;
            }
            aPreviousPods = aPods;
        }
        
    }
    
    
    // go to closer free zone
    for (auto& aMCit : aMoveablePods) {
        int aPods = aMCit._pods;
        // get non zero reachable utility
        int aPreviousPods = aPods;
        while(aPods > 0)
        {
            int aMinDistPlat = iW.zoneCount;
            int aMinDistFree = iW.zoneCount;
            int aMinId = -1;
            int aMinIdFree = -1;
            int aMaxUtility = 0;
            for(auto j : iW._orderedIdByStaticUtility)
            {
                if(iW._currentZoneStatus[j] != iW.myId && !aTargetedZone[j])// && (iW._lastDiscoveredZone[j] == -1 || iW._turnCount - iW._lastDiscoveredZone[j] > 5)) // freeplace or enemy place
                {
                    // is it reachable from i ?
                    //cerr << "Distance :" << iW._distanceMap[i][j] << endl;
                    if(iW._distanceMap[aMCit._zIdOrg][j] > 0 && iW.platSrc[j] >= 0)
                    {
                        if( iW._distanceMap[aMCit._zIdOrg][j] < aMinDistPlat)
                        {
                                aMinDistPlat = iW._distanceMap[aMCit._zIdOrg][j];
                                aMinId = j;
                                aMaxUtility = iW.platSrc[j];
                        }
                    }
                    else if(iW._distanceMap[aMCit._zIdOrg][j] > 0) //explore free space
                    {
                        if( iW._distanceMap[aMCit._zIdOrg][j] < aMinDistFree)
                        {
                                aMinDistFree = iW._distanceMap[aMCit._zIdOrg][j];
                                aMinIdFree = j;
                        }
                    }
                }
            }
        
            int aTargetId = -1;
            
            if(aMinId != -1 && aMinId != aMCit._zIdOrg)
            {
               aTargetId = aMinId;
            }
            else if(aMinIdFree != -1 && aMinIdFree != aMCit._zIdOrg)
            {
                aTargetId = aMinIdFree;
            }
            if(aTargetId != -1) // set a pod to go to this target
            {
                // set the target    
                aMCit._zIdDest = aTargetId;
                aMCit._pods = 1;
                aPods--;
                //aReachedZone[aMCit._zIdDest] = true;
                aTargetedZone[aTargetId] = true;
                oMoveCommandLst.push_back(aMCit);
                cerr << "pod in " << aMCit._zIdOrg << " targets " << aTargetId << endl;
            }
                
        
            // bloom the rest ...
            if(aPods >= 6 && false)
            {
                while(aPods > 0)
                {
                    for(auto aId : iW._linksSet[aMCit._zIdOrg])
                    {
                        aMCit._zIdDest = aId;
                        aMCit._pods = 1;
                        aPods--;
                        //aReachedZone[aMCit._zIdDest] = true;
                        oMoveCommandLst.push_back(aMCit);
                    }
                }
            }
            
            if (aPreviousPods == aPods)
            {
                aPods--;
            }
            aPreviousPods = aPods;
        }
    }
    
    // swap with closer target
    for(auto& aMCit1 : oMoveCommandLst)
    {
        // find the closest other target.
        int aClosestTarget = -1;
        int aClosestOrg = -1;
        int aMinDist = iW._distanceMap[aMCit1._zIdOrg][aMCit1._zIdDest];
        for(auto& aMCit2 : oMoveCommandLst)
        {
            if(aMCit1._zIdDest != aMCit2._zIdDest || aMCit1._zIdOrg != aMCit2._zIdOrg) //not same pod
            {
                if(iW._distanceMap[aMCit2._zIdOrg][aMCit2._zIdDest] > 1 && iW._distanceMap[aMCit1._zIdOrg][aMCit1._zIdDest] > 1 && iW._distanceMap[aMCit1._zIdOrg][aMCit2._zIdDest] < aMinDist)
                {
                    aMinDist = iW._distanceMap[aMCit1._zIdOrg][aMCit2._zIdDest];
                    aClosestTarget = aMCit2._zIdDest;
                    aClosestOrg = aMCit2._zIdOrg;
                }
            }
        }
        
        for(auto& aMCit2 : oMoveCommandLst)
        {
            if(aMCit1._zIdDest != aMCit2._zIdDest || aMCit1._zIdOrg != aMCit2._zIdOrg) //not same pod
            {
                if(aMCit2._zIdDest == aClosestTarget && aMCit2._zIdOrg == aClosestOrg)
                {
                    // swap target id
                    int aTarget1 = aMCit1._zIdDest;
                    aMCit1._zIdDest = aMCit2._zIdDest;
                    aMCit2._zIdDest = aTarget1;
                }
            }
        }
        
    }
    //}
    // now set the next move
    cerr << "after swap" << endl;
    vector<bool> aReachedZone(iW.zoneCount, false);
    for(auto& aMCit : oMoveCommandLst)
    {
        int aTargetId = aMCit._zIdDest;
        cerr << "pod in " << aMCit._zIdOrg << " targets " << aTargetId << endl;
        // we want to go from i to aTargetId
        // find next move from i -> this dist - 1 in the reverse distance table ...
        int aMinDisTarget = iW._distanceMap[aMCit._zIdOrg][aTargetId];
        list<int> aNextMoveList;
        for(auto aId : iW._linksSet[aMCit._zIdOrg])
        {
            if(iW._distanceMap[aTargetId][aId] == aMinDisTarget - 1)// && !aReachedZone[aId])
            {
                aNextMoveList.push_back(aId);
            }
        }
        if(!aNextMoveList.empty())
        {
            int aMaxPlatMove = -1, aNonReachedMove = -1, aNextMove = -1;
            int aMaxPlat = 0;
            for(auto aId: aNextMoveList) // look for max platinum or non alreached zone
            {
                aNextMove = aId;
                if(!aReachedZone[aId])
                {
                    aNonReachedMove = aId;
                }
                if(aMaxPlat < iW.platSrc[aId])
                {
                    aMaxPlatMove = aId;
                    aMaxPlat = iW.platSrc[aId];
                }
            }
            
            if(aMaxPlatMove == aNonReachedMove && aNonReachedMove != -1)
            {
                aMCit._zIdDest = aNonReachedMove;
            }
            else if(aNonReachedMove != -1)
            {
                aMCit._zIdDest = aNonReachedMove;
            }
            else if(aMaxPlatMove != -1)
            {
                aMCit._zIdDest = aMaxPlatMove;
            }
            else
            {
                aMCit._zIdDest = aNextMove;
            }
            aReachedZone[aMCit._zIdDest] = true;
        }
        
    }
}

int main()
{
    std::srand(std::time(0));
    
    int playerCount; // the amount of players (2 to 4)
    int myId; // my player ID (0, 1, 2 or 3)
    int zoneCount; // the amount of zones on the map
    int linkCount; // the amount of links between all zones
    cin >> playerCount >> myId >> zoneCount >> linkCount; cin.ignore();
    World w(playerCount, myId, zoneCount, linkCount);
    
    //vector<int> platSrc(zoneCount);
    for (int i = 0; i < zoneCount; i++) {
        int zoneId; // this zone's ID (between 0 and zoneCount-1)
        int platinumSource; // the amount of Platinum this zone can provide per game turn
        cin >> zoneId >> platinumSource; cin.ignore();
        w.platSrc[zoneId] = platinumSource;
        w._orderedIdByPlat.push_back(zoneId);
        w._orderedIdByStaticUtility.push_back(zoneId);
    }
    
    //vector<vector<bool> > aLinks(zoneCount, vector<bool>(zoneCount,false));
    //vector<int> aNumLinks(zoneCount, 0);
    for (int i = 0; i < linkCount; i++) {
        int zone1;
        int zone2;
        cin >> zone1 >> zone2; cin.ignore();
        w.aLinks[zone1][zone2] = true;
        w.aLinks[zone2][zone1] = true;
        w._linksSet[zone1].insert(zone2);
        w._linksSet[zone2].insert(zone1);
        w.aNumLinks[zone1]++;
        w.aNumLinks[zone2]++;
    }

    w.findGateways();

    auto aSortByPlat = [&w](int a, int b) {
        return w.platSrc[b] < w.platSrc[a];   
    };

    w._orderedIdByPlat.sort(aSortByPlat);

    //vector<int> aStaticPotential(zoneCount, 0);
    const clock_t begin_time = clock();
    //w.computeStaticPotential(aStaticPotential); // potential only due to geography
    w.computeStaticUtility();
    w.sortByStaticUtility( w._orderedIdByStaticUtility);
    //w._orderedIdByStaticUtility.sort(aSortByUtility);
    
    w.computeDistanceMap();
    cerr << "execution time:" << float( clock () - begin_time ) * 1000 /  CLOCKS_PER_SEC << "ms" << endl;
    // game loop
    
    //ostringstream aStaticPotData;
    //for (int i = 0; i < zoneCount; ++i) {
    //   aStaticPotData << " " << i << ":" << aStaticPotential[i];
    //}
    //cerr << aStaticPotData.str() << endl;
    // strategy to test -> flew, eternal defense, perzone optimisation.
    
    w._turnCount = 1;
    
    while (1) {
        const clock_t loop_time = clock();
        int platinum; // my available Platinum
        cin >> platinum; cin.ignore();
         w._myPlat = platinum;
        //vector<int> potential(aStaticPotential);
        bool aNewPlatSrcDiscovered = false;
        list<int> aFreeZoneWithPlat;
        for (int i = 0; i < zoneCount; i++) {
            int zId, aCurrentZoneStatus, aPods[2]; // this zone's ID
            cin >> zId ;
            cin >> aCurrentZoneStatus >> aPods[0] >> aPods[1] >> w._currentPodsPerZone[zId][2] >> w._currentPodsPerZone[zId][3]; cin.ignore();
            //if(w._currentPodsPerZone[zId][2] == 1)
            {
                w._currentZoneStatus[zId] = aCurrentZoneStatus;
                w._currentPodsPerZone[zId][0] = aPods[0];
                w._currentPodsPerZone[zId][1] = aPods[1];
                if(w.platSrc[zId] != w._currentPodsPerZone[zId][3])
                {
                    w.platSrc[zId] = w._currentPodsPerZone[zId][3];
                    aNewPlatSrcDiscovered = true;
                }
                w._lastDiscoveredZone[zId] = w._turnCount;
            }
            
            if(w._currentZoneStatus[zId] == -1 && w.platSrc[zId] > 0)
            {
                aFreeZoneWithPlat.push_back(zId);
            }
            w._targetedZone[zId] = false;
            if(w._turnCount == 1)
            {
                if(w._currentZoneStatus[zId] == w.myId)
                {
                    w._myBase = zId;
                }
                else if(w._currentZoneStatus[zId] != -1)
                {
                    w._enemyBase = zId;
                }
            }
        }

        if(w._turnCount == 1)
        {
             w.findLimitZones();
             if(w._enemyBase + w._myBase == w.zoneCount - 1 )
             {
                 w._symetrical = true;
             }
        }
        
        if(w._symetrical) // synchronize platinum source without visiting
        {
            for (int i = 0; i < zoneCount; i++)
            {
                if(w.platSrc[i] > 0 && w.platSrc[w.zoneCount - 1 - i] == 0)
                {
                    w.platSrc[w.zoneCount - 1 - i] = w.platSrc[i];
                }
            }
        }

        if(aNewPlatSrcDiscovered)
        {
            w.computeStaticUtility();
            w.sortByStaticUtility( w._orderedIdByStaticUtility);
        }

        if(w._turnCount == 1){
           cerr << "my base : " << w._myBase << " and enemey base : " << w._enemyBase << endl; 
        }

        w.buildGlobalStat();
        w.computeStrategy();


        list<SpawnCommand> aSpawnCommandLst;
        list<MoveCommand> aMoveCommandLst;
        
        if(w._distanceMap[w._myBase][w._enemyBase] < 7)
        {
            //basic spawn everything
            SpawnCommand aSC;
            aSC._zId = 0; // this is always my zone
            aSC._pods = platinum / 20;
            aSpawnCommandLst.push_back(aSC);
            
            w.sendAllToEnemyBase(aMoveCommandLst);
        }
        else if(true)
        {
            // build defence startegy
            // todo : tag the pod not to move ! + use move to optimize defense
            checkAndBuildDefense(w, platinum,  aSpawnCommandLst, aMoveCommandLst);
            
            // how many platinium to spawn on free spot
            int aUsedPlatinum = 0;
            for(auto& aSC: aSpawnCommandLst)
            {
                aUsedPlatinum += aSC._pods * 20;
            }
            int aRemainPlat = platinum - aUsedPlatinum;
            
            //if(aFreeZoneWithPlat.empty())
            //if(w._globalStrategy == Attack)
            if(false)
            {
            //w.sendAllToEnemyBase(aMoveCommandLst);
            //Attack !
            applyAggressiveStrategy(w, aRemainPlat, aSpawnCommandLst, aMoveCommandLst);
            
            //how many free pod to move -> all not not to move
            moveToClosestFreeOrEnemyZone(w, aMoveCommandLst);
            }
            else
            {
                
            //how many free pod to move -> all not not to move
            moveToClosestFreeOrEnemyZone(w, aMoveCommandLst);
            
            //Attack !
            //applyAggressiveStrategy(w, aRemainPlat, aSpawnCommandLst, aMoveCommandLst);
            w.sendAllToEnemyBase(aMoveCommandLst);
            
            }
            //w.sendAllToEnemyBase(aMoveCommandLst);
            
        }
        
        ostringstream aMovCommand;
        ostringstream aSpawnCommand;
        
        if(!aSpawnCommandLst.empty())
        {
            aSpawnCommand.clear();
            generateSpawnCommand(aSpawnCommandLst, aSpawnCommand);
        }
        
        if(!aMoveCommandLst.empty())
        {
            aMovCommand.clear();
            generateMoveCommand(aMoveCommandLst, aMovCommand);
        }
        
        if(aMovCommand.str().empty())
        {
            aMovCommand << "WAIT";
        }
        if(aSpawnCommand.str().empty())
        {
            aSpawnCommand << "WAIT";
        }
        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        cout << aMovCommand.str() << endl; // first line for movement commands, second line for POD purchase (see the protocol in the statement for details)
        cout << aSpawnCommand.str() << endl;
        
        ++w._turnCount;
        
        // save current status
        w._previousZoneStatus = w._currentZoneStatus;
        w._previousPodsPerZone = w._currentPodsPerZone;
        w._previousPlat = w._myPlat;
        // update current status with our own move
        
        cerr << "execution time:" << float( clock () - loop_time ) * 1000 /  CLOCKS_PER_SEC << "ms" << endl;
    }
}

void World::sendAllToEnemyBase(list<MoveCommand>& oMoveCommandLst)
{
    //send all pods to enemy base
    list<MoveCommand> aMoveableList;
    computeMoveablePods(*this, oMoveCommandLst, aMoveableList);
    for (auto& aMCit : aMoveableList) {
        // find zone to get closer to enemy zone
        for(auto aId : _linksSet[aMCit._zIdOrg])
        {
            if(_distanceMap[_enemyBase][aId] == _distanceMap[_enemyBase][aMCit._zIdOrg] - 1)
            {
                aMCit._zIdDest = aId;
            }
        }
    }
    oMoveCommandLst.insert(oMoveCommandLst.end(),aMoveableList.begin(),aMoveableList.end() );
}

void World::computeStrategy()
{
    //first global strategy
    int aMyZone = 0, aEnemyZone = 0;
    bool aFreeZoneWithPlat = false;
    for (int i = 0; i < zoneCount; ++i) {
        int aZoneStatus = _currentZoneStatus[i]; 
        if(aZoneStatus == myId)
        {
            aMyZone++;
        }
        else if (aZoneStatus != -1)
        {
            aEnemyZone++;
        }
        else if(platSrc[i] > 0)
        {
            aFreeZoneWithPlat = true;
        }
    }
    
    _globalStrategy = Attack;
    if(aFreeZoneWithPlat && _myPlat <= _enemyPlat && aMyZone <= aEnemyZone)
    {
        _globalStrategy = Explore;
    }
    
// per subArea Strategy
    for (auto aSubArea : _subAreaSets) {
        int aMyZone = 0, aEnemyZone = 0, aMyPods = 0, aEnemyPods = 0;
        bool aFreeZoneWithPlat = false;
        for (auto aZId : aSubArea.second) {
            int aZoneStatus = _currentZoneStatus[aZId]; 
            if(aZoneStatus == myId)
            {
                aMyZone++;
                aMyPods += _currentPodsPerZone[aZId][aZoneStatus];
            }
            else if (aZoneStatus != -1)
            {
                aEnemyZone++;
                aEnemyPods += _currentPodsPerZone[aZId][aZoneStatus];
            }
            else if(platSrc[aZId] > 0)
            {
                aFreeZoneWithPlat = true;
            }
        }
        
        StrategyType aStrategy = NoStrategy;
        if(aFreeZoneWithPlat)
        {
            if (_myPlat <= _enemyPlat && aMyZone <= aEnemyZone)
            {
                aStrategy = Explore;
            }
        }
        if(aMyPods > aEnemyPods)
        {
            aStrategy = Attack;
        }
        else if(aMyPods == aEnemyPods)
        {
            aStrategy = Defend;
        }
        else
        {
            aStrategy = Flew;
        }
        _perSubAreaStrategy[aSubArea.first] = aStrategy;
    }
    
}

void World::findSubArea(int iStartingGate,const list<int>& iExitGroup, set<int>& iSubArea, vector<int>& ioExploredIds)
{
    if(ioExploredIds[iStartingGate] != -1 && ioExploredIds[iExitGroup.front()] != -1)
    {
        return;
    }
    // so we start at iStartingGate, with zone in ExitGroup as a begining, and we fill the subarea

    ioExploredIds[iStartingGate] = iStartingGate; // we wont explore the gate
    iSubArea.insert(iStartingGate);
    list<int> aZonesToExplore;
    aZonesToExplore.push_back(iExitGroup.front()); // the zone to explore start with one of the exit group

    // explore
    while(!aZonesToExplore.empty())
    {
        int aCurZone = aZonesToExplore.front();
        aZonesToExplore.pop_front();
        
        iSubArea.insert(aCurZone);
        ioExploredIds[aCurZone] = iStartingGate;
        // get neighbour to explore
        for (auto aNextId : _linksSet[aCurZone]) 
        {
            if(ioExploredIds[aNextId] == -1)
            {
                if(_subAreaGates.find(aNextId) == _subAreaGates.end()) // a linked zone not yet explored and not a gate
                {
                    //cerr << "explore from:" << j << " from " << aCurZone << endl;
                    aZonesToExplore.push_back(aNextId);
                }
                else
                {
                    iSubArea.insert(aNextId); // include the gate
                }
                ioExploredIds[aNextId] = iStartingGate;
            }
        }
    }
}

void World::findGateways()
{
    cerr << "findGateways" << endl;
// gateways are zone which can separate 2 sub-area. So they have at least 2 non adjacent neighbour zone, max 4.
    for (auto aZId : _orderedIdByPlat) {
        if(_linksSet[aZId].size() >= 2 && _linksSet[aZId].size() <= 4)
        {
            list<list<int>> aZio; // group by adjacent neighbour zone
            map<int, bool> aGroupedZone;
            for (auto aNextId : _linksSet[aZId]) {
                if(aGroupedZone.find(aNextId) == aGroupedZone.end() )
                {
                    aGroupedZone[aNextId] = true;
                    list<int> aGroup;
                    aGroup.push_back(aNextId);
                    for (auto aGroupId : aGroup) {
                    
                        for (auto aOtherNextId : _linksSet[aZId]) {
                            if(aGroupedZone.find(aOtherNextId) == aGroupedZone.end() )
                            {
                                if(aLinks[aGroupId][aOtherNextId])
                                {
                                    aGroupedZone[aOtherNextId] = true;
                                    aGroup.push_back(aOtherNextId);
                                }
                            }
                        }
                    }
                    if(aGroup.size() == 1)
                    {
                        if(_linksSet[*aGroup.begin()].size() > 1)
                        {
                            aZio.push_back(aGroup);
                        }
                    }
                    else
                    {
                        aZio.push_back(aGroup);
                    }
                }
            }
            if(aZio.size() >= 2)
            {
                _gateways[aZId].swap(aZio); 
            }
        }
    }
    
    for (auto aGate : _gateways) 
    {
        //cerr << "Gateway " << aGate.first << " with " << aGate.second.size() << " exit" << endl;
        //check that one exit isn't a gateway
        
            
        for (auto aExitGroup : aGate.second) {
            bool aOnlyNonGateway = true;
            for (auto aGroupId : aExitGroup) {
                if(_gateways.find(aGroupId) != _gateways.end())
                {
                    aOnlyNonGateway = false;
                    break;
                }
            }
        
            if(aOnlyNonGateway)
            {
                _subAreaGates.insert(aGate.first);
                
            }
        }
    }
    
    vector<int> aExploredIds(zoneCount, -1);
    for (auto aZId : _subAreaGates) 
    {
        //cerr << "aSubArea start at zone " << aZId << " contains:" << endl;
        for (auto aExitGroup : _gateways[aZId]) {
            set<int> aSubArea;
            findSubArea(aZId, aExitGroup, aSubArea, aExploredIds);
            if(aSubArea.size() > 0)
            {
                _subAreaSets[aExitGroup.front()].swap(aSubArea);
            }
        }
    }
    
    cerr << "found " << _subAreaSets.size() << " subArea(s)" << endl;
    for(auto aSub: _subAreaSets)
    {
        cerr << "subArea " << aSub.first << endl;
        for(auto aZId: aSub.second)
        {
            cerr << " " << aZId;
        }
        cerr << endl;
    }
}

void World::subPropagateStatic(int iStaticPotential,int iInd, map<int, int>& ioAddPot)
{
    int aStaticPotential = iStaticPotential - 2;
    if(ioAddPot.find(iInd) != ioAddPot.end())
    {
        ioAddPot[iInd] += iStaticPotential;
    }
    else
    {
        ioAddPot[iInd] = iStaticPotential;
    }
    if(aStaticPotential > 0)
    {
        for(int i = 0; i < zoneCount; ++i)
        {
            if(aLinks[iInd][i])
            {
                subPropagateStatic(aStaticPotential, i, ioAddPot);
            }
        }
    }
}

void World::buildGlobalStat()
{
    vector<int> aTotalPod(playerCount,0);
    vector<int> aTotalZone(playerCount,0);
    for (auto aZoneId : _orderedIdByPlat) {
        for(int i=0; i < playerCount; ++i)
        {
            aTotalPod[i] += _currentPodsPerZone[aZoneId][i];
            if(_currentZoneStatus[aZoneId] == i)
            {
                aTotalZone[i] ++;
            }
        }
    }
    
    // get last
    int aMinZone = zoneCount;
    int aMinPlayerId = -1;
    for(int i=0; i < playerCount; ++i)
    {
        if(aTotalZone[i] < aMinZone)
        {
            aMinZone = aTotalZone[i];
            aMinPlayerId = i;
        }
    }
    
    if(aMinPlayerId == myId)
    {
        _iamLast = true;
    }
    else
    {
         _iamLast = false;
    }
    
    // check it is a sparse world (lots of 1 plat case)
    vector<int> aPltSrcStats(7,0);
    int aSrcNum = 0;
    for (int i = 0; i < zoneCount; i++) {
        aPltSrcStats[platSrc[i]]++;
        if(platSrc[i] > 1)
        {
            aSrcNum++;
        }
    }
    
    cerr << "number of 1 platinum src " << aSrcNum << endl;
    
    if( aSrcNum <= platSrc[1] )
    {
        _isSparse = true;
    }
}

void World::computeDistanceMap()
{
    // first set to 0 all reachable nodes ... // -1 in distanceMap means unreachable.
    for (auto aIdOrg : _orderedIdByPlat) {
        for (auto aIdDest : _orderedIdByPlat) {
            _distanceMap[aIdOrg][aIdDest] = 0;
        }
    }

    // then recurcively check each zone
    for (auto aIdOrg : _orderedIdByPlat) {
        // first we will look only in IDs of the continent for the current zone, where we don't have calculated distance yet
        int aCurrentDist = 0;
        set<int> aPreviousCircle;
        aPreviousCircle.insert(aIdOrg);
        set<int> aCurrentCircle = _linksSet[aIdOrg];
        while(!aCurrentCircle.empty())
        {
            aCurrentDist++;
            set<int> aNextCircle;
            for(auto aId : aCurrentCircle)
            {
                _distanceMap[aIdOrg][aId] = aCurrentDist;
                aNextCircle.insert(_linksSet[aId].begin(), _linksSet[aId].end());
            }
            // compute next circle
            for(auto aId : aPreviousCircle)
            {
                aNextCircle.erase(aId);
            }
            for(auto aId : aCurrentCircle)
            {
                aNextCircle.erase(aId);
            }
            aPreviousCircle.swap(aCurrentCircle);
            aCurrentCircle.swap(aNextCircle);
        }
    }
}

void World::computeStaticUtility()
{
    for(int zId=0; zId < zoneCount; ++zId)
    {
        // get best neighbour according Lvl 1 utility
        int aMax = 0;
        int aMaxId = -1;
        for(auto aNextId: _linksSet[zId])
        {
            if(platSrc[aNextId] > aMax)
            {
                aMax = platSrc[aNextId];
                aMaxId = aNextId;
            }
        }
        if(aMaxId >= 0)
        {
            _staticUtility[zId] = 2 * platSrc[zId] + platSrc[aMaxId];
        }
        else
        {
            _staticUtility[zId] = 2 * platSrc[zId];
        }
        
        //cerr << " " << _staticUtility[zId] << endl;
    }
}

void World::findLimitZones() // find the zone at equal distance beetween 2 bases
{
    list<pair<int,int>> aLimitZone; //index / distance
    for(int i = 0; i < zoneCount; ++i)
    {
        if(_distanceMap[_myBase][i] == _distanceMap[_enemyBase][i] 
        || _distanceMap[_myBase][i] == _distanceMap[_enemyBase][i] + 1)
        {
            aLimitZone.push_back(make_pair(i, _distanceMap[_myBase][i]));
        }
    }
    for(auto aZidPair:aLimitZone)
    {
        cerr << "zoneLimit, dist : " << aZidPair.first << "," << aZidPair.second << endl;
    }
}

template<class T> 
void World::sortByStaticUtility(list<T> &oList)
{
    auto aSortByUtility = [this](int a, int b) {
        return this->_staticUtility[b] < this->_staticUtility[a];   
    };
    
    oList.sort(aSortByUtility);
}

