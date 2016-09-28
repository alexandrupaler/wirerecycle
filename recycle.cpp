#include <stdio.h>
#include <vector>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <algorithm>
#include <climits>

#include "cnotcounter.h"
#include "gatenumbers.h"
#include "circuitmatrix.h"

#include "trim.h"
#include "recyclegate.h"

#include "cuccaro.h"
#include "circconvert.h"
#include "causalgraph.h"

#define PREFERLOWNUMBEREDWIRES true
#define PREFERANYWIRE false

using namespace std;

causalgraph causal;

set<int> allReachableAncillaOutputs;
map<int, set<int> > perInputReachedOutputs;

void writeGMLFile(const char* fname)
{
	FILE* fg = fopen(fname, "w");
	fprintf(fg, "graph [\n");
	fprintf(fg, "directed 1\n");

	for(vector<recyclegate>::iterator it=causal.circuit.begin(); it!=causal.circuit.end(); it++)
	{
		fprintf(fg, "node [\n");
		fprintf(fg, " id %d\n label \"%s\" \n", it->id, it->label.c_str());
		fprintf(fg, "]\n");
	}

	for(vector<recyclegate>::iterator it=causal.circuit.begin(); it!=causal.circuit.end(); it++)
	{
		for(set<int>::iterator it2 = it->willPush.begin(); it2 != it->willPush.end(); it2++)
		{
			fprintf(fg, "edge [\n");
			fprintf(fg, " source %d\n target %d \n", it->id, *it2);
			fprintf(fg, "]\n");
		}
	}

	fprintf(fg, "]");
	fclose(fg);
}

void initialiseSimQueue()
{
	allReachableAncillaOutputs.clear();
	perInputReachedOutputs.clear();

	allReachableAncillaOutputs.insert(causal.outAncillae.begin(), causal.outAncillae.end());

	for(int i=0; i<causal.getNrQubits(); i++)
	{
		if(causal.inAncillae.find(i) == causal.inAncillae.end())
			continue;

		set<int> vis;
		set<int> out;
		causal.reachOutputs(vis, out, i);

		perInputReachedOutputs[i] = out;
	}
}

void updateSimQueue(int outputId, set<int>& receivedOutputs)
{
	//this is not an output any more
	allReachableAncillaOutputs.erase(outputId);

	for(map<int, set<int> >::iterator it = perInputReachedOutputs.begin(); it!= perInputReachedOutputs.end(); it++)
	{
		//if from this input the specified output was found
		if(it->second.find(outputId) != it->second.end())
		{
			//this is not an output any more
			it->second.erase(outputId);
			it->second.insert(receivedOutputs.begin(), receivedOutputs.end());
		}
	}
}

vector<vector<int> > sortInputsForICM()
{
	vector<vector<int> > ret;
	for(int i=0; i<causal.getNrQubits(); i++)
	{
		if(causal.inAncillae.find(i) == causal.inAncillae.end())
			continue;

		if(perInputReachedOutputs.find(i) == perInputReachedOutputs.end())
					continue;

		vector<int> el(3);
		el[0] = i;
		el[1] = i;
		el[2] = i;
		ret.push_back(el);
	}

	return ret;
}

vector<vector<int> > sortInputs()
{
//	//use priority queue?
	vector<vector<int> > ret;

	for(map<int, set<int> >::iterator it = perInputReachedOutputs.begin(); it!= perInputReachedOutputs.end(); it++)
	{
		vector<int> el(3);
		el[0] = it->first;
		el[1] = causal.circuit[it->first].level;
		el[2] = allReachableAncillaOutputs.size() - it->second.size();

		ret.push_back(el);
	}

	if(ret.size() == 0)
		return ret;

	//bubble sort for the moment...should be replaced with something better
	for(int i=0; i<ret.size() - 1; i++)
	{
		for(int j=i+1; j<ret.size(); j++)
		{
			if((ret[i][1] < ret[j][1]) || (ret[i][1] == ret[j][1] && causal.circuit[ret[i][0]].wires[0] < causal.circuit[ret[j][0]].wires[0]))
			{
				vector<int> x = ret[i];
				ret[i] = ret[j];
				ret[j] = x;
			}
		}
	}

	return ret;
}

int chooseClosestAncillaOutputWire(int inputId, bool preferLowNumberedWires)
{
	int ret = -1;

	int inputWire = causal.circuit[inputId].wires[0];
	int dif = INT_MAX;

	for(set<int>::iterator it = allReachableAncillaOutputs.begin(); it != allReachableAncillaOutputs.end(); it++)
	{
		if(perInputReachedOutputs[inputId].find(*it) == perInputReachedOutputs[inputId].end())
		{
			//not in reachable
			int wire = causal.circuit[*it].wires[0];
			int ndif = abs(inputWire - wire);

			//prefer only wires which are higher than the inputwire
			if(preferLowNumberedWires && wire > inputWire)
				continue;

			if(ndif < dif)
			{
				dif = ndif;
				ret = *it;
			}
		}
	}

	return ret;
}

void outputGraph(int& nrConnect)
{
	causal.computeLevels();
	causal.updateLabels();
	char fname[1024];
	sprintf(fname, "circuit%03d.raw.gml", nrConnect);
	writeGMLFile(fname);
}

/*
 * Method M1 from paper
 */
