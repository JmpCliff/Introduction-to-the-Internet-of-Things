//-*- coding : GBK -*-

#include <iostream>
#include "tchar.h"
#include <string>
#include "WzSerialPort.cpp"
#include <windows.h>

using namespace std;

#define BUTTON_FLAG 1001
#define BUTTON_COM 1002
#define TEXT_COM 2001

HINSTANCE hg_app;
WzSerialPort remote;
HANDLE ATB=0;
HANDLE hMutex = NULL;
HWND Button_flag;
HWND Button_com;
HWND Text_com;

int flag=0;
int remoteIsOpen=0;
//int remoteIsOn=0;

char com_name[0x20]={0};

DWORD WINAPI AltTAB(LPVOID lpParameter);


typedef struct Thread_args{
    WzSerialPort * r;
}Thread_args;

LRESULT CALLBACK WindowProc(
	_In_  HWND hwnd,
	_In_  UINT uMsg,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
);

int CALLBACK WinMain(
    _In_  HINSTANCE hInstance,
    _In_  HINSTANCE hPrevInstance,
    _In_  LPSTR lpCmdLine,
    _In_  int nCmdShow
  )
{

	WNDCLASS wc = { };
    wc.style=CS_HREDRAW | CS_VREDRAW;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName =_T("My Class");
	wc.hInstance = hInstance;


	RegisterClass(&wc);
	

	HWND hwnd = CreateWindow(
		_T("My Class"),			        //类名，要和刚才注册的一致
		_T("摸鱼大师Jmp.Cliff"),	    //窗口标题文字
		WS_OVERLAPPEDWINDOW,            //窗口外观样式
		38,					            //窗口相对于父级的X坐标
		20,					            //窗口相对于父级的Y坐标
		540,				            //窗口的宽度
		200,				            //窗口的高度
		NULL,				            //没有父窗口，为NULL
		NULL,				            //没有菜单，为NULL
		hInstance,			            //当前应用程序的实例句柄
		NULL);				            //没有附加数据，为NULL
	if(hwnd == NULL)                    //检查窗口是否创建成功
		return 0;
 

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
    FreeConsole();	

	MSG msg;

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

// 在WinMain后实现
LRESULT CALLBACK WindowProc(
	_In_  HWND hwnd,
	_In_  UINT uMsg,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam
)
{
    WaitForSingleObject(hMutex, INFINITE);  //互斥量
	switch(uMsg)
	{
	    case WM_DESTROY:
		    {
			    PostQuitMessage(0);
			    return 0;
		    }
        case WM_CREATE:
            {
                Button_flag=CreateWindow(_T("Button"),_T("Start!"),WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,30,50,100,50,hwnd,(HMENU)BUTTON_FLAG,hg_app,NULL);
                Button_com=CreateWindow(_T("Button"),_T("打开串口"),WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,180,50,100,50,hwnd,(HMENU)BUTTON_COM,hg_app,NULL);
                Text_com=CreateWindow(
				TEXT("EDIT"),TEXT(""),WS_VISIBLE|WS_CHILD|WS_BORDER|ES_AUTOHSCROLL, 320, 60, 140, 30, hwnd, (HMENU)TEXT_COM, hg_app,NULL);
                break;
            }
        case WM_COMMAND:
            {
                switch(LOWORD(wParam))
                {
                    case BUTTON_FLAG:
                        {
                            if(flag){
                                flag=0;
                                //remoteIsOn=0;
                                if(ATB)
                                     CloseHandle(ATB);
                                else
                                     exit(0);
                                ATB=0;
                                SendMessage(HWND(lParam), WM_SETTEXT, NULL,(LPARAM)"Start!");
                            }else{

                                if(!remoteIsOpen){
                                    MessageBox(hwnd, _T("请先打开串口"), _T("提示"), MB_OK | MB_ICONINFORMATION);
                                    break;	
                                }
                                if(ATB)
                                    CloseHandle(ATB);
                                Thread_args tg;
                                tg.r=&remote;
                                flag=1;
                                //remoteIsOn=1;
                                ATB=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)AltTAB,&tg,0,NULL);
                                SendMessage(HWND(lParam), WM_SETTEXT, NULL,(LPARAM)_T("Stop"));
                            }
                            break;
                        }
                    case BUTTON_COM:
                        {
                            if(flag){
                                MessageBox(hwnd, _T("请先点击stop停止监控"), _T("提示"), MB_OK | MB_ICONINFORMATION);	
                                break;
                            }
                            if(ATB)
                                CloseHandle(ATB);
                            
                            memset(com_name,0,0x20);
                            GetDlgItemText(hwnd,TEXT_COM,com_name,0x20);    //这里有宽字符串问题会报错，但是我这里运行起来毫无问题，可以忽略
                            remote.close();
                            if(remote.open(com_name,9600,0,8,1)){
                                remoteIsOpen=1;
						        MessageBox(hwnd, _T("成功打开串口，请您确定该串口为设备的蓝牙串口"), _T("提示"), MB_OK | MB_ICONINFORMATION);	  
                            }else{
                                remoteIsOpen=0;
                                MessageBox(hwnd, _T("打开蓝牙串口失败！"), _T("提示"), MB_OK | MB_ICONINFORMATION);	
                            }

                        }
                }
                break;
            }
	}
    ReleaseMutex(hMutex);
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


DWORD WINAPI AltTAB(LPVOID lpParameter){
    WaitForSingleObject(hMutex, INFINITE);  //互斥量
    Thread_args * tg=(Thread_args *)lpParameter;
    WzSerialPort * r=tg->r;
    if(remoteIsOpen){
        ReleaseMutex(hMutex);
        while(1){
            WaitForSingleObject(hMutex, INFINITE);  //互斥量
            char buf[1024];
            r->receive(buf,1024);
            if(!strncmp(buf,"CATCH!",6)&&flag){
                keybd_event(VK_MENU,0,0,0);
                keybd_event(VK_TAB,0,0,0);
                keybd_event(VK_TAB,0,KEYEVENTF_KEYUP,0);
                keybd_event(VK_MENU,0,KEYEVENTF_KEYUP,0);
                flag=0;
                SendMessage(Button_flag, WM_SETTEXT, NULL,(LPARAM)_T("Start!"));
            }
            memset(buf,0,1024);
            ReleaseMutex(hMutex);
        }
    }
}