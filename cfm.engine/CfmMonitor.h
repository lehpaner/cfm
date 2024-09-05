/****************************** Module Header ******************************\
* Module Name:  CfmMonitor.h
* Project:      S000
*
*
* Device monitor is thread watching the state of devices managed with engine
*
\***************************************************************************/
#pragma once
#include "ConfigParams.h"

#include "Thread.h"
#include "TcpSocket.h"
#include "StateMachine.h"
#include "Timers.h"
#include "DatabaseManager.h"
#include "Parse.h"
#include <set>


namespace cfm::application {
    /***
    * CLogMessageListener - cals listens the scoket for the log messages
    **/
    class CLogMessageListener : public CThread {
    public:
        static CLogMessageListener* getInstance();
        DWORD Run(LPVOID /* arg */);
        void AddSocket(TcpSocket* p, int sidId) {
//            mutex->Lock();
            if (sidMap.find(p) != sidMap.end())
                sidMap.erase(p);
            if (-1 != sidId)
                sidMap[p] = sidId;
            socketList.insert(p);
//            mutex->Unlock();
        }
        void RemoveSocket(TcpSocket* p) {
 //            mutex->Lock();
            socketList.erase(p);
            if (sidMap.find(p) != sidMap.end())
                sidMap.erase(p);
//            mutex->Unlock();
        }
        int GetListSize() {
            int size;

//            mutex->Lock();
            size = socketList.size();
//            mutex->Unlock();

            return size;
        }
        ~CLogMessageListener() {
//            mutex->Lock();
            while (!socketList.empty()) {
                sidMap.erase(*socketList.begin());
                socketList.erase(socketList.begin());
            }
//            mutex->Unlock();

//            delete mutex;
        }

    private:
        CLogMessageListener() {
//            mutex = new Mutex();
        }
//        Mutex* mutex;
        std::set<TcpSocket*> socketList;
        std::map<TcpSocket*, int> sidMap;
        static CLogMessageListener* lmlInstance;
    };


    class CMessageSender {
    private:
        CMessageSender() {
//            mutex = new Mutex();
        }
//        Mutex* mutex;
        std::set<TcpSocket*> socketList;
        static CMessageSender* smInstance;

    public:
        bool Send(std::string s, void* p) {
            bool ret = false;

            std::set<TcpSocket*>::iterator it;
//            mutex->Lock();
            if ((it = socketList.find((TcpSocket*)p)) != socketList.end())
            {
                (*it)->Send(s);
                ret = true;
            }
//            mutex->Unlock();

            return ret;
        }
        static CMessageSender* getInstance() {
            if (smInstance == NULL)
                smInstance = new CMessageSender();

            return smInstance;
        }
        void AddSocket(TcpSocket* p) {
//            mutex->Lock();
            socketList.insert(p);
//            mutex->Unlock();
        }
        void RemoveSocket(TcpSocket* p) {
//            mutex->Lock();
            socketList.erase(p);
//            mutex->Unlock();
        }
        ~CMessageSender() {
//            mutex->Lock();
            while (!socketList.empty())
                socketList.erase(socketList.begin());
//            mutex->Unlock();

//            delete mutex;
        }
    };

    /*****
    * CCfmMonitor - monitor class 
    ***/
    class CCfmMonitor : public CThread {
    public:
        static CCfmMonitor* getInstance();
        DWORD Run(LPVOID /* arg */);
        unsigned long GetCommandTicket() { return iCfmCommandTicket++; }

    private:
        CCfmMonitor() {
            lml = CLogMessageListener::getInstance();
            ms = CMessageSender::getInstance();
        }
        ~CCfmMonitor() {
            delete ms;
            delete lml;
        }

        static CCfmMonitor* monInstance;  /**< Puntatore all'instanza del singleton SID Monitor */
        CLogMessageListener* lml;
        CMessageSender* ms;
        unsigned long iCfmCommandTicket;
    };

