#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <set>
#include <cstdio>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;

#define INFINITY 9999


//1205082

int sockfd;
int routerIpAddress;

map<int, int> updateStatus;

typedef struct {
    unsigned int destination, nextHop, costOfLink;
} RoutingData;


vector<RoutingData> routingTable;
vector<RoutingData> baseRoutingTable;
vector<string> listOfRouters;
set<string> vertexSet;

int ipParser(string ipAddress) {
    char *input = new char[ipAddress.length() + 1];
    strcpy(input, ipAddress.c_str());

    char *token = strtok(input, ".");

    vector<string> ip;
    while (token != NULL) {
        ip.push_back(token);
        token = strtok(NULL, ".");
    }

    return stoi(ip.back());
}


string makeRoutingPacket() {
    string packet = "$rtbl";

    for (int i=0; i<routingTable.size(); i++) {
        packet = packet + "$" + to_string(routingTable[i].destination) + "-" + to_string(routingTable[i].nextHop) + "-" + to_string(routingTable[i].costOfLink);
    }


    return packet;

}

void printRoutingTable() {

    cout<<"\nmy routing table\ndestination \t|\t next hop \t|\t cost \n";
    cout<<"-------------------------------------------------------\n";
    for (int i=0; i<routingTable.size(); i++) {
        // if (routingTable[i].costOfLink != 9999) {
        cout<<"192.168.10."<<routingTable[i].destination+1<<" \t|\t 192.168.10."<<routingTable[i].nextHop+1<<" \t|\t "<<routingTable[i].costOfLink<<endl;
        // } else {
        //     cout<<"192.168.10."<<routingTable[i].destination+1<<" \t|\t ------------ \t|\t "<<routingTable[i].costOfLink<<endl;
        // }
    }

    cout<<endl<<endl;

}

int lookUpRoutingTable(string destination) {

    int to = ipParser(destination) - 1;

    for (int i=0; i<routingTable.size(); i++) {
        if (routingTable[i].destination == to) {
            return routingTable[i].nextHop+1;
        }
    }

}



void write() {

    string buffer = makeRoutingPacket();

    for (int i=0; i<listOfRouters.size(); i++) {

        struct sockaddr_in routerAddress;

        routerAddress.sin_family = AF_INET;
        routerAddress.sin_port = htons(4747);
        inet_pton(AF_INET,listOfRouters[i].c_str(),&routerAddress.sin_addr);


        int sent_bytes = sendto(sockfd, buffer.c_str(), 1024, 0, (struct sockaddr*) &routerAddress, sizeof(sockaddr_in));

        if (sent_bytes != -1) {
            //printf("sent to [%s:%hu]: %s\n", inet_ntoa(routerAddress.sin_addr), ntohs(routerAddress.sin_port), buffer.c_str());
        }

    }

}

vector<string> packetParser(string packet, string delim) {


    char *input = new char[packet.length() + 1];
    strcpy(input, packet.c_str());

    char *token = strtok(input, delim.c_str());

    vector<string> data;
    while (token != NULL) {
        data.push_back(token);
        token = strtok(NULL, delim.c_str());
    }


    return data;
}

