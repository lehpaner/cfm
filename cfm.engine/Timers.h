/****************************** Module Header ******************************\
* Module Name:  Timers.h
* Project:      S000
*
*
* Management of timers
*
\***************************************************************************/
#pragma once

#include "Mutex.h"

#define         ms10                    10L
#define         ms50                    (ms10*5L)
#define         ms100                   (ms50*2L)
#define         ms500                   (ms50*10L)
#define         SECS                    (ms50*20L)
#define         MINS                    (SECS*60L)
#define         HOURS                   (MINS*60L)
namespace cfm::application {

    class TTimerData {
    private:
        unsigned int value;
        unsigned int refreshValue;
        unsigned int timeoutParameter;
        void* timeoutPtrParameter;
        CMutex* mutex;

        void (*f)(unsigned int, unsigned int, void*);

        friend class TTimers;

    public:
        TTimerData() {
            memset(this, 0, sizeof(class TTimerData));
            mutex = new CMutex();
        }
        ~TTimerData() {
            delete mutex;
        }
    };

    class TTimers {
    private:
        unsigned int numTimers;
        int baseTime;
        TTimerData* t;

    public:
        /* parameters : ( unsigned int ) timer's index,
                        ( unsigned int ) number of tics,
                        ( void (*)() ) timeout procedure address,
                        ( unsigned int ) free used parameter,
                        ( void * ) free used pointer parameter. 
        */
        void setTimeout(unsigned int timer, unsigned int value, void (*func)(unsigned int, unsigned int, void*), unsigned int timeoutParameter, void* timeoutPtrParameter);
        
        /* parameters : ( unsigned int ) timer's index. */
        void resTimeout(unsigned int timer);
        
        /* parameters : ( unsigned int ) timer's index. */
        unsigned int timeoutRunning(unsigned int timer);
        

        /* returned value : ( unsigned int ) remaining tics. Can also be used like
                              a boolean to know if timer is expired */


        void refreshTimeout(unsigned int timer);

        /* parameters : ( unsigned int ) timer's index. */


        void decTimers();

        TTimers(int, int, void (*)(TTimers*) = NULL);
        ~TTimers();
    };

    typedef unsigned ThreadFunc_t;

    class TTimersThread : public TTimers {
    private:
        int baseTime;
        bool shutdown;
        HANDLE m_thread;
        DWORD m_dwThreadId;

        static ThreadFunc_t timerThread(void* timerObject);

    protected:

    public:
        HANDLE GetThread() { return m_thread; }
        unsigned GetThreadId() { return m_dwThreadId; }

        TTimersThread(int n, int msBase);
        ~TTimersThread();
    };

} //end namespace