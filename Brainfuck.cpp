#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;
string s;
int ptr=50000,top=0;
int paper[1000001];
int st[1000001];
stringstream ss; 
int main(int argc,char *argv[])
{
	if(argc>=2)
	{
		ifstream fin(argv[1]);
		if(!fin)
		{
			cerr<<"File Not Found.\n";
			return 0;
		}
		fin.tie(0);
	    ss.tie(0);
		ss<<fin.rdbuf();
		s=ss.str();
	}
	ios::sync_with_stdio(0);
	cin.tie(0);
	cout.tie(0);
	if(argc<2)
	{
		getline(cin,s);
	}
	for(int i=0;i<s.size();i++)
	{
		if(!top&&s[i]==']')
		{
			cout<<"Runtime Error"<<endl;
			return 0;
		}
		if(s[i]=='+')
		{
			paper[ptr]++;
		}
		else if(s[i]=='-')
		{
			paper[ptr]--;
		}
		else if(s[i]=='[')
		{
			st[++top]=i;
		}
		else if(s[i]==']')
		{
			if(paper[ptr])
			{
				i=st[top];
			}
			if(!paper[ptr])
			{
				top--;
			}
		}
		else if(s[i]=='>')
		{
			ptr++;
			if(ptr>=1000000)
			{
				cout<<"Memory Limit Exceed\n";
				return 0;
			}
		}
		else if(s[i]=='<')
		{
			ptr--;
			if(ptr==0)
			{
				cout<<"Runtime Error\n";
				return 0;
			}
		}
		else if(s[i]==',')
		{
			paper[ptr]=cin.get();
		}
		else if(s[i]=='.')
		{
			cout<<char(paper[ptr]);
		}
		else
		{
			continue;
		}
	}
	return 0;
}
