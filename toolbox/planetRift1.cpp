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


class Continent: public list<int>
{
    public:
    Continent():_isSaturated(false), _isLost(false), _platinumToSpend(0), _platinumSrc(0), _totalPods(4), _totalPlat(4), _totalZone(4), _sumOtherPods(0), _freePlat(0), _pltSrcNum(0) {};
    bool _isSaturated; 
    bool _isLost; // no free space nor my Id present
    int _platinumToSpend;
    int _platinumSrc;
    vector<int> _totalPods;
    vector<int> _totalPlat;
    vector<int> _totalZone;
    int _sumOtherPods;
    int _freePlat;
    int _pltSrcNum;
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
    platSrc(iZoneCount,0),
    aLinks(iZoneCount, vector<bool>(iZoneCount,false)),
    _linksSet(iZoneCount),
    aNumLinks(iZoneCount,0),
    _currentPodsPerZone(iZoneCount, vector<int>(4,0)),
    _previousPodsPerZone(iZoneCount, vector<int>(4,0)),
    _currentZoneStatus(iZoneCount,-1),
    _previousZoneStatus(iZoneCount,-1),
    _distanceMap(iZoneCount, vector<int>(iZoneCount,-1)),
    _staticUtility(iZoneCount, 0),
    _oneHasntSpawned(false),
    _isSparse(false) {}
    
    void findContinent();
    void checkContinentsAreSaturated(const vector<int>& iZoneStatus);
    void subPropagateStatic(int iStaticPotential,int i, map<int, int>& ioAddPot);
    void buildGlobalStat();
    void buildStatPerContinent();
    void computeDistanceMap();
    void computeStaticUtility();
    template<class T> 
    void sortByStaticUtility(list<T> &oList);

    int playerCount; // the amount of players (2 to 4)
    int myId; // my player ID (0, 1, 2 or 3)
    int zoneCount; // the amount of zones on the map
    int linkCount; // the amount of links between all zones
    int _totalPltSrc;
    vector<int> platSrc;
    vector<vector<bool> > aLinks;
    vector<set<int> > _linksSet;
    vector<int> aNumLinks;
    list<Continent> aContinentLst;
    vector<vector<int> > _currentPodsPerZone;
    vector<vector<int> > _previousPodsPerZone;
    vector<int> _currentZoneStatus;
    vector<int> _previousZoneStatus;
    vector<vector<int> > _distanceMap;
    list<int> _orderedIdByPlat;
    vector<int> _staticUtility; // LvL2 , LVL1 is platSrc ...
    list<int> _orderedIdByStaticUtility;
    
    bool _oneHasntSpawned;
    bool _isSparse;
    bool _iamLast;
};


void distributePlatinum(World& iW, int iTotalPlatinum, bool iFirstTurn)
{
    
    for (auto& aCont : iW.aContinentLst) {
            aCont._platinumToSpend = 0;
        }
    
    if(iFirstTurn) // at least one per continent if some platinum on it
    {
        for (auto& aCont : iW.aContinentLst) 
        {
            if(aCont._platinumSrc > 0)
            {
                aCont._platinumToSpend += 20;
                iTotalPlatinum -= 20;
            }
        }
    }
    
    while(iTotalPlatinum >= 20)
    {
        for (auto& aCont : iW.aContinentLst) {
            int random_variable = std::rand() % iW.zoneCount;
            if(!aCont._isSaturated && !aCont._isLost)
            {
                if(aCont.size() > random_variable)
                {
                    aCont._platinumToSpend += 20;
                    //cerr << "add 20 to continent " << aCont.size() << endl;
                    iTotalPlatinum -= 20;
                    if(iTotalPlatinum < 20)
                    {
                        break;
                    }
                }
            }
        }
        
    }
}



class SpawnCommand
{
    public:
    SpawnCommand():_zId(0), _pods(0), _numPlat(0), _utility(0) {};
    
    int _zId; // where to spawn
    int _pods; // how many pods
    int _numPlat; // number of platinum on this site
    int _utility; // the utility of this Spawn for prioritizing
};

class MoveCommand
{
    public:
    MoveCommand():_zIdOrg(0),_zIdDest(0), _pods(0), _utility(0) {};
    
