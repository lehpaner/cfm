/****************************** Module Header ******************************\
* Module Name:  EventManager.h
* Project:      S000
*
* Extends windows thread in order to monitor devices and relative events
*
\***************************************************************************/
#pragma once
#include "framework.h"
#include "Thread.h"
#include "Logger.h"
#include "CfmDomainObjects.h"


namespace cfm::application {
    
    class CEventManager : public CThread {
    public:
        CEventManager();
        DWORD Run(LPVOID /* arg */);
        int GetThreadId() { return m_ThreadCtx.m_dwTID; }

    private:
        CLogger* sLogger;

        //Buffer utilizzato per i messaggi di log
        char logBuf[512];			                   /**< Char array used for the message logging */
        std::map<int, domain::CfmEvent_Table > mappaEventi;
        domain::CfmDevices_Table* CheckDevice(unsigned long iDev, char* msg);
    };
}
