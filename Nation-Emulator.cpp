#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#ifndef __linux__
	#include <windows.h>
	#include <conio.h> 
#endif
#include <ctime>
#include <set>
using namespace std;
const int stuckcnt=5000;//到stuckcnt时直接枚举,不使用随机数 
string l[7]={"","石器时代","青铜时代","铁器时代","工业时代","原子时代","信息时代"};
string so[7]={"","石斧步兵","持矛步兵","铁甲骑兵","步兵连","坦克连","重装合成旅"};
struct nation
{
	string name;
	long long army;
	int level;
	bool alive;
};
int n;
int cnt; 
set<int>nuclear;
nation x[10000001];
#ifndef __linux__
	void lose()
	{
		for(int i=1;i<=n;i++)
		{
			if(x[i].alive)
			{
				return;
			}
		}
		cout<<"所有国家都灭亡了,世界再次恢复了沉寂......\n";
		system("pause");
		exit(0);
	}
	void win()
	{
		for(int i=1;i<=n;i++)
		{
			if(x[i].alive)
			{
				cout<<x[i].name<<"称霸了世界!\n";
				cout<<x[i].name<<"数据:\n";
				cout<<"战力:"<<x[i].army<<endl;
				cout<<"文明等级:"<<l[x[i].level]<<endl;
				system("pause");
				exit(0); 
			}
		}
	}
	void randth()
	{
		int b=rand()%7+1;
	//	cout<<cnt<<endl;
		if(cnt==0)
		{
			lose();
		}
		if(cnt==1)
		{
			win();
		}
		if(b==1)//战争
		{
			int a=rand()%n+1;
			int c=0;
			while(x[a].alive==0)	
			{
				c++;
				a=rand()%n+1;
				if(c==stuckcnt)
				{
					for(int i=1;i<=n;i++)
					{
						if(x[i].alive)
						{
							a=i;
							break;
						}
						else if(i==n)
						{
							win();
						}
					}
				}
			}
			int b=rand()%n+1;
			c=0;
			while(b==a||x[b].alive==0)	
			{
				c++;
				b=rand()%n+1;
				if(c==stuckcnt)
				{
					for(int i=1;i<=n;i++)
					{
						if(x[i].alive&&i!=a)
						{
							b=i;
							break; 
						}
						else if(i==n)
						{
							win();
						}
					}
				}
			}
			cout<<x[a].name<<"向"<<x[b].name<<"宣战\n";
			if(x[a].army>x[b].army)
			{
				cout<<x[a].name<<"将"<<x[b].name<<"打败\n";
				x[b].army/=2;
				x[a].army-=x[b].army*1.5;
			}
			else if(x[a].army==x[b].army)
			{
				cout<<x[a].name<<"与"<<x[b].name<<"打了个平手\n";
			}
			else
			{
				cout<<x[b].name<<"将"<<x[a].name<<"打败\n";
				x[a].army/=2;
				x[b].army-=x[a].army*1.5;
			}
		} 
		else if(b==2)//科技爆炸 
		{
			int a=rand()%n+1;
			int c=0;
			while(x[a].alive==0)	
			{
				c++;
				a=rand()%n+1;
				if(c==stuckcnt)
				{
					for(int i=1;i<=n;i++)
					{
						if(x[i].alive)
						{
							a=i;
							break;
						}
						else if(i==n)
						{
							win();
						}
					}
				}
			}
			if(x[a].level<6)
			{
				x[a].level++;
				cout<<x[a].name<<"发生科技爆炸,进入"<<l[x[a].level]<<endl;
				x[a].army+=x[a].level*100;
				if(x[a].level==5)
				{
					nuclear.insert(a);
				}
			}
		}
		else if(b==3)
		{
			int a=rand()%n+1;
			int c=0;
			while(x[a].alive==0)	
			{
				c++;
				a=rand()%n+1;
				if(c==stuckcnt)
				{
					for(int i=1;i<=n;i++)
					{
						if(x[i].alive)
						{
							a=i;
							break;
						}
						else if(i==n)
						{
							win();
						}
					}
				}
			}
			int t=rand()%10+30;
			cout<<x[a].name<<"制造了"<<t*5<<"个"<<so[x[a].level]<<",战力增加"<<(x[a].level*x[a].level)*t<<endl;
			x[a].army+=(x[a].level*x[a].level)*t;
		}
		else if(nuclear.size()>1&&b==4)
		{
			int a=rand()%nuclear.size()+1;
			set<int>::iterator ss=nuclear.begin();
			for(int j=1;j<=a;j++,ss++)
			{
				ss++;
			}
			a=*ss;
			int b=rand()%n+1;
			int c=0;
			while(b==a||x[b].alive==0)	
			{
				c++;
				b=rand()%n+1;
				if(c==stuckcnt)
				{
					for(int i=1;i<=n;i++)
					{
						if(x[i].alive&&i!=a)
						{
							b=i;
							break;
						}
						else if(i==n)
						{
							win();
						}
					}
				}
			}
			if(x[a].level>=5)
			{
				cout<<x[a].name<<"对"<<x[b].name<<"发动了核打击\n";
				if(x[a].level==6)
				{
					if(x[b].level==6)
					{
						int xx=rand()%10+1;
						if(xx<=5)
						{
							cout<<x[b].name<<"拦截并击毁了核弹\n";
						}
						else
						{
							int xxxx=rand()%3+1;
							if(xxxx==1)
							{
								cout<<">>>";
								cout<<x[a].name<<"将"<<x[b].name<<"用大量核弹炸成了一片废墟,"<<x[b].name<<"灭亡了\n";
								if(x[b].level>=5)
								{
									nuclear.erase(b);
								}
								Sleep(1000);
								x[b].alive=0;
								cnt--;
							}
							else
							{
								cout<<">>>";
								cout<<x[a].name<<"将"<<x[b].name<<"用大量核弹炸成了一片废墟,"<<x[b].name<<"文明降级\n";
								if(x[b].level>=5)
								{
									nuclear.erase(b);
								}
								x[b].army/=3;
								x[b].level--;
								x[b].level=max(x[b].level,1);
								Sleep(1000);
								cnt--;
							}
						}
					}
					else
					{
						int xxxx=rand()%3+1;
						if(xxxx==1)
						{
							cout<<">>>";
							cout<<x[a].name<<"将"<<x[b].name<<"用大量核弹炸成了一片废墟,"<<x[b].name<<"灭亡了\n";
							if(x[b].level>=5)
							{
								nuclear.erase(b);
							}
							Sleep(1000);
							x[b].alive=0;
							cnt--;
						}
						else
						{
							cout<<">>>";
							cout<<x[a].name<<"将"<<x[b].name<<"用大量核弹炸成了一片废墟,"<<x[b].name<<"文明降级\n";
							if(x[b].level>=5)
							{
								nuclear.erase(b);
							}
							x[b].army/=2;
							x[b].level--;
							x[b].level=max(x[b].level,1);
							Sleep(1000);
						}
					}
				}
				else
				{
					int yy=rand()%10+1;
					if(yy<=7)
					{
						if(x[b].level==6)
						{
							int xx=rand()%10+1;
							if(xx<=5)
							{
								cout<<x[b].name<<"拦截并击毁了所有核弹\n";
							}
							else
							{
								int xxxx=rand()%3+1;
								if(xxxx==1)
								{
									cout<<">>>";
									cout<<x[a].name<<"将"<<x[b].name<<"用大量核弹炸成了一片废墟,"<<x[b].name<<"灭亡了\n";
									if(x[b].level>=5)
									{
										nuclear.erase(b);
									}
									Sleep(1000);
									x[b].alive=0;
									cnt--;
								}
								else
								{
									cout<<">>>";
									cout<<x[a].name<<"将"<<x[b].name<<"用大量核弹炸成了一片废墟,"<<x[b].name<<"文明降级\n";
									x[b].army/=2;
									if(x[b].level>=5)
									{
										nuclear.erase(b);
									}
									x[b].level--;
									x[b].level=max(x[b].level,1);
									Sleep(1000);
								}
							}
						}
						else
						{
							int xxxx=rand()%3+1;
							if(xxxx==1)
							{
								cout<<">>>";
								cout<<x[a].name<<"将"<<x[b].name<<"用大量核弹炸成了一片废墟,"<<x[b].name<<"灭亡了\n";
								if(x[b].level>=5)
								{
									nuclear.erase(b);
								}
								Sleep(1000);
								x[b].alive=0;
								cnt--;
							}
							else
							{
								cout<<">>>";
								cout<<x[a].name<<"将"<<x[b].name<<"用大量核弹炸成了一片废墟,"<<x[b].name<<"文明降级\n";
								x[b].army/=2;
								if(x[b].level>=5)
								{
									nuclear.erase(b);
								}
								x[b].level--;
								x[b].level=max(x[b].level,1);
								Sleep(1000);
							}
						}
					}
					else
					{
						cout<<x[a].name<<"射向"<<x[b].name<<"的核弹打偏了\n";
					}
				}
			}
		}
		else if(b==5)
		{
			int ccc=rand()%5+1;
			if(ccc<=2)
			{
				int a=rand()%n+1;
				int c=0;
				while(x[a].alive==0)	
				{
					c++;
					a=rand()%n+1;
					if(c==stuckcnt)
					{
						for(int i=1;i<=n;i++)
						{
							if(x[i].alive)
							{
								a=i;
								break;
							}
							else if(i==n)
							{
								win();
							}
						}
					}
				}
				int xxx=rand()%20+1;
				if(xxx<=3)
				{
					cout<<">>>"<<x[a].name<<"的群众起义,"<<x[a].name<<"灭亡了\n";
					Sleep(1000);
					x[a].alive=0;
					cnt--;
					if(x[a].level>=5)
					{
						nuclear.erase(a);
					}
				}
				else
				{
					cout<<x[a].name<<"的群众起义,"<<x[a].name<<"政府成功镇压\n";
					x[a].army*=0.7;
				}
			}
		}
		else if(b==6)
		{
			int ccc=rand()%5+1;
			if(ccc==1)
			{
				int a=rand()%n+1;
				int c=0;
				while(x[a].alive==0)	
				{
					c++;
					a=rand()%n+1;
					if(c==stuckcnt)
					{
						for(int i=1;i<=n;i++)
						{
							if(x[i].alive)
							{
								a=i;
								break;
							}
							else if(i==n)
							{
								win();
							}
						}
					}
				}
				int xxx=rand()%10+1;
				if(x[a].level>=4)
				{
					cout<<x[a].name<<"爆发瘟疫,";
					cout<<x[a].name<<"成功控制住了疫情\n";
					x[a].army*=0.8;
				}
				else
				{
					if(xxx<=2)
					{
						cout<<">>>"<<x[a].name<<"爆发瘟疫,"<<x[a].name<<"灭亡了\n"; 
						Sleep(1000);
						x[a].alive=0;
						cnt--;
						if(x[a].level>=5)
						{
							nuclear.erase(a);
						}
					}
					else
					{
						cout<<x[a].name<<"爆发瘟疫,";
						cout<<x[a].name<<"成功控制住了疫情\n";
						x[a].army*=0.6;
					}
				}
			}
		}
		else if(b==7)
		{
			int ccc=rand()%5+1;
			if(ccc==1)
			{
				int a=rand()%n+1;
				int c=0;
				while(x[a].alive==0)	
				{
					c++;
					a=rand()%n+1;
					if(c==stuckcnt)
					{
						for(int i=1;i<=n;i++)
						{
							if(x[i].alive)
							{
								a=i;
								break;
							}
							else if(i==n)
							{
								win();
							}
						}
					}
				}
				int xxx=rand()%100+1;
				if(xxx<=5||xxx>=90)
				{
					if(x[a].level>=5)
					{
						cout<<x[a].name<<"的核电站堆芯熔毁,发生氢气爆炸,"<<x[a].name<<"政府迅速控制住了事态\n";
						x[a].army*=0.9;	
						if(x[a].name=="Japan"||x[a].name=="japan"||x[a].name=="JP"||x[a].name=="jp"||x[a].name=="岛国"||x[a].name=="日本"||x[a].name=="小日子")
						{
							cout<<"并于2023年8月24日排出了受堆芯污染的核污水\n";//恶搞小日子 
						}
					}
				}
			}
		}
		for(int i=1;i<=n;i++)
		{
			if(x[i].army<=0&&x[i].alive)
			{
				cout<<">>>"<<x[i].name<<"的军队和人口被消耗殆尽,"<<x[i].name<<"灭亡了\n";
				cnt--;
				Sleep(1000);
				x[i].alive=0;
				if(x[i].level>=5)
				{
					nuclear.erase(i);
				}
			}
		}
	}
#endif
int main()
{
	#ifndef __linux__
		srand(time(0));
		LANGID lang=GetUserDefaultUILanguage();
		if(lang!=2052)
		{
			cout<<"I\'m sorry that my Emulator did not support your language.I suggest you to use a Computer With Simplified Chinese(zh-CN).\nPress any key to continue...\n";
			getch();
			system("cls");
		}
		system("color b");
		system("title 国家模拟器");
		cout<<"国家模拟器,Copyright 2025 ZZCjas\n";
		cout<<"国家数:";
		cin>>n;
		cout<<"各个国家的名字:\n";
		for(int i=1;i<=n;i++)
		{
			cin>>x[i].name;
		}
		system("cls");
		cout<<n<<"个国家在一片大陆上诞生了......\n";
		cnt=n;
		for(int i=1;i<=n;i++) 
		{
			x[i].army=100;
			x[i].level=1;
			x[i].alive=1;
		}
		Sleep(1000);
		srand(time(0));
		while(cnt>1)
		{
			randth();
			Sleep(100);
		} 
		win();
	#endif
	#ifdef __linux__
		cout<<"Error:Please Run the program under Windows\n";
	#endif
	return 0;
}