    int _zIdOrg; // from where to move
    int _zIdDest; // to where
    int _pods; // how many pods
    int _utility; // the utility of this Move for prioritizing
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


void firstSpawn(World& iW, int iTotalPlatinum, list<SpawnCommand>& oSpawnCommandLst)
{
    
    // how many pod ?
    int aPodToDistribute = iTotalPlatinum / 20;
    
    if(iW.playerCount == 4)
    {
        // antarctic rush
        // find the biggest of continent with size < aPodToDistribute
        int aMaxMinSize = 0;
        for (auto& aCont : iW.aContinentLst) 
        {
            if(aCont.size() <= aPodToDistribute && aCont.size() > aMaxMinSize)
            {
                aMaxMinSize = aCont.size();
            }
        }
        for (auto& aCont : iW.aContinentLst) 
        {
            if(aCont.size() == aMaxMinSize)
            {
                // here spawn on platinum 
                auto aId = aCont.begin();
                int aMinPodToDistrib = 4;
                while(aPodToDistribute > 0 && iW.platSrc[*aId] > 0 || aMinPodToDistrib > 0)
                {
                    SpawnCommand aSC;
                    aSC._pods = max( iW.platSrc[*aId] / 2 + 1, 1);
                    aSC._zId = *aId;
                    aSC._numPlat = iW.platSrc[*aId];
                    aSC._utility = iW._staticUtility[*aId];
                    oSpawnCommandLst.push_back(aSC);
                    aPodToDistribute -= aSC._pods;
                    aMinPodToDistrib -= aSC._pods;
                    aId++;
                    if(aId == aCont.end())
                    {
                        aId = aCont.begin();
                    }
                }
            }
        }
    }
    
    if(iW.playerCount >= 2)
    {
        // generate basic spawn
        // default look at all world
        
        list<int> aZidLstToConsider = iW._orderedIdByStaticUtility;
        
        auto sortByPltSrc = [](Continent& a, Continent& b) {
                return b._pltSrcNum < a._pltSrcNum;   
            };
        
        if(iW._isSparse) // concentrate on one continent with most platinum 
        {
            iW.aContinentLst.sort(sortByPltSrc);
            auto aCont = iW.aContinentLst.begin();
            aZidLstToConsider = *aCont;
        }
        else if(iW.playerCount > 2)
        {
             // limit to 2 continent with highest platinum src
             //iW.aContinentLst.sort(sortByPltSrc);
            if(iW.aContinentLst.size() >= 2)
            {
                auto aCont = iW.aContinentLst.begin();
                aZidLstToConsider = *aCont;
                aCont++;
                aZidLstToConsider.insert(aZidLstToConsider.end(), aCont->begin(), aCont->end());
                cerr << "aZidLstToConsider " << aZidLstToConsider.size() << endl; 
            }
        }
        iW.sortByStaticUtility(aZidLstToConsider);
        //auto aId = iW._orderedIdByStaticUtility.begin();
        auto aId = aZidLstToConsider.begin();
        while(aPodToDistribute > 0)
        {
            cerr << "id " << *aId << " utility " << iW._staticUtility[*aId] << endl;
            
            SpawnCommand aSC;
            aSC._pods = 1;
            aSC._zId = *aId;
            aSC._numPlat = iW.platSrc[*aId];
            aSC._utility = iW._staticUtility[*aId];
            oSpawnCommandLst.push_back(aSC);
            ++aId;
            --aPodToDistribute;
        }
        
        // analyse and update
        
        // if lowest < biggest / 2 -> spawn 2, if lowest < biggest / 3  -> spawn 3
        auto aBiggest = oSpawnCommandLst.begin();
        auto aLowest = oSpawnCommandLst.rbegin();
        while(aBiggest != oSpawnCommandLst.end())
        {
            if((aLowest->_numPlat <= aBiggest->_numPlat / 2 && aBiggest->_pods == 1)
                || (aLowest->_numPlat <= aBiggest->_numPlat / 3 && aBiggest->_pods <= 2))
            {
                aBiggest->_pods += aLowest->_pods;
                ++aLowest;
                oSpawnCommandLst.pop_back();
            }
            else
            {
                ++aBiggest;
            }
        }
        
        // randomize the lowest spawn if possible
        // so get all the lowest spawn command
        cerr << "got here list size :" << oSpawnCommandLst.size() << endl;
        SpawnCommand aLowestSC = oSpawnCommandLst.back();
        
        //looks for all potential zone with same utility$
        vector<int> aSameUtilityZone;
        for (auto aZId : iW._orderedIdByPlat) {
            if(iW._staticUtility[aZId] < aLowestSC._utility)
            {
                break; // don't need to go further
            }
            else if(iW._staticUtility[aZId] == aLowestSC._utility)
            {
                aSameUtilityZone.push_back(aZId);
            }
        }
        std::random_shuffle(aSameUtilityZone.begin(), aSameUtilityZone.end());
        auto aRandIdIt = aSameUtilityZone.begin();
        // then randomize all with the lowest utility
        for (auto& aSC : oSpawnCommandLst) {
            if(aSC._utility == aLowestSC._utility)
            {
                if(aRandIdIt == aSameUtilityZone.end())
                {
                    break;
                }
                aSC._zId = *aRandIdIt;
                ++aRandIdIt;
                
            }
        }
        return;
    }
}

void spawnOnFreePlatSrc(World& iW, int iTotalPlatinum, list<SpawnCommand>& oSpawnCommandLst, list<int>& iFreePlatSrcLst, list<MoveCommand> &ioMoveCommand, bool iNonNull)
{
    auto aId = iFreePlatSrcLst.begin();
    while(iTotalPlatinum >= 20 && aId != iFreePlatSrcLst.end())
    {
        if(iW.platSrc[*aId] > 0 || !iNonNull && iW.platSrc[*aId] == 0)
        {
            // don't spawn if free pod next -> it will probably used to move
            bool aSpawnHere = true;
            for(auto aNextId: iW._linksSet[*aId] )
            {
                if(iW._currentPodsPerZone[aNextId][iW.myId] > 0 )
                {
                    //cerr << "found pods near " << *aId << endl;
                    bool aIsFree = true;
                    // we have a pod,  is it free ?
                    for(auto& aMC:ioMoveCommand)
                    {
                        if(aMC._zIdOrg == aNextId)
                        {
                            if(aMC._pods >= iW._currentPodsPerZone[aNextId][iW.myId] )
                            {
                                aIsFree = false;
                            }
                            break; 
                        }
                    }
                    
                    if(aIsFree)
                    {
                        aSpawnHere = false;
                        //cerr << "don't spawn on " << *aId << endl;
                    }
                }
            }
            
            // TODO spawn several if surrounded by enemy ?
            if(aSpawnHere)
            {
                SpawnCommand aSC;
                aSC._pods = 1;
                aSC._zId = *aId;
                iTotalPlatinum -= 20;
                oSpawnCommandLst.push_back(aSC);
            }
        }
        ++aId;
    }
}

void computeMoveablePods(World& iW, const list<MoveCommand>& iMoveCommandLst, list<MoveCommand>& oMoveCommandLst)
{
for (int i = 0; i < iW.zoneCount; i++) {
        if(iW._currentPodsPerZone[i][iW.myId] >= 1)
        {
            // find not moveable with same origine in oMovecommandLst
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
                aSC._zId = aZinD._zId;
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

void lastChanceSpawn(World& iW, int iPlatinum,  list<SpawnCommand>& oSpawnList)
{
    //last chance spawn
    for (auto& aCont : iW.aContinentLst) 
    {
        int aCountAvailableSpawnZone = 0;
        int aLastZId = 0;
        for( auto& aZId: aCont)
        {
            if(iW._currentZoneStatus[aZId] == -1 || iW._currentZoneStatus[aZId] == iW.myId)
            {
                ++aCountAvailableSpawnZone;
                aLastZId = aZId;
            }
        }
        
        if(aCountAvailableSpawnZone == 1)
        {
            // spawn all we can or as much as enemy
            
            // enemy count
            int aAvailablePods = iPlatinum / 20;
            // spawn only if you can beat other or 4
            if(aAvailablePods >= 4 || aCont._sumOtherPods <= aAvailablePods)
            {
                int aSpawnNum = min(min(aCont._sumOtherPods, aAvailablePods),4);//useful to defend one turn
                if(aSpawnNum > 0)
                {
                    SpawnCommand aSC;
                    aSC._pods = aSpawnNum;
                    aSC._zId = aLastZId;
                    oSpawnList.push_back(aSC);
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
                int aPods = min(min(anAvailablePods,aThreatPerZone[aZId]),4);
                aSC._pods = aPods;
                anAvailablePods -= aPods;
                oSpawnList.push_back(aSC);
            }
        }
    }
        
    // move to attack
    //list<int> aMyPodCloseToEnemy;
    for (auto& aMCp : aMoveablePods) {
        for (auto aNextId : iW._linksSet[aMCp._zIdOrg]) 
        {
            if(iW._currentZoneStatus[aNextId] != iW.myId && iW._currentZoneStatus[aNextId] != -1)
            {
                aMCp._zIdDest = aNextId;
                oMoveCommandLst.push_back(aMCp);
                break;
            }
        }
    }
    
    // do we still have available Pods to spawn !! -> spawn closer to enemy zone
    if(anAvailablePods > 0)
    {
        // find enemy zone with freespace around
        map<int,list<int> > aFreespacesAround;
        for (int i = 0; i < iW.zoneCount; i++) {
            if(iW._currentZoneStatus[i] != iW.myId && iW._currentZoneStatus[i] != -1)
            {
                // an ennemy zone
                for (auto aNextId : iW._linksSet[i]) {
                    if(iW._currentZoneStatus[aNextId] == -1)
                    {
                        aFreespacesAround[i].push_back(aNextId);
                    }
                }
            }
        }
        
        //sort by utility
        list<int> aEnemyZone;
        for (auto& aPair : aFreespacesAround) {
            aEnemyZone.push_back(aPair.first);
        }
        iW.sortByStaticUtility(aEnemyZone);
        
        for (auto aEnId : aEnemyZone) {
            if(anAvailablePods > 0)
            {
                iW.sortByStaticUtility(aFreespacesAround[aEnId]);
                for (auto aFreeId : aFreespacesAround[aEnId]) {
                    if(anAvailablePods > 0)
                    {
                        SpawnCommand aSC;
                        aSC._zId = aFreeId;
                        aSC._pods = 1;
                        anAvailablePods -= 1;
                        oSpawnList.push_back(aSC);
                    }
                }
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
    vector<bool> aReachedZone(iW.zoneCount, false);
    // go to closer free zone
    for (auto& aMCit : aMoveablePods) {

        int aMinDistPlat = iW.zoneCount;
        int aMinDistFree = iW.zoneCount;
        int aMinId = -1;
        int aMinIdFree = -1;
        int aMaxUtility = 0;
        int aPods = aMCit._pods;
        // get non zero reachable utility
        int aPreviousPods = aPods;
        while(aPods > 0)
        {
            for(int j = 0; j < iW.zoneCount; j++)
            {
                if(iW._currentZoneStatus[j] != iW.myId) // freeplace or enemy place
                {
                    // is it reachable from i ?
                    //cerr << "Distance :" << iW._distanceMap[i][j] << endl;
                    if(iW._distanceMap[aMCit._zIdOrg][j] > 0 && iW.platSrc[j] > 0)
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
        
            if(aMinId != -1 && aMinId != aMCit._zIdOrg)
            {
                // we want to go from i to aMinId
                // find next move from i -> this dist - 1 in the reverse distance table ...
                int aNextMove = -1;
                int aNoneReachedZone = -1;
                for(auto aId : iW._linksSet[aMCit._zIdOrg])
                {
                    if(iW._distanceMap[aMinId][aId] == aMinDistPlat - 1 && !aReachedZone[aId])
                    {
                        aNextMove = aId;
                    }
                    else if(!aReachedZone[aId])
                    {
                        aNoneReachedZone = aId;
                    }
                }
                if(aNextMove != -1)
                {
                    aMCit._zIdDest = aNextMove;
                }
                else if(aNoneReachedZone != -1)
                {
                    aMCit._zIdDest = aNoneReachedZone;
                }
                else
                {
                    aMCit._zIdDest = aMinId;
                }
                aMCit._pods = 1;
                aPods--;
                aReachedZone[aMCit._zIdDest] = true;
                oMoveCommandLst.push_back(aMCit);
                //cerr << "find min move" << endl;
            }
            else if(aMinIdFree != -1 && aMinIdFree != aMCit._zIdOrg)
            {
                // we want to go from i to aMinId
                // find next move from i -> this dist - 1 in the reverse distance table ...
                int aNextMove = -1;
                int aNoneReachedZone = -1;
                for(auto aId : iW._linksSet[aMCit._zIdOrg])
                {
                    if(iW._distanceMap[aMinIdFree][aId] == aMinDistFree - 1 && !aReachedZone[aId])
                    {
                        aNextMove = aId;
                    }
                    else if(!aReachedZone[aId])
                    {
                        aNoneReachedZone = aId;
                    }
                }
                
                if(aNextMove != -1)
                {
                    aMCit._zIdDest = aNextMove;
                }
                else if(aNoneReachedZone != -1)
                {
                    aMCit._zIdDest = aNoneReachedZone;
                }
                else
                {
                    aMCit._zIdDest = aMinIdFree;
                }
                aMCit._pods = 1;
                aPods--;
                aReachedZone[aMCit._zIdDest] = true;
                oMoveCommandLst.push_back(aMCit);
                //cerr << "find min move" << endl;
            }
        
            // bloom the rest ...
            if(aPods >= 6)
            {
                while(aPods > 0)
                {
                    for(auto aId : iW._linksSet[aMCit._zIdOrg])
                    {
                        aMCit._zIdDest = aId;
                        aMCit._pods = 1;
                        aPods--;
                        aReachedZone[aMCit._zIdDest] = true;
                        oMoveCommandLst.push_back(aMCit);
                    }
                }
            }
            
            if (aPreviousPods == aPods)
            {
                aPods--;
                aPreviousPods = aPods;
            }
        }
    }
}


/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
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


    w.findContinent(); // now we can apply a per continent strategy

    auto aSortContByPlat = [](Continent a, Continent b) {
        return b._platinumSrc < a._platinumSrc;   
    };
    w.aContinentLst.sort(aSortContByPlat);

    auto aSortByPlat = [&w](int a, int b) {
        return w.platSrc[b] < w.platSrc[a];   
    };

    w._orderedIdByPlat.sort(aSortByPlat);
    for (auto& aCont : w.aContinentLst) {
        aCont.sort(aSortByPlat);
        w._totalPltSrc += aCont._platinumSrc;
        //cerr << "continent Size: " <<  aCont.size() << endl;
    }

    
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
    
    int aTurnCount = 1;
    
    while (1) {
        const clock_t loop_time = clock();
        int platinum; // my available Platinum
        cin >> platinum; cin.ignore();
        //vector<int> potential(aStaticPotential);
        list<int> aFreePlatSrcLst;
        list<int> aFreeZoneLst;
        w._oneHasntSpawned = false;
        for (int i = 0; i < zoneCount; i++) {
            int zId; // this zone's ID
            cin >> zId ;
            cin >> w._currentZoneStatus[zId] >> w._currentPodsPerZone[zId][0] >> w._currentPodsPerZone[zId][1] >> w._currentPodsPerZone[zId][2] >> w._currentPodsPerZone[zId][3]; cin.ignore();
            //cerr << _currentPodsPerZone[zId][0] << _currentPodsPerZone[zId][1] << _currentPodsPerZone[zId][2] << _currentPodsPerZone[zId][3] << endl;
            if(w._currentZoneStatus[zId] == -1)
            {
                aFreeZoneLst.push_back(zId);
                //if( w.platSrc[zId] > 0)
                {
                    aFreePlatSrcLst.push_back(zId);
                }
            }
        }


        w.checkContinentsAreSaturated(w._currentZoneStatus);
        w.buildGlobalStat();

        list<SpawnCommand> aSpawnCommandLst;
        list<MoveCommand> aMoveCommandLst;
        
        if(aTurnCount == 1)
        {
            firstSpawn(w, platinum, aSpawnCommandLst);
        }
        else if(aTurnCount == 2 && w._oneHasntSpawned)
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
            
            // don't move // don't spawn
            //Attack !
            applyAggressiveStrategy(w, aRemainPlat, aSpawnCommandLst, aMoveCommandLst);
        }
        else if(w._iamLast && w.playerCount > 2)
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
            
            // don't move // don't spawn
            //Attack !
            applyAggressiveStrategy(w, aRemainPlat, aSpawnCommandLst, aMoveCommandLst);
            
            //how many free pod to move -> all not not to move
            moveToClosestFreeOrEnemyZone(w, aMoveCommandLst);
        }
        else
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
            
            // lastChanceSpawn
            list<SpawnCommand> aLastChanceSpawnCom;
            lastChanceSpawn(w, aRemainPlat,  aLastChanceSpawnCom);
            //aSpawnCommandLst.splice(aSpawnCommandLst.end(),aLastChanceSpawnCom);
            
            //if(aFreeZoneLst.size() > 0 && aRemainPlat >= 20)
            // todo limit to continent where present in multiplayer game -> to defend.
            if(aFreePlatSrcLst.size() > 0 && aRemainPlat >= 20)
            {
                aFreePlatSrcLst.sort(aSortByPlat);
                bool aNonNul = true;
                spawnOnFreePlatSrc(w, aRemainPlat, aSpawnCommandLst, aFreePlatSrcLst, aMoveCommandLst,aNonNul);
                
                // aFreeZoneLst.sort(aSortByPlat);
                // spawnOnFreePlatSrc(w, aRemainPlat, aSpawnCommandLst, aFreeZoneLst, aMoveCommandLst);
            }
            
            
            // how many platinium to spawn on free spot
            aUsedPlatinum = 0;
            for(auto& aSC: aSpawnCommandLst)
            {
                aUsedPlatinum += aSC._pods * 20;
            }
            aRemainPlat = platinum - aUsedPlatinum;
            
            //Attack !
            applyAggressiveStrategy(w, aRemainPlat, aSpawnCommandLst, aMoveCommandLst);
            
            //how many free pod to move -> all not not to move
            moveToClosestFreeOrEnemyZone(w, aMoveCommandLst);
            
            aUsedPlatinum = 0;
            for(auto& aSC: aSpawnCommandLst)
            {
                aUsedPlatinum += aSC._pods * 20;
            }
            aRemainPlat = platinum - aUsedPlatinum;
            
            if(aFreePlatSrcLst.size() > 0 && aRemainPlat >= 20)
            {
                aFreePlatSrcLst.sort(aSortByPlat);
                bool aNonNul = false;
                spawnOnFreePlatSrc(w, aRemainPlat, aSpawnCommandLst, aFreePlatSrcLst, aMoveCommandLst,aNonNul);
                
                // aFreeZoneLst.sort(aSortByPlat);
                // spawnOnFreePlatSrc(w, aRemainPlat, aSpawnCommandLst, aFreeZoneLst, aMoveCommandLst);
            }
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
        
        ++aTurnCount;
        
        // save current status
        w._previousZoneStatus = w._currentZoneStatus;
        w._previousPodsPerZone = w._currentPodsPerZone;
        // update current status with our own move
        
        cerr << "execution time:" << float( clock () - loop_time ) * 1000 /  CLOCKS_PER_SEC << "ms" << endl;
    }
}

void World::findContinent()
{
    // find all continent 
    // a continent is a group of zone without any exterior links ...
    vector<int> aContinentIds(zoneCount, -1); // -1 means it isn't explored
    // explore all zone and assign them a continent
    int aContinentInd = 0;
    for(int i=0; i < zoneCount; ++i)
    {
        if(aContinentIds[i] == -1)
        {
            // begin a new continent
            Continent aContinent;
            //cerr << "findContinent begin, continentSize, id =" << aContinent.size() << ", " << aContinentInd << endl;
            // initialize exploration from zone i
            list<int> aZonesToExplore;
            aZonesToExplore.push_back(i);
            
            // explore
            while(!aZonesToExplore.empty())
            {
                int aCurZone = aZonesToExplore.front();
                aZonesToExplore.pop_front();
                
                //cerr << "add to continent:" << aCurZone << " to " << aContinentInd << endl;
                aContinent.push_back(aCurZone);
                aContinentIds[aCurZone] = aContinentInd;
                aContinent._platinumSrc += platSrc[aCurZone];
                // get neighbour to explore
                for(int j =0; j < zoneCount; ++j)
                {
                    if(aLinks[aCurZone][j] && aContinentIds[j] == -1) // a linked zone not yet explored
                    {
                        //cerr << "explore from:" << j << " from " << aCurZone << endl;
                        aZonesToExplore.push_back(j);
                        aContinentIds[j] = aContinentInd;
                    }
                }
            }
            
            
            // finished exploring the continent, saving it
            //cerr << "findContinent, continentSize, id =" << aContinent.size() << ", " << aContinentInd << endl;
            aContinentLst.push_back(aContinent);
            aContinentInd++;
        }
    }
}

void World::checkContinentsAreSaturated(const vector<int>& iZoneStatus)
{
    // loop per Continent
    for (auto& aContIt : aContinentLst) {
        if(!aContIt._isSaturated && !aContIt._isLost) // once saturated no need to check
        {
            bool aIsSaturated = true;
            bool aLost = true;
            for (auto& aId : aContIt) {
                if(iZoneStatus[aId] != myId)
                {
                    aIsSaturated = false;
                }
                
                if(iZoneStatus[aId] == -1 || iZoneStatus[aId] == myId)
                {
                    aLost = false;
                }
            }
            aContIt._isSaturated = aIsSaturated;
            aContIt._isLost = aLost;
        }
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

void World::buildStatPerContinent()
{
      // loop per Continent
    for (auto& aContIt : aContinentLst) {
        aContIt._sumOtherPods = 0;
        aContIt._freePlat = 0;
        aContIt._pltSrcNum = 0;
        for(auto& aPods: aContIt._totalPods)
        {
            aPods = 0;
        }
        for(auto& aPlat: aContIt._totalPlat)
        {
            aPlat = 0;
        }
        for(auto& aZone: aContIt._totalZone)
        {
            aZone = 0;
        }
        
        if(!aContIt._isSaturated && !aContIt._isLost) // once saturated no need to check
        {
            for (auto aId : aContIt) {
                if(_currentZoneStatus[aId] == -1)
                {
                    aContIt._freePlat += platSrc[aId];
                }
                else
                {
                    aContIt._totalPlat[_currentZoneStatus[aId]] += platSrc[aId];
                    aContIt._totalZone[_currentZoneStatus[aId]]++;
                }
                
                for(int i=0; i < playerCount; ++i)
                 {
                    aContIt._totalPods[i] += _currentPodsPerZone[aId][i];
                    if(i != myId)
                    {
                        aContIt._sumOtherPods += _currentPodsPerZone[aId][i];
                    }
                }
                
                if(platSrc[aId] > 0)
                {
                    aContIt._pltSrcNum++;
                }
            }
        }
    }  
}

void World::buildGlobalStat()
{
    buildStatPerContinent(); //
    
    vector<int> aTotalPod(playerCount,0);
    vector<int> aTotalZone(playerCount,0);
    for (auto& aContIt : aContinentLst) {
        for(int i=0; i < playerCount; ++i)
        {
            aTotalPod[i] += aContIt._totalPods[i];
            aTotalZone[i] += aContIt._totalZone[i];
        }
    }
    
    
    for(int i=0; i < playerCount; ++i)
    {
        if(i != myId && aTotalPod[i] == 0)
        {
            
            _oneHasntSpawned = true;
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
    for (auto& aContIt : aContinentLst) 
    {
        for (auto aIdOrg : aContIt) {
            for (auto aIdDest : aContIt) {
                _distanceMap[aIdOrg][aIdDest] = 0;
            }
        }
    }
    
    // then recurcively check each zone
    for (auto& aContIt : aContinentLst) 
    {
        for (auto aIdOrg : aContIt) {
            // first we will look only in IDs of the continent for the current zone, where we don't have calculated distance yet
            int aCurrentDist = 0;
            set<int> aPreviousCircle;
            aPreviousCircle.insert(aIdOrg);
            set<int> aCurrentCircle = _linksSet[aIdOrg];
            set<int> aTotalSetOfId;
            aTotalSetOfId.insert(aContIt.begin(), aContIt.end());
            
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

template<class T> 
void World::sortByStaticUtility(list<T> &oList)
{
    auto aSortByUtility = [this](int a, int b) {
        return this->_staticUtility[b] < this->_staticUtility[a];   
    };
    
    oList.sort(aSortByUtility);
}

