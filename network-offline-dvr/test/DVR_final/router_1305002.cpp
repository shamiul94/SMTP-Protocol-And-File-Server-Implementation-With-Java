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
    string nbr;
    int cost;
	int recvClock;
	int status;
};

vector<string> neighbors;
set<string> routers;
vector<RoutingTableEntry> routingTable;
vector<Edge> links;

int sendClock = 0;
bool entryChanged = false;

struct RoutingTableEntry getRouteEntry(string row, string delim)
{
	char *t = new char[row.length()+1];
	strcpy(t,row.c_str());
	struct RoutingTableEntry rte;
	vector<string> entries;
	char *tok = strtok(t,delim.c_str());

	while(tok!=NULL)
	{
		entries.push_back(tok);
		tok = strtok(NULL,delim.c_str());
	}

	rte.destination = entries[0];
	rte.nextHop = entries[1];
	rte.cost = atoi(entries[2].c_str());
	entries.clear();
	return rte;
}

vector<RoutingTableEntry> extractTableFromPacket(string packet)
{
    vector<RoutingTableEntry> rt;
    char *str = new char[packet.length()+1];
	strcpy(str, packet.c_str());
    char *token = strtok(str,":");
	vector<string> entries;
    while(token != NULL)
    {
        entries.push_back(token);
		token = strtok(NULL,":");
    }
	struct RoutingTableEntry rte;
	for(int i = 0; i<entries.size(); i++)
	{
		rte = getRouteEntry(entries[i],"#");
		rt.push_back(rte);
		//cout<<"dest : "<<rte.destination<<"  next : "<<rte.nextHop<<"  cost : "<<rte.cost<<endl;
	}
    return rt;
}

string makeTableIntoPacket(vector<RoutingTableEntry> rt)
{
    string packet = "ntbl"+routerIpAddress;
    for(int i = 0; i<rt.size(); i++)
    {
        packet = packet + ":" + rt[i].destination + "#" + rt[i].nextHop + "#" + to_string(rt[i].cost);
    }
    return packet;
}


void printTable()
{
    cout<<"\t------\t"<<routerIpAddress<<"\t------\t"<<endl;
    cout<<"Destination  \tNext Hop \tCost"<<endl;
    cout<<"-------------\t-------------\t-----"<<endl;
    for(int i = 0; i<routingTable.size(); i++)
    {
        if(!routingTable[i].destination.compare(routerIpAddress)) continue;
        cout<<routingTable[i].destination<<"\t"<<routingTable[i].nextHop<<"\t"<<routingTable[i].cost<<endl;
    }
    cout<<"--------------------------------------"<<endl;
}

int getNeighbor(string nip)
{
	for(int i = 0;  i<links.size(); i++)
	{
		if(!nip.compare(links[i].nbr))
		{
			return i;
		}
	}
}

bool isNeighbor(string nip)
{
	for(int i = 0; i<links.size(); i++)
	{
		if(!nip.compare(links[i].nbr))
			return true;
	}
	return false;
}

void updateRoutingTableForNeighbor(string nip, vector<RoutingTableEntry> nrt)
{
    int tempCost;
    for(int i = 0; i<routers.size(); i++)
    {
        for(int j = 0; j<links.size(); j++)
        {
            if(!nip.compare(links[j].nbr))
            {
				tempCost = links[j].cost + nrt[i].cost;
                if(!nip.compare(routingTable[i].nextHop)||(tempCost<routingTable[i].cost && routerIpAddress.compare(nrt[i].nextHop)!=0))
                {
					if(routingTable[i].cost != tempCost)
					{
						routingTable[i].nextHop = nip;
						routingTable[i].cost = tempCost;
                    	entryChanged = true;
					}
					break;
                }
            }
        }
    }
    if(entryChanged==true)
        printTable();
	entryChanged = false;
}

void initRouter(string routerIp, string topology)
{
    ifstream topo(topology.c_str());
    string r1, r2;
    int cost;

    while(!topo.eof())
    {
        topo>>r1>>r2>>cost;

        routers.insert(r1);
        routers.insert(r2);

        struct Edge e;

        if(!r1.compare(routerIp))
        {
			if(!isNeighbor(r2))
			{
            	neighbors.push_back(r2);
            	e.nbr = r2;
            	e.cost = cost;
				e.status = 1;
				e.recvClock = 0;
            	links.push_back(e);
			}
        }
        else if(!r2.compare(routerIp))
        {
			if(!isNeighbor(r1))
			{
		        neighbors.push_back(r1);
		        e.nbr = r1;
		        e.cost = cost;
				e.status = 1;
				e.recvClock = 0;
		        links.push_back(e);
			}
        }
    }

    topo.close();

    struct RoutingTableEntry route;
    string dest = "192.168.10.";
    for(int i = 0; i<routers.size(); i++)
    {
		char x = i+1+'0';
        string temp = dest + x;
        if(find(neighbors.begin(),neighbors.end(),temp)!=neighbors.end())
        {
            for(int j = 0; j<links.size(); j++)
            {
                if(!links[j].nbr.compare(temp))
                {
                    route.destination = temp;
                    route.nextHop = temp;
                    route.cost = links[j].cost;
                }
            }
        }
        else if(!routerIp.compare(temp))
        {
            route.destination = temp;
            route.nextHop = temp;
            route.cost = 0;
        }
        else
        {
            route.destination = temp;
            route.nextHop = "\t-";
            route.cost = INF;
        }
        routingTable.push_back(route);
    }
    printTable();
}

