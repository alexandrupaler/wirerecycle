#include <climits>
#include <cmath>
#include "gatenumbers.h"
#include "causalgraph.h"

void causalgraph::constructFrom(circconvert& cc)
{
	for(list<circgate>::iterator it=cc.gates.begin(); it!=cc.gates.end(); it++)
	{
		recyclegate gate(*it);
		circuit.push_back(gate);
	}

	nrQubits = cc.inputs.size();

	//introduce input and output nodes
	for(int i=0; i < cc.inputs.size(); i++)
	{
		recyclegate input;
		input.wires.push_back(i);

		if(cc.inputs[i] != '-')
		{
			inAncillae.insert(i);
			//input.type = -1;
			input.type = AA;//todo:temp
		}
		else
			input.type = INPUT;
		circuit.insert(circuit.begin() + i, input);

		recyclegate output;
		output.wires.push_back(i);

		if(cc.outputs[i] != '-')
		{
			outAncillae.insert(cc.inputs.size() + cc.gates.size() + i);
			//output.type = -2;
			output.type = MA;//todo: temp
		}
		else
			output.type = OUTPUT;
		circuit.insert(circuit.end(), output);
	}

	//set the id attribute of the gates
	for(int i=0; i<circuit.size(); i++)
	{
		circuit[i].id = i;
	}

	vector<int> lastSeen;
	for(int i=0; i < cc.inputs.size(); i++)
		lastSeen.push_back(i);//the ids of the inputs

	//skip the inputs
	for(vector<recyclegate>::iterator it = circuit.begin() + cc.inputs.size(); it != circuit.end(); it++)
	{
		for(vector<int>::iterator itw=it->wires.begin(); itw != it->wires.end(); itw++)
		{
			connectNodes(lastSeen[*itw], it->id);
			lastSeen[*itw] = it->id;
		}
	}
}

int causalgraph::getAndSetMaxPrevLevel(int& currid)
{
	//ia maximul de la vecinii care imping
	int maxPrevLevel = INT_MIN;
	for (set<int>::iterator it = circuit[currid].pushedBy.begin(); it != circuit[currid].pushedBy.end(); it++)
	{
		if(circuit[*it].level == -1)
		{
			int id = *it;
			int max = getAndSetMaxPrevLevel(id);
			//un mini setter ...
			circuit[*it].level = max + 1;
		}

		if (maxPrevLevel < circuit[*it].level)
		{
			maxPrevLevel = circuit[*it].level;
		}
	}

	return maxPrevLevel;
}

void causalgraph::connectNodes(int previd, int currid)
{
	circuit[previd].addWillPush(currid);
	circuit[currid].addPushedBy(previd);
}

void causalgraph::updateLabels()
{
	for (vector<recyclegate>::iterator it = circuit.begin(); it != circuit.end(); it++)
	{
		it->generateLabel();
	}
}

void causalgraph::equalizeLevels()
{
	for(vector<recyclegate>::reverse_iterator it = circuit.rbegin(); it != circuit.rend(); it++)
	{
		//vezi care este nivelul minim urmator
		int min = INT_MAX;
		for(set<int>::iterator it2 = it->willPush.begin(); it2 != it->willPush.end(); it2++)
		{
			if(min > circuit[*it2].level)
			{
				min = circuit[*it2].level;
			}
		}

		if(min != INT_MAX)
		{
			int dist = min - (it->level + 1);

			it->level += dist;
		}
	}
//	for(vector<recyclegate>::iterator it = circuit.begin(); it != circuit.end(); it++)
//	{
//		set<int> toUpdate;
//		toUpdate.insert(it->pushedBy.begin(), it->pushedBy.end());
//
//		while(toUpdate.size() > 0)
//		{
//			//printf("sz %d\n", toUpdate.size());
//
//			int maxPrevLevel = INT_MIN;
//			int idmax = -1;
//			for(set<int>::iterator it2 = toUpdate.begin(); it2 != toUpdate.end(); it2++)
//			{
//				if(maxPrevLevel < circuit[*it2].level)
//				{
//					maxPrevLevel = circuit[*it2].level;
//					idmax = *it2;
//				}
//			}
//
//			int dif = it->level - maxPrevLevel - 1;
//			//printf("maxPrevLevel %d level %d dif %d\n", dif, maxPrevLevel, it->level);
//
//			set<int> visited;
//			updateBackwardHierarchy(visited, idmax, dif);
//
//			set<int> result;
//			set_difference(toUpdate.begin(), toUpdate.end(), visited.begin(), visited.end(), inserter(result, result.begin()));
//
//			toUpdate = result;
//		}
//	}
}

void causalgraph::replaceQubitIndex(set<int>& visited, int curr, int oldvalue, int newvalue)
{
	if(visited.find(curr) != visited.end())
		return;

	visited.insert(curr);
	std::vector<int>::iterator it;
	it = find (circuit[curr].wires.begin(), circuit[curr].wires.end(), oldvalue);
	if(it != circuit[curr].wires.end())
		*it = newvalue;


	for(set<int>::iterator it=circuit[curr].willPush.begin(); it!=circuit[curr].willPush.end(); it++)
	{
		replaceQubitIndex(visited, *it, oldvalue, newvalue);
	}
}

