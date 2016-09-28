#include <climits>

#include "gatenumbers.h"
#include "recyclegate.h"
#include "trim.h"

recyclegate::recyclegate(circgate& g)
{
	//copy attributes
	type = g.type;
	wires = g.wires;
	level = 0;
	label = " ";
}

recyclegate::recyclegate()
{
	level = 0;
	label = " ";
}

void recyclegate::generateLabel()
{
	string name = " ";
	if(gatenumbers::getInstance()->isInitialisationNumber(type))
	{
		name = "input" + toString(id);
	}
		if(gatenumbers::getInstance()->isMeasurementNumber(type))
	{
		name = "output" + toString(wires[0]);
	}
	else
	{
		name = toString(id + 1);
	}

	label = name + " [" + toString(level) + "]";
}

void recyclegate::print()
{
	printf("label:%s level:%d id:%d type:%d on lines:", label.c_str(), level, id, type);
	for(vector<int>::iterator it = wires.begin(); it != wires.end(); it++)
	{
		printf("%d ", *it);
	}
	printf("\npushes: ");
	for(set<int>::iterator it = willPush.begin(); it != willPush.end(); it++)
	{
		printf("%d ", *it);
	}
	printf("\npushed by: ");
	for(set<int>::iterator it = pushedBy.begin(); it != pushedBy.end(); it++)
	{
		printf("%d ", *it);
	}
	printf("\n-------\n");
}

void recyclegate::addWillPush(int nr)
{
	if(willPush.find(nr) == willPush.end())
	{
		willPush.insert(nr);
	}
}

void recyclegate::addPushedBy(int nr)
{
	pushedBy.insert(nr);
}

void recyclegate::getMinMax(int& min, int& max)
{
	if(wires.size() == 0)
		return;

	min = INT_MAX;
	max = INT_MIN;
	for(vector<int>::iterator it = wires.begin(); it != wires.end(); it++)
	{
		if(min > *it)
			min = *it;
		if(max < *it)
			max = *it;
	}
}
