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
const string list[6]={
	"",
	"https://zzcjas.github.io/",
	"https://github.com/ZZCjas/Tools/raw/main/",
	"https://external.githubfast.com/https/raw.githubusercontent.com/ZZCjas/Tools/main/",
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
string link;
string alltol(string s)
{
	for(int i=0;i<s.size();i++)
	{
		s[i]=tolower(s[i]);
	}
	return s;
}
int main(int argc,char *argv[])
{
	fstream check("ZSUS.ini");
	if(!check)
	{
		FILE *p=fopen("ZSUS.ini","w");
		fprintf(p,"# Write the links that you want to check there\n");
		for(int i=1;i<=5;i++)
		{
			fprintf(p,"%s\n",list[i].c_str());
		}
		fclose(p);
//		config<<"# Write the links that you want to check down there\n";
//		for(int i=1;i<=5;i++)
//		{
//			config<<list[i]<<'\n';
//		}
//		config.flush();
	}
	if(argc==1)
	{
		cout<<"ZZCjas\'s Software Updating&Installing System(ZSUS)\nPower By Urlmon and tar\nInput the name of the software that you want to install or update:";
		cin>>s;
		getchar();
	}
	else
	{
		cout<<"ZZCjas\'s Software Updating&Installing System(ZSUS)\nPower By Urlmon and tar\n";
		s=argv[1];
	}
	ifstream config("ZSUS.ini");
	while(getline(config,link))
	{
		if(link[0]=='#')
		{
			continue;
		}
		cout<<"Checking "<<link<<'\n'; 
		if(DownloadFiles((link+s+".zip").c_str(),(s+".tmp").c_str())==TRUE) 
		{
			ifstream fin(s+".zip");
			if(!fin)
			{
				color(2);
				cout<<"Download:Finished\nNow unzipping...\n";
				rename((s+".tmp").c_str(),(s+".zip").c_str());
				color(7);
				system(("tar>nul 2>nul -xzvf "+s+".zip").c_str());
				color(2);
				cout<<"Installed from "<<link<<" Successfully.\n";
				color(7);
				system("pause");
				return 0;
			}
			if(system(("fc>nul 2>nul /B"+(s+".tmp")+" "+s+".zip").c_str())!=0)
			{
				remove((s+".zip").c_str());
				rename((s+".tmp").c_str(),(s+".zip").c_str());
				color(2);
				cout<<"Download:Finished\nNow unzipping...\n";
				color(7);
				system(("tar>nul 2>nul -xzvf "+s+".zip").c_str());
				color(2);
				cout<<"Updated from "<<link<<" Successfully.\n";
				color(7);
				system("pause");
				return 0;
			}
			else
			{
				color(2);
				cout<<"Your "<<s<<" is already the newest version on the "<<link<<'\n';
				color(7);
			}
		}
		else if(DownloadFiles((link+s+".cpp").c_str(),(s+".tmp").c_str())==TRUE)
		{
			ifstream fin(s+".cpp");
			if(!fin)
			{
				color(2);
				cout<<"Download:Finished\nNow compiling...\n";
				color(7);
				system(("g++ "+(s+".cpp")+" -o "+(s+".exe")+" -O2 -Wall -std=c++11").c_str());
				color(2);
				cout<<"Installed from "<<link<<" Successfully.\n";
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
				cout<<"Updated from "<<link<<" Successfully.\n";
				color(7);
				system("pause");
				return 0;
			}
			else
			{
				color(2);
				cout<<"Your "<<s<<" is already the newest version on the "<<link<<'\n';
				color(7);
			}
		}
		else
		{
			color(4);
			cout<<"ERROR:"<<link+s+".cpp"<<" Not found\n";
			cout<<"ERROR:"<<link+s+".zip"<<" Not found\n";
			color(7);
		}
	}
	color(2);
	cout<<"Your "<<s<<" is Already the latest Release version";
	color(7);
}
