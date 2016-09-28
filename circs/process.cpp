#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <vector>
#include <map>

using namespace std;

map<string, int> convert;
vector<int> inA;
vector<int> outA;

int main(int argc, char** argv)
{
	//printf("Hello World!\n");

	bool started = false;
	string line;
  ifstream myfile (argv[1]);
  if (myfile.is_open())
  {
    while ( getline (myfile, line) )
    {
			istringstream is(line);
	
			string s;
			is >> s;

			if(s.compare(".variables") == 0)
			{
				int nr = 0;
				while(!is.eof())
				{
					is >> s;
					convert[s] = nr;
					nr++;

					//cout << s << " " << convert[s] << endl;
				}
			}
			else if(s.compare(".constants") == 0)
			{
				is >> s;
				for(int i=0; i<s.size(); i++)
				{
					if(s[i] != '-')
						inA.push_back(i);
				}

				cout << "in " << s << endl;
			}
			else if(s.compare(".garbage") == 0)
			{
				is >> s;
				for(int i=0; i<s.size(); i++)
				{
					if(s[i] != '-')
						outA.push_back(i);
				}

				cout << "out " << s << endl;
			}
			else if(s.compare(".begin") == 0)
			{
				started = true;
				continue;
			}
			else if(s.compare(".end") == 0)
			{
				break;
			}

			if(started)
			{
				cout << s << " ";
				//read gate by gate
				while(!is.eof())
				{
					is >> s;
					cout << convert[s] << " ";
				}
				cout << endl;
			}
    }
    myfile.close();
  }
}
