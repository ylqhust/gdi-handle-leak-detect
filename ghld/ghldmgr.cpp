#include "ghldmgr.h"
#include <windows.h>
#include "StackWalker.h"
#include "detours.h"
#include <iostream>
#include "gdifunctions.h"


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