#include <iostream>
#include <string>
#include "WzSerialPort.cpp"
#include <windows.h>

using namespace std;

int main(){
    WzSerialPort r;
    if(r.open("COM7",9600,0,8,1)){
        
        while(1){    
            char buf[1024];
            r.receive(buf,1024);
            if(strcmp("CATCH!",buf)){
                cout<<buf;
                keybd_event(VK_MENU,0,0,0);
                keybd_event(VK_TAB,0,0,0);
                keybd_event(VK_TAB,0,KEYEVENTF_KEYUP,0);
                keybd_event(VK_MENU,0,KEYEVENTF_KEYUP,0);
                break;
            }
        }
    }

}
