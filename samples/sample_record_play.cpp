//
// Created by xingwg on 7/21/22.
//

#include <iostream>
#include "unistd.h"
#include <cstring>
#include "HCNetSDK.h"

//typedef HWND(WINAPI *PROCGETCONSOLEWINDOW)();
//PROCGETCONSOLEWINDOW GetConsoleWindowAPI;

typedef void (CALLBACK *PlayDataCallBackFunc)(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser);


void CALLBACK *fPlayDataCallBack_V40(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser) {
    switch (dwDataType) {
        case NET_DVR_SYSHEAD:  //系统头
            break;
        case NET_DVR_STREAMDATA:   //码流数据
            std::cout << "len: " << dwBufSize << std::endl;
            break;
        case NET_DVR_CHANGE_FORWARD:
            break;
        case NET_DVR_CHANGE_REVERSE:
            break;
        default: //其他数据
            std::cout << "len: " << dwBufSize << std::endl;
            break;
    }
}


int main() {

    //---------------------------------------
    // 初始化
    NET_DVR_Init();
    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);

    //---------------------------------------
    // 获取控制台窗口句柄
//    HMODULE hKernel32 = GetModuleHandle((LPCWSTR)"kernel32");
//    GetConsoleWindowAPI = (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32, "GetConsoleWindow");

    //---------------------------------------
    // 注册设备
    LONG lUserID;

    //登录参数，包括设备地址、登录用户、密码等
    NET_DVR_USER_LOGIN_INFO struLoginInfo = { 0 };
    struLoginInfo.bUseAsynLogin = 0; //同步登录方式
    strcpy(struLoginInfo.sDeviceAddress, "192.168.2.180"); //设备IP地址
    struLoginInfo.wPort = 8000; //设备服务端口
    strcpy(struLoginInfo.sUserName, "admin"); //设备登录用户名
    strcpy(struLoginInfo.sPassword, "jsjd1234"); //设备登录密码

    //设备信息, 输出参数
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = { 0 };

    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    if (lUserID < 0) {
        printf("Login failed, error code: %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return -1;
    }

//    HWND hWnd = GetConsoleWindow(); //获取窗口句柄

    NET_DVR_VOD_PARA_V50 struVodPara = { 0 };
    struVodPara.dwSize = sizeof(struVodPara);
    struVodPara.struIDInfo.dwSize = sizeof(NET_DVR_STREAM_INFO);
    struVodPara.struIDInfo.dwChannel = 33;  // NVR channel start 33
    struVodPara.hWnd = 0;

    struVodPara.struBeginTime.wYear = 2022;
    struVodPara.struBeginTime.byMonth = 7;
    struVodPara.struBeginTime.byDay = 17;
    struVodPara.struBeginTime.byHour = 9;
    struVodPara.struBeginTime.byMinute = 0;
    struVodPara.struBeginTime.bySecond = 0;
    struVodPara.struEndTime.wYear = 2022;
    struVodPara.struEndTime.byMonth = 7;
    struVodPara.struEndTime.byDay = 17;
    struVodPara.struEndTime.byHour = 10;
    struVodPara.struEndTime.byMinute = 0;
    struVodPara.struEndTime.bySecond = 0;

    //---------------------------------------
    //按时间回放
    int hPlayback;
    hPlayback = NET_DVR_PlayBackByTime_V50(lUserID, &struVodPara);
    if (hPlayback < 0) {
        printf("NET_DVR_PlayBackByTime_V40 fail,last error %d\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return -1;
    }

    if (!NET_DVR_SetPlayDataCallBack_V40(hPlayback, reinterpret_cast<PlayDataCallBackFunc>(fPlayDataCallBack_V40), nullptr)) {
        printf("NET_DVR_SetPlayDataCallBack_V40 fail, last error %d\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return -1;
    }

    //---------------------------------------
    //开始
    if (!NET_DVR_PlayBackControl_V40(hPlayback, NET_DVR_PLAYSTART, nullptr, 0, nullptr, nullptr)) {
        printf("play back control failed [%d]\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return -1;
    }

    sleep(150);  //millisecond

    //停止回放
    if (!NET_DVR_StopPlayBack(hPlayback)) {
        printf("failed to stop file [%d]\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return -1;
    }

    //注销用户
    NET_DVR_Logout(lUserID);

    //释放SDK资源
    NET_DVR_Cleanup();
    return 0;
}