string makeIP(unsigned char * raw)
{
    int ipSegment[4];
    for(int i = 0; i<4; i++)
        ipSegment[i] = raw[i];
    string ip = to_string(ipSegment[0])+"."+to_string(ipSegment[1])+"."+to_string(ipSegment[2])+"."+to_string(ipSegment[3]);
    return ip;
}

void updateTableForCostChange(string nbr, int changedCost, int oldCost)
{
    for(int i = 0; i<routers.size(); i++)
    {
        if(!nbr.compare(routingTable[i].nextHop))
        {
            if(!nbr.compare(routingTable[i].destination))
            {
                routingTable[i].cost = changedCost;
            }
            else
            {
                routingTable[i].cost = routingTable[i].cost - oldCost + changedCost;
            }
            entryChanged = true;
        }
		else if(!nbr.compare(routingTable[i].destination) && routingTable[i].cost>changedCost)
		{
			routingTable[i].cost = changedCost;
			routingTable[i].nextHop = nbr;
			entryChanged = true;
		}
    }
    if(entryChanged == true)
        printTable();
	entryChanged = false;
}

void sendTable()
{
    string tablePacket = makeTableIntoPacket(routingTable);
    for(int i = 0; i<neighbors.size(); i++)
    {
        struct sockaddr_in router_address;

        router_address.sin_family = AF_INET;
        router_address.sin_port = htons(4747);
        inet_pton(AF_INET,neighbors[i].c_str(),&router_address.sin_addr);

        int sent_bytes = sendto(sockfd, tablePacket.c_str(), 1024, 0, (struct sockaddr*) &router_address, sizeof(sockaddr_in));
		if(sent_bytes!=-1)
		{
			//cout<<"routing table : "<<routerIpAddress<<" sent to : "<<neighbors[i]<<endl;
		}
	}
}


void forwardMessage(string dest, string length, string msg)
{
	string forwardPckt = "frwd#"+dest+"#"+length+"#"+msg;
	string next;
	for(int i = 0; i<routers.size(); i++)
	{
		if(!dest.compare(routingTable[i].destination))
		{
			next = routingTable[i].nextHop;
			break;
		}
	}
	struct sockaddr_in router_address;

    router_address.sin_family = AF_INET;
    router_address.sin_port = htons(4747);
    inet_pton(AF_INET,next.c_str(),&router_address.sin_addr);

    int sent_bytes = sendto(sockfd, forwardPckt.c_str(), 1024, 0, (struct sockaddr*) &router_address, sizeof(sockaddr_in));
	cout<<msg.c_str()<<" packet forwarded to "<<next.c_str()<<" (printed by "<<routerIpAddress.c_str()<<")\n";
}


void updateTableForLinkFailure(string nbr)
{
	for(int i = 0; i<routingTable.size(); i++)
	{
		if(!nbr.compare(routingTable[i].nextHop))
		{
			if(!nbr.compare(routingTable[i].destination) || !isNeighbor(routingTable[i].destination))
			{
				routingTable[i].nextHop = "\t-";
				routingTable[i].cost = INF;
				entryChanged = true;
			}
			else if(isNeighbor(routingTable[i].destination))
			{
				routingTable[i].nextHop = routingTable[i].destination;
				routingTable[i].cost = links[getNeighbor(routingTable[i].destination)].cost;
				entryChanged = true;
			}
		}
	}
	if(entryChanged == true)
		printTable();
	entryChanged = false;
}

