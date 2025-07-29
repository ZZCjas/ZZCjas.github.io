#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <conio.h>
#include <fstream>
using namespace std;
string s;
string name;
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
		string bf=s;
		s="";
		for(int i=0;i<bf.size();i++)
		{
			if(bf[i]!=' '&&bf[i]!='\n'&&bf[i]!='\r')
			{
				s+=bf[i];
			}
		}
		bf.clear();
	}
	if(argc<2)
	{
		cout<<"Highly-Efficient Brainfuck Compile System (Windows Version)\nCopyright 2025 ZZCjas.\nThis Program Requires G++.\nInput the Brainfuck code:";
		getline(cin,s);
	}
	cout<<"Input the Code\'s name:";
	getline(cin,name);
	int p=_dup(1);
	freopen((name+".cpp").c_str(),"w",stdout);
    printf("#include <iostream>\n");
    printf("#include <string>\n");
    printf("using namespace std;\n");
    printf("string s=\"%s\";\n",s.c_str());
    printf("int ptr=50000,top=0;\n");
    printf("int paper[1000001];\n");
    printf("int st[1000001];\n");
    printf("int main()\n");
    printf("{\n");
    printf("	for(int i=0;i<s.size();i++)\n");
    printf("	{\n");
    printf("		if(!top&&s[i]==']')\n");
    printf("		{\n");
    printf("			cout<<\"Runtime Error\"<<endl;\n");
    printf("			return 0;\n");
    printf("		}\n");
    printf("		if(s[i]=='+')\n");
    printf("		{\n");
    printf("			paper[ptr]++;\n");
    printf("			if(paper[ptr]>=256)\n");
    printf("			{\n");
    printf("				paper[ptr]=0;\n");
    printf("			}\n");
    printf("		}\n");
    printf("		else if(s[i]=='-')\n");
    printf("		{\n");
    printf("			paper[ptr]--;\n");
    printf("			if(paper[ptr]<=-1)\n");
    printf("			{\n");
    printf("				paper[ptr]=255;\n");
    printf("			}\n");
    printf("		}\n");
    printf("		else if(s[i]=='[')\n");
    printf("		{\n");
    printf("			st[++top]=i;\n");
    printf("		}\n");
    printf("		else if(s[i]==']')\n");
    printf("		{\n");
    printf("			if(paper[ptr])\n");
    printf("			{\n");
    printf("				i=st[top];\n");
    printf("			}\n");
    printf("			if(!paper[ptr])\n");
    printf("			{\n");
    printf("				top--;\n");
    printf("			}\n");
    printf("		}\n");
    printf("		else if(s[i]=='>')\n");
    printf("		{\n");
    printf("			ptr++;\n");
    printf("			if(ptr>=1000000)\n");
    printf("			{\n");
    printf("				cout<<\"Memory Limit Exceed\";\n");
    printf("				return 0;\n");
    printf("			}\n");
    printf("		}\n");
    printf("		else if(s[i]=='<')\n");
    printf("		{\n");
    printf("			ptr--;\n");
    printf("			if(ptr==0)\n");
    printf("			{\n");
    printf("				cout<<\"Runtime Error\";\n");
    printf("				return 0;\n");
    printf("			}\n");
    printf("		}\n");
    printf("		else if(s[i]==',')\n");
    printf("		{\n");
    printf("			paper[ptr]=cin.get();\n");
    printf("		}\n");
    printf("		else if(s[i]=='.')\n");
    printf("		{\n");
    printf("			cout<<char(paper[ptr]);\n");
    printf("		}\n");
    printf("		else\n");
    printf("		{\n");
    printf("			continue;\n");
    printf("		}\n");
    printf("	}\n");
    printf("	return 0;\n");
    printf("}\n");
    fflush(stdout);
    _dup2(p,1);
    cout<<"Code Maked.Using G++ to Compile..."<<endl;
    fflush(stdout);
    if(system(("g++ "+name+".cpp -o "+name+".exe -O2").c_str())==0)
    {
    	cout<<"Compiled Finished!"<<endl;
	}
	else
	{
		cout<<"Error!"<<endl;
	}
	cout<<"Compiling is done and Thanks for using!"<<endl<<"Find the Latest Version of this programe on https://zzcjas.github.io/bf2c.cpp"<<endl;
	cout<<"Press any key to end the program...";
	fflush(stdout);
	_getch();
    return 0;
}
