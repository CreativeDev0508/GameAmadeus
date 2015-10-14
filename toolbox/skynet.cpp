#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <list>
#include <algorithm>

using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main()
{
    int N; // the total number of nodes in the level, including the gateways
    int L; // the number of links
    int E; // the number of exit gateways
    cin >> N >> L >> E; cin.ignore();
    bool Links[N][N];
    int nodeLinks[N];
    int nodeLinksToGate[N];
    bool isNodeGate[N];
    for (int i = 0; i < N; i++)
    {
        isNodeGate[i] = false;
        nodeLinks[i] = 0;
        nodeLinksToGate[i] = 0;
        for (int j = 0; j < N; j++)
            Links[i][j] = false;
    }
    for (int i = 0; i < L; i++) {
        int N1; // N1 and N2 defines a link between these nodes
        int N2;
        cin >> N1 >> N2; cin.ignore();
        Links[N1][N2] = true;
        Links[N2][N1] = true;
        nodeLinks[N1]++;
        nodeLinks[N2]++;
    }
    
    
    int Gates[E];
    int GatesLink[E];
    for (int i = 0; i < E; i++) {
        int EI; // the index of a gateway node
        cin >> EI; cin.ignore();
        isNodeGate[EI] = true;
        Gates[i] = EI;
        GatesLink[i]=0;
        for (int j = 0; j < N; j++) {
            if(Links[Gates[i]][j] == true)
             {
                GatesLink[i]++;
                nodeLinksToGate[j]++;
             }
        }
    }
    
    int S0 = 0;

    // find critical path : a suite of nodes with one link to a gateway, finished with a node with 2 links to a gateway
    
    list<set<int> > aCriticalPathLst;
    list<int> aDoubleLinkNodes;
    // first find nodes (note gateway) with 2 Links to Gateway
    for (int i = 0; i < N; i++) {
        if(!isNodeGate[i])
        {
            if(nodeLinksToGate[i] == 2)
            {
                aDoubleLinkNodes.push_back(i);
            }
        }
    }
    
    cerr << "DoubleLinkNodes :" << aDoubleLinkNodes.size() << endl;
    
    // find critical path
    for (auto iNd : aDoubleLinkNodes) {
        set<int> aCritPath; // in fact a group of node, where ever you start you'll find a path ...
        aCritPath.insert(iNd); // the double node is part of the path
        list<int> aNodeToCheckLst, aNextNodesToCheck; // we don't put gateways in it !
        aNodeToCheckLst.push_back(iNd);
        while(!aNodeToCheckLst.empty())
        {
            aNextNodesToCheck.clear();
            for (auto jNd : aNodeToCheckLst) {
                // get the node (not a gateway) connected to it and having one link to a gateway ?
                set<int> aConNdLst;

                for( int i = 0; i < N; ++i)
                {
                    if(Links[jNd][i] && !isNodeGate[i] && nodeLinksToGate[i] == 1)
                    {
                        // i is connected to jNd and i isn't a gate
                        // does i have a link to a gate ?
                        for (int g = 0; g < E; g++) {
                            if(Links[Gates[g]][i])
                            {
                                // yes ! so it is in the group
                                aConNdLst.insert(i);
                            }
                        }
                    }
                }

                
                // the connected valid Nodes are already in the group
                for (auto iConNd : aConNdLst) {
                    if(aCritPath.find(iConNd) == aCritPath.end())
                    {
                        aCritPath.insert(iConNd);
                        aNextNodesToCheck.push_back(iConNd);
                    }
                }
                
            }
            //transfer aNextNodesToCheck to aNodeToCheckLst
            aNodeToCheckLst.clear();
            aNodeToCheckLst.swap(aNextNodesToCheck);
        }
        aCriticalPathLst.push_back(aCritPath);
    }

    cerr << "Number of critical Path:" << aCriticalPathLst.size() << endl;
    for (auto iCritPath : aCriticalPathLst) {
        cerr << "CritPath size:" << iCritPath.size() << endl;
    }

    // game loop
    while (1) {
        int SI; // The index of the node on which the Skynet agent is positioned this turn
        cin >> SI; cin.ignore();

        // strategy is simple, if agent is close to gateways, cut the link
        // if not close, try to isolate agent ... not the last link he was in.

        int R1 = 0,R2 = 0; // the result
        int selectedGate = 0; // the selected Gate
        bool aCriticalPathSelection = false;
        
        // critical path selection if skynet is about to enter a critical path ...
        
        auto aDbLnkNd = aDoubleLinkNodes.begin();
        auto aCrtPath = aCriticalPathLst.begin();
        for (auto iCritPath : aCriticalPathLst) {
            
            // is skynet close to this critical path
            for (auto iNdCrit : iCritPath) {
                if(Links[SI][iNdCrit])
                {
                    aCriticalPathSelection = true;
                    R1 = *aDbLnkNd;
                    // find a gate from this Node
                    for (int j = 0; j < E; j++) {
                        if(Links[Gates[j]][R1] == true)
                        {
                            R2 = Gates[j];
                            selectedGate = j;
                        }
                    }
                    
                }
            }
            
            // aDoubleLinkNodes[i]; the double link to target
            if(aCriticalPathSelection) 
            {
                break; // we want to escape if we found a critical path
            }
            ++aDbLnkNd;
            ++aCrtPath;
        }
        
        // is there any danger ?
        bool danger = false;
        for (int i = 0; i < E; i++) {
            if(Links[Gates[i]][SI] == true || Links[SI][Gates[i]] == true)
            {
                danger = true;
            }
        }
        
        if (!danger && !aCriticalPathSelection && !aCriticalPathLst.empty()) // we are not in danger but we have critical path existing, just clean them
        {
            // take the first critical path you have
            aDbLnkNd = aDoubleLinkNodes.begin();
            aCrtPath = aCriticalPathLst.begin();
            aCriticalPathSelection = true;
            R1 = *aDbLnkNd;
            // find a gate from this Node
            for (int j = 0; j < E; j++) {
                if(Links[Gates[j]][R1] == true)
                {
                    R2 = Gates[j];
                    selectedGate = j;
                }
            }
        }
        
        if(aCriticalPathSelection) 
        {
            // this path isn't critical anymore
            aCriticalPathLst.erase(aCrtPath);
            aDoubleLinkNodes.erase(aDbLnkNd);
        }
        
        
        if(!aCriticalPathSelection) 
        { // standard strategy works
        
            // select a link from a gateway, take node with the most links to gateway and close to agent
            // max links to gateway
            int maxLinks = 0;
            int maxPrio = 0;
            bool urgent = false; // select only nodes connected to gateway
            
            for (int i = 0; i < N; i++)
            {
                int nbLinks = 0;
                int maxGateLink = 0;
                int currentGate = 0;
                for (int j = 0; j < E; j++) {
                        if(Links[Gates[j]][i] == true)
                        {
                            nbLinks++;
                            if(GatesLink[j] > maxGateLink)
                            {
                                currentGate = j;
                                maxGateLink = GatesLink[j];
                            }
                        }
                }
                int prio = 0;
                
                if(nbLinks > 0)
                {
                    if(nbLinks > maxLinks)
                    {
                        prio++;
                        maxLinks = nbLinks;
                    }
                    
                    bool isCloseToAgent = (Links[i][SI] == true || Links[SI][i] == true);
                    
                    if(isCloseToAgent)
                    {
                        prio ++; 
                    }
                    
                    if(prio > maxPrio)
                    {
                        R1 = i;
                        R2 = Gates[currentGate];
                        selectedGate = currentGate;
                    }
                
                }
                 
            }
            
            // is agent close to gateway ? -> select link to destroy
            
            for (int i = 0; i < E; i++) {
                if(Links[Gates[i]][SI] == true || Links[SI][Gates[i]] == true)
                {
                    R1 = Gates[i];
                    R2 = SI;
                    selectedGate = i;
                }
            }
        }


        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        Links[R1][R2] = false;
        Links[R2][R1] = false;
        nodeLinks[R1]--;
        nodeLinks[R2]--;
        GatesLink[selectedGate]--;
        
        S0 = SI;
        cout << R1 << " " << R2 << endl; // Example: 0 1 are the indices of the nodes you wish to sever the link between
    }
}