void updateRoutingTable(vector<RoutingData> neighbourRoutingTable) {

    typedef struct edge {
        unsigned int from, to, cost;
    } Link;

    vector<Link> neighbourList;
    int routerIpAddress;

    for (int i=0; i<routingTable.size(); i++) {
        if (!routingTable[i].costOfLink) {
            routerIpAddress = routingTable[i].destination;
            break;
        }
    }


    for (int i=0; i<routingTable.size(); i++) {
        neighbourList.push_back({routerIpAddress, routingTable[i].destination, routingTable[i].costOfLink});
    }

    int neighbourRouterIpAddress;

    for (int i=0; i<neighbourRoutingTable.size(); i++) {
        if (!neighbourRoutingTable[i].costOfLink) {
            neighbourRouterIpAddress = neighbourRoutingTable[i].destination;
            break;
        }
    }

    for (int i=0; i<neighbourRoutingTable.size(); i++) {
        neighbourList.push_back({neighbourRouterIpAddress, neighbourRoutingTable[i].destination, neighbourRoutingTable[i].costOfLink});
    }


    int noOfVertices =  vertexSet.size();

    int distance[noOfVertices][noOfVertices], nextHopList[noOfVertices][noOfVertices];

    for (int i=0; i<noOfVertices; i++) {
        for (int j=0; j<noOfVertices; j++) {
            distance[i][j] = (i==j) ? 0 : INFINITY;
            nextHopList[i][j]=j;
        }
    }


    for (int i=0; i<neighbourList.size(); i++) {

        if (distance[neighbourList[i].from][neighbourList[i].to] > neighbourList[i].cost ||
                distance[neighbourList[i].to][neighbourList[i].from] > neighbourList[i].cost) {

            distance[neighbourList[i].from][neighbourList[i].to] = neighbourList[i].cost;
            distance[neighbourList[i].to][neighbourList[i].from] = neighbourList[i].cost;

        }
    }




    for(int k=0; k<noOfVertices; k++) {
        for(int i=0; i<noOfVertices; i++) {
            for(int j=0; j<noOfVertices; j++) {
                if(distance[i][j]>distance[i][k]+distance[k][j]) {
                    distance[i][j]=distance[i][k]+distance[k][j];
                    nextHopList[i][j]=nextHopList[i][k];
                }
            }
        }
    }


    neighbourRoutingTable.clear();

    for(int i=0; i<noOfVertices; i++) {
        neighbourRoutingTable.push_back({i, nextHopList[routerIpAddress][i], distance[routerIpAddress][i]});
    }

    bool changed = false;

    for(int i=0; i<routingTable.size(); i++) {
        if (routingTable[i].costOfLink != neighbourRoutingTable[i].costOfLink) {
            routingTable[i].destination = neighbourRoutingTable[i].destination;
            routingTable[i].nextHop = neighbourRoutingTable[i].nextHop;
            routingTable[i].costOfLink = neighbourRoutingTable[i].costOfLink;
            changed = true;
        }
    }



    if (changed) {
        printRoutingTable();
    }

    updateStatus[neighbourRouterIpAddress]++;

}

void decodeRoutingPacket(string packet) {


    vector<string> data = packetParser(packet, "$");

    vector<RoutingData> neighbourRoutingTable;

    //cout<<"\nreceived routing table\ndestination \t|\t next hop \t|\t cost \n";
    //cout<<"-------------------------------------------------------\n";

    for (int i=0; i<data.size(); i++) {
        vector<string> entry = packetParser(data[i], "-");
        // if (routingTable[i].costOfLink != 9999) {
        neighbourRoutingTable.push_back({stoi(entry[0]), stoi(entry[1]), stoi(entry[2])});
        //  cout<<"192.168.10."<<neighbourRoutingTable[i].destination+1<<" \t|\t 192.168.10."<<neighbourRoutingTable[i].nextHop+1<<" \t|\t "<<neighbourRoutingTable[i].costOfLink<<endl;
        // } else {
        //     cout<<"192.168.10."<<routingTable[i].destination+1<<" \t|\t ------------ \t|\t "<<routingTable[i].costOfLink<<endl;
        // }
    }

    //cout<<endl<<endl;

    updateRoutingTable(neighbourRoutingTable);


}

void forwardMessage(string ipTo, string data, string message) {
    string nextHop = "192.168.10." + to_string(lookUpRoutingTable(ipTo));
    struct sockaddr_in nextHopAddress;

    nextHopAddress.sin_family = AF_INET;
    nextHopAddress.sin_port = htons(4747);
    inet_pton(AF_INET, nextHop.c_str(), &nextHopAddress.sin_addr);


    int sent_bytes = sendto(sockfd, data.c_str(), 1024, 0, (struct sockaddr*) &nextHopAddress, sizeof(sockaddr_in));

    if (sent_bytes != -1) {
        cout<<message<<" packet forwarded to "<<nextHop<<endl;
    }

}


typedef union {
    char data[4];
    unsigned char ip[4];
    short int s;
} Convert;

int clockCounter = 0;


void restartRouter() {

    routingTable.clear();
    for (int i=0; i<baseRoutingTable.size(); i++) {
        routingTable.push_back({baseRoutingTable[i].destination, baseRoutingTable[i].nextHop, baseRoutingTable[i].costOfLink});
    }
}

