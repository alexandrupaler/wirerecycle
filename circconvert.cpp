#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <algorithm>
#include <climits>
#include <vector>
#include <string>

#include "gatenumbers.h"
#include "circconvert.h"
#include "trim.h"

using namespace std;

void circconvert::makeGates(vector<string> file)
{
	//citeste inputuri
	istringstream inin(file[0]);
	string s;
	inin >> s;
	inin >> inputs;

	istringstream outout(file[1]);
	outout >> s;
	outout >> outputs;

	for(int i=2; i<file.size(); i++)
	{
		string line2 = file[i];

		circgate currGate(line2);

		gates.push_back(currGate);
	}

}

vector<string> circconvert::readFile(char* fname)
{
	vector<string> ret;

	std::ifstream ifs (fname, std::ifstream::in);

	string line;

	//citeste inputuri
	getline(ifs, line);

	ret.push_back(line);

	getline(ifs, line);

	ret.push_back(line);

	while(ifs.good())
	{
		string line2;
		getline(ifs, line2);

		if(line2.size() == 0)
		{
			continue;
		}
		ret.push_back(line2);
	}

	return ret;
}

circconvert::circconvert(vector<string> file)
{
	makeGates(file);
}

circconvert::circconvert(char* fname)
{
	vector<string> file = circconvert::readFile(fname);
	makeGates(file);
}

void circconvert::updateWiresStartingFromGate(list<circgate>::iterator after, int minWireNumber, int incrValue)
{
	//after++;
	//for(list<circgate>::iterator it=after; it!=gates.end(); it++)
	for(list<circgate>::iterator it=gates.begin(); it!=after; it++)
	{
		for(int j=0; j<it->wires.size(); j++)
		{
			if(it->wires[j] > minWireNumber)
				it->wires[j] += incrValue;
		}
	}
	for(list<circgate>::iterator it=after; it!=gates.end(); it++)
	{
		for(int j=0; j<it->wires.size(); j++)
		{
			if(it->wires[j] >= minWireNumber)
				it->wires[j] += incrValue;
		}
	}
}

void circconvert::replaceNonICM()
{
	for(list<circgate>::iterator it=gates.begin(); it != gates.end(); it++)
	{
		if(it->type == 'T')
		{
			vector<int> wires = it->wires;
			map<int, int> dict;
			dict[0] = wires[0];
			dict[1] = wires[1];
			dict[2] = wires[2];

//			WIRE WIRE WIRE CTRL WIRE WIRE WIRE CTRL WIRE CTRL WIRE WIRE CTRL TGATE
//			WIRE CTRL WIRE WIRE WIRE CTRL WIRE WIRE TGATE TGT WIRE TGATE TGT PGATE
//			HGATE TGT TGATE TGT TGATE TGT TGATE TGT TGATE WIRE HGATE WIRE WIRE WIRE

			string g[] = {"h 2", "c 1 2", "t 2", "c 0 2", "t 2", "c 1 2", "t 2", "c 0 2", "t 1", "t 2", "c 0 1", "h 2", "t 1", "c 0 1", "t 0", "p 1"};

			for(int i=0; i<16; i++)
			{
				circgate c(g[i]);
				c.replaceWires(dict);
				gates.insert(it, c);
			}

			it = gates.erase(it);
			it--;
		}
		else if(it->type == 'h')
		{
			int nrgates = 3;

			map<int, int> dict;
			dict[0] = it->wires[0];

			string g[] = {"p 0", "v 0", "p 0"};
			for(int i=0; i<3; i++)
			{
				circgate gate(g[i]);
				gate.replaceWires(dict);
				gates.insert(it, gate);
			}

			//delete the old gate
			it = gates.erase(it);//advances the iterator
			it--;
		}
	}
}

void circconvert::configInputs(int qubit, string ins)
{
	for(int i=qubit; i<qubit + ins.size(); i++)
	{
		if(ins[i - qubit] != 'x')
			inputs[i] = ins[i - qubit];
	}
}

void circconvert::configOutputs(int qubit, string outs)
{
	for(int i=qubit; i<qubit + outs.size(); i++)
	{
		if(outs[i - qubit] != 'x')
			outputs[i] = outs[i - qubit];
	}
}

void circconvert::replaceICM()
{
	for(list<circgate>::iterator it=gates.begin(); it != gates.end(); it++)
	{
		if(it->type == 'p')
		{
			int qubit = it->wires[0];

			int nrqubits = 1;

			string ins = "x0";
			string outs = "1x";

			includeWires(nrqubits, qubit + 1);
			updateWiresStartingFromGate(it, qubit, 1);

			//a bagat inainte de qubit
			configInputs(qubit, ins);
			configOutputs(qubit, outs);

			string g[] = {"c 1 0"};
			circgate gate(g[0]);

			map<int, int> dict;
			dict[0] = qubit;
			dict[1] = qubit + 1;
			gate.replaceWires(dict);

			gates.insert(it, gate);

			it = gates.erase(it);
			it--;
		}
		else if(it->type == 'v')
		{
			int qubit = it->wires[0];

			string ins = "x0";
			string outs = "1x";

			includeWires(1, qubit + 1);
			updateWiresStartingFromGate(it, qubit, 1);

			configInputs(qubit, ins);
			configOutputs(qubit, outs);

			string g[] = {"c 0 1"};
			circgate gate(g[0]);

			map<int, int> dict;
			dict[0] = qubit;
			dict[1] = qubit + 1;
			gate.replaceWires(dict);

			gates.insert(it, gate);

			it = gates.erase(it);
			it--;
		}
		else if(it->type == 't')
		{
			int qubit = it->wires[0];

			string ins = "x00000";
			string outs = "11111x";

			includeWires(5, qubit + 1);
			updateWiresStartingFromGate(it, qubit, 5);

			configInputs(qubit, ins);
			configOutputs(qubit, outs);

			map<int, int> dict;
			for(int i=0; i<=5; i++)
				dict[i] = qubit + i;

			string g[] = {"c 1 0", "c 1 2", "c 3 1", "c 4 2", "c 3 5", "c 4 5"};
			for(int i=0; i<6; i++)
			{
				circgate gate(g[i]);
				gate.replaceWires(dict);
				gates.insert(it, gate);
			}

			it = gates.erase(it);
			it--;
		}
	}
}

void circconvert::includeWires(int howMany, int where)
{
	vector<char> empty(howMany, '-');
	inputs.insert(inputs.begin() + where, empty.begin(), empty.end());
	outputs.insert(outputs.begin() + where, empty.begin(), empty.end());
}

void circconvert::makeAncillaInput(int qubit)
{
	configInputs(qubit, "0");
}

void circconvert::makeAncillaOutput(int qubit)
{
	configInputs(qubit, "-");
}

void circconvert::makeComputeInput(int qubit)
{
	configInputs(qubit, "1");
}

void circconvert::makeComputeOutput(int qubit)
{
	configInputs(qubit, "-");
}

void circconvert::print()
{
	for(list<circgate>::iterator it=gates.begin(); it!=gates.end(); it++)
	{
		printf("%c ", it->type);
		for(int i=0; i<it->wires.size(); i++)
			printf("%d ", it->wires[i]);
		printf("\n");
	}
}
