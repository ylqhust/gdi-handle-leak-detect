#include "ghldmgr.h"
#include <windows.h>
#include "StackWalker.h"
#include "detours.h"
#include <iostream>

void MyStrCpy2(char* szDest, size_t nMaxDestSize, const char* szSrc)
{
    if (nMaxDestSize <= 0)
        return;
    strncpy_s(szDest, nMaxDestSize, szSrc, _TRUNCATE);
    // INFO: _TRUNCATE will ensure that it is null-terminated;
    // but with older compilers (<1400) it uses "strncpy" and this does not!)
    szDest[nMaxDestSize - 1] = 0;
} // 

class StackWalkerToConsole : public StackWalker
{
protected:
    virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry& entry) override
    {
        CHAR   buffer[STACKWALK_MAX_NAMELEN];
        size_t maxLen = STACKWALK_MAX_NAMELEN;
#if _MSC_VER >= 1400
        maxLen = _TRUNCATE;
#endif
        if ((eType != lastEntry) && (entry.offset != 0))
        {
            if (entry.name[0] == 0)
                MyStrCpy2(entry.name, STACKWALK_MAX_NAMELEN, "(function-name not available)");
            if (entry.undName[0] != 0)
                MyStrCpy2(entry.name, STACKWALK_MAX_NAMELEN, entry.undName);
            if (entry.undFullName[0] != 0)
                MyStrCpy2(entry.name, STACKWALK_MAX_NAMELEN, entry.undFullName);
            if (entry.lineFileName[0] == 0)
            {
                MyStrCpy2(entry.lineFileName, STACKWALK_MAX_NAMELEN, "(filename not available)");
                if (entry.moduleName[0] == 0)
                    MyStrCpy2(entry.moduleName, STACKWALK_MAX_NAMELEN, "(module-name not available)");
                _snprintf_s(buffer, maxLen, "%p (%s): %s: %s\n", (LPVOID)entry.offset, entry.moduleName,
                    entry.lineFileName, entry.name);
            }
            else
                _snprintf_s(buffer, maxLen, "%s (%d): %s\n", entry.lineFileName, entry.lineNumber,
                    entry.name);
            buffer[STACKWALK_MAX_NAMELEN - 1] = 0;
            printf("%s", buffer);
        }
    }
    virtual void OnOutput(LPCSTR szText) {}
};

HDC(WINAPI* OLD_GetDC)(HWND) = GetDC;

HDC WINAPI New_GetDC(HWND hwnd)
{
    StackWalkerToConsole sw;
    sw.ShowCallstack();
    return OLD_GetDC(hwnd);
}

void testGetDC()
{
    HDC dc = GetDC(NULL);
    ReleaseDC(NULL, dc);
}

namespace ylq
{
    GhldMgr::GhldMgr()
    {
        _init = false;
    }

    GhldMgr::~GhldMgr()
    {
        release();
    }

    void GhldMgr::init()
    {
        if (!_init)
        {
            std::cout << "begin init" << std::endl;
            _init = hook();
        }
        else
        {
            std::cout << "already init" << std::endl;
        }
    }

    void GhldMgr::release()
    {
        if (_init)
        {
            std::cout << "begin release" << std::endl;
            unhook();
            _init = false;
        }
        else
        {
            std::cout << "already release" << std::endl;
        }
    }

    bool GhldMgr::hook()
    {
        std::cout << "begin hook" << std::endl;
        LONG result = DetourTransactionBegin();
        if (result != NULL)
        {
            std::cout << "DetourTransactionBegin error: " << result << std::endl;
            return false;
        }
        result = DetourUpdateThread(GetCurrentThread());
        if (result != NULL)
        {
            std::cout << "DetourUpdateThread error: " << result << std::endl;
            return false;
        }
        result = DetourAttach(&OLD_GetDC, New_GetDC);
        if (result != NULL)
        {
            std::cout << "DetourAttach error: " << result << std::endl;
            return false;
        }
        result = DetourTransactionCommit();
        if (result != NULL)
        {
            std::cout << "DetourTransactionCommit error: " << result << std::endl;
            return false;
        }
        std::cout << "hook success" << std::endl;
        return true;
    }

    void GhldMgr::unhook()
    {
        std::cout << "begin unhook" << std::endl;
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&OLD_GetDC, New_GetDC);
        DetourTransactionCommit();
    }
}