/****************************** Module Header ******************************\
* Module Name:  CfmMonitor.cpp
* Project:      S000
*
*
* Instance exchange monitor is 4 echange engine2engine messages
*
\***************************************************************************/

#include "CfmMonitor.h"
#include "BaseEngine.h"
#include "Messages.h"

#define BASE_TIME       (ms100) // Base dei tempi del timer (granularita')

int idThSaraMonitor = 0;
static bool quit = false;

cfm::application::CMessageSender* cfm::application::CMessageSender::smInstance = nullptr;

namespace cfm::application {
    CShellSocket::CShellSocket(ISocketHandler& h) : TcpSocket(h) { // Costruttore
    
        addEvent(IDLE, MACRO_EVENT(SOCKET_EVENT, ON_ACCEPT), &CShellSocket::idleOnAccept);

        addEvent(WAIT_USER, MACRO_EVENT(RAW_LINE, 0), &CShellSocket::waitUserRawLine);

        addEvent(WAIT_PASSWORD, MACRO_EVENT(RAW_LINE, 0), &CShellSocket::waitPasswordRawLine);

        addEvent(CONNECTED, MACRO_EVENT(SOCKET_EVENT, ON_DELETE), &CShellSocket::idleOnDelete);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("help")), &CShellSocket::connectedHelp);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("quit")), &CShellSocket::connectedQuit);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("list")), &CShellSocket::connectedList);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("mode")), &CShellSocket::connectedMode);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("states")), &CShellSocket::connectedStates);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("listregion")), &CShellSocket::connectedListRegion);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("updatesid")), &CShellSocket::connectedUpdateSid);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("rulesreload")), &CShellSocket::connectedRulesReload);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("diag")), &CShellSocket::connectedDiag);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("nodiag")), &CShellSocket::connectedNoDiag);
#ifdef ENABLE_DETACH
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("detach")), &ShellSocket::connectedDetach);
#endif
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("resolve")), &CShellSocket::connectedResolve);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("count")), &CShellSocket::connectedCount);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("startsid")), &CShellSocket::connectedStartSid); // Avvia un sid specifico del regionale
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("stopsid")), &CShellSocket::connectedStopSid); // Ferma un sid specifico del regionale
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("suspend")), &CShellSocket::connectedSuspend); // Ferma tutti i sid del regionale
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("resume")), &CShellSocket::connectedResume); // Avvia tutti i sid del regionale
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("terminate")), &CShellSocket::connectedTerminate); // Termina applicazione
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("manageregion")), &CShellSocket::connectedManageRegion); // Master: prende in carico regione specifica
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("restoreregion")), &CShellSocket::connectedRestore); // Master: rilascia regione specifica
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("status")), &CShellSocket::connectedStatus);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("polling")), &CShellSocket::connectedPolling);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("dumpmemory")), &CShellSocket::dumpMemory);

        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("sharePerson")), &CShellSocket::connectedSharePerson); //sharePerson Parm1:SaraPersonID, Parm2:SidID
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("person")), &CShellSocket::connectedManagePerson); //sharePerson Parm1:SidID
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("badge")), &CShellSocket::connectedManageBadge); //sharePerson Parm1:SidID

        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("shareBadge")), &CShellSocket::connectedShareBadge);
        addEvent(CONNECTED, MACRO_EVENT(COMMAND, getCmdId("shareTimeSchedule")), &CShellSocket::connectedShareTimeSchedule);

//Pekez        SetLineProtocol();

        timer = new TTimersThread(CFM_MONITOR_TIMERS_NO, BASE_TIME);