void receive()
{
	struct sockaddr_in router_address;
    socklen_t addrlen;

	while(true)
	{
		char buffer[1024];
		bytes_received = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*) &router_address, &addrlen);
		if(bytes_received!=-1)
		{
			string recv(buffer);
			string head = recv.substr(0,4);
			if(!head.compare("show"))
			{
				printTable();
			}
			else if(!head.compare("clk "))
            {
				sendClock++;
				sendTable();

				for(int i = 0; i<links.size(); i++)
				{
					if(sendClock-links[i].recvClock>3 && links[i].status == 1)
					{
						cout<<"----- link down with : "<<links[i].nbr<<" -----"<<endl;
						links[i].status = -1;
						updateTableForLinkFailure(links[i].nbr);
					}
				}
            }
            else if(!head.compare("ntbl"))
            {
                string nip = recv.substr(4,12);
				int index = getNeighbor(nip);
				links[index].status = 1;
				links[index].recvClock = sendClock;
				//cout<<"receiver : "<<routerIpAddress<<" sender : "<<nip<<" recv clk : "<<links[index].recvClock<<endl;
				int length = recv.length()-15;
				char pckt[length];
                for (int i=0; i<length; i++) {
                    pckt[i] = buffer[16+i];
                }
				string packet(pckt);
                vector<RoutingTableEntry> ntbl = extractTableFromPacket(pckt);
                updateRoutingTableForNeighbor(nip, ntbl);
            }
			else if(!head.compare("send"))
            {
                //forward given message to destination router
				unsigned char *ip1 = new unsigned char[5];
				unsigned char *ip2 = new unsigned char[5];
				string temp1 = recv.substr(4,4);
				string temp2 = recv.substr(8,4);
				for(int i = 0; i<4; i++)
				{
					ip1[i] = temp1[i];
					ip2[i] = temp2[i];
				}
                string sip1 = makeIP(ip1);
                string sip2 = makeIP(ip2);

				unsigned char *c1 = new unsigned char[3];
				string msgLength = recv.substr(12,2);
				int length = 0;
				c1[0] = msgLength[0];
				c1[1] = msgLength[1];
				int x0,x1;
				x0 = c1[0];
				x1 = c1[1]*256;
				length = x1+x0;
				char msg[length+1];
                for (int i=0; i<length; i++) {
                    msg[i] = buffer[14+i];
                }
				msg[length] = '\0';
				string message(msg);
				//forwarding function
				if(!sip2.compare(routerIpAddress))
				{
					cout<<message<<" packet reached destination (printed by "<<sip2<<")\n";
				}
				else
					forwardMessage(sip2,msgLength,message);
            }
            else if(!head.compare("frwd"))
            {
                //forward until reach destination
				vector<string> fmsgs;
				char * msg = new char[recv.length() + 1];
				strcpy(msg, recv.c_str());
				char *token = strtok(msg,"#");
				while(token!=NULL)
				{
					fmsgs.push_back(token);
					token = strtok(NULL,"#");
				}

				//forwarding function
				if(!fmsgs[1].compare(routerIpAddress))
				{
					cout<<fmsgs[3]<<" packet reached destination (printed by "<<fmsgs[1]<<")\n";
				}
				else
					forwardMessage(fmsgs[1],fmsgs[2],fmsgs[3]);
				fmsgs.clear();
            }
            else if(!head.compare("cost"))
            {
                //codes for updating link cost
				unsigned char *ip1 = new unsigned char[5];
				unsigned char *ip2 = new unsigned char[5];
				string temp1 = recv.substr(4,4);
				string temp2 = recv.substr(8,4);
				for(int i = 0; i<4; i++)
				{
					ip1[i] = temp1[i];
					ip2[i] = temp2[i];
				}
                string sip1 = makeIP(ip1);
                string sip2 = makeIP(ip2);
				unsigned char *c1 = new unsigned char[3];
				string tempCost = recv.substr(12,2);
				//cout<<tempCost<<endl;
				int changedCost = 0;
				c1[0] = tempCost[0];
				c1[1] = tempCost[1];
				int x0,x1;
				x0 = c1[0];
				x1 = c1[1]*256;
				changedCost = x1+x0;
				//cout<<changedCost<<endl;
                string nbr;
                int oldCost;
                for(int i = 0 ; i<links.size(); i++)
                {
                    if(!sip1.compare(links[i].nbr))
                    {
                        oldCost = links[i].cost;
                        links[i].cost = changedCost;
                        nbr = sip1;
                    }
                    else if(!sip2.compare(links[i].nbr))
                    {
                        oldCost = links[i].cost;
                        links[i].cost = changedCost;
                        nbr = sip2;
                    }
                }
                //codes for update table according to link cost change
				updateTableForCostChange(nbr,changedCost,oldCost);
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


    int bind_flag;
    struct sockaddr_in client_address;

    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(4747);
    inet_pton(AF_INET, argv[1], &client_address.sin_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bind_flag = bind(sockfd, (struct sockaddr*) &client_address, sizeof(sockaddr_in));

	if(!bind_flag) cout<<"Connection successful!!"<<endl;
	else cout<<"Connection failed!!!"<<endl;

    cout<<"--------------------------------------"<<endl;

	receive();

    return 0;
}
