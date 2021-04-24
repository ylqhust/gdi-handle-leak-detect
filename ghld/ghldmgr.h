#pragma once


namespace ylq
{
    class GhldMgr
    {
    public:
        static GhldMgr* Instance()
        {
            static GhldMgr instance;
            return &instance;
        }

        void init();

    private:
        void release();
        bool hook();
        void unhook();
        GhldMgr();
        ~GhldMgr();
    private:
        bool _init;
    };
}
