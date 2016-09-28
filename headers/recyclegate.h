#ifndef RECYCLEGATE_H__
#define RECYCLEGATE_H__

#include <stdio.h>
#include <vector>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <string>

#include <algorithm>
#include "circgate.h"

using namespace std;

class recyclegate : public circgate{
public:
	//vector<int> wires;
	set<int> willPush;
    set<int> pushedBy;
	//char type;
	int id;
	int level;
	string label;

	recyclegate();
	recyclegate(circgate& g);

	void generateLabel();
	void print();

	void addWillPush(int nr);

    void addPushedBy(int nr);

	void getMinMax(int& min, int& max);
};

#endif /* RECYCLEGATE_H_ */
