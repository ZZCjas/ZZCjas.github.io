#include <urlmon.h>
#include <windows.h>
#include <fstream>
#include <iostream>
using namespace std;
BOOL FileExistsStatus(const CHAR* path)
{
	DWORD dwAttribute=GetFileAttributes((LPCSTR)path);
	if(dwAttribute==0XFFFFFFFF) 
	{
		return 0; 
	}
	else 
	{
		return 1;
	}
}
BOOL DownloadFiles(const CHAR* url,const CHAR* downloadPath)
{
	if(URLDownloadToFile(NULL,(LPCSTR)url,(LPCSTR)downloadPath,0,0)==S_OK&&FileExistsStatus(downloadPath)) 
	{
		return true;
	}
	else
	{
		return false;
	}
}
string s;
string list[6]={
	"",
	"https://zzcjas.github.io/",
	"https://github.com/ZZCjas/Tools/raw/main/",
	"https://githubfast.com/ZZCjas/Tools/raw/main/",
	"https://hub.nuaa.cf/ZZCjas/Tools/raw/main/",
	"https://kgithub.com/ZZCjas/Tools/raw/main/"
};
bool f=1;
void color(int x)
{
	if(x>=0&&x<=15)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),x);
	}
	else
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),7);
	}
}
int main(int argc,char *argv[])
{
	if(argc==1)
	{
		cout<<"ZZCjas\'s Software Install&Updating System(ZSUS)\nInput the name of my software that you want to install&update:";
		cin>>s;
	}
	else
	{
		s=argv[1];
	}
	for(int i=1;i<=5;i++)
	{
		if(GetUserDefaultUILanguage()==2052&&i==2)
		{
			color(6); 
			cout<<"WARNING:Skipping github.com Because it is hard to visit it in Chinese Mainland\n";
			color(7);
			continue;
		}
		if(DownloadFiles((list[i]+s+".zip").c_str(),(s+".tmp").c_str())==TRUE) 
		{
			ifstream fin(s+".zip");
			if(!fin)
			{
				color(2);
				cout<<"Download:Finished\nNow unzipping...";
				color(7);
				system(("tar -xzvf "+s+".zip").c_str());
				cout<<"Installed from "<<list[i]+s+".zip"<<" Successfully.\n";
				system("pause");
				return 0;
			}
			if(system(("fc>nul 2>nul /B"+(s+".tmp")+" "+s+".zip").c_str())!=0)
			{
				remove((s+".zip").c_str());
				rename((s+".tmp").c_str(),(s+".zip").c_str());
				color(2);
				cout<<"Download:Finished\nNow unzipping...";
				color(7);
				system(("tar -xzvf "+s+".zip").c_str());
				cout<<"Updated from "<<list[i]+s+".zip"<<" Successfully.\n";
				system("pause");
				return 0;
			}
			else
			{
				color(2);
				cout<<"As same as "<<list[i]+s+".zip"<<'\n';
				color(7);
			}
		}
		else if(DownloadFiles((list[i]+s+".cpp").c_str(),(s+".tmp").c_str())==TRUE)
		{
			ifstream fin(s+".cpp");
			if(!fin)
			{
				color(2);
				cout<<"Download:Finished\nNow compiling...";
				color(7);
				system(("g++ "+(s+".cpp")+" -o "+(s+".exe")+" -O2 -Wall -std=c++11").c_str());
				color(2);
				cout<<"Installed from "<<list[i]+s+".cpp"<<" Successfully.\n";
				color(7);
				system("pause");
				return 0;
			}
			if(system(("fc>nul 2>nul "+(s+".tmp")+" "+s+".cpp").c_str())!=0)
			{
				remove((s+".cpp").c_str());
				rename((s+".tmp").c_str(),(s+".cpp").c_str());
				color(2);
				cout<<"Download:Finished\nNow compiling...\n";
				color(7);
				system(("g++ "+(s+".cpp")+" -o "+(s+".exe")+" -O2 -Wall -std=c++11").c_str());
				color(2);
				cout<<"Updated from "<<list[i]+s+".cpp"<<" Successfully.\n";
				color(7);
				system("pause");
				return 0;
			}
			else
			{
				color(2);
				cout<<"As same as "<<list[i]+s+".cpp"<<'\n';
				color(7);
			}
		}
		else
		{
			color(4);
			cout<<"ERROR:"<<list[i]+s+".cpp"<<" Not found\n";
			cout<<"ERROR:"<<list[i]+s+".zip"<<" Not found\n";
			color(7);
		}
	}
	color(2);
	cout<<"Your "<<s<<" is Already the latest Release version";
	color(7);
}