    class CShellSocket : public TcpSocket, public CStateMachine <CShellSocket> {
        typedef enum {
            ON_ACCEPT = 1,
            ON_DELETE,

            SOCKET_EVENTS_NO
        } SOCKET_EVENTS;

        typedef enum {
            SOCKET_EVENT = 1,
            COMMAND,
            TIMEOUT,
            RAW_LINE,

            SHELL_SOCKET_CLASSES_NO
        } SHELL_SOCKET_CLASSES;

        typedef enum {
            IDLE,
            WAIT_USER,
            WAIT_PASSWORD,
            CONNECTED,

            SHELL_SOCKET_STATES_NO
        } SHELL_SOCKET_STATES;

        /*! \enum SARA_MONITOR_TIMERS
         *  Elenco dei timers utilizzati dal processo.
         */
        typedef enum {
            TIMER_LIST = 0,                 /**< ID del TIMER utilizzato per il list temporizzato (list -t [seconds]) */
            TIMER_LISTREGION,               /**< ID del TIMER utilizzato per il listregion temporizzato (listregion -t [seconds]) */
            TIMER_STATES,                   /**< ID del TIMER utilizzato per il states temporizzato (states -t [seconds]) */
            TIMER_COUNT,                    /**< ID del TIMER utilizzato per il count temporizzato (count -t [seconds]) */

            CFM_MONITOR_TIMERS_NO
        } CFM_MONITOR_TIMERS;

        TTimersThread* timer;

        std::map<std::string, int> cmdMap;
        std::string line;
        std::string lineToRepeat[CFM_MONITOR_TIMERS_NO];
        std::string user;
        std::string cmdRest;
//        Mutex* mutex;
        //class LockedMutex {
        //    Mutex* mtx;

        //public:
        //    LockedMutex(Mutex* m)
        //    {
        //        mtx = m;

        //        mtx->Lock();
        //    }
        //    ~LockedMutex()
        //    {
        //        mtx->Unlock();
        //    }
        //};

        bool checkUserPassword(std::string& user, std::string& userPassword) {
            std::string encryptedPassword;

            if (!DBManager::getInstance()->GetUserPassword(user, encryptedPassword))
                return false;

            //MD5Context ctx;
            //unsigned char buf[16];    //128 bit

            //MD5Init(&ctx);
            //MD5Update(&ctx, (md5byte const*)userPassword.c_str(), userPassword.size());
            //MD5Final(buf, &ctx);

            char code[34];
            //code[32] = 0;
            //snprintf(code, 33, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            //    buf[0], buf[1], buf[2], buf[3],
            //    buf[4], buf[5], buf[6], buf[7],
            //    buf[8], buf[9], buf[10], buf[11],
            //    buf[12], buf[13], buf[14], buf[15]);

            return (encryptedPassword == code);
        }
        bool cmdExists(std::string s) {
            return (cmdMap.find(s) != cmdMap.end());
        }
        int getCmdId(std::string s) {
            std::map<std::string, int>::iterator it;

            if ((it = cmdMap.find(s)) == cmdMap.end()) {
                cmdMap[s] = cmdMap.size();

                return cmdMap.find(s)->second;
            }
            return it->second;
        }
        char* toLower(char* data) {
            for (char* p = data; *p; *p = tolower(*p), p++);

            return data;
        }
        std::string& toLower(std::string& s) {
            for (std::string::size_type i = 0; i < s.length(); i++)
                s[i] = tolower(s[i]);

            return s;
        }
        void Send(const std::string& s, int f = 0) {
 //Pekmez            if (Ready())
//Pekmez                TcpSocket::Send(s, f);
        }
        void listC();
        void repetitionCheck(int index);
        static void saraMonTo(unsigned int timer, unsigned int param, void* ptrParam);

