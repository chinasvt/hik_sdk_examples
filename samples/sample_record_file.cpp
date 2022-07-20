//
// Created by xingwg on 7/20/22.
//
#include <unistd.h>
#include <iostream>
#include <cstring>
#include "HCNetSDK.h"


int saveRecordFile(int userId, char *srcfile, char *destfile) {
    int bRes = 1;
    int hPlayback = 0;

    //按文件下载
    if ((hPlayback = NET_DVR_GetFileByName(userId, srcfile, destfile)) < 0) {
        printf("GetFileByName failed. error[%d]\n", NET_DVR_GetLastError());
        bRes = -1;
        return bRes;
    }

    //开始下载
    if (!NET_DVR_PlayBackControl_V40(hPlayback, NET_DVR_PLAYSTART, NULL, 0, NULL, NULL)) {
        printf("play back control failed [%d]\n", NET_DVR_GetLastError());
        bRes = -1;
        return bRes;
    }

    //获取下载进度
    int nPos = 0;
    for (nPos = 0; nPos < 100 && nPos >= 0; nPos = NET_DVR_GetDownloadPos(hPlayback)) {
        printf("Be downloading...%d %%\n", nPos);
        sleep(5);  //millisecond
    }
    printf("have got %d\n", nPos);

    if (nPos < 0 || nPos > 100) {
        printf("download err [%d]\n", NET_DVR_GetLastError());
        bRes = -1;
        return bRes;
    } else {
        return 0;
    }

    //停止下载，释放资源
    if (!NET_DVR_StopGetFile(hPlayback)) {
        printf("failed to stop get file [%d]\n", NET_DVR_GetLastError());
        bRes = -1;
        return bRes;
    }
}


int main() {
    // 初始化
    NET_DVR_Init();
    DWORD sdk_version = NET_DVR_GetSDKVersion();

    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(5000, 3);
    NET_DVR_SetReconnect(10000, true);

    // 注册设备
    LONG lUserID;

    //登录参数，包括设备地址、登录用户、密码等
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    struLoginInfo.bUseAsynLogin = 0; //同步登录方式
    strcpy(struLoginInfo.sDeviceAddress, "192.168.2.180"); //设备IP地址
    struLoginInfo.wPort = 8000; //设备服务端口
    strcpy(struLoginInfo.sUserName, "admin"); //设备登录用户名
    strcpy(struLoginInfo.sPassword, "jsjd1234"); //设备登录密码

    //设备信息, 输出参数
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};

    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    if (lUserID < 0) {
        printf("Login failed, error code: %d\n", NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return -1;
    }

    NET_DVR_FILECOND_V50 pFindCond = {0};
    pFindCond.dwFileType = 0xFF;
    pFindCond.struStreamID.dwSize = sizeof(NET_DVR_STREAM_INFO);
    pFindCond.struStreamID.dwChannel = 33;
    pFindCond.byIsLocked = 0xFF;
    pFindCond.byNeedCard = 0;
    pFindCond.struStartTime.wYear = 2022;
    pFindCond.struStartTime.byMonth = 7;
    pFindCond.struStartTime.byDay = 20;
    pFindCond.struStartTime.byHour = 9;
    pFindCond.struStartTime.byMinute = 14;
    pFindCond.struStartTime.bySecond = 0;
    pFindCond.struStopTime.wYear = 2022;
    pFindCond.struStopTime.byMonth = 7;
    pFindCond.struStopTime.byDay = 20;
    pFindCond.struStopTime.byHour = 9;
    pFindCond.struStopTime.byMinute = 15;
    pFindCond.struStopTime.bySecond = 0;

    int lFindHandle = NET_DVR_FindFile_V50(lUserID, &pFindCond);
    if (lFindHandle < 0) {
        printf("find file fail,last error %d\n", NET_DVR_GetLastError());
        return -1;
    }

    //逐个获取查询文件结果
    NET_DVR_FINDDATA_V50 struFileData;
    char sFileName[100] = {0}; //需要下载的文件
    int iFileNum = 0;
    while (true) {
        int result = NET_DVR_FindNextFile_V50(lFindHandle, &struFileData);
        if (result == NET_DVR_ISFINDING) {
            continue;
        } else if (result == NET_DVR_FILE_SUCCESS) {
            iFileNum++;
            printf("find file no: %d, file name:%s\n", iFileNum, struFileData.sFileName);
            if (iFileNum == 1) {
                strcpy(sFileName, struFileData.sFileName);
            }
            continue;
        } else if (result == NET_DVR_FILE_NOFIND || result == NET_DVR_NOMOREFILE) {
            break;
        } else {
            printf("find file fail for illegal get file state");
            break;
        }
    }

    //停止查找，释放资源
    if (lFindHandle >= 0)
        NET_DVR_FindClose_V30(lFindHandle);

    // 按文件下载，本示例是下载查找到的第一个文件实际开发可以将查找的文件添加到列表里面，然后根据需要选择文件下载
    printf("download the file: %s\n", sFileName);
    char strSaveFile[256] = {0};
    sprintf(strSaveFile, "./%s.mp4", sFileName);
    saveRecordFile(lUserID, sFileName, strSaveFile);

    //注销用户
    NET_DVR_Logout(lUserID);
    NET_DVR_Cleanup();

    return 0;
}