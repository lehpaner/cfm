#pragma once
/****************************** Module Header ******************************\
* Module Name:  BaseEngine.h
* Project:      S000
*
*
* Base Engine class that is base class for different types of engine 
*
\***************************************************************************/
#include "framework.h"
#include <set>
#include "Logger.h"
#include "DatabaseManager.h"
#include "EventManager.h"
#include "DeviceMonitor.h"
#include "TcpSocket.h"

typedef int enumOperationMode ;

namespace cfm::application {
class CBaseEngine {
    static int tid;
    static std::map <int, CBaseEngine*> smInstance;

protected:
    struct MSG_FOR_OTHER {
        int message;
        WPARAM wParam;
        LPARAM lParam;
        MSG_FOR_OTHER(int m, WPARAM w, LPARAM l) {
            message = m;
            wParam = w;
            lParam = l;
        }
    };
    static MSG msg;
    static std::set<int> regionIdSet;
    static std::set<MSG_FOR_OTHER*> regionSet;
    static CLogger* sLogger;
    static DBManager* DBM;
    static CEventManager* srk;

    int regionId;
    CDeviceMonitor* pSidMon;
    TcpSocket* monitorSocket;



    CBaseEngine(int id); // Costruttore

    static void startDB();
    static void cleanDB();
    static void printConf(enumOperationMode opMode);
    static bool startOther(enumOperationMode opMode);
    static bool sendToOtherStateMachine(int message, WPARAM wParam, LPARAM lParam);
    static void sendToAllOtherStateMachines(int message, WPARAM wParam, int id);

public:
    static bool terminate;

    static CBaseEngine* getInstance(int id);
    static CBaseEngine* getInstance();
    static int getInstancesSize();
    static void destroyInstance(int id);
    static void destroyInstances();
    static CBaseEngine* init(HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow);
    static CBaseEngine* receive();
    static CBaseEngine* forOtherStateMachines();
    static int getEvent();
    static std::set<int>GetRegionIdSet() { return regionIdSet; }
    static int GetThreadId() { return tid; }
    static CEventManager* getEventManagerInstance() { return srk; }
    virtual std::string getState() { return ""; }
    virtual ~CBaseEngine();
    virtual void executeStateMachine(unsigned int ev) = 0;
    int GetRegionId() { return regionId; }
};

//CBaseEngine* getAModeInstance(int id);
//CBaseEngine* getCModeInstance(int id) { return nullptr; }
//CBaseEngine* getMasterInstance(int id) { return nullptr; }
//CBaseEngine* getUModeInstance(int id) { return nullptr; }

} //end namespace