        /* State Machine Actions: BEGIN ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
        void idleOnAccept();
        void waitUserRawLine();
        void waitPasswordRawLine();
        void idleOnDelete();
        void connectedQuit();
        void connectedHelp();
        void connectedList();
        void connectedMode();
        void connectedStates();
        void connectedListRegion();
        void connectedUpdateSid();
        void connectedRulesReload();
        void connectedDiag();
        void connectedNoDiag();
        void connectedDetach();
        void connectedResolve();
        void connectedCount();
        void connectedStartSid();
        void connectedStopSid();
        void connectedSuspend();
        void connectedResume();
        void connectedTerminate();
        void connectedManageRegion();
        void connectedRestore();
        void connectedStatus();
        void connectedPolling();
        void dumpMemory();

        void connectedManagePerson();				/* restituisce la lista delle persone definite sul sid*/
        void connectedManageBadge();

        void connectedSharePerson();			/* Condivide una persona con altri sistemi */
        void connectedShareBadge();				/* Condivide un badge con altri sistemi */
        void connectedShareTimeSchedule();		/* Condivide un time range con altri sistemi */

        /* State Machine Actions: END ------------------------------------------------------------ */

    public:
        CShellSocket(ISocketHandler& h);
        ~CShellSocket();

        void OnAccept() {
            CMessageSender::getInstance()->AddSocket(this);
            execute(MACRO_EVENT(SOCKET_EVENT, ON_ACCEPT));
        }
        void OnLine(const std::string& ln) {
 //           LockedMutex lm(mutex);

            line = ln;

            if (state < CONNECTED) {
                execute(MACRO_EVENT(RAW_LINE, 0));
                return;
            }
            Parse pa(ln);
            std::string cmd = pa.getword();
            cmdRest = pa.getrest();

            toLower(cmd);

            if (!cmd.empty())
                if (!cmdExists(cmd))
                    Send("Huh?\r\n");
                else
                    execute(MACRO_EVENT(COMMAND, cmdMap.find(cmd)->second));

            Send("Cmd>");
        }
        void OnDelete()
        {
            execute(MACRO_EVENT(SOCKET_EVENT, ON_DELETE));

            CMessageSender::getInstance()->RemoveSocket(this);
            CLogMessageListener::getInstance()->RemoveSocket(this);
        }
        void OnConnectFailed() {
        }
#ifdef ENABLE_RESOLVER
        void OnResolved(int id, ipaddr_t a, port_t port) {
        }
        void OnResolved(int id, const std::string& name, port_t port) {
            Send("Resolve id " + Utility::l2string(id) + " = " + name + "\n");
        }

#endif
#ifdef ENABLE_DETACH
        void OnDetached() {
            Send("\r\nDetached.\r\nCmd>");
        }
#endif
        void InitSSLServer() {
            //if (isSSL)
            //    InitializeContext(sessionId.c_str(), ca.c_str(), password.c_str(), SSLv23_method());
        }
        void Init() {
 /*           if (getSSLParameters(ca, password, sessionId)) {
                if (ca.empty() || sessionId.empty()) {
                    return;
                }
                isSSL = true;
                EnableSSL();
            }
            else*/
                isSSL = false;
        }
        bool getSSLParameters(std::string& ca, std::string& password, std::string& sessionId) {
            DBManager* DBM = DBManager::getInstance();

            //if (!DBM->getRegionList(cfmRegCfg.SARA_REGION_ID()))
            //    return false;

            //if (!DBM->vRegionList[0].EnableSSL)
            //    return false;

            //char out[256];
            //unsigned long len = sizeof(out) - 1;

            //if (!dbStringDataDecrypt((char*)DBM->vRegionList[0]->SSLPassword.c_str(), DBM->vRegionList[0].SSLPassword.size(), out, &len))
            //    return false;

            //password = ((char*)out);
            //ca = DBM->vRegionList[0].CAFile;
            //sessionId = DBM->vRegionList[0].SessionID;

            return true;
        }

    private:
        std::string ca, password, sessionId;
        bool isSSL;
    };

} //end namespace