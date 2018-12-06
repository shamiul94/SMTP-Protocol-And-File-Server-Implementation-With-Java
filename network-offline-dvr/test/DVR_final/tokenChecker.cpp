#include<bits/stdc++.h>

using namespace std;

struct routingTableEntry
{
	string dest;
	string next;
	int cost;
};

struct routingTableEntry getRouteEntry(string row, string delim)
{
	char *t = new char[row.length()+1];
	strcpy(t,row.c_str());
	struct routingTableEntry rte;
	vector<string> entries;
	char *tok = strtok(t,delim.c_str()); 

	while(tok!=NULL)
	{
		entries.push_back(tok);
		tok = strtok(NULL,delim.c_str());
	}
	
	rte.dest = entries[0];
	rte.next = entries[1];
	rte.cost = atoi(entries[2].c_str());
	entries.clear();
	return rte; 
}

int main()
{
	string pckt = ":192.168.10.1#192.168.10.1#0:192.168.10.2#192.168.10.2#10:192.168.10.3#192.168.10.3#3:192.168.10.4#\t-#99999";
		//string pckt = "2#4#45:4#3#234:";
	char * str = new char[pckt.length()+1];
	strcpy(str,pckt.c_str());
	//vector<RoutingTableEntry> rt;
    char *token = strtok(str,":");
	int count=0;
	vector<string> entries;
    while(token != NULL)
    {
		cout<<token<<endl;
      	
        /*char *tableRow = strtok(token,"#");
		
        while(tableRow!=NULL)
        {
			cout<<tableRow<<endl;
           // entries.push_back(tableRow);
            tableRow = strtok(NULL,"#");
			//cout<<"chek "<<tableRow<<endl;
        }*/
        //struct RoutingTableEntry rte;
        //rte.destination = entries[0];
        //rte.nextHop = entries[1];
        //rte.cost = atoi(entries[2].c_str());
       // entries.clear();
        //rt.push_back(rte);
		entries.push_back(token);
        token = strtok(NULL,":");
		//cout<<"chek "<<token<<endl;
		count++;
    }
	
	cout<<"extract counter : "<<entries.size()<<endl;
	//char **eachRow = new char*[entries.size()];
	//char **tableRow = new char*[entries.size()];
	struct routingTableEntry rte;
	for(int i = 0; i<entries.size(); i++)
	{	
		rte = getRouteEntry(entries[i],"#");
		cout<<"dest : "<<rte.dest<<"next : "<<rte.next<<"cost : "<<rte.cost<<endl;
	}
	
	/*char *t1 = strtok(eachRow[0],"#");
	cout<<t1<<endl;
	while(t1!=NULL)
	{
		t1 = strtok(NULL,"#");
		cout<<t1<<endl;
	}
	char *t2 = strtok(eachRow[1],"#");
	cout<<t2<<endl;
	while(t2!=NULL)
	{
		t2 = strtok(NULL,"#");
		cout<<t2<<endl;
	}
	char *t3 = strtok(eachRow[2],"#");
	cout<<t3<<endl;
	while(t3!=NULL)
	{
		t3 = strtok(NULL,"#");
		cout<<t3<<endl;
	}
	char *t4 = strtok(eachRow[3],"#");
	cout<<t4<<endl;
	while(t4!=NULL)
	{
		t4 = strtok(NULL,"#");
		cout<<t4<<endl;
	}

	*/
	return 0;
}