void updateLinkStatus() {

    bool changed = false;

    vector<RoutingData> temp = baseRoutingTable;

    for (auto it: updateStatus) {
        if (!it.second) {
            for (int b=0; b<temp.size(); b++) {
                if (temp[b].destination==it.first) {
                    temp[b].costOfLink = INFINITY;
                    changed = true;
                }
            }
        }
    }

    updateStatus.clear();

    if (changed) {

        routingTable.clear();

        for (int i=0; i<temp.size(); i++) {
            routingTable.push_back({temp[i].destination, temp[i].nextHop, temp[i].costOfLink});
        }

    }
}

void updateCost(string ipFrom, string ipTo, int cost) {

    int from = ipParser(ipFrom) - 1;
    int to = ipParser(ipTo) - 1;

    vector<RoutingData> temp = baseRoutingTable;

    for (int b=0; b<temp.size(); b++) {
        if (temp[b].destination != from || temp[b].destination != to) {
            temp[b].costOfLink = cost;
        }
    }

    string buffer = "$rst";

    for (int i=0; i<listOfRouters.size(); i++) {

        if (listOfRouters[i].compare(ipFrom) || listOfRouters[i].compare(ipTo)) {

            struct sockaddr_in routerAddress;

            routerAddress.sin_family = AF_INET;
            routerAddress.sin_port = htons(4747);
            inet_pton(AF_INET,listOfRouters[i].c_str(),&routerAddress.sin_addr);


            int sent_bytes = sendto(sockfd, buffer.c_str(), 1024, 0, (struct sockaddr*) &routerAddress, sizeof(sockaddr_in));

            if (sent_bytes != -1) {
                // printf("sent to [%s:%hu]: %s\n", inet_ntoa(routerAddress.sin_addr), ntohs(routerAddress.sin_port), buffer.c_str());
            }

        }

    }


    routingTable.clear();
    for (int i=0; i<temp.size(); i++) {
        routingTable.push_back({temp[i].destination, temp[i].nextHop, temp[i].costOfLink});
    }
}


void read() {

    while(true) {


        struct sockaddr_in routerAddress;
        char buffer[1024];
        socklen_t addrlen;

        int bytes_received = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*) &routerAddress, &addrlen);

        if (bytes_received != -1) {
            //printf("\treceived from [%s:%hu]: %s\n", inet_ntoa(routerAddress.sin_addr), ntohs(routerAddress.sin_port), buffer);

            string packet(buffer);


            string header = packet.substr(0, 4);

            if (!header.compare("clk ")) {

                if (clockCounter == 3) {
                    updateLinkStatus();
                    clockCounter = 0;
                } else {
                    clockCounter++;
                }

                write();

            } else if (!header.compare("cost")) {
                Convert convert;
                char raw[16];

                strcpy(convert.data, packet.substr(4, 4).c_str());
                sprintf(raw, "%u.%u.%u.%u",convert.ip[0], convert.ip[1] ,convert.ip[2], convert.ip[3]);
                string ipFrom(raw);

                strcpy(convert.data, packet.substr(8, 4).c_str());
                sprintf(raw, "%u.%u.%u.%u",convert.ip[0], convert.ip[1] ,convert.ip[2], convert.ip[3]);
                string ipTo(raw);

                strcpy(convert.data, packet.substr(12, 2).c_str());
                int cost=convert.s;


                cout<<ipFrom<<"\t"<<ipTo<<"\t"<<cost<<endl;
                updateCost(ipFrom, ipTo, cost);

            } else if (!header.compare("$rtb")) {
                decodeRoutingPacket(packet.substr(5, packet.length()-5));
            } else if (!header.compare("show")) {
                printRoutingTable();
            } else if (!header.compare("$rst")) {
                restartRouter();
            } else if (!header.compare("send")) {

                Convert convert;
                char raw[16];

                strcpy(convert.data, packet.substr(4, 4).c_str());
                sprintf(raw, "%u.%u.%u.%u",convert.ip[0], convert.ip[1] ,convert.ip[2], convert.ip[3]);
                string ipFrom(raw);

                strcpy(convert.data, packet.substr(8, 4).c_str());
                sprintf(raw, "%u.%u.%u.%u",convert.ip[0], convert.ip[1] ,convert.ip[2], convert.ip[3]);
                string ipTo(raw);

                strcpy(convert.data, packet.substr(12, 2).c_str());
                int messageLength=convert.s;

                char msg[messageLength];
                for (int i=0; i<messageLength; i++) {
                    msg[i] = buffer[14+i];
                }

                string message(msg);

                string data = "frwd" + ipTo + to_string(messageLength) + message;

                forwardMessage(ipTo, data, message);


            } else if (!header.compare("frwd")) {

                Convert convert;
                char raw[16];

                strcpy(convert.data, packet.substr(4, 4).c_str());
                sprintf(raw, "%u.%u.%u.%u",convert.ip[0], convert.ip[1] ,convert.ip[2], convert.ip[3]);
                string ipTo(raw);
                cout<<ipTo<<"\t";
                strcpy(convert.data, packet.substr(8, 2).c_str());
                int messageLength=convert.s;
                cout<<messageLength<<"\t";
                char msg[messageLength];
                for (int i=0; i<messageLength; i++) {
                    msg[i] = buffer[10+i];
                }

                string message(msg);
                cout<<message<<endl;

                if (ipParser(ipTo)-1 == routerIpAddress) {
                    cout<<message<<" packet reached destination\n";

                } else {
                    string data = "frwd" + ipTo + to_string(messageLength) + message;

                    forwardMessage(ipTo, data, message);
                }
            }


        }


    }

}




