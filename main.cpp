
#include <unistd.h>
#include <iostream>
#include <cstring>
#include "HCNetSDK.h"

using namespace std;

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
        sleep(5000);  //millisecond
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

int main1() {

    //---------------------------------------
    // 初始化
    NET_DVR_Init();
    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);

    //---------------------------------------
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

    //查询条件
    NET_DVR_FILECOND_V50 struFileCond = {0};
    struFileCond.dwFileType = 0xFF;
    struFileCond.struStreamID.dwSize = sizeof(NET_DVR_STREAM_INFO);
    struFileCond.struStreamID.dwChannel = 33;
    struFileCond.byIsLocked = 0xFF;
    struFileCond.byNeedCard = 0;
    struFileCond.struStartTime.wYear = 2022;
    struFileCond.struStartTime.byMonth = 7;
    struFileCond.struStartTime.byDay = 16;
    struFileCond.struStartTime.byHour = 0;
    struFileCond.struStartTime.byMinute = 0;
    struFileCond.struStartTime.bySecond = 0;
    struFileCond.struStopTime.wYear = 2022;
    struFileCond.struStopTime.byMonth = 7;
    struFileCond.struStopTime.byDay = 16;
    struFileCond.struStopTime.byHour = 0;
    struFileCond.struStopTime.byMinute = 15;
    struFileCond.struStopTime.bySecond = 0;

    //---------------------------------------
    //查找录像文件
    int lFindHandle = NET_DVR_FindFile_V50(lUserID, &struFileCond);
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
    if (lFindHandle >= 0) {
        NET_DVR_FindClose_V30(lFindHandle);
    }

    /*按文件下载，本示例是下载查找到的第一个文件
    实际开发可以将查找的文件添加到列表里面，然后根据需要选择文件下载*/
    printf("download the file:%s\n", sFileName);
    char strSaveFile[256] = {0};
    sprintf(strSaveFile, "./%s.mp4", sFileName);
    saveRecordFile(lUserID, sFileName, strSaveFile);

    //注销用户
    NET_DVR_Logout(lUserID);

    //释放SDK资源
    NET_DVR_Cleanup();
    return 0;
}


int main() {

    //---------------------------------------
    // 初始化
    NET_DVR_Init();

    //设置连接时间与重连时间
    NET_DVR_SetConnectTime(5000, 3);
    NET_DVR_SetReconnect(10000, true);
    NET_DVR_SetRecvTimeOut(500000);

    //---------------------------------------
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

    NET_DVR_PLAYCOND struDownloadCond = {0};
    struDownloadCond.dwChannel = 0;

    struDownloadCond.struStartTime.dwYear = 2022;
    struDownloadCond.struStartTime.dwMonth = 7;
    struDownloadCond.struStartTime.dwDay = 16;
    struDownloadCond.struStartTime.dwHour = 9;
    struDownloadCond.struStartTime.dwMinute = 0;
    struDownloadCond.struStartTime.dwSecond = 0;
    struDownloadCond.struStopTime.dwYear = 2022;
    struDownloadCond.struStopTime.dwMonth = 7;
    struDownloadCond.struStopTime.dwDay = 16;
    struDownloadCond.struStopTime.dwHour = 9;
    struDownloadCond.struStopTime.dwMinute = 15;
    struDownloadCond.struStopTime.dwSecond = 0;

    //---------------------------------------
    //按时间下载
    int hPlayback;
    hPlayback = NET_DVR_GetFileByTime_V40(lUserID, "./test.mp4", &struDownloadCond);
    if (hPlayback < 0) {
        printf("NET_DVR_GetFileByTime_V40 fail,last error %d\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return -1;
    }

    //---------------------------------------
    //开始下载
    if (!NET_DVR_PlayBackControl_V40(hPlayback, NET_DVR_PLAYSTART, NULL, 0, NULL, NULL)) {
        printf("Play back control failed [%d]\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return -1;
    }

    int nPos = 0;
    for (nPos = 0; nPos < 100 && nPos >= 0; nPos = NET_DVR_GetDownloadPos(hPlayback)) {
        printf("Be downloading... %d %%\n", nPos);
        sleep(5000);  //millisecond
    }
    if (!NET_DVR_StopGetFile(hPlayback)) {
        printf("failed to stop get file [%d]\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return -1;
    }
    if (nPos < 0 || nPos > 100) {
        printf("download err [%d]\n", NET_DVR_GetLastError());
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return -1;
    }
    printf("Be downloading... %d %%\n", nPos);

    //注销用户
    NET_DVR_Logout(lUserID);
    //释放SDK资源
    NET_DVR_Cleanup();
    return 0;
}