int recycleUsingWireSequence()
{
	initialiseSimQueue();

	vector<vector<int> > sortedin = sortInputsForICM();

	int nrConnect = 0;
	while(sortedin.size() > 0)
	{
		int inputId = sortedin[0][0];

		if(sortedin[0][2] == 0)
		{
			perInputReachedOutputs.erase(inputId);
			sortedin = sortInputsForICM();
			continue;
		}

		int outputId = chooseClosestAncillaOutputWire(inputId, PREFERLOWNUMBEREDWIRES);
		if(outputId != -1)
		{
			nrConnect++;
			updateSimQueue(outputId, perInputReachedOutputs[inputId]);

			causal.connectNodes(outputId, inputId);

			causal.circuit[outputId].type = CONNOUT;//temp value, todo
			causal.circuit[inputId].type = CONNIN;

			set<int> vis;
			causal.replaceQubitIndex(vis, outputId, causal.circuit[inputId].wires[0], causal.circuit[outputId].wires[0]);

			//outputGraph(nrConnect);
		}

		perInputReachedOutputs.erase(inputId);

		sortedin = sortInputsForICM();
	}

	return nrConnect;
}

/*
 * Method M2 from paper
 */
int recycleUsingSearch()
{
	initialiseSimQueue();

	vector<vector<int> > sortedin = sortInputs();

	int nrConnect = 0;
	while(sortedin.size() > 0)
	{
		int inputId = sortedin[0][0];

		if(sortedin[0][2] == 0)
		{
			perInputReachedOutputs.erase(inputId);
			sortedin = sortInputs();
			continue;
		}

		set<int> visited;
		vector<int> path;
		//set<int> outputs;
		vector<int> shortest;
		causal.findShortestPath(visited, perInputReachedOutputs[inputId], path, shortest, false, -1, inputId);

		if(shortest.size() > 0)
		{
			nrConnect++;
			//printf("The results...\n");
			int outputId = causal.moveInputAfterOutput(shortest, inputId);

			updateSimQueue(outputId, perInputReachedOutputs[inputId]);

			outputGraph(nrConnect);
		}

		perInputReachedOutputs.erase(inputId);

		sortedin = sortInputs();
	}

	return nrConnect;
}

vector<vector<int> > extractSortedInputsList()
{
	vector < vector<int> > remainingIns;
	for (int i = 0; i < causal.nrQubits; i++)
	{
		//if (causal.circuit[i].type == -1 || causal.circuit[i].type == INPUT)
		if(gatenumbers::getInstance()->isInitialisationNumberButNotConnection(causal.circuit[i].type))
		{
			vector<int> el(2);
			el[0] = i;
			el[1] = causal.circuit[i].wires[0];
			remainingIns.push_back(el);
		}
	}

	//sort by wire
	for (int i = 0; i < remainingIns.size() - 1; i++)
	{
		for (int j = i + 1; j < remainingIns.size(); j++)
		{
			if (remainingIns[i][1] > remainingIns[j][1])
			{
				vector<int> s = remainingIns[i];
				remainingIns[i] = remainingIns[j];
				remainingIns[j] = s;
			}
		}

	}

	return remainingIns;
}

circuitmatrix createCircuitMatrix(vector<vector<int> >& remainingIns)
{
	//construct a vector of qubitlines
	int length = causal.getMaxLevel() + 1;
	vector<qubitline> temp;
	for (int k = 0; k < remainingIns.size(); k++)
	{
		//hardcoded length
		qubitline wireQubit(length, WIRE);
		temp.push_back(wireQubit);
	}

	//use the temp vector to initialise the circuitmatrix
	circuitmatrix circ(temp);

	//construct the circuitmatrix from the causalgraph
	//for the ICM representation it should contain only cnot gates
	cnotcounter ccounter;
	for(vector<recyclegate>::iterator it = causal.circuit.begin(); it != causal.circuit.end(); it++)
	{
		int column = it->level;
		int controlline = it->wires[0];
		int operation = it->type;

		if(it->wires.size() > 1)
		{
			int cnot = ccounter.getNextCnotNumber();

			printf("%d \n", cnot);

			int targetline = it->wires[1];
			circ[controlline][column] = cnot + 0;//+0 because control
			circ[targetline][column] = cnot + 1;//+1 because target
		}
		else if(it->wires.size() == 1)
		{
			circ[controlline][column] = operation;
		}
	}

	return circ;
}

void renameWires(vector<vector<int> >& remainingIns)
{
	//receives a sorted list, so renaming should not be problematic
	//rename wires
	for (int k = 0; k < remainingIns.size(); k++)
	{
		set<int> v;//empty set of visited nodes
		causal.replaceQubitIndex(v, remainingIns[k][0], remainingIns[k][1]/*old value*/, k/*new value*/);
//		printf("repl %d %d\n", remainingIns[k][1], k);
	}
}

int main(int argc, char** argv)
{
	int cuccbits = atoi(argv[2]);

	vector<string> file;

	if(cuccbits == 0)
	{
		file = circconvert::readFile(argv[1]);
	}
	else
	{
		cuccaro cucc;
		file = cucc.makeCircuit(cuccbits, 0);
	}

	circconvert convert(file);

	int decomposeICM = atoi(argv[3]);
	if(decomposeICM == 1)
	{
		convert.replaceNonICM();
		convert.replaceNonICM();
		convert.replaceICM();
	}

	causal.constructFrom(convert);

	causal.computeLevels();


	int nrConnect = 0;
	int method = atoi(argv[4]);
	if(method == 0)
		nrConnect = recycleUsingWireSequence();
	else
		nrConnect = recycleUsingSearch();

	//write gml file to be visualised using yEd for example
	//writeGMLFile("circuit.raw.gml");

	causal.computeLevels();

	vector<vector<int> > remainingIns = extractSortedInputsList();
	renameWires (remainingIns);

	circuitmatrix circ  = createCircuitMatrix(remainingIns);
	circ.printCirc();

	return 1;
}