void causalgraph::findShortestPath(set<int>& visited, set<int>& outputs, vector<int> path, vector<int>& shortest, int stepback, int prev, int curr)
{
	if(visited.find(curr) != visited.end())
	{
		return;
	}

	if(visited.empty())
	{
		set<int> vis2;
		reachOutputs(vis2, outputs, curr);
		//avoid cycles of outputs
		visited.insert(outputs.begin(), outputs.end());
	}

	visited.insert(curr);

	//do not consider nodes where output and input were joined
	if(!gatenumbers::getInstance()->isConnectionElement(circuit[curr].type))
		path.push_back(curr);

	//check solution
	if(stepback>0 &&  gatenumbers::getInstance()->isAncillaOutput(circuit[curr].type) && outputs.find(curr) == outputs.end())//todo
	{
		//int distwire = circuit[path.back()].wires[0] - circuit[path.front()].wires[0];

		/*METHOD STEPBACK*/
		if (shortest.size() == 0 || (path.size() < shortest.size()-1 || stepback < -shortest.back() ))

		/*METHD DISTWIRE*/
		//if (shortest.size() == 0 || (/*path.size() <= shortest.size()-1 || */abs(distwire)<= abs(-shortest.back()) ))
		{
			shortest.clear();
			shortest.insert(shortest.begin(), path.begin(), path.end());

			//shortest.push_back(-distzero);

			/*METODA CU STEPBACK*/
			//last element is number of stepbacks - not an id
			shortest.push_back(-stepback);
			//shortest.insert(curr);

			/*METODA CU DISTWIRE*/
			//shortest.push_back(-distwire);

		}
	}

	for(set<int>::iterator it=circuit[curr].pushedBy.begin(); it!=circuit[curr].pushedBy.end(); it++)
	{
		if(*it != prev)
		{
			//fa pas inapoi
			//findShortestPath(visited, shortest, true, curr, *it);
			int counts = 1;
			//if(circuit[*it].type == -3 || circuit[*it].type == -4)
			if(gatenumbers::getInstance()->isConnectionElement(circuit[*it].type))
				counts = 0;
			findShortestPath(visited, outputs, path, shortest, stepback + counts, curr, *it);
		}
	}

	//FORWARD
	for(set<int>::iterator it=circuit[curr].willPush.begin(); it!=circuit[curr].willPush.end(); it++)
	{
		//fa pas inapoi
		findShortestPath(visited, outputs, path, shortest, stepback, curr, *it);
	}
}

int causalgraph::moveInputAfterOutput(vector<int> shortest, int inputId)
{
	int outputId = -1;

	/*the last element is just the number of stepbacks*/
	for (vector<int>::iterator it = shortest.begin(); it != (shortest.end()--); it++)
	{
		//if (circuit[*it].type == -2)

		gatenumbers* gn = gatenumbers::getInstance();
		int x = circuit[*it].type;
		if (gn->isAncillaOutput(x))
		{
			outputId = *it;

			//change type to not reconnect another input
			circuit[outputId].type = CONNOUT;//temp value, todo
			circuit[inputId].type = CONNIN;

			connectNodes(outputId, inputId);

			set<int> vis;

			//replaceQubitIndex(vis, outputId, circuit[inputId].wires[0], circuit[outputId].wires[0]);

			break;
		}
	}

	//computeLevels();
	//equalizeLevels();

	return outputId;
}

void causalgraph::computeLevels()
{
	for(vector<recyclegate>::iterator it = circuit.begin(); it != circuit.end(); it++)
	{
		//reset level
		if(it->pushedBy.size() == 0)//input
		{
			it->level = 0;
		}
		else
		{
			//reset
			it->level = -1;
		}
	}

	//take the outputs
	for(vector<recyclegate>::iterator it = circuit.begin(); it != circuit.end(); it++)
	{
		//reset level
		if(it->willPush.size() == 0)//output
		{
			int max = getAndSetMaxPrevLevel(it->id);
			it->level = max + 1;
		}
	}
}

void causalgraph::reachOutputs(set<int>& visited, set<int>& outputs, int curr)
{
	if(visited.find(curr) != visited.end())
		return;

	visited.insert(curr);

	if(gatenumbers::getInstance()->isAncillaOutput(circuit[curr].type))
	{
		outputs.insert(curr);
	}

	for(set<int>::iterator it = circuit[curr].willPush.begin(); it != circuit[curr].willPush.end(); it++)
	{
		reachOutputs(visited, outputs, *it);
	}
}

int causalgraph::getMaxLevel()
{
	int max = INT_MIN;
	for(int i=0; i<circuit.size(); i++)
		if(max < circuit[i].level)
			max = circuit[i].level;
	return max;
}

int causalgraph::getNrQubits()
{
	return nrQubits;
}