void initializeRouter(string ipAddress, string fileName) {

    typedef struct edge {
        unsigned int from, to, cost;
    } Link;

    vector<Link> neighbourList;


    string addressOfRouterOne, addressOfRouterTwo;
    unsigned int costOfLink;


    ifstream topoFile(fileName);

    while (topoFile.good()) {
        topoFile>>addressOfRouterOne>>addressOfRouterTwo>>costOfLink;

        vertexSet.insert(addressOfRouterOne);
        vertexSet.insert(addressOfRouterTwo);

        if (!addressOfRouterOne.compare(ipAddress) || !addressOfRouterTwo.compare(ipAddress)) {
            Link link = {ipParser(addressOfRouterOne), ipParser(addressOfRouterTwo), costOfLink};
            neighbourList.push_back(link);
        }
    }

    int noOfVertices =  vertexSet.size();

    int distance[noOfVertices][noOfVertices], nextHopList[noOfVertices][noOfVertices];

    for (int i=0; i<noOfVertices; i++) {
        for (int j=0; j<noOfVertices; j++) {
            distance[i][j] = (i==j) ? 0 : INFINITY;
            nextHopList[i][j]=j;
        }
    }


    for (int i=0; i<neighbourList.size(); i++) {
        distance[neighbourList[i].from-1][neighbourList[i].to-1] = neighbourList[i].cost;
        distance[neighbourList[i].to-1][neighbourList[i].from-1] = neighbourList[i].cost;
    }


    for(int k=0; k<noOfVertices; k++) {
        for(int i=0; i<noOfVertices; i++) {
            for(int j=0; j<noOfVertices; j++) {
                if(distance[i][j]>distance[i][k]+distance[k][j]) {
                    distance[i][j]=distance[i][k]+distance[k][j];
                    nextHopList[i][j]=k;
                }
            }
        }
    }

    routerIpAddress = ipParser(ipAddress) - 1;

    for(int i=0; i<noOfVertices; i++) {
        routingTable.push_back({i, nextHopList[routerIpAddress][i], distance[routerIpAddress][i]});
        baseRoutingTable.push_back({i, nextHopList[routerIpAddress][i], distance[routerIpAddress][i]});
        if (routingTable[i].costOfLink != INFINITY && i != routerIpAddress) {
            updateStatus[i] = 0;
            listOfRouters.push_back("192.168.10."+to_string(i+1));
        }
    }


    printRoutingTable();

}




int main(int argc, char *argv[]) {

    if (argc != 2) {
        cout<<"usage: router <ip address>\n";
        exit(0);
    }

    string ipAddress = argv[1];// = "192.168.10.1";


    initializeRouter(ipAddress, "topo.txt");


    struct sockaddr_in client_address;

    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(4747);
    inet_pton(AF_INET, ipAddress.c_str(), &client_address.sin_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bind(sockfd, (struct sockaddr*) &client_address, sizeof(sockaddr_in));

    read();

}
