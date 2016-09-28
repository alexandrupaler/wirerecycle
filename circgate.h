#ifndef CIRCGATE__
#define CIRCGATE__

#include <vector>
#include <map>
#include <string>
#include <iostream>

#include "trim.h"

using namespace std;

class circgate{
public:
	char type;
	vector<int> wires;

	circgate()
	{

	}

	circgate(string s)
	{
		istringstream in(trim(s));
		string typ;
		in >>  typ;

		type = typ[0];//a single char..not good?

		while(!in.eof())
		{
			int nr;
			in >> nr;
			wires.push_back(nr);
		}

	};

	void replaceWires(map<int, int>& dict)
	{
		for(int i=0; i<wires.size(); i++)
		{
			if(dict.find(wires[i]) != dict.end())
				wires[i] = dict[wires[i]];
		}
	}
};

#endif
