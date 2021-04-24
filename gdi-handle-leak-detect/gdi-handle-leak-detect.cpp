// gdi-handle-leak-detect.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>
#include <thread>


int main()
{
    DWORD detectedProcessId = 0;
    std::cout << "enter detected process id:";
    std::cin >> detectedProcessId;
    std::cout << "get process id: " << detectedProcessId << std::endl;

    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, detectedProcessId);
    if (process == NULL)
    {
        std::cout << "OpenProcess " << detectedProcessId << " failed, GetLastError() return: " << GetLastError() << ", you can try launch this tools as admin." << std::endl;
        return -1;
    }

    char dllName[10] = { 0 };
    LPVOID pRemoteData = VirtualAllocEx(process, NULL, sizeof(dllName), MEM_COMMIT, PAGE_READWRITE);
    if (pRemoteData == NULL)
    {
        std::cout << "VirtualAllocEx failed, GetLastError() return: " << GetLastError() << ", you can try launch this tools as admin." << std::endl;
        return -1;
    }

    sprintf_s(dllName, sizeof(dllName), "%s", "ghld.dll");
    if (WriteProcessMemory(process, pRemoteData, dllName, sizeof(dllName), NULL) != TRUE)
    {
        std::cout << "WriteProcessMemory failed, GetLastError() return: " << GetLastError() << ", you can try launch this tools as admin." << std::endl;
        return true;
    }

    HANDLE remoteThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)::LoadLibraryA, pRemoteData, 0, NULL);
    if (remoteThread == NULL)
    {
        std::cout << "CreateRemoteThread failed, GetLastError() return: " << GetLastError() << ", you can try launch this tools as admin." << std::endl;
        return -1;
    }
    WaitForSingleObject(remoteThread, INFINITE);

    DWORD dwLibMod;
    if (GetExitCodeThread(remoteThread, &dwLibMod) == TRUE)
    {
        std::cout << "remote lib: " << dwLibMod << std::endl;
    }
    else
    {
        std::cout << "cant get remote lib" << std::endl;
    }
    CloseHandle(remoteThread);
    VirtualFreeEx(process, pRemoteData, sizeof(dllName), MEM_RELEASE);
    while (true)
    {
        char c = getchar();
        if (c == 'e')
            break;
    }
    std::cout << "clear" << std::endl;
    remoteThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)::FreeLibrary, (LPVOID)dwLibMod, 0, NULL);
    if (remoteThread == NULL)
    {
        std::cout << "CreateRemoteThread failed, GetLastError() return: " << GetLastError() << ", you can try launch this tools as admin." << std::endl;
        return -1;
    }
    WaitForSingleObject(remoteThread, INFINITE);
    CloseHandle(remoteThread);

    CloseHandle(process);

    return 0;
}
