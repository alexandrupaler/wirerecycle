#ifndef CAUSAL_GRAPH_H__
#define CAUSAL_GRAPH_H__

#include <vector>
#include "circconvert.h"

using namespace std;

class causalgraph{
public:
	vector<recyclegate> circuit;
	vector<int> lastSeen;
	set<int> inAncillae;
	set<int> outAncillae;
	int nrQubits;

	int getNrQubits();
	int getMaxLevel();

	void constructFrom(circconvert& cc);
	void connectNodes(int previd, int currid);
	void updateLabels();

	void computeLevels();
	void equalizeLevels();

	void reachOutputs(set<int>& visited, set<int>& outputs, int curr);

//	void updateForwardHierarchy(set<int>& visited, int curr, int distance);
	//intr-un fel este identica cu functia din circconvert
	void replaceQubitIndex(set<int>& visited, int curr, int oldvalue, int newvalue);
//	void updateBackwardHierarchy(set<int>& visited, int curr, int distance);

	void findShortestPath(set<int>& visited, set<int>& outputs, vector<int> path, vector<int>& shortest, int stepback, int prev, int curr);

	int moveInputAfterOutput(vector<int> shortest, int inputId);

private:
	int getAndSetMaxPrevLevel(int& currid);
};

#endif
