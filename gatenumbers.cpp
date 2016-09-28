#include "gatenumbers.h"

gatenumbers* gatenumbers::instance = 0;

bool gatenumbers::isInitialisationNumber(int val)
{
	return instance->initialisations.find(val) != instance->initialisations.end();
}

bool gatenumbers::isInitialisationNumberButNotConnection(int val)
{
	return isInitialisationNumber(val) && (val != CONNIN);
}

bool gatenumbers::isMeasurementNumberButNotConnection(int val)
{
	return isMeasurementNumber(val) && (val != CONNOUT);
}

bool gatenumbers::isMeasurementNumber(int val)
{
	return instance->measurements.find(val) != instance->measurements.end();
}

bool gatenumbers::isAncillaOutput(int val)
{
	return isMeasurementNumberButNotConnection(val) && val != OUTPUT;
}

bool gatenumbers::isAncillaInput(int val)
{
	return isInitialisationNumberButNotConnection(val) && val != INPUT;
}

bool gatenumbers::isConnectionElement(int val)
{
	return val==CONNIN || val==CONNOUT;
}

gatenumbers* gatenumbers::getInstance()
{
	if(instance == 0)
		instance = new gatenumbers();

	return instance;
}

gatenumbers::gatenumbers()
{
	/**/
	int myints1[]= {MX, MZ, MXZ, MZX, MA, MYZ, MY, OUTPUT, CONNOUT};
	measurements=set<int>(myints1, myints1+9);

	/**/
	int myints2[]= {AA, YY, ZERO, PLUS, INPUT, CONNIN};
	initialisations=set<int>(myints2, myints2+6);
}
