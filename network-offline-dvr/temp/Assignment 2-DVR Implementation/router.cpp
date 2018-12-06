#include <bits/stdc++.h>
using namespace std;

string myIP ; 
map<string, int> ipMap;
string RouterList[1000]; 
int nodeNo = 0;

set<


void initializeRoutingTable(char *name)
{
    string line;

    ifstream file;
    file.open(name);

    while (!file.eof())
    {
        getline(file, line);

        char *temp = new char[line.length() + 1];
        strcpy(temp, line.c_str());

        char *token;

        char *u, *v, *c;
        string ip1, ip2, weight;
        int cost;

        u = strtok(temp, " ");
        v = strtok(NULL, " ");
        c = strtok(NULL, " ");

        ip1 = u;
        ip2 = v;
        weight = c;

        std::istringstream(weight) >> cost;

        if (ipMap.find(ip1) == ipMap.end())
        {
            ipMap[ip1] = nodeNo;
            nodeNo++;
        }
        if (ipMap.find(ip2) == ipMap.end())
        {
            ipMap[ip2] = nodeNo;
            nodeNo++;
        }

        cout << ip2 << " " << ipMap[ip2] << endl;
        cout << ip1 << " " << ipMap[ip1] << endl;

    }

    file.close();
}



int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        cout << "Try again" << endl;
        cout << "./router <ip-address> <topology-file-name>" << endl;
        return 0;
    }

    myIP = argv[1]; 

    initializeRoutingTable(argv[2]); 

    

    return 0;
}