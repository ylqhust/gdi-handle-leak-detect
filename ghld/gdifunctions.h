#include <windows.h>
#include <iostream>
#include "StackWalker.h"


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

/**
* https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-bitblt
* 这些是完整的gdi相关函数调用,如果需要做一个完整的检测,需要hook这些函数
**/

HDC (WINAPI* OLD_GetDC)(HWND) = GetDC;
int (WINAPI* OLD_ReleaseDC)(HWND, HDC) = ReleaseDC;

HDC WINAPI New_GetDC(HWND hwnd)
{
    StackWalkerToConsole sw;
    sw.ShowCallstack();
    return OLD_GetDC(hwnd);
}