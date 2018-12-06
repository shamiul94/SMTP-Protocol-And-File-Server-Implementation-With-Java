#include<iostream>
#include<cstdio>

using namespace std;

int main()
{
	for(int i = 0; i<256; i++)
	{	
		unsigned char ch = i;
		printf("%d : %c\n",i,ch);
	}
	printf("\n"); 
	return 0;
}
