#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <direct.h>
using namespace std;
void delfile()
{
	cout<<"����ɾ�������������ļ�..."; 
	system("del /f C:\\autorun.inf");
	system("del /f D:\\autorun.inf");
	system("del /f E:\\autorun.inf");
	system("del /f C:\\virus.com");
	system("del /f D:\\virus.com");
	system("del /f E:\\virus.com");
}
BOOL UpPrivilegeValue()
{
    HANDLE hToken=0;
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ALL_ACCESS,&hToken))
    {
        return FALSE;
    }
    LUID luid;
    if(!LookupPrivilegeValue(0,SE_DEBUG_NAME,&luid))
    {
        CloseHandle(hToken);
        return FALSE;
    }
    TOKEN_PRIVILEGES Tok;
    Tok.PrivilegeCount=1;
    Tok.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
    Tok.Privileges[0].Luid=luid;
    if(FALSE==AdjustTokenPrivileges(hToken,FALSE,&Tok,sizeof(Tok),0,0))
    {
        CloseHandle(hToken);
        return FALSE;
    }
    if(GetLastError()==ERROR_NOT_ALL_ASSIGNED)
    {
        CloseHandle(hToken);
        return FALSE;
    }
    CloseHandle(hToken);
    return TRUE;
}
DWORD WINAPI shadu(LPVOID Param)
{
	while(1)
	{
		cout<<"��������Microsoft Defenderȫ�̲�����ɱ...";
		system("\"C:\\Program Files\\Windows Defender\\mpcmdrun.exe\" -ScanType 2");
		Sleep(20000); 
	}
}
void CreateReg(HKEY Root,char*szSubKey,char* ValueName,char* Data)
{
    HKEY key;
    long Ret=RegCreateKeyEx(Root,szSubKey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&key,NULL);
    Ret=RegSetValueEx(key,ValueName,0,REG_SZ,(BYTE*)Data,strlen(Data));
    RegCloseKey(key);
}
void kill()
{
	UpPrivilegeValue();
	while(1)
	{
		DWORD ProcessId=0;
	    HANDLE hProcessSnapShot=NULL;
	    PROCESSENTRY32 pe32;
	    ZeroMemory(&pe32,sizeof(pe32));
	    hProcessSnapShot=::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,NULL);
	    pe32.dwSize=sizeof(PROCESSENTRY32);
	    if(Process32First(hProcessSnapShot,&pe32))
	    {
	        do
	        { 
	            if("����.exe"==(string)(pe32.szExeFile))
	            {
	            	cout<<"����һ����������,PID="<<pe32.th32ProcessID<<endl;
	                ProcessId=pe32.th32ProcessID;
	                break;
	            }
	        }while(Process32Next(hProcessSnapShot,&pe32));
	    }
	    ::CloseHandle(hProcessSnapShot);
	    HANDLE hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,ProcessId);
	    TerminateProcess(hProcess,0);
	    cout<<"�Ѿ�kill��\n";
	    Sleep(10000);
	    delfile();
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Run","����.exe","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\calc.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\explorer.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\qq.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\Taskmgr.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\notepad.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\write.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\studentmain.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\iexplorer.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\WINWORD.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\POWERPNT.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\ONENOTE.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\EXCEL.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\mspaint.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\SecHealthUI.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\360safe.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\360tray.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\AST.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\avp.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\360tray.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\SecHealthUI.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\Mcshield.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\kxetray.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\avcentre.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\avgaurd.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\hipstray.exe","Debugger","");
	    CreateReg(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\cmd.exe","Debugger","");
		Sleep(10000);
	}
}
int main()
{
	cout<<"����ɨ��...";
	CreateThread(NULL,4096,&shadu,NULL,NULL,NULL);
	kill();
}
