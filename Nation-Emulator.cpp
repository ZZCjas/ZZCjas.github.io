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
const int stuckcnt=5000;//��stuckcntʱֱ��ö��,��ʹ������� 
string l[7]={"","ʯ��ʱ��","��ͭʱ��","����ʱ��","��ҵʱ��","ԭ��ʱ��","��Ϣʱ��"};
string so[7]={"","ʯ������","��ì����","�������","������","̹����","��װ�ϳ���"};
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
		cout<<"���й��Ҷ�������,�����ٴλָ��˳���......\n";
		system("pause");
		exit(0);
	}
	void win()
	{
		for(int i=1;i<=n;i++)
		{
			if(x[i].alive)
			{
				cout<<x[i].name<<"�ư�������!\n";
				cout<<x[i].name<<"����:\n";
				cout<<"ս��:"<<x[i].army<<endl;
				cout<<"�����ȼ�:"<<l[x[i].level]<<endl;
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
		if(b==1)//ս��
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
			cout<<x[a].name<<"��"<<x[b].name<<"��ս\n";
			if(x[a].army>x[b].army)
			{
				cout<<x[a].name<<"��"<<x[b].name<<"���\n";
				x[b].army/=2;
				x[a].army-=x[b].army*1.5;
			}
			else if(x[a].army==x[b].army)
			{
				cout<<x[a].name<<"��"<<x[b].name<<"���˸�ƽ��\n";
			}
			else
			{
				cout<<x[b].name<<"��"<<x[a].name<<"���\n";
				x[a].army/=2;
				x[b].army-=x[a].army*1.5;
			}
		} 
		else if(b==2)//�Ƽ���ը 
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
				cout<<x[a].name<<"�����Ƽ���ը,����"<<l[x[a].level]<<endl;
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
			cout<<x[a].name<<"������"<<t*5<<"��"<<so[x[a].level]<<",ս������"<<(x[a].level*x[a].level)*t<<endl;
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
				cout<<x[a].name<<"��"<<x[b].name<<"�����˺˴��\n";
				if(x[a].level==6)
				{
					if(x[b].level==6)
					{
						int xx=rand()%10+1;
						if(xx<=5)
						{
							cout<<x[b].name<<"���ز������˺˵�\n";
						}
						else
						{
							int xxxx=rand()%3+1;
							if(xxxx==1)
							{
								cout<<">>>";
								cout<<x[a].name<<"��"<<x[b].name<<"�ô����˵�ը����һƬ����,"<<x[b].name<<"������\n";
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
								cout<<x[a].name<<"��"<<x[b].name<<"�ô����˵�ը����һƬ����,"<<x[b].name<<"��������\n";
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
							cout<<x[a].name<<"��"<<x[b].name<<"�ô����˵�ը����һƬ����,"<<x[b].name<<"������\n";
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
							cout<<x[a].name<<"��"<<x[b].name<<"�ô����˵�ը����һƬ����,"<<x[b].name<<"��������\n";
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
								cout<<x[b].name<<"���ز����������к˵�\n";
							}
							else
							{
								int xxxx=rand()%3+1;
								if(xxxx==1)
								{
									cout<<">>>";
									cout<<x[a].name<<"��"<<x[b].name<<"�ô����˵�ը����һƬ����,"<<x[b].name<<"������\n";
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
									cout<<x[a].name<<"��"<<x[b].name<<"�ô����˵�ը����һƬ����,"<<x[b].name<<"��������\n";
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
								cout<<x[a].name<<"��"<<x[b].name<<"�ô����˵�ը����һƬ����,"<<x[b].name<<"������\n";
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
								cout<<x[a].name<<"��"<<x[b].name<<"�ô����˵�ը����һƬ����,"<<x[b].name<<"��������\n";
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
						cout<<x[a].name<<"����"<<x[b].name<<"�ĺ˵���ƫ��\n";
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
					cout<<">>>"<<x[a].name<<"��Ⱥ������,"<<x[a].name<<"������\n";
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
					cout<<x[a].name<<"��Ⱥ������,"<<x[a].name<<"�����ɹ���ѹ\n";
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
					cout<<x[a].name<<"��������,";
					cout<<x[a].name<<"�ɹ�����ס������\n";
					x[a].army*=0.8;
				}
				else
				{
					if(xxx<=2)
					{
						cout<<">>>"<<x[a].name<<"��������,"<<x[a].name<<"������\n"; 
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
						cout<<x[a].name<<"��������,";
						cout<<x[a].name<<"�ɹ�����ס������\n";
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
						cout<<x[a].name<<"�ĺ˵�վ��о�ۻ�,����������ը,"<<x[a].name<<"����Ѹ�ٿ���ס����̬\n";
						x[a].army*=0.9;	
						if(x[a].name=="Japan"||x[a].name=="japan"||x[a].name=="JP"||x[a].name=="jp"||x[a].name=="����"||x[a].name=="�ձ�"||x[a].name=="С����")
						{
							cout<<"����2023��8��24���ų����ܶ�о��Ⱦ�ĺ���ˮ\n";//���С���� 
						}
					}
				}
			}
		}
		for(int i=1;i<=n;i++)
		{
			if(x[i].army<=0&&x[i].alive)
			{
				cout<<">>>"<<x[i].name<<"�ľ��Ӻ��˿ڱ����Ĵ���,"<<x[i].name<<"������\n";
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
		system("title ����ģ����");
		cout<<"����ģ����,Copyright 2025 ZZCjas\n";
		cout<<"������:";
		cin>>n;
		cout<<"�������ҵ�����:\n";
		for(int i=1;i<=n;i++)
		{
			cin>>x[i].name;
		}
		system("cls");
		cout<<n<<"��������һƬ��½�ϵ�����......\n";
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
