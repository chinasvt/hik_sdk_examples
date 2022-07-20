//
// Created by xingwg on 7/19/22.
//
#include <unistd.h>
#include <iostream>
#include <cstring>
#include "HCNetSDK.h"

void CALLBACK g_RealDataCallBack_V30(LONG lRealHandle, DWORD dwDataType,
                                     unsigned char pBuffer, DWORD dwBufSize, void *dwUser) {
    switch (dwDataType) {
        case NET_DVR_SYSHEAD: //系统头
            break;
        case NET_DVR_STREAMDATA:   //码流数据
            std::cout << "len: " << dwBufSize << std::endl;
            break;
        default: //其他数据
            std::cout << "len: " << dwBufSize << std::endl;
            break;
    }
}

void CALLBACK g_ExceptionCallBack(DWORD dwType, LONG lUserID, LONG lHandle, void *pUser) {
    char tempbuf[256] = {0};
    switch (dwType) {
        case EXCEPTION_RECONNECT:    //预览时重连
            printf("----------reconnect--------%ld\n", time(nullptr));
            break;
        default:
            break;
    }
}


int main() {
    // 初始化
    NET_DVR_Init();
    DWORD sdk_version = NET_DVR_GetSDKVersion();

    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(5000, 3);
    NET_DVR_SetReconnect(10000, true);

    //设置异常消息回调函数
    NET_DVR_SetExceptionCallBack_V30(0, nullptr, g_ExceptionCallBack, nullptr);

    // 注册设备
    LONG lUserID;

    //登录参数，包括设备地址、登录用户、密码等
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    struLoginInfo.bUseAsynLogin = 0; //同步登录方式
    strcpy(struLoginInfo.sDeviceAddress, "192.168.2.200"); //设备IP地址
    struLoginInfo.wPort = 8000; //设备服务端口
    strcpy(struLoginInfo.sUserName, "admin"); //设备登录用户名
    strcpy(struLoginInfo.sPassword, "njhk1234"); //设备登录密码

    //设备信息, 输出参数
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};

    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    if (lUserID < 0) {
        printf("Login failed, error code: %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return -1;
    }

    //启动预览并设置回调数据流
    LONG lRealPlayHandle;

    NET_DVR_PREVIEWINFO struPlayInfo = {0};
    struPlayInfo.hPlayWnd = 0;         //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
    struPlayInfo.lChannel = 1;         //预览通道号
    struPlayInfo.dwStreamType = 0;         //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
    struPlayInfo.dwLinkMode = 0;         //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
    struPlayInfo.bBlocked = 1;         //0- 非阻塞取流，1- 阻塞取流

    lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo,
                                           reinterpret_cast<REALDATACALLBACK>(g_RealDataCallBack_V30), nullptr);
    if (lRealPlayHandle < 0) {
        printf("NET_DVR_RealPlay_V40 error, %d\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return -1;
    }

    sleep(10000);

    //关闭预览
    NET_DVR_StopRealPlay(lRealPlayHandle);

    //注销用户
    NET_DVR_Logout(lUserID);
    NET_DVR_Cleanup();

    return 0;
}