//        mutex = new Mutex();
    }

    CShellSocket::~CShellSocket() {
        delete timer;
//        delete mutex;
    }


    /* State Machine Actions: BEGIN ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

    /*! \brief State: IDLE \n
     * Event: SOCKET_EVENT - ON_ACCEPT
     */
    void CShellSocket::idleOnAccept() {
        Send("User:");
        state = WAIT_USER;
    }

    /*! \brief State: WAIT_USER \n
     * Event: RAW_LINE - 0
     */
    void CShellSocket::waitUserRawLine(){
        user = line;
        Send("Password:");
        state = WAIT_PASSWORD;
    }

    /*! \brief State: WAIT_PASSWORD \n
     * Event: RAW_LINE - 0
     */
    void CShellSocket::waitPasswordRawLine() {
        if (!checkUserPassword(user, line)) {
//Pekmez            SetCloseAndDelete();

            state = IDLE; // Probabilmente non serve perché l'oggetto viene rilasciato.

            return;
        }
        std::string sClientName = "Localhost";//GetRemoteHostname();
        std::string sHostName = "Localhost"; // Utility::GetLocalHostname();
        Send("***********************************************************\n\n\r");
        Send("Welcome in CFM 2.0.0\r\n");
        Send("CFM Ag - Copyright 20XX\r\n\r\n");
        Send("CFM Instance: #1 on host: ");
        Send(sHostName.c_str());
        Send("\r\n");
        Send("Remote console ip: ");
        Send(sClientName.c_str());
        Send("\r\n\r\n");
        Send("Push button Esc or type \"quit\" to exit\n\r");
        Send("Type \"help\" for having information about using interface\n\n\r");
        Send("Cmd (quit,list,suspend,resume,stopsid <id sid>...)\n\n\r");
        Send("***********************************************************\r\n");

        Send("Cmd>");

        state = CONNECTED;
    }

    /*! \brief State: CONNECTED \n
     * Event: SOCKET_EVENT - ON_DELETE
     */
    void CShellSocket::idleOnDelete() {
    }

    void CShellSocket::connectedHelp() {
        char buff[256];
        char buff2[16];
        int i = 1;

        const char* format = "%-4s%-28s%-40s%\r\n";

        Send("\r\nSara Console command list:\r\n\r\n");

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "help", "Prints this list");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "quit", "Disconnects the console");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "list [-c][-t [<seconds>]]", "Prints all managed SIDs. If -c all configured SIDs. If -t periodically repeated, default 2 seconds; to stop it, type another \"list\" command");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "terminate", "Terminates the application");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "listregion [-t [<seconds>]]", "Prints all managed regions by the Master. If -t periodically repeated, default 2 seconds; to stop it, type another \"listregion\" command");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "states [-t [<seconds>]]", "Prints current state of all state machines. If -t periodically repeated, default 2 seconds; to stop it, type another \"states\" command");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "mode", "Prints current operation mode (Master/Regional)");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "detach", "Detaches the socket");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "resolve", "Resolves");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "count [-t [<seconds>]]", "Prints connected socket number. If -t periodically repeated, default 2 seconds; to stop it, type another \"count\" command");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "diag [<sid uid>]", "Prints all diagnostic info. If <sid uid> = 0 prints System info else SID <sid uid> info");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "nodiag", "Stops diagnostic info");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "updatesid <sid uid> [<site>]", "Updates requested SID");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "rulesreload", "Reloads all rules and actions from DB");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "startsid <sid uid>", "Starts requested SID");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "stopsid <sid uid>", "Stops requested SID");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "suspend", "Suspends SIDs managing");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "resume", "Resumes SIDs managing");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "manageregion <region id>", "Manages SIDs of requested region");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "restoreregion <region id>", "Releasse SIDs of requested region");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "status", "Status");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "polling", "Prints running state");
        Send(buff);

        sprintf(buff2, "%d.", i++);
        sprintf(buff, format, buff2, "sharePerson", "Share a person object with another system");
        Send(buff);

        Send("\r\n");
    }

    void CShellSocket::connectedQuit() {
        Send("Goodbye!\r\n");
//Pekmez        SetCloseAndDelete();

        state = IDLE; // Probabilmente non serve perché l'oggetto viene rilasciato.
    }

    void CShellSocket::connectedList() {
        timer->resTimeout(TIMER_LIST);

        if (cmdRest == "-c") {
            listC();
            return;
        }
        std::set<int>regionIdSet = CBaseEngine::getInstance()->GetRegionIdSet();
        int id;
        char buff[256];
        char buff2[128] = { 0 };

        const char* format = "%-22s%-20s%-14s\r\n";

        sprintf(buff, format, "SystemId", "RegionId", "State");

        Send(buff);
        std::string s(strlen(buff) - 2, '-');
        s += "\r\n";
        Send(s);

        while (!regionIdSet.empty())
        {
            id = *regionIdSet.begin(); regionIdSet.erase(id);

            std::map<int, CfmDeviceHandleMap>::iterator i;
            std::map<int, CfmDeviceHandleMap> mappaSID = CDeviceMonitor::getInstance(id)->GetSIDMap();

            for (i = mappaSID.begin(); i != mappaSID.end(); i++) {
                std::string sStatus = DBManager::getInstance()->StateSIDToString(i->second.Status);
                sprintf(buff2, "[%d%]", i->first);
                if (i->second.SIDIf)
                    sprintf(buff2, "%s %s", buff2, i->second.SIDIf->GetAlias().c_str());
//Pekmez                sprintf(buff, format, buff2, IntToStr(id).c_str(), sStatus.c_str());

                Send(buff);
            }
        }
        repetitionCheck(TIMER_LIST);
    }

    void CShellSocket::connectedMode() {
        //if (DBManager::getInstance()->isMaster())
        //    Send("Master " + IntToStr(saraRegCfg.SARA_REGION_ID()) + "\r\n");
        //else
        //    Send("Regional " + IntToStr(saraRegCfg.SARA_REGION_ID()) + "\r\n");
    }

    void CShellSocket::connectedStates() {
        std::set<int>regionIdSet = CBaseEngine::getInstance()->GetRegionIdSet();
        int id;
        char buff[256];

        const char* format = "%-16s%-20s%-20s%";

        sprintf(buff, format, "RegionId", "Main AMode/Master", "SID Monitor");

        if (DBManager::getInstance()->isMaster())
            sprintf(buff + strlen(buff), "%-20s", "Region Monitor");

        strcat(buff, "\r\n");

        Send(buff);
       /* std::string s(strlen(buff) - 2, '-');
        s += "\r\n";
        Send(s);

        while (!regionIdSet.empty())
        {
            id = *regionIdSet.begin(); regionIdSet.erase(id);

            sprintf(buff, format, IntToStr(id).c_str(), CSaraMain::getInstance(id)->getState().c_str(), ThSIDMonitor::getInstance(id)->getState().c_str());

            if (ThRegionMonitor::existsInstance(id))
                sprintf(buff + strlen(buff), "%-20s", ThRegionMonitor::getInstance(id)->getState().c_str());

            strcat(buff, "\r\n");

            Send(buff);
        }
        repetitionCheck(TIMER_STATES);*/
    }

    void CShellSocket::connectedListRegion() {
        DBManager* DBM = DBManager::getInstance();
        if (!DBM->isMaster()) {
            Send("Command not supported on regional.\r\n");
            return;
        }
        int myRegionId = cfmRegCfg.SARA_REGION_ID();
        char buff[256];
        char buff2[128];
        if (!DBM->getRegionList()) {
            Send("Command not supported without DB connection.\r\n");
            return;
        }
        const char* format = "%-22s%-20s%-14s%-8s\r\n";

        sprintf(buff, format, "RegionId", "Host Name", "Slave State", "Master State");

        Send(buff);
        /*std::string s(strlen(buff) - 2, '-');
        s += "\r\n";
        Send(s);

        for (int i = 0, id; i < DBM->vRegionList.size(); i++)
            if ((id = DBM->vRegionList[i]->Id) && ThRegionMonitor::existsInstance(id))
            {
                std::string sStatus = (DBM->vRegionList[i]->Failover)
                    ? "Failover"
                    : "Running";

                sprintf(buff2, "[%d%] %s", id, DBM->vRegionList[i]->Name.c_str());
                sprintf(buff, format, buff2, DBM->vRegionList[i]->HostName.c_str(),
                    sStatus.c_str(), CSaraMain::getInstance(id)->getState().c_str());

                Send(buff);
            }
        repetitionCheck(TIMER_LISTREGION);*/
    }

    void CShellSocket::connectedUpdateSid() {
        if (!DBManager::getInstance()->isMaster()) {
            // OSC Solo per i test 
            //Send("Command not supported on regional.\r\n");
            Send("You have sent the command on a regional instance.....\r\n");
            //return;
        }
        if (cmdRest.empty())
        {
            Send("Syntax error, need parameter.\r\n");
            return;
        }
        Parse pa(cmdRest);
        pa.EnableQuote(true);

        int SIDUID = pa.getvalue();
        int region = DBManager::getInstance()->GetRegionFromSIDUID(SIDUID);

        /*    if (region == saraRegCfg.SARA_REGION_ID())
            {
                Send("Update not supported on my own SID.\r\n");
                return;
            }*/
        if (!CDeviceMonitor::existsInstance(region)) {
            Send("SID not supported.\r\n");
            return;
        }
        //UPDATE_MESSAGE* pCommandUpdate = new UPDATE_MESSAGE;
        //std:string siteName = pa.getword();
        //if (siteName.length() > 0 && siteName[0] == '"' && siteName[siteName.length() - 1] == '"')
        //    siteName = siteName.substr(1, siteName.length() - 2);

        //pCommandUpdate->SIDUID = SIDUID;
        //pCommandUpdate->strParam = siteName;

        //ThSIDMonitor::getInstance(region)->req_update_sid(pCommandUpdate, this);
    }

    void CShellSocket::connectedRulesReload() {
        //PostThreadMessage(CSaraMain::getEventManagerInstance()->GetThreadId(), RULES_RELOAD_REQ, 0, (LPARAM)this);

        //std::set<int>regionIdSet = CSaraMain::getInstance()->GetRegionIdSet();

        //while (!regionIdSet.empty())
        //{
        //    int id = *regionIdSet.begin(); regionIdSet.erase(id);

        //    if (ThRegionMonitor::existsInstance(id))
        //        PostThreadMessage(ThRegionMonitor::getInstance(id)->GetThreadId(), RULES_RELOAD_REQ, (WPARAM)this, NULL);
        //}
    }

    void CShellSocket::connectedDiag() {
        int sidId = -1;

        //if (!cmdRest.empty()) {
        //    sidId = atoi(cmdRest.c_str());

        //    if (sidId)
        //    {
        //        if (!CDeviceMonitor::existsInstance(DBManager::getInstance()->GetRegionFromSIDUID(sidId)))
        //        {
        //            Send("SID non supported.\r\n");
        //            return;
        //        }
        //    }
        //}
        //CLogMessageListener::getInstance()->AddSocket(this, sidId);
    }

    void CShellSocket::connectedNoDiag() {
        CLogMessageListener::getInstance()->RemoveSocket(this);
    }

    void CShellSocket::connectedDetach() {
        //if (!Detach())
        //{
        //    Send("Detach() call failed\n");
        //}
        //else
        //{
        //    Send("Ok.\r\n");
        //}
    }

    void CShellSocket::connectedResolve() {
        //Resolve( arg );
        //ipaddr_t a;
        //if (Utility::u2ip(cmdRest, a))
        //{
        //    std::string tmp;
        //    Utility::l2ip(a, tmp);
        //    Send("Resolved: " + tmp + "\r\n");
        //}
        //else
        //{
        //    Send("Resolve failed: " + cmdRest + "\r\n");
        //}
    }

    void CShellSocket::connectedCount(){
        //Send("Socket count: " + Utility::l2string((long)Handler().GetCount()) + "\r\n");

        //repetitionCheck(TIMER_COUNT);
    }

    void CShellSocket::connectedStartSid() {
        //if (cmdRest.empty())
        //{
        //    Send("Syntax error, need parameter.\r\n");
        //    return;
        //}
        //int SIDUID = atoi(cmdRest.c_str());
        //int region = DBManager::getInstance()->GetRegionFromSIDUID(SIDUID);

        //if (!ThSIDMonitor::existsInstance(region))
        //{
        //    Send("SID not supported.\r\n");
        //    return;
        //}
        //ThSIDMonitor::getInstance(region)->req_start_sid(SIDUID, this);
    }

    void CShellSocket::connectedStopSid() {
        //if (cmdRest.empty())
        //{
        //    Send("Syntax error, need parameter.\r\n");
        //    return;
        //}
        //int SIDUID = atoi(cmdRest.c_str());
        //int region = DBManager::getInstance()->GetRegionFromSIDUID(SIDUID);

        //if (!ThSIDMonitor::existsInstance(region))
        //{
        //    Send("SID not supported.\r\n");
        //    return;
        //}
        //ThSIDMonitor::getInstance(region)->req_stop_sid(SIDUID, this);
    }

    void CShellSocket::connectedSuspend() {
        PostThreadMessage(CBaseEngine::getInstance()->GetThreadId(), SUSPEND, (WPARAM)this, 0);
    }

    void CShellSocket::connectedResume() {
        PostThreadMessage(CBaseEngine::getInstance()->GetThreadId(), RESUME, (WPARAM)this, 0);
    }

    void CShellSocket::connectedTerminate() {
        PostThreadMessage(CBaseEngine::getInstance()->GetThreadId(), TERMINATE_REQ, 0, 0);

        Send("Terminating...\r\n");
    }

    void CShellSocket::connectedManageRegion() {
        if (!DBManager::getInstance()->isMaster()) {
            Send("Command not supported on region.\r\n");
            return;
        }
        if (cmdRest.empty()) {
            Send("Syntax error, need parameter.\r\n");
            return;
        }
        //int regionId = atoi(cmdRest.c_str());

        //if (!ThRegionMonitor::existsInstance(regionId)) {
        //    Send("Region not found or not manageable.\r\n");
        //    return;
        //}
        //PostThreadMessage(ThRegionMonitor::getInstance(regionId)->GetThreadId(), REQ_MANAGE_REGION, (WPARAM)this, NULL);
    }

    void CShellSocket::connectedRestore() {
        if (!DBManager::getInstance()->isMaster())
        {
            Send("Command not supported on region.\r\n");
            return;
        }
        if (cmdRest.empty())
        {
            Send("Syntax error, need parameter.\r\n");
            return;
        }
        int regionId = atoi(cmdRest.c_str());

/*        if (!ThRegionMonitor::existsInstance(regionId))
        {
            Send("Region not found or not manageable.\r\n");
            return;
        }
        PostThreadMessage(ThRegionMonitor::getInstance(regionId)->GetThreadId(), REQ_RESTORE_REGION, (WPARAM)this, NULL)*/;
    }

    void CShellSocket::connectedStatus() {
    }

    void CShellSocket::connectedPolling() {
        PostThreadMessage(CBaseEngine::getInstance()->GetThreadId(), POLLING, (WPARAM)this, 0);
    }

    void CShellSocket::dumpMemory() {
        HANDLE hLogFile;
        SYSTEMTIME st;
        GetLocalTime(&st);
        char szLocalDate[255], szLocalTime[255], szFileName[255];
        GetDateFormatA(LOCALE_USER_DEFAULT, 0, &st, "yyyy-MM-dd", szLocalDate, 255);
        GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &st, "HH-mm-ss", szLocalTime, 255);
        sprintf_s(szFileName, sizeof(szFileName), "DumpMemory_%s_%s.txt", szLocalDate, szLocalTime);
        hLogFile = CreateFileA(szFileName, GENERIC_WRITE,
            FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, NULL);
        if (hLogFile > (HANDLE)0) {
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_WARN, hLogFile);

            _CrtDumpMemoryLeaks();

            _RPT0(_CRT_WARN, "file message\n");
            CloseHandle(hLogFile);
        }
    }


    void CShellSocket::connectedSharePerson() {
        if (cmdRest.empty()) {
            Send("Syntax error, need parameter.\r\n");
            return;
        }

        Parse pa(cmdRest);
        pa.EnableQuote(true);

        int SIDUID = pa.getvalue();
        int idPerson = pa.getvalue();

        int iTicket = CCfmMonitor::getInstance()->GetCommandTicket(); /*< Ticket comando da inviare tramite CatCtrlAccess*/

    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    void CShellSocket::connectedManagePerson() {
        char buff[256];
        if (cmdRest.empty()) {
            Send("Syntax Error, need parameter.\r\n");
            return;
        }
        Parse pa(cmdRest);
        pa.EnableQuote(true);

        std::string cmd = pa.getword();
        if (cmd == "?") {
            sprintf(buff, "%s\r\n", "Command Summary:");
            Send(buff);
            sprintf(buff, "%-8s%-30s\r\n", "[list]", "[all:sidID]");
            Send(buff);
            sprintf(buff, "%-8s%-30s\r\n", "[modify]", "[FieldName, FieldValue]");
            Send(buff);
            sprintf(buff, "%-8s%-30s\r\n", "[savesid]", "[PersonID, SystemID]");
            Send(buff);
            sprintf(buff, "%-8s%-30s\r\n", "[deletesid]", "[PersonID, SystemID]");
            Send(buff);

        }
        if (cmd == "list") {
            int SIDUID = pa.getvalue();

            std::map<unsigned long, CfmPerson_Table> Persone = DBManager::getInstance()->getPersons(SIDUID);


            const char* format = "%-8s%-8s%-8s%-30s%-30s\r\n";

            sprintf(buff, format, "SaraID", "Type", "Status", "First_Name", "Last_Name");
            Send(buff);
            sprintf(buff, format, "--------", "--------", "---------", "-------------------------------", "-------------------------------");
            Send(buff);
            std::map<unsigned long, CfmPerson_Table>::iterator it;
            for (it = Persone.begin(); it != Persone.end(); it++)
            {
//Pekmez                sprintf(buff, format, IntToStr(it->second.Id).c_str(), IntToStr(it->second.PersonTypeId).c_str(), IntToStr(it->second.Status).c_str(), it->second.FirstName.c_str(), it->second.LastName.c_str());
                Send(buff);
            }
        }
        if (cmd == "modify") {
            int saraID = pa.getvalue();
            bool dirty = false;
            CfmPerson_Table Persona = DBManager::getInstance()->getPersonById(saraID);
            if (Persona.Id == 0)
            {
                sprintf(buff, "%-20s%-40s\r\n", "[modify Persona]", "Persona not found");
                Send(buff);
                return;
            }
            std::string fieldName = pa.getword();
            std::string fieldVal = pa.getword();
            if (fieldName == "PersonTypeId")
            {
                Persona.PersonTypeId = atoi(fieldVal.c_str());
                dirty = true;
            }
            if (fieldName == "Status")
            {
                Persona.Status = atoi(fieldVal.c_str());
                dirty = true;
            }
            if (fieldName == "FirstName")
            {
                Persona.FirstName = fieldVal;
                dirty = true;
            }
            if (fieldName == "LastName")
            {
                Persona.LastName = fieldVal;
                dirty = true;
            }
            if (fieldName == "Telephone1")
            {
                Persona.Telephone1 = fieldVal;
                dirty = true;
            }
            if (fieldName == "Telephone2")
            {
                Persona.Telephone2 = fieldVal;
                dirty = true;
            }
            if (fieldName == "Deleted")
            {
                Persona.Deleted = atoi(fieldVal.c_str());
                dirty = true;
            }
            if (dirty)DBManager::getInstance()->updatePerson(Persona);
        }

        if (cmd == "savesid") {
            int saraID = pa.getvalue();
            bool dirty = false;
            CfmPerson_Table Persona = DBManager::getInstance()->getPersonById(saraID);

            if (Persona.Id == 0) {
                sprintf(buff, "%-20s%-40s\r\n", "[savesid Person]", "Person not found");
                Send(buff);
                return;
            }
            int sid = pa.getvalue();

            int region = DBManager::getInstance()->GetRegionFromSIDUID(sid);

            if (!CDeviceMonitor::existsInstance(region))
            {
                Send("SID not supported.\r\n");
                return;
            }
            ChangeEvent* ev = new ChangeEvent();
            ev->fObjectID = Persona.Id;
            ev->fSID = sid;
            ev->fObjectType = "Person";
            Person* eP = new Person();
            eP->id = Persona.Id;
            eP->status = Persona.Status;
            eP->FirstName = Persona.FirstName;
            eP->LastName = Persona.LastName;
            eP->Address1 = Persona.Address1;
            eP->Address2 = Persona.Address2;
            eP->PersonTypeId = Persona.PersonTypeId;
            //		eP->EmployeeNumber=Persona.mapSubId;
            ev->data = eP;
            //		CatCtrlAccess::getInstance()->performDataChange(ev);

            CDeviceMonitor::getInstance(region)->req_modify_data(sid, (void*)ev);

            sprintf(buff, "%-20s%-40s\r\n", "[sharesid Person]", "Remember to exec Update of SID...");
            Send(buff);
        }
    }
    /////////////////////////////////////////////BADGE////////////////////////////////////////////////////
    void CShellSocket::connectedManageBadge() {
        char buff[256];
        if (cmdRest.empty()) {
            Send("Syntax Error, need parameter.\r\n");
            return;
        }
        Parse pa(cmdRest);
        pa.EnableQuote(true);

        std::string cmd = pa.getword();
        if (cmd == "?") {
            sprintf(buff, "%s\r\n", "Command Summary:");
            Send(buff);
            sprintf(buff, "%-8s%-30s\r\n", "[list]", "[all:sidID]");
            Send(buff);
            sprintf(buff, "%-8s%-30s\r\n", "[modify]", "[FieldName, FieldValue]");
            Send(buff);
            sprintf(buff, "%-8s%-30s\r\n", "[savesid]", "[PersonID, SystemID]");
            Send(buff);
            sprintf(buff, "%-8s%-30s\r\n", "[deletesid]", "[PersonID, SystemID]");
            Send(buff);

        }
        if (cmd == "list") {
            int SIDUID = pa.getvalue();

            std::map<unsigned long, CfmBadge_Table> Badges = DBManager::getInstance()->getBadges(SIDUID);


            const char* format = "%-8s%-8s%-8s%-30s%-15s%-10s\r\n";

            sprintf(buff, format, "SaraID", "Person", "Status", "Description", "EncodedNumber", "PIN");
            Send(buff);
            sprintf(buff, format, "--------", "--------", "---------", "-------------------------------", "---------------", "----------");
            Send(buff);
            std::map<unsigned long, CfmBadge_Table>::iterator it;
            for (it = Badges.begin(); it != Badges.end(); it++)
            {
//Pekmez                sprintf(buff, format, IntToStr(it->second.Id).c_str(), IntToStr(it->second.PersonId).c_str(), IntToStr(it->second.Status).c_str(), it->second.Description.c_str(), it->second.EncodedNumber.c_str(), it->second.PIN.c_str());
                Send(buff);
            }
        }
        if (cmd == "modify") {
            int saraID = pa.getvalue();
            bool dirty = false;
            CfmBadge_Table tmpBadge = DBManager::getInstance()->getBadgeById(saraID);

            if (tmpBadge.Id == 0)
            {
                sprintf(buff, "%-20s%-40s\r\n", "[modify Badge]", "Badge not found");
                Send(buff);
                return;
            }
            std::string fieldName = pa.getword();
            std::string fieldVal = pa.getword();

            if (fieldName == "Description")
            {
                tmpBadge.Description = fieldVal;
                dirty = true;
            }
            if (fieldName == "PersonId")
            {
                tmpBadge.PersonId = atoi(fieldVal.c_str());
                dirty = true;
            }
            if (fieldName == "EncodedNumber")
            {
                tmpBadge.EncodedNumber = fieldVal;
                dirty = true;
            }
            if (fieldName == "AliasNumber")
            {
                tmpBadge.AliasNumber = fieldVal;
                dirty = true;
            }
            if (fieldName == "Status")
            {
                tmpBadge.Status = atoi(fieldVal.c_str());
                dirty = true;
            }
            if (fieldName == "PIN")
            {
                tmpBadge.PIN = fieldVal;
                dirty = true;
            }
            if (fieldName == "Deleted")
            {
                tmpBadge.Deleted = atoi(fieldVal.c_str());
                dirty = true;
            }
            if (dirty)DBManager::getInstance()->updateBadge(tmpBadge);

        }

        if (cmd == "savesid")
        {
            int saraID = pa.getvalue();
            bool dirty = false;
            CfmBadge_Table TBadge = DBManager::getInstance()->getBadgeById(saraID);

            if (TBadge.Id == 0)
            {
                sprintf(buff, "%-20s%-40s\r\n", "[savesid Badge]", "Badge not found");
                Send(buff);
                return;
            }
            int sid = pa.getvalue();

            int region = DBManager::getInstance()->GetRegionFromSIDUID(sid);

            if (!CDeviceMonitor::existsInstance(region))
            {
                Send("SID not found.\r\n");
                return;
            }
            ChangeEvent* ev = new ChangeEvent();
            ev->fObjectID = TBadge.Id;
            ev->fSID = sid;
            ev->fObjectType = "Badge";
            Badge* BB = new Badge();
            BB->id = "W2233888"; // IntToStr(TBadge.Id); //Da mettere Id del SID
            BB->status = TBadge.Status;
            BB->PersonId = "W33445"; // IntToStr(TBadge.PersonId);
            BB->EncodedNumber = TBadge.EncodedNumber;
            BB->AliasNumber = TBadge.AliasNumber;
            BB->PIN = TBadge.PIN;
            ev->data = BB;
            CDeviceMonitor::getInstance(region)->req_modify_data(sid, (void*)ev);

            sprintf(buff, "%-20s%-40s\r\n", "[savesid Badge]", "Remember to exec Update of SID...");
            Send(buff);
        }
    }


    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    void CShellSocket::connectedShareBadge() {
        if (cmdRest.empty())
        {
            Send("Syntax error, need parameter.\r\n");
            return;
        }

        Parse pa(cmdRest);
        pa.EnableQuote(true);

        int SIDUID = pa.getvalue();
        int idPerson = pa.getvalue();
        int iTicket = CCfmMonitor::getInstance()->GetCommandTicket(); /*< Ticket comando da inviare tramite CatCtrlAccess*/
    }

    void CShellSocket::connectedShareTimeSchedule() {
        if (cmdRest.empty()) {
            Send("Syntax error, need parameter.\r\n");
            return;
        }

        Parse pa(cmdRest);
        pa.EnableQuote(true);

        int SIDUID = pa.getvalue();
        int idPerson = pa.getvalue();
        int iTicket = CCfmMonitor::getInstance()->GetCommandTicket(); /*< Ticket comando da inviare tramite CatCtrlAccess*/
    }



    /* State Machine Actions: END ------------------------------------------------------------ */



    //Inizializzazione dei singleton
    CCfmMonitor* CCfmMonitor::monInstance = NULL;
    CLogMessageListener* CLogMessageListener::lmlInstance = NULL;

    /**
      * @fn		SaraMonitor::getInstance(LPVOID)
      * @brief	Crea (se la prima volta che viene chiamato) o ritorna l'istanza del singleton
      * @author Diego Gangale
      * @date	02 04 2010	Created
      * @return Il puntatore all'istanza del singleton
      *
      */
    CCfmMonitor* CCfmMonitor::getInstance()
    {
        if (monInstance == NULL)
            monInstance = new CCfmMonitor();

        return monInstance;
    }

    /*! \brief Funzione di timeout lanciata dal timer mediante il thread timerThread
     *
     * Invia timeout a se stesso.
     *
     * \param timer indice del timer
     * \param parameter parametro intero eventuale passato
     * \param ptrParam parametro puntatore eventuale passato
     */
    void CShellSocket::saraMonTo(unsigned int timer, unsigned int param, void* ptrParam)
    {
        CShellSocket* p = (CShellSocket*)ptrParam;

        p->Send("\n");

        p->OnLine(p->lineToRepeat[timer]);
    }

    void CShellSocket::listC() {
        char buff[256];
        char buff2[128];
        DBManager* DBM = DBManager::getInstance();

        const char* format = "%-22s%-8s\r\n";

        sprintf(buff, format, "SystemId", "RegionId");

        Send(buff);
        std::string s(strlen(buff) - 2, '-');
        s += "\r\n";
        Send(s);

        //for (int i = 0; i < DBM->vSystemList.size(); i++)
        //{
        //    sprintf(buff2, "[%d%] %s", DBM->vSystemList[i]->Id, DBM->vSystemList[i]->alias.c_str());
        //    sprintf(buff, format, buff2, IntToStr(DBM->vSystemList[i]->SARARegionId).c_str());

        //    Send(buff);
        //}
    }

    void CShellSocket::repetitionCheck(int index) {
        int seconds = 0;

        if (cmdRest == "-t")
            seconds = 2;
        else
            if (1 != sscanf(cmdRest.c_str(), "-t %d", &seconds))
                seconds = 0;

        if (seconds)
        {
            lineToRepeat[index] = line;
            timer->setTimeout(index, seconds * SECS, saraMonTo, 0, this);
        }
        else
            timer->resTimeout(index);
    }

    /**
      * @fn SaraMonitor::Run(LPVOID)
      * @brief Codice eseguito dal thread SaraMonitor
      * @author Diego Gangale
      * @date 10/04/2010 Created
      *
      *
      */
    DWORD CCfmMonitor::Run(LPVOID /* arg */) {
        CLogMessageListener::getInstance()->Start();

//        StdoutLog log;
//        SocketHandler h(&log);
//
//#ifdef ENABLE_RESOLVER
//        h.EnableResolver(9999);
//#endif
//        //	Utility::ResolveLocal();
//        printf(" *** My hostname: %s\n", Utility::GetLocalHostname().c_str());
//        printf(" *** My local IP: %s\n", Utility::GetLocalAddress().c_str());
//
//        // line server
//        ListenSocket<CShellSocket> ss(h);
//        if (ss.Bind(1027))
//        {
//            printf("Bind 1027 failed\n");
//            exit(-1);
//        }
//        h.Add(&ss);
//
//        // wait for resolver to really start
//#ifdef ENABLE_RESOLVER
//        printf("Waiting for resolver ...");
//        while (!h.ResolverReady())
//            ;
//        printf(" resolver ready!\n");
//#endif
//
//        h.Select(0, 0);
//        while (!quit) {
//            h.Select(1, 0);
//            if (!ss.Ready())
//                Sleep(1000);
//        }
        return 0;
    }

    DWORD CLogMessageListener::Run(LPVOID /* arg */) {
        idThSaraMonitor = this->m_ThreadCtx.m_dwTID;

        while (true) //Ci vuole una variabile da sbloccare
        {
            try {
                MSG msg;
                GetMessage(&msg, NULL, 0, 0);

                if (msg.message == SARA_MONITOR_LOG)
                {
                    if (msg.wParam != NULL)
                    {
                        char* messaggio = (char*)msg.wParam;
                        int id = (int)msg.lParam;
                        std::string str = messaggio;
                        str += "\r\n";

                        std::set<TcpSocket*>::iterator it;
//                        mutex->Lock();
                        for (it = socketList.begin(); it != socketList.end(); it++)
                        {
                            if (sidMap.find(*it) != sidMap.end() && id != sidMap[*it])
                                continue;

                            if ((*it)->Ready())
                                (*it)->Send(str);
                        }
//                        mutex->Unlock();

                        delete[] messaggio;
                    }
                }
            }
            catch (...)
            {
            }
        }
    }

    /**
      * @fn		LogMessageListener::getInstance(LPVOID)
      * @brief	Crea (se la prima volta che viene chiamato) o ritorna l'istanza del singleton
      * @author Diego Gangale
      * @date	02 04 2010	Created
      * @return Il puntatore all'istanza del singleton
      *
      */
    CLogMessageListener* CLogMessageListener::getInstance() {
        if (lmlInstance == NULL)
            lmlInstance = new CLogMessageListener();

        return lmlInstance;
    }

}