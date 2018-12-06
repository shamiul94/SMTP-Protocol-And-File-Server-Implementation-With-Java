#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <string>
#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define INF 99999

using namespace std;

int sockfd;
char buffer[1024];
int bytes_received;
string routerIpAddress;



struct RoutingTableEntry
{
    string destination;
    string nextHop;
    int cost;
};

struct Edge
{
    string src;
    string dst;
    int cost;
};

vector<string> neighbors;
set<string> routers;
vector<RoutingTableEntry> routingTable;
vector<Edge> links;

void printTable()
{
    cout<<routerIpAddress<<endl;
    cout<<"Destination  \tNext Hop \tCost"<<endl;
    cout<<"-------------\t-------------\t-----"<<endl;
    for(int i = 0; i<routingTable.size(); i++)
    {
        if(!routingTable[i].destination.compare(routerIpAddress)) continue;
        cout<<routingTable[i].destination<<"\t"<<routingTable[i].nextHop<<"\t"<<routingTable[i].cost<<endl;
    }
    cout<<endl;
}

void initRouter(string routerIp, string topology)
{
    ifstream topo(topology.c_str());
    //cout<<"inside init  "<<routerIp<<"   "<<topology<<endl;
    string r1, r2;
    int cost;

    while(!topo.eof())
    {
        topo>>r1>>r2>>cost;

        routers.insert(r1);
        routers.insert(r2);

        struct Edge e;

        if(r1.compare(routerIp)==0)
        {
            neighbors.push_back(r2);
            //e = {r1,r2,cost};
            e.src = r1;
            e.dst = r2;
            e.cost = cost;
            links.push_back(e);
        }
        else if(r2.compare(routerIp)==0)
        {
            neighbors.push_back(r1);
            //e = {r1,r2,cost};
            e.src = r1;
            e.dst = r2;
            e.cost = cost;
            links.push_back(e);
        }
    }

    //cout<<"inside init  "<<routerIp<<"   "<<topology<<endl;

    topo.close();

    struct RoutingTableEntry route;
    string dest = "192.168.10.";
    for(int i = 0; i<routers.size(); i++)
    {
        //char buf[2];
        //itoa(i+1,buf,10);
        //string temp = dest + string::to_string(i+1);
		char x = i+1+'0';
        string temp = dest + x;
        if(find(neighbors.begin(),neighbors.end(),temp)!=neighbors.end())
        {
            for(int j = 0; j<links.size(); j++)
            {
                if(!links[j].src.compare(temp)||!links[j].dst.compare(temp))
                {
                    //route = {temp, temp, links[j].cost};
                    route.destination = temp;
                    route.nextHop = temp;
                    route.cost = links[j].cost;
                }
            }
        }
        else if(!routerIp.compare(temp))
        {
            //route = {temp,temp,0};
            route.destination = temp;
            route.nextHop = temp;
            route.cost = 0;
        }
        else
        {
            //route = {temp, "\t-", INF};
            route.destination = temp;
            route.nextHop = "\t-";
            route.cost = INF;
        }
        routingTable.push_back(route);
    }
    printTable();
}

void receive()
{
	struct sockaddr_in router_address;
    socklen_t addrlen;
	while(1)
	{
		string recv;
		bytes_received = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*) &router_address, &addrlen);
		if(bytes_received!=-1)
		{
			recv = buffer;
			string head = recv.substr(0,4);
			if(!head.compare("show"))
			{
				printTable();			
			}
		}
	}
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        cout<<"router : "<<argv[1]<<"<ip address>\n";
        exit(1);
    }

	routerIpAddress = argv[1];
    initRouter(argv[1], argv[2]);

	//for(int i = 0; i<neighbors.size(); i++)
		//cout<<neighbors[i]<<endl;
    //routerIpAddress = "192.168.10.1";
    //initRouter(routerIpAddress, "topo.txt");

    int bind_flag;
    struct sockaddr_in client_address;

    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(4747);
    inet_pton(AF_INET, argv[1], &client_address.sin_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bind_flag = bind(sockfd, (struct sockaddr*) &client_address, sizeof(sockaddr_in));

	cout<<bind_flag<<endl;
	receive();	

    return 0;
}
