/****************************** Module Header ******************************\
* Module Name:  Mutex.h
* Project:      S000
*
*
* Mutex class to be removed
*
\***************************************************************************/
#pragma once
#include "framework.h"

namespace cfm::application {

    class CMutex {
    public:
        CMutex() { m_mutex = ::CreateMutexA(NULL, FALSE, NULL); }
        ~CMutex() {::CloseHandle(m_mutex); }
        virtual void Lock() const { 
            DWORD d = WaitForSingleObject(m_mutex, INFINITE);
            /// \todo check 'd' for result
        }
        virtual void Unlock() const { ::ReleaseMutex(m_mutex); }

    private:
        HANDLE m_mutex;
    };

} //end namespace