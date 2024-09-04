/****************************** Module Header ******************************\
* Module Name:  DatabaseManager.cpp
* Project:      S000
*
*
* Thread class implements runnable dedicated to database operations...
*
\***************************************************************************/
#include <sstream>
#include "framework.h"
#include "BaseEngine.h"
#include "DatabaseManager.h"
#include "Logger.h"
#include "Messages.h"

#include "ConfigParams.h"

extern  cfm::application::CConfigParms cfmRegCfg;	/**< Registry Configuration to be removed */

static std::string GetUTCTimeByLocalTime(std::string sDateTime) {
    std::string strDateTime;
    CHAR szLocalDate[255], szLocalTime[255], szDateTime[255];
    struct tm t, * utc, * newtime;
    time_t t_of_day;
    LPSYSTEMTIME lpST = NULL;

    try {
        int pos = sDateTime.find_first_of("-/");
        while (pos != -1)
        {
            sDateTime = sDateTime.substr(0, pos) + sDateTime.substr(pos + 1);
            pos = sDateTime.find_first_of("-/");
        }

        time(&t_of_day);					/* Get time as long integer. */
        newtime = localtime(&t_of_day);	/* Convert to local time. */

        t.tm_year = 2024;// StrToInt(sDateTime.substr(0, 4).c_str()) - 1900;
        t.tm_mon = 9;//StrToInt(sDateTime.substr(4, 2).c_str());
        t.tm_mday = 10;// StrToInt(sDateTime.substr(6, 2).c_str());
        t.tm_hour = 8;// StrToInt(sDateTime.substr(9, 2).c_str());
        t.tm_min = 10;// StrToInt(sDateTime.substr(12, 2).c_str());
        t.tm_sec = 10;//StrToInt(sDateTime.substr(15, 2).c_str());
        t.tm_isdst = newtime->tm_isdst;

        t_of_day = mktime(&t);
        utc = gmtime(&t_of_day);

        lpST = new SYSTEMTIME();

        lpST->wYear = utc->tm_year + 1900;
        lpST->wMonth = utc->tm_mon;
        lpST->wDay = utc->tm_mday;
        lpST->wHour = utc->tm_hour;
        lpST->wMinute = utc->tm_min;
        lpST->wSecond = utc->tm_sec;
        lpST->wMilliseconds = 999;// StrToInt(sDateTime.substr(18, 3).c_str());

        GetDateFormatA(LOCALE_USER_DEFAULT, 0, lpST, "yyyyMMdd", szLocalDate, 255);
        GetTimeFormatA(LOCALE_USER_DEFAULT, 0, lpST, "HH:mm:ss", szLocalTime, 255);
        sprintf(szDateTime, "%s %s.%i", szLocalDate, szLocalTime, lpST->wMilliseconds);

        strDateTime = std::string(szDateTime);

        delete lpST;
    }
    catch (std::exception& e) {
        if (lpST)
            delete lpST;
        return "";
    }
    return strDateTime;
}

namespace cfm::application {

    std::atomic<DBManager*> DBManager::smInstance;
    std::mutex DBManager::m_mutex;
    /***
    * Locking is performed on level of get instance
    * considering that we want to use only one connection to db
    * the calls to db manager is managed on getInstance level
    */
    DBManager* DBManager::getInstance() {
        DBManager* tmp = smInstance.load(std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_acquire);
        if (tmp == nullptr) {
            std::lock_guard<std::mutex> lock(m_mutex);
            tmp = smInstance.load(std::memory_order_relaxed);
            if (tmp == nullptr) {
                tmp = new DBManager;
                std::atomic_thread_fence(std::memory_order_release);
                smInstance.store(tmp, std::memory_order_relaxed);
            }
        }
        return tmp;
    }

    DBManager::DBManager() {
        addEvent(STATE_NOT_CONNECTED, DB_CONNECT, &DBManager::notConnectedDbConnect);
        addEvent(STATE_NOT_CONNECTED, DB_RECONNECT, &DBManager::notConnectedDbReConnect);

        addEvent(STATE_CONNECTING, DB_CONNECT_TIMEOUT, &DBManager::connectingConnectTimeout);

        addEvent(STATE_CONNECTED, DB_RELEASE, &DBManager::connectedDbRelease);
        addEvent(STATE_CONNECTED, DB_CLEAN, &DBManager::connectedDbClean);
        addEvent(STATE_CONNECTED, DB_INSERT_OCR, &DBManager::connectedInsertOcr);
        addEvent(STATE_CONNECTED, DB_INSERT_TRACK, &DBManager::connectedInsertTrack);
        addEvent(STATE_CONNECTED, DB_INSERT_LOGMESSAGE, &DBManager::connectedInsertLogMess);
        addEvent(STATE_CONNECTED, DB_INSERT_LOGEVENTAND, &DBManager::connectedInsertLogEventAnd);
        addEvent(STATE_CONNECTED, DB_INSERT_LOGEVENTOR, &DBManager::connectedInsertLogEventOr);
        addEvent(STATE_CONNECTED, DB_EXECUTE_OPERATION, &DBManager::connectedDbExecuteOp);
        addEvent(STATE_CONNECTED, DB_INSERT_LOGEVENT, &DBManager::connectedInsertLogEvent);
        addEvent(STATE_CONNECTED, DB_INSERT_LOGCOMMAND, &DBManager::connectedInsertLogCommand);
        addEvent(STATE_CONNECTED, DB_INSERT_LOGACTION, &DBManager::connectedInsertLogAction);
        addEvent(STATE_CONNECTED, DB_INSERT_LOGRULE, &DBManager::connectedInsertLogRule);
        addEvent(STATE_CONNECTED, DB_UPDATE_LOGACTION, &DBManager::connectedUpdateLogAction);
        addEvent(STATE_CONNECTED, DB_UPDATE_LOGRULE, &DBManager::connectedUpdateLogRule);
        addEvent(STATE_CONNECTED, DB_UPDATE_SYSTEMSTATUS, &DBManager::connectedUpdateSystemStatus);
        addEvent(STATE_CONNECTED, DB_UPDATE_MAINTENANCEREQ, &DBManager::connectedUpdateMaintenanceReq);
        addEvent(STATE_CONNECTED, DB_UPDATE_PHISICALDEVICESTATUS, &DBManager::connectedUpdatePhisicalStatus);
        addEvent(STATE_CONNECTED, DB_UPDATE_LOGICALDEVICESTATUS, &DBManager::connectedUpdateLogicalStatus);
        addEvent(STATE_CONNECTED, DB_UPDATE_DELETEDEVICE, &DBManager::connectedUpdateDeleteDevice);
        addEvent(STATE_CONNECTED, DB_ERASE_OLD_ROWS, &DBManager::connectedEraseOldRows);
        addEvent(STATE_CONNECTED, DB_TIMER, &DBManager::connectedTimer);


        bTerminated = false;
        logBuffer[sizeof(logBuffer) - 1] = 0;
        asyncErrCounter = 0;
    }

    DBManager::~DBManager() {
        //delete syncConn;
        //delete asyncConn;

        //delete (Mutex*)mutex;
    }


    /* --------------------------------------------------------------------------------------- */
    /* State Machine Actions: BEGIN ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
    /* --------------------------------------------------------------------------------------- */

    /*! \brief State: STATE_NOT_CONNECTED \n
    *   Event: DB_CONNECT
    */
    void DBManager::notConnectedDbConnect() {
        KillTimer(NULL, timer);

        if (ConnectDb()) {
            state = STATE_CONNECTED;

            PostThreadMessage(CBaseEngine::getInstance()->GetThreadId(), DB_READY, 0, 0);

            CLogger::getInstance()->Log("[DB MANAGER] [Connect()] [STATE CONNECTED]");

            timer = SetTimer(NULL, DB_TIMER, 55000, TimerMinute);

        } else {
            state = STATE_CONNECTING;

            PostThreadMessage(CBaseEngine::getInstance()->GetThreadId(), DB_NOT_READY, 0, 0);

            timer = SetTimer(NULL, DB_CONNECT_TIMEOUT, 5000, MyTimerProc);

            CLogger::getInstance()->Log("[DB MANAGER] [Connect()] [CONNECTING FAILURE]");
            CLogger::getInstance()->Log("[DB MANAGER] [Connect()] [STATE NOT CONNECTED]");
        }
    }

    /*! \brief State: STATE_NOT_CONNECTED \n
    * Event: DB_RECONNECT
    */
    void DBManager::notConnectedDbReConnect()  {
        ReleaseDb();
        notConnectedDbConnect();
    }
    /*! \brief State: STATE_CONNECTING \n
    * Event: DB_CONNECT_TIMEOUT
    */
    void DBManager::connectingConnectTimeout() {
        KillTimer(NULL, timer);

        if (ConnectDb()) {
            state = STATE_CONNECTED;
            PostThreadMessage(CBaseEngine::getInstance()->GetThreadId(), DB_READY, 0, 0);
            CLogger::getInstance()->Log("[DB MANAGER] [Connect()] [STATE CONNECTED]");
            timer = SetTimer(NULL, DB_TIMER, 55000, TimerMinute);
        } else {
            PostThreadMessage(CBaseEngine::getInstance()->GetThreadId(), DB_NOT_READY, 0, 0);

            timer = SetTimer(NULL, DB_CONNECT_TIMEOUT, 5000, MyTimerProc);

            CLogger::getInstance()->Log("[DB MANAGER] [Connect()] [CONNECTING FAILURE]");
            CLogger::getInstance()->Log("[DB MANAGER] [Connect()] [STATE NOT CONNECTED]");
        }
    }
    /*! \brief State: STATE_CONNECTED \n
    * Event: DB_RELEASE
    */
    void DBManager::connectedDbRelease() {
        state = STATE_NOT_CONNECTED;
        ReleaseDb();
    }
    /*! \brief State: STATE_CONNECTED \n
    * Event: DB_CLEAN
    */
    void DBManager::connectedDbClean() {
        //Si cancellano le righe delle tabelle fino al giorno prima e si resettano i contatori

        // OSC 17/11/2010	Inizio
        //ExecuteSyncQuery("delete from EventLog"); // where IdEventLog <= 500");
        //SaraLogger::getInstance()->Log("[DB MANAGER] EventLog Table data deleted");

        //Reinizializzo in contatori
        //ActionMonitor::getInstance()->InitCounters();
        //SaraLogger::getInstance()->Log("[DB MANAGER] Counters Resetted");
        // OSC 17/11/2010	Fine
    }
    /*! \brief State: STATE_CONNECTED \n
    * Event: DB_INSERT_OCR
    */
    void DBManager::connectedInsertOcr() {
        SaveOCR((domain::CfmLogOCR_Table*)msg.wParam);
    }
    /*! \brief State: STATE_CONNECTED \n
    * Event: DB_INSERT_TRACK
    */
    void DBManager::connectedInsertTrack() {
        SaveTrack((domain::CfmLogTrack_Table*)msg.wParam);
    }
    /*! \brief State: STATE_CONNECTED \n
    * Event: DB_INSERT_LOGMESSAGE
    */
    void DBManager::connectedInsertLogMess() {
        SaveLogMessage((domain::CfmLog_Table*)msg.wParam);
    }
    /*! \brief State: STATE_CONNECTED \n
    * Event: DB_INSERT_LOGEVENTAND
    */
    void DBManager::connectedInsertLogEventAnd() {
        SaveEventAndLog((domain::CfmEventAndLog_Table*)msg.wParam);
    }


    /*! \brief State: STATE_CONNECTED \n
    * Event: DB_INSERT_LOGEVENTOR
    */
    void DBManager::connectedInsertLogEventOr() {
        SaveEventOrLog((domain::CfmEventOrLog_Table*)msg.wParam);
    }

    /*! \brief State: STATE_CONNECTED \n
     * Event: DB_EXECUTE_OPERATION
     */
    void DBManager::connectedDbExecuteOp() {
        char* p = (char*)msg.wParam;
        assert(p);
        ExecuteQueryOnAsyncConnection(p);
        delete[] p;
    }

    /*! \brief State:  \n
     * Event: DB_INSERT_LOGEVENT
     */
    void DBManager::connectedInsertLogEvent() {
        SaveEventLog((domain::CfmEventLog_Table*)msg.wParam);
    }

    void DBManager::connectedInsertLogCommand() {
        SaveCommandLog((domain::CfmCommandLog_Table*)msg.wParam);
    }

    /*! \brief State: STATE_CONNECTED \n
     * Event: DB_INSERT_LOGACTION
     */
    void DBManager::connectedInsertLogAction() {
        SaveActionLog((domain::CfmActionLog_Table*)msg.wParam);
    }

    /*! \brief State: STATE_CONNECTED \n
     * Event: DB_INSERT_LOGRULE
     */
    void DBManager::connectedInsertLogRule() {
        SaveRuleLog((domain::CfmRuleLog_Table*)msg.wParam);
    }

    /*! \brief State: STATE_CONNECTED \n
     * Event: DB_UPDATE_LOGACTION
     */
    void DBManager::connectedUpdateLogAction() {
        UpdateActionLog((domain::CfmActionLog_Table*)msg.wParam);
    }

    /*! \brief State: STATE_CONNECTED \n
     * Event: DB_UPDATE_LOGRULE
     */
    void DBManager::connectedUpdateLogRule() {
        UpdateRuleLog((domain::CfmRuleLog_Table*)msg.wParam);
    }

    /*! \brief State: STATE_CONNECTED \n
     * Event: DB_UPDATE_SYSTEMSTATUS
     */
    void DBManager::connectedUpdateSystemStatus() {
        SaveSystemStatus((domain::CfmSystem_Table*)msg.wParam);
    }

    /*! \brief State: STATE_CONNECTED \n
     * Event: DB_UPDATE_MAINTENANCEREQ
     */
    void DBManager::connectedUpdateMaintenanceReq() {
        SaveSystemMaintenanceReq((domain::CfmSystem_Table*)msg.wParam);
    }

    /*! \brief State: STATE_CONNECTED \n
     * Event: DB_UPDATE_PHISICALDEVICESTATUS
     */
    void DBManager::connectedUpdatePhisicalStatus() {
        SavePhisicalDeviceStatus((domain::CfmDevices_Table*)msg.wParam);
    }

    /*! \brief State: STATE_CONNECTED \n
     * Event: DB_UPDATE_LOGICALDEVICESTATUS
     */
    void DBManager::connectedUpdateLogicalStatus() {
        SaveLogicalDeviceStatus((domain::CfmDevices_Table*)msg.wParam);
    }

    /*! \brief State: STATE_CONNECTED \n
     * Event: DB_UPDATE_DELETEDEVICE
     */
    void DBManager::connectedUpdateDeleteDevice() {
        SaveDeleteDeviceStatus((domain::CfmDevices_Table*)msg.wParam);
    }

    /*! \brief State: STATE_CONNECTED \n
     * Event: DB_ERASE_OLD_ROWS
     */
    void DBManager::connectedEraseOldRows() {
        EraseOldRows();
    }
    /*! \brief State: STATE_CONNECTED \n
    * Event: DB_TIMER
    */
    void DBManager::connectedTimer() {
        static int lastMinute = 0;
        LPSYSTEMTIME lpST = new SYSTEMTIME();
        GetLocalTime(lpST);

        KillTimer(NULL, timer);
        /* PEKMEZ
        if (lastMinute != saraRegCfg.MINUTE() && lpST->wHour == saraRegCfg.HOUR() && lpST->wMinute == saraRegCfg.MINUTE()) {
            std::string logBuffer = ("[SID Monitor] Sending Erase old rows (Track_Log, OCR_Log, EventLog) to DBManager");
            CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW);
            EraseOldRowsRequest();
        }*/
        lastMinute = lpST->wMinute;
        delete lpST;

        timer = SetTimer(NULL, DB_TIMER, 55000, TimerMinute);

    }
    /* --------------------------------------------------------------------------------------- */
    /* State Machine Actions: END   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
    /* --------------------------------------------------------------------------------------- */

    //----------------------------------------------------------------------------
    void DBManager::SetConnectionParams(const std::string& DSN) {
        sConnectionString = DSN;
    }

    //----------------------------------------------------------------------------

    /**
    * @fn DBManager::Connect()
    * @brief		Avvia le due connessioni (SYNC/ASYNC) verso il DBMS ODBC
    * @author		Diego Gangale
    * @date		04/05/2007	Created
    * @date		23/07/2007	Remarked
    * @return		TRUE se entrambe le connessioni sono state esegutite con successo
    * connectionString="Data Source=(LocalDb)\MSSQLLocalDB;Initial Catalog=SCHAFFHAUSEN;Integrated Security=SSPI;AttachDBFilename=|DataDirectory|\cfmdata.mdf" providerName="System.Data.SqlClient" />
    * connectionString="Data Source=(LocalDb)\MSSQLLocalDB;Initial Catalog=SCHAFFHAUSEN;Integrated Security=SSPI;AttachDBFilename=|DataDirectory|\cfmdata.mdf" providerName="System.Data.SqlClient" />
    */
    bool DBManager::ConnectDb() {
        int m_timeout = 10;
        try {
            syncConn = std::make_unique<nanodbc::connection>(sConnectionString, m_timeout);
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(std::string(e.what()), e.state(), (int)e.native(), "CONNECTION TO DB");
            return false;
        }
        try {
            asyncConn = std::make_unique<nanodbc::connection>(sConnectionString, m_timeout);
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(std::string(e.what()), e.state(), (int)e.native(), "CONNECTION TO DB");
            return false;
        }
        

 //      // if (!syncConn->connect(sDSN, sUser, sPwd)) {
 //       if (!syncConn->connect(L"Data Source=(LocalDb)\MSSQLLocalDB;Initial Catalog=SCHAFFHAUSEN;Integrated Security=SSPI;AttachDBFilename=|DataDirectory|\SCHAFFHAUSEN.mdf providerName=System.Data.SqlClient", L"", L"")) {
 //           std::wstring message, code;
 //           ManageSQLError(message, code, syncConn->last_error_status_code(message, code));
 //           return false;
 //       }
 ////       if (!asyncConn->connect(sDSN, sUser, sPwd)) {
 //       if (!asyncConn->connect(L"Data Source=(LocalDb)\MSSQLLocalDB;Initial Catalog=SCHAFFHAUSEN;Integrated Security=SSPI;AttachDBFilename=|DataDirectory|\SCHAFFHAUSEN.mdf providerName = System.Data.SqlClient", L"", L"")) {
 //           std::wstring message, code;
 //           ManageSQLError(message, code, asyncConn->last_error_status_code(message, code));
 //           syncConn->disconnect();
 //           return false;
 //       }
        return true;
    }
    //----------------------------------------------------------------------------
    void DBManager::Connect() {
        PostThreadMessage(GetThreadId(), DB_CONNECT, 0, 0);
    }
    //----------------------------------------------------------------------------

    void DBManager::Release() {
        PostThreadMessage(GetThreadId(), DB_RELEASE, 0, 0);
    }

    //----------------------------------------------------------------------------

    void DBManager::ReleaseDb() {
        CLogger::getInstance()->Log("[DB MANAGER] [Release()] [DISCONNECTING]");

        asyncConn->disconnect();
        syncConn->disconnect();
    }

    //----------------------------------------------------------------------------

#define QUERY_ENTER(q,retValue)\
	bool r = true;\
\
    if (!IsConnected() || !syncConn->connected())\
        return retValue;\
\
	try\
	{\
        LockedMutex lm(mutex);\
        tiodbc::statement aStatement;\
\
        if (!aStatement.execute_direct(*syncConn, q))\
		{\
            std::string message, code;\
            ManageSQLError(message, code, aStatement.last_error_status_code(message, code), q);\
            return retValue;\
		}
//----------------------------------------------------------------------------
#define ASYNCQUERY_ENTER(q)\
	bool r = true;\
\
    if (!IsConnected() || !asyncConn->connected())\
        return;\
\
	try\
	{\
        tiodbc::statement aStatement;\
\
        if (!aStatement.execute_direct(*asyncConn, q))\
		{\
            std::string message, code;\
            ManageSQLError(message, code, aStatement.last_error_status_code(message, code), q);\
            return;\
		}
//----------------------------------------------------------------------------

#define QUERY_EXIT(q)\
\
    }\
	catch(tiodbc::exception &e)\
	{\
        r = false;\
        ManageSQLError(e.message, e.code, e.res, q);\
	}\
	catch(std::exception &e)\
	{\
        r = false;\
		e.what();\
	}
    //----------------------------------------------------------------------------
    bool DBManager::getSystemList() {
        bool r = true;

        if (!IsConnected())
            return false;

        if (!setIsMaster(cfmRegCfg.SARA_REGION_ID()))
            return false;

        std::stringstream query(std::stringstream::in | std::stringstream::out);
        query << "SELECT S.Id, S.SystemAlias, S.Description, S.Deployment, SI.LocalDll, S.RealTimeContext, S.ThreadPriority, S.RemoteIPAddress, S.RemoteIPPort, S.SARARegionId, S.MaintenanceReq, S.SIDId FROM System AS S JOIN SID AS SI ON S.SIDId = SI.Id";
        if (!isMaster()) {
            query << " WHERE SARARegionId = ";
            query << cfmRegCfg.SARA_REGION_ID();
        }

        try {
            // Ripulisce la map prima di ricaricarla
            std::vector<domain::CfmSystem_Table>::iterator itSystem;
            vSystemList.clear();
            auto result = nanodbc::execute(*syncConn, query.str());
            while (result.next()) {
                domain::CfmSystem_Table elt;
                elt.Id = result.get<int>(0);
                elt.alias = result.get<std::string>(1);
                elt.description = result.get<std::string>(2);
                elt.deployment = result.get<std::string>(3);
                elt.localDll = result.get<std::string>(4);
                elt.realTimeContext = (bool)result.get<short>(5);
                elt.ThreadPriority = result.get<int>(7);
                elt.RemoteIPAddress = result.get<std::string>(8);
                elt.RemoteIPPort = result.get<int>(9);
                elt.SARARegionId = result.get<int>(10);
                elt.enabled = !result.get<short>(11);
                elt.SIDId = result.get<int>(12);
                //Insert into vector
                vSystemList.push_back(elt);
            }
        } catch (nanodbc::database_error& e) {
            //Database related Error
            r = false;
            ManageSQLError(e.what(), e.state(), (int)e.native(), "getSystemList");
        } catch (std::exception& e) {
            //Errore
            ManageSQLError(e.what(), "inknown", -1, "getSystemList");
            r = false;
        }
        return r;
    }
    //----------------------------------------------------------------------------
    bool DBManager::getSystemList(int sid) {
        bool retval = true;
        
        std::stringstream query(std::stringstream::in | std::stringstream::out);
        query << "SELECT S.Id, S.SystemAlias, S.Description, S.Deployment, SI.LocalDll, S.RealTimeContext, S.ThreadPriority, S.RemoteIPAddress, S.RemoteIPPort, S.SARARegionId, S.MaintenanceReq, S.SIDId from System as S join SID as SI on S.SIDId = SI.Id WHERE S.Id = ";
        query << sid;
        if (!isMaster()) {
            query << " AND SARARegionId = ";
            query << cfmRegCfg.SARA_REGION_ID();
        }
        //deletes this system from vector of systems
        for (std::vector<domain::CfmSystem_Table>::iterator it = vSystemList.begin(); it != vSystemList.end(); it++)
            if ((*it).Id == sid)  {
                vSystemList.erase(it);
                break;
            }
        //Load ststem definition
        try {
            auto result = nanodbc::execute(*syncConn, query.str());
            while (result.next()) {
                domain::CfmSystem_Table elt;
                elt.Id = result.get<int>(0);
                elt.alias = result.get<std::string>(1);
                elt.description = result.get<std::string>(2);
                elt.deployment = result.get<std::string>(3);
                elt.localDll = result.get<std::string>(4);
                elt.realTimeContext = (bool)result.get<short>(5);
                elt.ThreadPriority = result.get<int>(7);
                elt.RemoteIPAddress = result.get<std::string>(8);
                elt.RemoteIPPort = result.get<int>(9);
                elt.SARARegionId = result.get<int>(10);
                elt.enabled = !result.get<short>(11);
                elt.SIDId = result.get<int>(12);
                //Insert into vector
                vSystemList.push_back(elt);
            }

        } catch (nanodbc::database_error& e) {
            ManageSQLError(e.what(), e.state(), (int)e.native(), "getSystemList");
            retval = false;
        }
        return retval;
    }
    //----------------------------------------------------------------------------
    bool DBManager::setIsMaster(int id) {
        bool retval = true;
        std::stringstream query(std::stringstream::in | std::stringstream::out);
        query << "SELECT Id from SARARegion WHERE Id <> " << id << " AND ParentRegionId = " << id;

        bIsMaster = false;
        try {
            auto result = nanodbc::execute(*syncConn, query.str());
            bIsMaster = result.rows() > 0;
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(e.what(), e.state(), (int)e.native(), query.str());
            retval = false;
        }
        return retval;
    }
    //----------------------------------------------------------------------------

    bool DBManager::getAdapterList() {
        bool retval = false ;
        std::stringstream query(std::stringstream::in | std::stringstream::out);
        query << "select a.Id, a.Name, a.Description, AT.LocalDll, a.SARARegionId, a.AdaptersStatesId from Adapters as A join AdapterType as AT on  A.AdapterTypeId = AT.Id WHERE A.SARARegionId=";
        query << cfmRegCfg.SARA_REGION_ID();

        try {
            auto result = nanodbc::execute(*syncConn, query.str());
            while (result.next()) {
                domain::CfmAdapter_Table elt{};
                elt.Id = result.get<int>(0);
                elt.Name = result.get<std::string>(1);
                elt.Description = result.get<std::string>(2);
                elt.DllPath = result.get<std::string>(3);
                elt.SARARegionId = result.get<int>(4);
                elt.AdapterStatesId = result.get<int>(5);
                vAdapterList.push_back(elt);
            }
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(e.what(), e.state(), (int)e.native(), "getAdapterList");
            retval = false;
        }
        return retval;
    }
    //----------------------------------------------------------------------------
    bool DBManager::getRegionList(int regionId) {
        bool retval = true;
        while (vRegionList.size()) {
            vRegionList.clear();
        }
        std::stringstream query(std::stringstream::in | std::stringstream::out);
        query << "select Id, Name, Description, HostName, IpAddress, IpGateway, PortNumber, Failover, AutoRestore, EnableSSL, CAFile, SSLPassword, SessionID from SARARegion";
        if (regionId)
            query << " WHERE Id=" << regionId;

        try {
            auto result = nanodbc::execute(*syncConn, query.str());
            while (result.next()) {
                domain::CfmRegion_Table elt{};
                elt.Id = result.get<int>(0);
                elt.Name = result.get<std::string>(1);
                elt.Description = result.get<std::string>(2);
                elt.HostName = result.get<std::string>(3);
                elt.IpGateway = result.get<std::string>(4);
                elt.PortNumber = result.get<int>(5);
                elt.Failover = (bool)result.get<short>(6);
                elt.AutoRestore = (bool)result.get<short>(7);
                elt.EnableSSL = (bool)result.get<short>(8);
                elt.CAFile = result.get<std::string>(9);
                elt.SSLPassword = result.get<std::string>(10);
                elt.SessionID = result.get<std::string>(11);

                vRegionList.push_back(elt);
            }
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(e.what(), e.state(), (int)e.native(), "getRegionList");
            retval = false;
        }
        return retval;

    }
    //----------------------------------------------------------------------------

    std::map<std::string, domain::CfmEventRules_Table> DBManager::getEventRules() {
        std::map<std::string, domain::CfmEventRules_Table> mappaEventiRegole;

        std::string query = "SELECT DeviceId, EventId, RuleId FROM EventRules";
        try {
            auto result = nanodbc::execute(*syncConn, query);
            while (result.next()) {
                domain::CfmEventRules_Table elt{};
                elt.DeviceId = result.get<int>(0);
                elt.EventId = result.get<int>(1);
                elt.RuleId = result.get<int>(2);
                std::stringstream ss;
                ss << elt.EventId << '|' << elt.DeviceId;
                mappaEventiRegole.emplace(ss.str(), elt);
            }
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(e.what(), e.state(), (int)e.native(), query);
        }
        return mappaEventiRegole;
    }
    //----------------------------------------------------------------------------

    std::map<int, CListaAzioni> DBManager::getActionRules() {
        bool r = true;

        //Si passa l'id della regola e si recupera la clase CListaAzioni
        std::map<int, CListaAzioni> mappaEventiRegoleAzioni{};

        if (!IsConnected() || !syncConn->connected())
            return mappaEventiRegoleAzioni;

        std::map<int, domain::CfmRules_Table> mappaEventiRegole = getRules();
        std::string query = "select Id, DeviceId, ActionId, RuleId from ActionRules where RuleId = ?";

        try {
            nanodbc::statement aStatement;

            std::map<int, domain::CfmRules_Table>::iterator iRule;
            for (iRule = mappaEventiRegole.begin(); iRule != mappaEventiRegole.end(); iRule++) {
                CListaAzioni cAzioni{};

                aStatement.prepare(*syncConn, query);
                aStatement.bind<int>(0, &iRule->second.Id);
                auto result = nanodbc::execute(aStatement);
                while (result.next()) {
                    domain::CfmActionRules_Table elt{};

                    elt.Id = result.get<int>(0);
                    elt.DeviceId = result.get<int>(1);
                    elt.ActionId = result.get<int>(2);
                    elt.RuleId = result.get<int>(3);

                    cAzioni.PushAction(elt);
                }
            }
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(e.what(), e.state(), (int)e.native(), "getEventRules");
        }
        return mappaEventiRegoleAzioni;
    }
        //----------------------------------------------------------------------------

    std::map<int, domain::CfmEvent_Table> DBManager::getEvents() {
        if (!mapEvents.empty())
                return mapEvents;

        std::string query = "SELECT Id, Description, Alias, CategoryId FROM Events";

        try {
            auto result = nanodbc::execute(*syncConn, query);
            while (result.next()) {
                domain::CfmEvent_Table elt{};
                elt.Id = result.get<int>(0);
                elt.description = result.get<std::string>(1);
                elt.alias = result.get<std::string>(2);
                elt.CategoryId = result.get<int>(4);
                mapEvents.emplace(elt.Id, elt);
            }
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(e.what(), e.state(), (int)e.native(), "getEventRules");
        }
        return mapEvents;
    }

    //----------------------------------------------------------------------------

    std::map<int, domain::CfmAction_Table> DBManager::getActions() {
        if (!mappaAzioni.empty())
                return mappaAzioni;
        try {
                auto result = nanodbc::execute(*syncConn, NANODBC_TEXT("SELECT Id, Description, Alias, CategoryId FROM Actions"));
                while (result.next()) {
                    domain::CfmAction_Table ac;
                    ac.Id = result.get<int>(0);
                    ac.description = result.get<std::string>(1);
                    ac.alias = result.get<std::string>(2);
                    ac.CategoryId = result.get<int>(3);
                    mappaAzioni.emplace(ac.Id, ac);
                }
        } catch (nanodbc::database_error &e) {
        ManageSQLError(std::string(e.what()), e.state(), (int)e.native(), "SELECT Id, Description, Alias, CategoryId FROM Actions");
        }
        return mappaAzioni;
    }
    //----------------------------------------------------------------------------

    std::map<std::string, domain::CfmAction_Table> DBManager::getActionsByNameCat() {
        if (!mappaAzioniByNameCat.empty())
            return mappaAzioniByNameCat;

        std::string query = "SELECT Id, Description, Alias, CategoryId FROM Actions";

        try {
            auto result = nanodbc::execute(*syncConn, query);
            while (result.next()) {
                domain::CfmAction_Table elt;
                elt.Id = result.get<int>(0);
                elt.description = result.get<std::string>(1);
                elt.alias = result.get<std::string>(2);
                elt.CategoryId = result.get<int>(3);
                std::stringstream sKey;
                sKey << elt.alias << "|" << elt.CategoryId;
                mappaAzioniByNameCat.emplace(sKey.str(), elt);
            }
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(std::string(e.what()), e.state(), (int)e.native(), "SELECT Id, Description, Alias, CategoryId FROM Actions");
        }
        return mappaAzioniByNameCat;
    }

    //----------------------------------------------------------------------------

    /**
      * @fn DBManager::getEventAndRules()
      * @brief		Carica in memoria la tabella EventAndRules
      * @author		Oscar Casale
      * @date		  27/02/2009	Created
      * @return		Ritorna la hash-table contenente la tabella indicizzata per EventOpId
      *
      */
    std::map<std::string, domain::CfmEventAndRules_Table> DBManager::getEventAndRules() {

        std::map<std::string, domain::CfmEventAndRules_Table> mappaEventiAndRegole;
        std::string query = "SELECT Id, DeltaT, RuleId FROM EventAndRules";
        try {
            auto result = nanodbc::execute(*syncConn, query);
            while (result.next()) {
                domain::CfmEventAndRules_Table elt;
                elt.EventOpId = result.get<int>(0);
                elt.DeltaT = result.get<int>(1);
                elt.RuleId = result.get<int>(2);
    
                std::stringstream sKey;
                sKey << elt.EventOpId;
                mappaEventiAndRegole.emplace(sKey.str(), elt);
            }
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(std::string(e.what()), e.state(), (int)e.native(), "SELECT Id, DeltaT, RuleId FROM EventAndRules");
        }
        return mappaEventiAndRegole;
    }
    //----------------------------------------------------------------------------

    /**
    * @fn DBManager::getEventOrRules()
    * @brief		Carica in memoria la tabella EventOrRules
    * @author		Oscar Casale
    * @date		  27/02/2009	Created
    * @return		Ritorna la hash-table contenente la tabella indicizzata per EventOpId
    *
    */
    std::map<std::string, domain::CfmEventOrRules_Table> DBManager::getEventOrRules() {
        std::map<std::string, domain::CfmEventOrRules_Table> mappaEventiOrRegole;
        std::string query("SELECT Id, DeltaT, RuleId FROM EventOrRules;");
        try {
            auto result = nanodbc::execute(*syncConn, query);
            while (result.next()) {
                domain::CfmEventAndRules_Table elt;
                elt.EventOpId = result.get<int>(0);
                elt.DeltaT = result.get<int>(1);
                elt.RuleId = result.get<int>(2);

                std::stringstream sKey;
                sKey << elt.EventOpId;
                mappaEventiOrRegole.emplace(sKey.str(), elt);
            }
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(std::string(e.what()), e.state(), (int)e.native(), "SELECT Id, DeltaT, RuleId FROM EventOrRules");
        }
        return mappaEventiOrRegole;
    }
    //----------------------------------------------------------------------------

    /**
    * @fn DBManager::getEventAnd()
    * @brief		Carica in memoria la tabella EventAnd
    * @author		Oscar Casale
    * @date		  27/02/2009	Created
    * @return		Ritorna vettore
    *
    */
    std::vector<domain::CfmEventAnd_Table> DBManager::getEventAnd() {
        std::vector<domain::CfmEventAnd_Table> vectorEventAnd{};
        std::string query("SELECT EventAndRulesId, EventId, DeviceId FROM EventAnd");
        try {
            auto result = nanodbc::execute(*syncConn, query);
            while (result.next()) {
                domain::CfmEventAnd_Table elt;
                elt.EventId = result.get<int>(0);
                elt.DeviceId = result.get<int>(1);
                elt.EventOpId = result.get<int>(2);
                vectorEventAnd.emplace_back(elt);
            }
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(std::string(e.what()), e.state(), (int)e.native(), "SELECT EventAndRulesId, EventId, DeviceId FROM EventAnd");
        }
        return vectorEventAnd;
    }
    //----------------------------------------------------------------------------

    /**
    * @fn DBManager::getEventOr()
    * @brief		Carica in memoria la tabella EventOr
    * @author		Oscar Casale
    * @date		  27/02/2009	Created
    * @return		Ritorna vettore
    *
    */
    std::vector<domain::CfmEventOr_Table> DBManager::getEventOr() {
        std::vector<domain::CfmEventOr_Table> vectorEventOr;
        std::string query("SELECT EventOrRulesId, EventId, DeviceId FROM EventOr");
        try {
            auto result = nanodbc::execute(*syncConn, query);
            while (result.next()) {
                domain::CfmEventOr_Table elt;
                elt.EventId = result.get<int>(0);
                elt.DeviceId = result.get<int>(1);
                elt.EventOpId = result.get<int>(2);
                vectorEventOr.emplace_back(elt);
            }
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(std::string(e.what()), e.state(), (int)e.native(), "SELECT EventOrRulesId, EventId, DeviceId FROM EventOr");
        }
        return vectorEventOr;
    }
    //----------------------------------------------------------------------------

    void DBManager::LogTrack(int DeviceId, std::string SystemTrackId, float x, float y, float z, float Azimuth, float Range, float Elevation, int EventId) {
        domain::CfmLogTrack_Table* record = new domain::CfmLogTrack_Table();
        record->DeviceId = DeviceId;
        record->Azimuth = Azimuth;
        record->Range = Range;
        record->Elevation = Elevation;
        record->x = x;
        record->y = y;
        record->z = z;
        record->SystemTrackId = SystemTrackId;
        record->EventId = EventId;

        PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_INSERT_TRACK, (WPARAM)record, 0);
    }
    //----------------------------------------------------------------------------

    /**
    * @fn		DBManager::LogOCR
    * @date	01/10/2007 Inserito EventId per la correlazione all'evento
    *
     */
    void DBManager::LogOCR(int DeviceId, std::string LicensePlate, std::string TrailerPlate, std::string Container1, std::string Container2, std::string Container3, std::string Container4, int EventId,
        std::string img_path_1, std::string img_path_2, std::string img_path_3, std::string img_path_4, std::string img_path_5, std::string img_path_6) {
            domain::CfmLogOCR_Table* record = new domain::CfmLogOCR_Table();
            record->DeviceId = DeviceId;
            record->LicensePlate = LicensePlate;
            record->TrailerPlate = TrailerPlate;
            record->Container1 = Container1;
            record->Container2 = Container2;
            record->Container3 = Container3;
            record->Container4 = Container4;
            record->EventId = EventId;
            record->img_path_1 = img_path_1;
            record->img_path_2 = img_path_2;
            record->img_path_3 = img_path_3;
            record->img_path_4 = img_path_4;
            record->img_path_5 = img_path_5;
            record->img_path_6 = img_path_6;

            //Ivio il messaggio al thread del DBManager così effettua l'inserimento
            PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_INSERT_OCR, (WPARAM)record, 0);
    }
    //----------------------------------------------------------------------------

    void DBManager::SaveOCR(domain::CfmLogOCR_Table* record) {
        std::stringstream query(std::stringstream::in | std::stringstream::out);
        query << "INSERT INTO OCR_Log(DeviceId, DatabaseTS, ocr_1, ocr_2, ocr_3, ocr_4, ocr_5, ocr_6, EventId, img_path_1, img_path_2, img_path_3, img_path_4, img_path_5, img_path_6) values(";
        query << record->DeviceId << ",";
        query << "GETDATE(),";
        query << "'" << record->LicensePlate.c_str() << "',";
        query << "'" << record->TrailerPlate.c_str() <<  "',";
        query << "'" << record->Container1.c_str() << "',";
        query << "'" << record->Container2.c_str() << "',";
        query << "'" << record->Container3.c_str() << "',";
        query << "'" << record->Container4.c_str() << "',";
        query << record->EventId << ",";
        query << "'" << record->img_path_1.c_str() << "',";
        query << "'" << record->img_path_2.c_str() << "',";
        query << "'" << record->img_path_3.c_str() << "',";
        query << "'" << record->img_path_4.c_str() << "',";
        query << "'" << record->img_path_5.c_str() << "',";
        query << "'" << record->img_path_6.c_str() << "'";
        query << ")";

        delete record;
        if (!IsConnected() || !asyncConn->connected())
            return;
        try {
            nanodbc::execute(*asyncConn, query.str());
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(std::string(e.what()), e.state(), (int)e.native(), "insert into OCR_Log");
        }

     }
    //----------------------------------------------------------------------------

    void DBManager::SaveTrack(domain::CfmLogTrack_Table* record) {
        std::stringstream query(std::stringstream::in | std::stringstream::out);
        query << "insert into Tracks_Log(DeviceId, Timestamp, Azimuth, Range, Elevation, x, y, z, SystemTrackId, EventId) values(";
        query << record->DeviceId << ",";
        query << "GETDATE(),";
        query << record->Azimuth << ",";
        query << record->Range << ",";
        query << record->Elevation << ",";
        query << record->x << ",";
        query << record->y << ",";
        query << record->z << ",";
        query << "'" << record->SystemTrackId.c_str() << "',";
        query << record->EventId << ")";

        if (!IsConnected() || !asyncConn->connected())
            return;
        try {
            nanodbc::execute(*asyncConn, query.str());
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(std::string(e.what()), e.state(), (int)e.native(), "insert into Tracks_Log");
        }

    }
    //----------------------------------------------------------------------------
    /**
    * @fn DBManager::Run( LPVOID )
    * @brief Codice eseguito dal thread realtivo al DBManager
    *
    * Il DB Manager viene realizzato come un thread in quanto è permette
    * la scrittura asincrona su tabelle del database.
    * La funzionalità risulta estremamente utile in quando è richiesto il
    * logging di alcuni dati (ex. tracce, letture OCR) su tabelle del db.
    * Questo tipo di operazioni infatti eseguite in modalità sincrona da parte
    * del Kernel comporterebbero una drastica riduzione delle prestazioni del sistema
    * I messaggi gestiti ad oggi sono
    *
    * - DB_INSERT_OCR
    * - DB_INSERT_TRACK
    *
    */
    DWORD DBManager::Run(LPVOID /* arg */) {
            //Waiting for system messages

            while (!bTerminated) {
                GetMessage(&msg, NULL, 0, 0);

                //Messages management
                if (!execute(msg.message)) {
                    //Dispatch dei messaggi
                    DispatchMessage(&msg);
                }
            }
            return m_ThreadCtx.m_dwExitCode;
    }
    //----------------------------------------------------------------------------
    void DBManager::ManageSQLError(const std::string& message, const std::string& code, int res, const std::string& query) {
            if (message.empty() && code.empty() && !res)
                return;

            if (res == SQL_NO_DATA)
                return;

            snprintf(logBuffer, sizeof(logBuffer) - 1, "ERRORE DB Manager: [%s] %s", (char*)code.c_str(), (char*)message.c_str());
            if (!query.empty())
                snprintf(logBuffer, sizeof(logBuffer) - 1, "%s\nQuery: [%s]", logBuffer, query.c_str());

            if (code == "08S01" //Communication failure
                || code == "08001" // Client unable to establish connection
                || code == "08002" // Connection name in use
                || code == "08003" // Connection not open
                || code == "08004" // Server rejected the connection
                || code == "08007") // Connection failure during transaction
            {
                CLogger::getInstance()->Log("[DB MANAGER] [ManageSQLerror()] [COMMUNICATION LINK FAILURE]");

                if (state == DBManager::STATE_CONNECTED)
                {
                    state = DBManager::STATE_NOT_CONNECTED;

                    CLogger::getInstance()->Log("[DB MANAGER] [ManageSQLerror()] [STATE NOT CONNECTED]");

                    PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_RECONNECT, NULL, NULL);
                }
            }
            CLogger::getInstance()->Log(logBuffer);

            // Scrive la query nel log delle query
            CLogger::getInstance()->LogSQL((char*)query.c_str());
    }
    //----------------------------------------------------------------------------

    std::map<std::string, domain::CfmLicense_Table> DBManager::getLicenses() {
        std::map<std::string, domain::CfmLicense_Table> retval;
        std::stringstream query(std::stringstream::in | std::stringstream::out);

        if (!isMaster()) {
            query << "SELECT LocalDll, Instances, LicenseKey from v_Licenses where SARARegionId="
            << cfmRegCfg.SARA_REGION_ID();
        } else
             query << "SELECT VL.LocalDll, L.Instances, L.LicenseKey from v_Licenses as VL, License as L, SID as S, "
                << "SARARegion as SR "
                << "WHERE VL.LocalDll = S.LocalDll AND S.Id = L.SIDId "
                << "AND L.SARARegionId = SR.Id "
                << "and SR.RegionType = 1";

        try {
            auto result = nanodbc::execute(*syncConn, query.str());
            while (result.next()) {
                domain::CfmLicense_Table elt;
                elt.LocalDll = result.get<std::string>(0);
                elt.Instances = result.get<short>(1);
                elt.LicenseKey = result.get<std::string>(2);
//Pekmez                elt.ExpirationDate = ExtractExpirationDate(elt.LicenseKey);
                elt.LicenseKey = elt.LicenseKey.substr(0, 32);
                retval.emplace(elt.LocalDll, elt);
            }
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(std::string(e.what()), e.state(), (int)e.native(), "getLicenses");
        }
        return retval;
    }
    //----------------------------------------------------------------------------

    std::map<std::string, domain::CfmDevices_Table>  DBManager::getDevices() {
        std::map<std::string, domain::CfmDevices_Table> retval;
        std::string query("SELECT Id, SystemId, SubId, CategoryId, Description FROM Devices");
        try {
            auto result = nanodbc::execute(*syncConn, query);
            while (result.next()) {
                domain::CfmDevices_Table elt;
                elt.Id = result.get<int>(0);
                elt.SystemId = result.get<int>(1);
                elt.subId = result.get<std::string>(2);
                elt.catId = result.get<int>(3);
                elt.Description = result.get<int>(4);
                elt.bDeleted = (domain::stateRecDelete)0;
                std::stringstream key;
                key << elt.SystemId << '|' << elt.subId.c_str() << '|' << elt.catId;
                retval.emplace(key.str(), elt);
            }
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(std::string(e.what()), e.state(), (int)e.native(), "SELECT Id, SystemId, SubId, CategoryId, Description FROM Devices");
        }
        return retval;
    }
    //----------------------------------------------------------------------------
    void CListaAzioni::PushAction(domain::CfmActionRules_Table action) {
        this->lista.push_back(action);
    }
    //----------------------------------------------------------------------------

    std::map<int, domain::CfmRules_Table> DBManager::getRules() {
        std::map<int, domain::CfmRules_Table>  retval;
        std::string query("SELECT Id, Alias FROM Rules");
        try {
            auto result = nanodbc::execute(*syncConn, query);
            while (result.next()) {
                domain::CfmRules_Table elt;
                elt.Id = result.get<int>(0);
                elt.Alias = result.get<int>(1);
                retval.emplace(elt.Id, elt);
            }
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(std::string(e.what()), e.state(), (int)e.native(), "SELECT Id, Alias FROM Rules");
        }
        return retval;
    }
    //----------------------------------------------------------------------------

    bool DBManager::getCategoriesPriority() {
        bool r = false;
        std::map<int, domain::CfmCategory_Table>::iterator itCategories;
        mapCategoriesList.clear();
        mapCategoriesPriority.clear();


        std::string query("SELECT Id, Description, CatAlias, Actuator, Priority FROM Categories ORDER BY Priority");
        try {
            auto result = nanodbc::execute(*syncConn, query);
            while (result.next()) {
                domain::CfmCategory_Table elt;
                elt.Id = result.get<int>(0);
                elt.Description = result.get<std::string>(1);
                elt.Alias = result.get<std::string>(2);
                elt.Actuator = result.get<int>(3);
                elt.Priority = result.get<int>(4);
                mapCategoriesList.emplace(elt.Id, elt);
                mapCategoriesPriority.emplace(elt.Priority, elt);
            } 
        } catch (nanodbc::database_error& e) {
            ManageSQLError(e.what(), e.state(), (int)e.native(), "SELECT Id, Description, CatAlias, Actuator, Priority FROM Categories ORDER BY Priority");
        }

        return true;
    }
    //----------------------------------------------------------------------------

    std::map<std::string, domain::CfmCategory_Table> DBManager::getCategories() {
        std::map<std::string, domain::CfmCategory_Table> mappaCategorie;

        std::string query("SELECT Id, Description, CatAlias, Actuator, Priority FROM Categories");
        try {
            auto result = nanodbc::execute(*syncConn, query);
            while (result.next()) {
                domain::CfmCategory_Table elt;
                elt.Id = result.get<int>(0);
                elt.Description = result.get<std::string>(1);
                elt.Alias = result.get<int>(2);
                elt.Actuator = result.get<int>(3);
                elt.Priority = result.get<int>(4);
                std::stringstream key;
                key << elt.Id;
                mappaCategorie.emplace(key.str(), elt);
            }
        }
        catch (nanodbc::database_error& e) {
            ManageSQLError(e.what(), e.state(), (int)e.native(), "SELECT Id, Description, CatAlias, Actuator, Priority FROM Categories ORDER BY Priority");
        }
        return mappaCategorie;
    }
    //----------------------------------------------------------------------------

    int DBManager::InsertDevice(int iId, int iLogicalStatus, int iPhisicalStatus, int idCategory, std::string sDescription, 
        int iIdSystemId, std::string sSubId, int iParentId, int iSiteId, int iProcessState, std::string sDateTime) {

        if (mapCategoriesList.find(idCategory) == mapCategoriesList.end())
            return -1;

        int categoryId = idCategory;
        char queryBuf[4092]; 
        queryBuf[sizeof(queryBuf) - 1] = 0;

        std::string strDateTime;
        std::string strUTCDateTime;

        //if (sDateTime.empty())
        //    strDateTime = GetLocalTime();
        //else
        //    strDateTime = sDateTime;

        //strUTCDateTime = GetUTCTimeByLocalTime(strDateTime);

        //sDescription = ValidateStringDB(sDescription);

        //snprintf(queryBuf, sizeof(queryBuf) - 1,
        //    "INSERT INTO Devices(Id, LogicalStatus, PhisicalStatus, CategoryId, "
        //    " Description, SystemId, SubId, Deleted, Modified, Updated, SiteId, ParentId, ProcessState, UpdatedUTC)"
        //    " VALUES(%i,%i,%i,%i,'%s',%i,'%s',%i,'%s','%s'",
        //    iId, iLogicalStatus, iPhisicalStatus, categoryId, sDescription.c_str(),
        //    iIdSystemId, sSubId.c_str(), false, strDateTime.c_str(), strDateTime.c_str());

        //if (iSiteId > 0)
        //    snprintf(queryBuf, sizeof(queryBuf) - 1, "%s,%i", queryBuf, iSiteId);
        //else // Imposta NULL in caso di SiteId=0
        //    snprintf(queryBuf, sizeof(queryBuf) - 1, "%s,1", queryBuf);

        //if (iParentId != 0)
        //    snprintf(queryBuf, sizeof(queryBuf) - 1, "%s,%i", queryBuf, iParentId);
        //else // Imposta NULL in caso di iParentId=0
        //    snprintf(queryBuf, sizeof(queryBuf) - 1, "%s,NULL", queryBuf);

        //snprintf(queryBuf, sizeof(queryBuf) - 1, "%s,%i,'%s')", queryBuf, iProcessState, strUTCDateTime.c_str());

        //QUERY_ENTER(queryBuf, -1)

        //    QUERY_EXIT(queryBuf)

        //    return (r) ? 0 : -1;
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::UpdateDevice(int iId, int iPhisicalStatus, int iLogicalStatus, int idCategory,
        std::string sDescription, int iIdSystemId, std::string sSubId,
        int iParentId, int iSiteId, int iProcessState,
        std::string sDateTime, stRecDelete bDelete, int iExecuting) {

        if (mapCategoriesList.find(idCategory) == mapCategoriesList.end())
            return -1;

        //std::string strDateTime;
        //std::string strUTCDateTime;

        //if (sDateTime.empty())
        //    strDateTime = GetLocalTime();
        //else
        //    strDateTime = sDateTime;

        //strUTCDateTime = GetUTCTimeByLocalTime(strDateTime);
        //sDescription = ValidateStringDB(sDescription);

        //std::string queryBuf = "UPDATE Devices SET "
        //    "  PhisicalStatus = " + IntToStr(iPhisicalStatus) +
        //    ", LogicalStatus = " + IntToStr(iLogicalStatus) +
        //    ", CategoryId = " + IntToStr(idCategory) +
        //    ", Description = '" + sDescription + "'"
        //    ", SystemId = " + IntToStr(iIdSystemId) +
        //    ", SubId = '" + sSubId + "'"
        //    ", Modified = '" + strDateTime + "'"
        //    ", Updated = '" + strDateTime + "'"
        //    ", ProcessState = '" + IntToStr(iProcessState) + "'"
        //    ", Deleted = " + std::string((bDelete == recDeleted) ? "1" : "0") +
        //    ", Executing = " + IntToStr(iExecuting) +
        //    ", UpdatedUTC = '" + strUTCDateTime + "'";

        //if (iSiteId > 0)
        //    queryBuf += ", SiteId = " + IntToStr(iSiteId);
        //else // Imposta 1 in caso di SiteId=0
        //    queryBuf += ", SiteId = 1";

        //if (iParentId != 0)
        //    queryBuf += ", ParentId = " + IntToStr(iParentId);
        //else // Imposta NULL in caso di iParentId=0
        //    queryBuf += ", ParentId = NULL";

        //queryBuf += " WHERE Id = " + IntToStr(iId);

        //QUERY_ENTER(queryBuf, -1)

        //    QUERY_EXIT(queryBuf)

        //    return (r) ? 0 : -1;
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::UpdateExecuting(int IdDevice, int iExecuting) {
        /*std::string queryBuf = "UPDATE Devices SET Executing = " + IntToStr(iExecuting) + " WHERE Id = " + IntToStr(IdDevice);

        QUERY_ENTER(queryBuf, -1)

            QUERY_EXIT(queryBuf)

            return (r) ? 0 : -1;*/
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::DeleteTable(std::string sTableName) {
        //std::string query("delete from " + sTableName);

        //QUERY_ENTER(query, -1)

        //    QUERY_EXIT(query)

        //    return (r) ? 0 : -1;
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::InsertRule(int iId, std::string sAlias) {
        //std::string query = "insert into Rules(Id, Alias) values(" + IntToStr(iId) + ",'" + sAlias + "')";

        //QUERY_ENTER(query, -1)

        //    QUERY_EXIT(query)

        //    return (r) ? 0 : -1;
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::SetFailover(int idRegion, bool failover) {
        //std::stringstream query(std::stringstream::in | std::stringstream::out);
        //query << "UPDATE SARARegion SET Failover=" << failover << " WHERE id=" << idRegion;

        //QUERY_ENTER(query.str(), false)

        //    QUERY_EXIT(query.str())

        //    return r;
        return 0;
    }
    //----------------------------------------------------------------------------

    bool DBManager::GetUserPassword(std::string& user, std::string& password) {
        bool r = false;
        //std::stringstream query(std::stringstream::in | std::stringstream::out);
        //query << "SELECT Password FROM Users WHERE Account=" << "'" << user << "'";

        //QUERY_ENTER(query.str(), false)

        //    if (!aStatement.fetch_next())
        //        return false;

        //password = aStatement.field(1).as_string();

        //QUERY_EXIT(query.str())

            return r;
    }
    //----------------------------------------------------------------------------

    int DBManager::InsertEventRule(int iEventId, int iDeviceId, int iRuleId) {
        //std::string query = "insert into EventRules(EventId, DeviceId, RuleId) values("
        //    + IntToStr(iEventId) + ","
        //    + IntToStr(iDeviceId) + ","
        //    + IntToStr(iRuleId) + ")";

        //QUERY_ENTER(query, -1)

        //    QUERY_EXIT(query)

        //    return (r) ? 0 : -1;
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::InsertActionRule(int iId, int iActionId, int iDeviceId, int iRuleId) {
        //std::string query = "insert into ActionRules(Id, ActionId, DeviceId, RuleId) values("
        //    + IntToStr(iId) + ","
        //    + IntToStr(iActionId) + ","
        //    + IntToStr(iDeviceId) + ","
        //    + IntToStr(iRuleId) + ")";

        //QUERY_ENTER(query, -1)

        //    QUERY_EXIT(query)

        //    return (r) ? 0 : -1;
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::InsertRuleParameter(int iId, int iIdRule, int iIdActionRule, int iIdParameter, std::string sValue) {
  /*      std::string query = "insert into RuleParameters(Id, IdRule, IdActionRule, IdActionParameter, Value) values("
            + IntToStr(iId) + ","
            + IntToStr(iIdRule) + ","
            + IntToStr(iIdActionRule) + ","
            + IntToStr(iIdParameter) + ","
            + sValue + ")";

        QUERY_ENTER(query, -1)

            QUERY_EXIT(query)

            return (r) ? 0 : -1;*/
        return 0;
    }
    //----------------------------------------------------------------------------

    //Parametri di sotto sistema
    std::map<std::string, domain::CfmSystemParameters_Table>	DBManager::getSystemParameters(int iSysId) {
        std::map<std::string, domain::CfmSystemParameters_Table> mappaParametri;
        /*std::string query("select Id, Section, Name, Value, MasterValue, Encrypted, SystemId from SystemParameters where SystemId="
            + IntToStr(iSysId));

        QUERY_ENTER(query, mappaParametri)

            while (aStatement.fetch_next())
            {
                SaraSystemParameters_Table sp;

                sp.Id = aStatement.field(1).as_unsigned_long();
                sp.Section = aStatement.field(2).as_string();
                sp.Name = aStatement.field(3).as_string();
                sp.Value = aStatement.field(4).as_string();
                sp.MasterValue = aStatement.field(5).as_string();
                sp.Encrypted = (bool)aStatement.field(6).as_short();
                if (sp.Encrypted)
                {
                    sp.Value = std::string(decode_value((char*)sp.Value.c_str(), "ISISOFT"));
                }
                sp.SystemId = aStatement.field(7).as_unsigned_long();

                std::string key = sp.Section + '|' + sp.Name;
                mappaParametri[key] = sp;
            }
        QUERY_EXIT(query)*/

            return mappaParametri;
    }
    //----------------------------------------------------------------------------
    std::map<std::string, domain::CfmAdapterParameters_Table>	DBManager::getAdapterParameters(int id) {
        std::map<std::string, domain::CfmAdapterParameters_Table> mappaParametri;
        
        //std::string query("select AP.Id, AP.Section, AP.Name, AP.Value, AP.Encrypted, AP.AdapterId, A.SARARegionId from AdapterParameters as AP join Adapters as A on AP.AdapterId = A.Id where AdapterId="
        //    + IntToStr(id));

        //QUERY_ENTER(query, mappaParametri)

        //    while (aStatement.fetch_next())
        //    {
        //        SaraAdapterParameters_Table sp;

        //        sp.Id = aStatement.field(1).as_unsigned_long();
        //        sp.Section = aStatement.field(2).as_string();
        //        sp.Name = aStatement.field(3).as_string();
        //        sp.Value = aStatement.field(4).as_string();
        //        sp.Encrypted = (bool)aStatement.field(5).as_short();
        //        if (sp.Encrypted)
        //        {
        //            sp.Value = std::string(decode_value((char*)sp.Value.c_str(), "ISISOFT"));
        //        }
        //        sp.AdapterId = aStatement.field(6).as_unsigned_long();
        //        sp.SARARegionId = aStatement.field(7).as_unsigned_long();

        //        std::string key = sp.Section + '|' + sp.Name;
        //        mappaParametri[key] = sp;
        //    }
        //QUERY_EXIT(query)

        return mappaParametri;
    }
    //----------------------------------------------------------------------------

    void DBManager::LogMessage(std::string SourceTS, int iSourceId, int iSeverity, std::string msgType, std::string msg) {
        domain::CfmLog_Table* record = new domain::CfmLog_Table();

        LPSYSTEMTIME lpST = new SYSTEMTIME();
        //GetLocalTime(lpST);
        //char szLocalDate[255], szLocalTime[255], szDateTime[255];
        //GetDateFormat(LOCALE_USER_DEFAULT, 0, lpST, "yyyyMMdd", szLocalDate, 255);
        //GetTimeFormat(LOCALE_USER_DEFAULT, 0, lpST, "HH:mm:ss", szLocalTime, 255);
        //sprintf_s(szDateTime, sizeof(szDateTime), "%s %s.%i", szLocalDate, szLocalTime, lpST->wMilliseconds);

        //record->SourceTS = SourceTS;
        //record->HostTS = std::string(szDateTime);
        //record->iSeverity = iSeverity;
        //record->iSourceId = iSourceId;
        //record->sMsgType = msgType;
        //record->sMsg = msg;

        ////Ivio il messaggio al thread del DBManager così effettua l'inserimento
        //PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_INSERT_LOGMESSAGE, (WPARAM)record, 0);

        delete lpST;
    }
    //----------------------------------------------------------------------------

    void DBManager::SaveLogMessage(domain::CfmLog_Table* record) {
        if (!record)
            return;

        //record->sMsg = ValidateStringDB(record->sMsg);
        //record->sMsgType = ValidateStringDB(record->sMsgType);

        //std::string query = "insert into SaraLog(SourceTS, HostTS, DBTS, SourceId, Severity, MsgType, Message) values(";
        //query += "'" + record->SourceTS + "',";
        //query += "'" + record->HostTS + "',";
        //query += "GETDATE(),";
        //query += IntToStr(record->iSourceId) + ",";
        //query += IntToStr(record->iSeverity) + ",";
        //query += "'" + record->sMsgType + "',";
        //query += "'" + record->sMsg + "')";

        //delete record;

        //ASYNCQUERY_ENTER(query)

        //    QUERY_EXIT(query)
    }
    //----------------------------------------------------------------------------

    std::map<std::string, domain::CfmActionParametersJoin_Table*>  DBManager::getRulesParameters() {
        std::map<std::string, domain::CfmActionParametersJoin_Table*> mappaParametri;
        //std::string query("select RP.IdRule, RP.IdActionRule, AP.Id, AP.Alias, AP.Type, RP.Value, AR.ActionId, RP.Id from ActionParameters AP, RuleParameters RP, ActionRules AR where AP.Id = RP.IdActionParameter and AR.Id = RP.IdActionRule");

        //QUERY_ENTER(query, mappaParametri);

        //while (aStatement.fetch_next())
        //{
        //    SaraActionParametersJoin_Table* sp = new SaraActionParametersJoin_Table();

        //    sp->IdRule = aStatement.field(1).as_unsigned_long();
        //    sp->IdActionRule = aStatement.field(2).as_unsigned_long();
        //    sp->IdParameter = aStatement.field(3).as_unsigned_long();
        //    sp->Alias = aStatement.field(4).as_string();
        //    sp->Type = aStatement.field(5).as_string();
        //    sp->Value = aStatement.field(6).as_string();
        //    sp->IdAction = aStatement.field(7).as_unsigned_long();
        //    sp->Id = aStatement.field(8).as_unsigned_long();

        //    std::string key = IntToStr(sp->IdRule) + '|' + IntToStr(sp->IdActionRule) + '|' + sp->Alias;
        //    mappaParametri[key] = sp;
        //}
        //QUERY_EXIT(query)

            return mappaParametri;
    }
    //----------------------------------------------------------------------------

    std::vector<domain::CfmDevices_Table>  DBManager::getDevices(int iSubId, int iCat, long iSiteId) {
        std::vector<domain::CfmDevices_Table> listDevices;

        //std::string query = "SELECT D.Id, D.LogicalStatus, D.PhisicalStatus, D.CategoryId, D.Description, D.SystemId, "
        //    " D.SubId, D.Deleted, D.Modified, D.SiteId, D.Updated, D.ParentId, D.ProcessState, "
        //    " D.Executing, D.UpdatedUTC "
        //    " FROM Devices AS D "
        //    " WHERE D.CategoryId=" + IntToStr(iCat) +
        //    "	AND D.SystemId=" + IntToStr(iSubId);
        //if (iSiteId > 0)
        //    query += "	AND D.SiteId=" + IntToStr(iSiteId);

        //QUERY_ENTER(query, listDevices);

        //while (aStatement.fetch_next())
        //{
        //    SaraDevices_Table sdt;

        //    sdt.Id = aStatement.field(1).as_unsigned_long();
        //    sdt.LogicalStatus = (bool)aStatement.field(2).as_short();
        //    sdt.PhisicalStatus = aStatement.field(3).as_short();
        //    sdt.catId = aStatement.field(4).as_unsigned_long();
        //    sdt.Description = aStatement.field(5).as_string();
        //    sdt.SystemId = aStatement.field(6).as_unsigned_long();
        //    sdt.subId = aStatement.field(7).as_string();
        //    sdt.bDeleted = (aStatement.field(8).as_short()) ? recDeleted : recNoDeleted;
        //    sdt.sModifiedTS = aStatement.field(9).as_string();
        //    sdt.SiteId = aStatement.field(10).as_unsigned_long();
        //    sdt.sUpdatedTS = aStatement.field(11).as_string();
        //    sdt.ParentId = aStatement.field(12).as_unsigned_long();
        //    sdt.ProcessState = aStatement.field(13).as_short();
        //    sdt.Executing = aStatement.field(14).as_short();
        //    sdt.sUpdatedUTCTS = aStatement.field(15).as_string();

        //    listDevices.push_back(sdt);
        //}
        //QUERY_EXIT(query)

        //    std::vector<SaraDevices_Table>::iterator itDev = listDevices.begin();
        //for (; itDev != listDevices.end(); itDev++)
        //    itDev->DeviceParameters = GetDeviceParameters(itDev->Id);
        return listDevices;
    }
    //----------------------------------------------------------------------------

    std::vector<domain::CfmDevices_Table>  DBManager::getDevices(int iSubId, std::string sCat) {
        std::vector<domain::CfmDevices_Table> listDevices;

        /*std::string query = "SELECT D.Id, D.LogicalStatus, D.PhisicalStatus, D.CategoryId, D.Description, D.SystemId, "
            " D.SubId, D.Deleted, D.Modified, D.SiteId, D.Updated, D.ParentId, D.ProcessState, "
            " D.Executing, D.UpdatedUTC "
            " FROM Devices D, Categories C "
            " WHERE C.Id = D.CategoryId "
            "	AND C.CatAlias = '" + sCat + "'"
            "	AND SystemId=" + IntToStr(iSubId);

        QUERY_ENTER(query, listDevices);

        while (aStatement.fetch_next())
        {
            SaraDevices_Table sdt;

            sdt.Id = aStatement.field(1).as_unsigned_long();
            sdt.LogicalStatus = (bool)aStatement.field(2).as_short();
            sdt.PhisicalStatus = aStatement.field(3).as_short();
            sdt.catId = aStatement.field(4).as_unsigned_long();
            sdt.Description = aStatement.field(5).as_string();
            sdt.SystemId = aStatement.field(6).as_unsigned_long();
            sdt.subId = aStatement.field(7).as_string();
            sdt.bDeleted = (stRecDelete)aStatement.field(8).as_short();
            sdt.sModifiedTS = aStatement.field(9).as_string();
            sdt.SiteId = aStatement.field(10).as_unsigned_long();
            sdt.sUpdatedTS = aStatement.field(11).as_string();
            sdt.ParentId = aStatement.field(12).as_unsigned_long();
            sdt.ProcessState = aStatement.field(13).as_short();
            sdt.Executing = aStatement.field(14).as_short();
            sdt.sUpdatedUTCTS = aStatement.field(15).as_string();

            listDevices.push_back(sdt);
        }
        QUERY_EXIT(query)*/

        return listDevices;
    }
    //----------------------------------------------------------------------------

    void DBManager::LogEventAnd(int idEventAndLog, int iEventOpId, int iEvent, int iDevice, std::string Details, std::string sSourceTS) {
        domain::CfmEventAndLog_Table* record = new domain::CfmEventAndLog_Table();

        LPSYSTEMTIME lpST = new SYSTEMTIME();
        GetLocalTime(lpST);
        //char szLocalDate[255], szLocalTime[255], szDateTime[255];
        //GetDateFormat(LOCALE_USER_DEFAULT, 0, lpST, "yyyyMMdd", szLocalDate, 255);
        //GetTimeFormat(LOCALE_USER_DEFAULT, 0, lpST, "HH:mm:ss", szLocalTime, 255);
        //sprintf_s(szDateTime, sizeof(szDateTime), "%s %s.%i", szLocalDate, szLocalTime, lpST->wMilliseconds);

        //record->IdEventAndLog = idEventAndLog;
        //record->KernelTS = std::string(szDateTime);
        //record->SourceTS = sSourceTS;
        //record->IdEventAnd = iEventOpId;
        //record->EventId = iEvent;
        //record->DeviceId = iDevice;
        //record->Details = Details;

        ////Ivio il messaggio al thread del DBManager così effettua l'inserimento
        //PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_INSERT_LOGEVENTAND, (WPARAM)record, 0);

        delete lpST;
    }
    //----------------------------------------------------------------------------

    void DBManager::LogEventOr(int idEventOrLog, int iEventOpId, int iEvent, int iDevice, std::string Details, std::string sSourceTS) {
        domain::CfmEventOrLog_Table* record = new domain::CfmEventOrLog_Table();

        LPSYSTEMTIME lpST = new SYSTEMTIME();
        GetLocalTime(lpST);
        //char szLocalDate[255], szLocalTime[255], szDateTime[255];
        //GetDateFormat(LOCALE_USER_DEFAULT, 0, lpST, "yyyyMMdd", szLocalDate, 255);
        //GetTimeFormat(LOCALE_USER_DEFAULT, 0, lpST, "HH:mm:ss", szLocalTime, 255);
        //sprintf_s(szDateTime, sizeof(szDateTime), "%s %s.%i", szLocalDate, szLocalTime, lpST->wMilliseconds);

        //record->IdEventOrLog = idEventOrLog;
        //record->KernelTS = std::string(szDateTime);
        //record->SourceTS = sSourceTS;
        //record->IdEventOr = iEventOpId;
        //record->EventId = iEvent;
        //record->DeviceId = iDevice;
        //record->Details = Details;

        ////Ivio il messaggio al thread del DBManager così effettua l'inserimento
        //PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_INSERT_LOGEVENTOR, (WPARAM)record, 0);

        delete lpST;
    }
    //----------------------------------------------------------------------------

    void DBManager::DeleteLogEventAnd(int EventOpId, int EventId, int DeviceId) {
        char* command = new char[255];

        sprintf_s(command, 255, "DELETE FROM EventAndLog WHERE IdEventAndLog= ( SELECT MAX(IdEventAndLog) "
            "FROM EventAndLog WHERE IdEventAnd='%d' AND DeviceId='%d' AND EventId='%d');",
            EventOpId, DeviceId, EventId);

        //Ivio il messaggio al thread del DBManager così effettua l'inserimento
        PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_EXECUTE_OPERATION, (WPARAM)command, 0);
    }
    //----------------------------------------------------------------------------

    void DBManager::DeleteLogEventAnd(int EventOpId, std::string sTS) {
        char* command = new char[255];

        sprintf_s(command, 255, "DELETE FROM EventAndLog WHERE IdEventAnd = '%ld' AND SourceTS >= '%s';", EventOpId, sTS.c_str());

        //Ivio il messaggio al thread del DBManager così effettua l'inserimento
        PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_EXECUTE_OPERATION, (WPARAM)command, 0);
    }
    //----------------------------------------------------------------------------
    void DBManager::DeleteLogEventOr(int EventOpId, int EventId, int DeviceId)
    {
        char* command = new char[255];

        sprintf_s(command, 255, "DELETE FROM EventOrLog WHERE IdEventOrLog= (SELECT MAX(IdEventOrLog) "
            "FROM EventOrLog WHERE IdEventOr='%d' AND DeviceId='%d' AND EventId='%d');",
            EventOpId, DeviceId, EventId);

        //Ivio il messaggio al thread del DBManager così effettua l'inserimento
        PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_EXECUTE_OPERATION, (WPARAM)command, 0);
    }
    //----------------------------------------------------------------------------

    void DBManager::DeleteLogEventOr(int EventOpId, std::string sTS)
    {
        char* command = new char[255];

        sprintf_s(command, 255, "DELETE FROM EventOrLog "
            "WHERE IdEventOr = '%ld' AND SourceTS == '%s';", EventOpId, sTS.c_str());

        //Ivio il messaggio al thread del DBManager così effettua l'inserimento
        PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_EXECUTE_OPERATION, (WPARAM)command, 0);
    }
    //----------------------------------------------------------------------------

    void DBManager::LogEvent(unsigned int& idEventLog, int idEvent/*std::string sEventAlias*/, int iDevice, std::string Details, std::string sSourceTS) {
        domain::CfmEventLog_Table* record = new domain::CfmEventLog_Table();

        LPSYSTEMTIME lpST = new SYSTEMTIME();
        GetLocalTime(lpST);
        //char szLocalDate[255], szLocalTime[255], szDateTime[255];
        //GetDateFormat(LOCALE_USER_DEFAULT, 0, lpST, "yyyyMMdd", szLocalDate, 255);
        //GetTimeFormat(LOCALE_USER_DEFAULT, 0, lpST, "HH:mm:ss", szLocalTime, 255);
        //sprintf_s(szDateTime, sizeof(szDateTime), "%s %s.%i", szLocalDate, szLocalTime, lpST->wMilliseconds);

        //std::map<int, SaraEvent_Table*>	 mappaEventi = getEvents();
        //record->IdEventLog = idEventLog;
        //record->EventId = idEvent;//mappaEventi[sEventAlias]->Id;
        //record->KernelTS = std::string(szDateTime);
        //record->SourceTS = sSourceTS;
        //record->DeviceId = iDevice;
        //record->Details = Details;

        ////Ivio il messaggio al thread del DBManager così effettua l'inserimento
        //PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_INSERT_LOGEVENT, (WPARAM)record, 0);

        delete lpST;
    }
    //----------------------------------------------------------------------------

    void DBManager::LogCommand(long iDevice, int iUser, int iAction, std::string sMessage) {
        domain::CfmCommandLog_Table* record = new domain::CfmCommandLog_Table();

        LPSYSTEMTIME lpST = new SYSTEMTIME();
        GetLocalTime(lpST);
        char szLocalDate[255], szLocalTime[255], szDateTime[255];
        //GetDateFormat(LOCALE_USER_DEFAULT, 0, lpST, "yyyyMMdd", szLocalDate, 255);
        //GetTimeFormat(LOCALE_USER_DEFAULT, 0, lpST, "HH:mm:ss", szLocalTime, 255);
        //sprintf_s(szDateTime, sizeof(szDateTime), "%s %s.%i", szLocalDate, szLocalTime, lpST->wMilliseconds);
        delete lpST;

        record->ActionId = iAction;
        record->DeviceId = iDevice;
        record->UserId = iUser;
        record->Message = sMessage;
        record->SourceTS = std::string(szDateTime);

        //Ivio il messaggio al thread del DBManager così effettua l'inserimento
        PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_INSERT_LOGCOMMAND, (WPARAM)record, 0);
    }
    //----------------------------------------------------------------------------

    void DBManager::SaveEventAndLog(domain::CfmEventAndLog_Table* record) {
        //if (-1 == ExecuteQueryOnAsyncConnection("delete from EventAndLog where IdEventAndLog = " + IntToStr(record->IdEventAndLog)))
        //{
        //    delete record;
        //    return;
        //}
        //std::string query = "insert into EventAndLog(IdEventAndLog, KernelTS, DatabaseTS, SourceTS, EventId, DeviceId, EventAndRulesId, Details) values(";
        //query += IntToStr(record->IdEventAndLog) + ",";
        //query += "'" + record->KernelTS + "',";
        //query += "GETDATE(),";
        //query += "'" + record->SourceTS + "',";
        //query += IntToStr(record->EventId) + ",";
        //query += IntToStr(record->DeviceId) + ",";
        //query += IntToStr(record->IdEventAnd) + ",";
        //query += "'" + record->Details + "')";

        //delete record;

        //ASYNCQUERY_ENTER(query)

        //    QUERY_EXIT(query)
    }
    //----------------------------------------------------------------------------

    void DBManager::SaveEventOrLog(domain::CfmEventOrLog_Table* record) {
        //if (-1 == ExecuteQueryOnAsyncConnection("delete from EventOrLog where IdEventOrLog = " + IntToStr(record->IdEventOrLog)))
        //{
        //    delete record;
        //    return;
        //}
        //std::string query = "insert into EventOrLog(IdEventOrLog, KernelTS, DatabaseTS, SourceTS, EventId, DeviceId, EventOrRulesId, Details) values(";
        //query += IntToStr(record->IdEventOrLog) + ",";
        //query += "'" + record->KernelTS + "',";
        //query += "GETDATE(),";
        //query += "'" + record->SourceTS + "',";
        //query += IntToStr(record->EventId) + ",";
        //query += IntToStr(record->DeviceId) + ",";
        //query += IntToStr(record->IdEventOr) + ",";
        //query += "'" + record->Details + "')";

        //delete record;

        //ASYNCQUERY_ENTER(query)

        //    QUERY_EXIT(query)
    }
    //----------------------------------------------------------------------------

    void DBManager::SaveEventLog(domain::CfmEventLog_Table* record) {
        //if (-1 == ExecuteQueryOnAsyncConnection("delete from EventLog where IdEventLog = " + IntToStr(record->IdEventLog)))
        //{
        //    delete record;
        //    return;
        //}
        //std::string query = "insert into EventLog(IdEventLog, KernelTS, DatabaseTS, SourceTS, DeviceId, EventId, Details) values(";
        //query += IntToStr(record->IdEventLog) + ",";
        //query += "'" + record->KernelTS + "',";
        //query += "GETDATE(),";
        //query += "'" + record->SourceTS + "',";
        //query += IntToStr(record->DeviceId) + ",";
        //query += IntToStr(record->EventId) + ",";
        //query += "'" + record->Details + "')";

        //delete record;

        //ASYNCQUERY_ENTER(query)

        //    QUERY_EXIT(query)
    }
    //----------------------------------------------------------------------------

    unsigned int DBManager::GetMaxEventLogId(std::string NameTableEvent, std::string NameIdEvent, std::string DatetimeTS) {
        unsigned int ret = 0L;
        //std::string query = ("SELECT MAX(Ev." + NameIdEvent + ") FROM " + NameTableEvent + " AS Ev "
        //    "  WHERE " + DatetimeTS + " in ( SELECT MAX(" + DatetimeTS + ") FROM " + NameTableEvent + ")");

        //QUERY_ENTER(query, 0);

        //if (!aStatement.fetch_next())
        //    return 0;

        //ret = aStatement.field(1).as_unsigned_long();

        //// Per prevenire un eventuale riavvio del SARA durante il passaggio all'ora solare
        //ret += (ret) ? 1000 : 0;

        //QUERY_EXIT(query)

        return ret;
    }
    //----------------------------------------------------------------------------

    unsigned int DBManager::GetMaxEventLogId() {
        return GetMaxEventLogId("EventLog", "IdEventLog", "DatabaseTS");
    }
    //----------------------------------------------------------------------------

    // Aggiunta per la gestione del contatore della tabella EventOrLog
    //Per l'inizializzazione del contatore
    unsigned int DBManager::GetMaxEventOrLogId() {
        // OSC 17/11/2010
        return GetMaxEventLogId("EventOrLog", "IdEventOrLog", "DatabaseTS");

    }
    //----------------------------------------------------------------------------

    // Aggiunta per la gestione del contatore della tabella EventAndLog
    unsigned int DBManager::GetMaxEventAndLogId() {
        // OSC 17/11/2010
        return GetMaxEventLogId("EventAndLog", "IdEventAndLog", "DatabaseTS");
    }
    //----------------------------------------------------------------------------

    unsigned int DBManager::GetMaxRuleLogId() {
        // OSC 17/11/2010
        return GetMaxEventLogId("RuleLog", "IdRuleLog", "DatabaseTS");
        /*
            std::string query("select max(IdRuleLog), count(IdRuleLog) from RuleLog");

            QUERY_ENTER(query,0);

            if (!aStatement.fetch_next())
                return 0;

            if (1 != aStatement.field(2).as_unsigned_long())
                return 0;

            return (aStatement.field(1).as_unsigned_long());

            QUERY_EXIT(query)

            return 0;
        */
    }
    //----------------------------------------------------------------------------

    unsigned int DBManager::GetMaxActionLogId() {
        // OSC 17/11/2010
        return GetMaxEventLogId("ActionLog", "IdActionLog", "DatabaseTS");
        /*
            std::string query("select max(IdActionLog), count(IDActionLog) from ActionLog");

            QUERY_ENTER(query,0);

            if (!aStatement.fetch_next())
                return 0;

            if (1 != aStatement.field(2).as_unsigned_long())
                return 0;

            return (aStatement.field(1).as_unsigned_long());

            QUERY_EXIT(query)

            return 0;
        */
    }
    //----------------------------------------------------------------------------

    void DBManager::LogAction(int idActionLog, std::string sKernelTS, int idAction, int idRuleLog) {
        domain::CfmActionLog_Table* record = new domain::CfmActionLog_Table();

        LPSYSTEMTIME lpST = new SYSTEMTIME();
        GetLocalTime(lpST);
        char szLocalDate[255], szLocalTime[255], szDateTime[255];
        //GetDateFormat(LOCALE_USER_DEFAULT, 0, lpST, "yyyyMMdd", szLocalDate, 255);
        //GetTimeFormat(LOCALE_USER_DEFAULT, 0, lpST, "HH:mm:ss", szLocalTime, 255);
        //sprintf_s(szDateTime, sizeof(szDateTime), "%s %s.%i", szLocalDate, szLocalTime, lpST->wMilliseconds);

        record->IdActionLog = idActionLog;
        record->KernelTS = std::string(szDateTime);
        record->IdActionRule = idAction;
        record->IdRuleLog = idRuleLog;
        //record->ExecutionTime = iExecutionTime;
        //record->ExecutedTS = sExecutedTS;

        //Ivio il messaggio al thread del DBManager così effettua l'inserimento
        PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_INSERT_LOGACTION, (WPARAM)record, 0);

        delete lpST;
    }
    //----------------------------------------------------------------------------

    void DBManager::SaveActionLog(domain::CfmActionLog_Table* record) {
        //if (-1 == ExecuteQueryOnAsyncConnection("delete from ActionLog where IdActionLog = " + IntToStr(record->IdActionLog)))
        //{
        //    delete record;
        //    return;
        //}
        //std::string query = "insert into ActionLog(IdActionLog, StartTS, DatabaseTS, IdActionRule, IdRuleLog, ExecutedTS, ExecutionTime) values(";
        //query += IntToStr(record->IdActionLog) + ",";
        //query += "'" + record->KernelTS + "',";
        //query += "GETDATE(),";
        //query += IntToStr(record->IdActionRule) + ",";
        //query += IntToStr(record->IdRuleLog) + ",";
        //query += "'20070101 00:00:00.000',-1)";

        //delete record;

        //ASYNCQUERY_ENTER(query)

        //    QUERY_EXIT(query)
    }
    //----------------------------------------------------------------------------

    void DBManager::LogRule(int idRuleLog, std::string sStartTS, int idRule, int idEventLog) {
        domain::CfmRuleLog_Table* record = new domain::CfmRuleLog_Table();

        record->ExecutionTime = -1;
        record->ExecutedTS = "20070101 00:00:00.000";
        record->IdEventLog = idEventLog;
        record->IdRuleLog = idRuleLog;
        record->IdRule = idRule;
        record->StartTS = sStartTS;
        record->Succeded = false;

        //Ivio il messaggio al thread del DBManager così effettua l'inserimento
        PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_INSERT_LOGRULE, (WPARAM)record, 0);
    }
    //----------------------------------------------------------------------------

    void DBManager::SaveRuleLog(domain::CfmRuleLog_Table* record) {
        //if (-1 == ExecuteQueryOnAsyncConnection("delete from RuleLog where IdRuleLog = " + IntToStr(record->IdRuleLog)))
        //{
        //    delete record;
        //    return;
        //}
        //std::string query = "insert into RuleLog(IdRuleLog, StartTS, DatabaseTS, RuleId, IdEventLog, ExecutedTS, Succeded, ExecutionTime) values(";
        //query += IntToStr(record->IdRuleLog) + ",";
        //query += "'" + record->StartTS + "',";
        //query += "GETDATE(),";
        //query += IntToStr(record->IdRule) + ",";
        //query += IntToStr(record->IdEventLog) + ",";
        //query += "'" + record->ExecutedTS + "',";
        //query += IntToStr(record->Succeded) + ",";
        //query += IntToStr(record->ExecutionTime) + ")";

        //delete record;

        //ASYNCQUERY_ENTER(query)

        //    QUERY_EXIT(query)
    }
    //----------------------------------------------------------------------------

    void DBManager::SaveCommandLog(domain::CfmCommandLog_Table* record) {
        //record->Message = ValidateStringDB(record->Message);

        //std::string query = "INSERT INTO CommandLog(UserId, ActionId, DeviceId, StartTS, DatabaseTS, Message) values(";
        //query += IntToStr(record->UserId) + ",";
        //query += IntToStr(record->ActionId) + ",";
        //query += IntToStr(record->DeviceId) + ",";
        //query += "'" + record->SourceTS + "',";
        //query += "GETDATE(),";
        //query += "'" + record->Message + "')";

        //delete record;

        //ASYNCQUERY_ENTER(query)

        //    QUERY_EXIT(query)
    }
    //----------------------------------------------------------------------------

    void DBManager::UpdateAction(int idActionLog, int iExecutionTime, std::string sExecutedTS)
    {
        domain::CfmActionLog_Table* record = new domain::CfmActionLog_Table();

        record->IdActionLog = idActionLog;
        record->ExecutionTime = iExecutionTime;
        record->ExecutedTS = sExecutedTS;

        //Ivio il messaggio al thread del DBManager così effettua l'inserimento
        PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_UPDATE_LOGACTION, (WPARAM)record, 0);
    }
    //----------------------------------------------------------------------------

    void DBManager::UpdateActionLog(domain::CfmActionLog_Table* record) {
        //std::string query = "UPDATE ActionLog SET ExecutionTime = " + IntToStr(record->ExecutionTime)
        //    + ", ExecutedTS = '" + record->ExecutedTS
        //    + "' WHERE IdActionLog = " + IntToStr(record->IdActionLog);

        //delete record;

        //ASYNCQUERY_ENTER(query)

        //    QUERY_EXIT(query)
    }
    //----------------------------------------------------------------------------

    void DBManager::UpdateRule(int idRuleLog, std::string ExecutedTS, int ExecutionTime, bool Succeded) {
        domain::CfmRuleLog_Table* record = new domain::CfmRuleLog_Table();

        record->ExecutionTime = ExecutionTime;
        record->ExecutedTS = ExecutedTS;
        record->IdRuleLog = idRuleLog;
        record->Succeded = Succeded;

        //Ivio il messaggio al thread del DBManager così effettua l'inserimento
        PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_UPDATE_LOGRULE, (WPARAM)record, 0);
    }
    //----------------------------------------------------------------------------

    void DBManager::UpdateRuleLog(domain::CfmRuleLog_Table* record) {
 /*       std::string query = "UPDATE RuleLog SET ExecutionTime = "
            + IntToStr(record->ExecutionTime)
            + ", ExecutedTS = '"
            + record->ExecutedTS
            + "', Succeded = "
            + IntToStr(record->Succeded)
            + " WHERE IdRuleLog = "
            + IntToStr(record->IdRuleLog);

        delete record;

        ASYNCQUERY_ENTER(query)

            QUERY_EXIT(query)*/
    }
    //----------------------------------------------------------------------------

    std::map<int, domain::CfmActionParameters_Table*> DBManager::getActionParameters() {
        std::map<int, domain::CfmActionParameters_Table*>  mappaParametriAzioni;
        //std::string query("select Id, ActionId, Alias, Type from ActionParameters");

        //QUERY_ENTER(query, mappaParametriAzioni)

        //    while (aStatement.fetch_next())
        //    {
        //        SaraActionParameters_Table* p = new SaraActionParameters_Table();

        //        p->Id = aStatement.field(1).as_unsigned_long();
        //        p->IdAction = aStatement.field(2).as_unsigned_long();
        //        p->Alias = aStatement.field(3).as_string();
        //        p->Type = aStatement.field(4).as_string();

        //        mappaParametriAzioni[p->Id] = p;
        //    }
        //QUERY_EXIT(query)

            return mappaParametriAzioni;
    }
    //----------------------------------------------------------------------------

    //Metodo asincrono per la memorizzazione del nuovo stato del sotto sistema
    void DBManager::UpdateSystemStatus(int SIDUID, int Status) {
        domain::CfmSystem_Table* record = new domain::CfmSystem_Table();
        record->Id = SIDUID;
        record->Status = Status;

        PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_UPDATE_SYSTEMSTATUS, (WPARAM)record, 0);
    }
    //----------------------------------------------------------------------------

    void DBManager::SaveSystemStatus(domain::CfmSystem_Table* record) {
        //std::string query = "UPDATE System SET StateId = "
        //    + IntToStr(record->Status)
        //    + " WHERE Id = "
        //    + IntToStr(record->Id);

        //delete record;

        //ASYNCQUERY_ENTER(query)

        //    QUERY_EXIT(query)
    }
    //----------------------------------------------------------------------------

    void DBManager::SaveSystemMaintenanceReq(domain::CfmSystem_Table* record) {
        //std::string query = "UPDATE System SET MaintenanceReq = "
        //    + IntToStr(record->MaintenanceReq)
        //    + " WHERE Id = "
        //    + IntToStr(record->Id);

        //delete record;

        //ASYNCQUERY_ENTER(query)

        //    QUERY_EXIT(query)
    }
    //----------------------------------------------------------------------------

    void DBManager::UpdatePhisicalDeviceStatus(int iDeviceId, short int state, std::string datetime, short int processState) {
        domain::CfmDevices_Table* record = new domain::CfmDevices_Table();

        record->Id = iDeviceId;
        record->PhisicalStatus = state;
        record->ProcessState = processState;
      /*  if (datetime.empty())
            record->sUpdatedTS = GetLocalTime();
        else
            record->sUpdatedTS = datetime;*/

        //Pekmez record->sUpdatedUTCTS = GetUTCTimeByLocalTime(record->sUpdatedTS);

        //Ivio il messaggio al thread del DBManager così effettua l'update
        PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_UPDATE_PHISICALDEVICESTATUS, (WPARAM)record, 0);
    }
    //----------------------------------------------------------------------------

    void DBManager::UpdateLogicalDeviceStatus(int iDeviceId, short int state, std::string datetime) {
        domain::CfmDevices_Table* record = new domain::CfmDevices_Table();

        record->Id = iDeviceId;
        record->LogicalStatus = (state) ? true : false;
        /*if (datetime.empty())
            record->sUpdatedTS = GetLocalTime();
        else
            record->sUpdatedTS = datetime;*/

        //Pekmez record->sUpdatedUTCTS = GetUTCTimeByLocalTime(record->sUpdatedTS);

        //Ivio il messaggio al thread del DBManager così effettua l'update
        PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_UPDATE_LOGICALDEVICESTATUS, (WPARAM)record, 0);
    }
    //----------------------------------------------------------------------------

    void DBManager::UpdateDeleteDeviceStatus(int iDeviceId, domain::stateRecDelete state) {
        domain::CfmDevices_Table* record = new domain::CfmDevices_Table();
        record->Id = iDeviceId;
        record->bDeleted = state;
        //Pekmez record->sModifiedTS = GetLocalTime();

        //Ivio il messaggio al thread del DBManager così effettua l'update
        PostThreadMessage(this->m_ThreadCtx.m_dwTID, DB_UPDATE_DELETEDEVICE, (WPARAM)record, 0);
    }
    //----------------------------------------------------------------------------

    void DBManager::SavePhisicalDeviceStatus(domain::CfmDevices_Table* record) {
        //std::string query = "UPDATE Devices SET PhisicalStatus = " + IntToStr(record->PhisicalStatus)
        //    + ", ProcessState = '" + IntToStr(record->ProcessState) + "'"
        //    + ", Updated = '" + record->sUpdatedTS + "'"
        //    + ", UpdatedUTC = '" + record->sUpdatedUTCTS + "'"
        //    + " WHERE Id = " + IntToStr(record->Id);

        //delete record;

        //ASYNCQUERY_ENTER(query)

        //    QUERY_EXIT(query)
    }
    //----------------------------------------------------------------------------

    //Imposta lo stato logico di un punto
    void DBManager::SaveLogicalDeviceStatus(domain::CfmDevices_Table* record) {
        //std::string query = "UPDATE Devices SET LogicalStatus = " + IntToStr(record->LogicalStatus)
        //    + ", Updated = '" + record->sUpdatedTS + "'"
        //    + ", UpdatedUTC = '" + record->sUpdatedUTCTS + "'"
        //    + " WHERE Id = " + IntToStr(record->Id);

        //delete record;

        //ASYNCQUERY_ENTER(query)

        //    QUERY_EXIT(query)
    }
    //----------------------------------------------------------------------------

    void DBManager::SaveDeleteDeviceStatus(domain::CfmDevices_Table* record) {
        //std::string query = "UPDATE Devices SET Deleted = " + std::string((record->bDeleted == recDeleted) ? "1" : "0")
        //    + ", Modified = '" + record->sModifiedTS
        //    + "' WHERE Id = " + IntToStr(record->Id);

        //delete record;

        //ASYNCQUERY_ENTER(query)

        //    QUERY_EXIT(query)
    }
    //----------------------------------------------------------------------------

    //Cancella tutte le righe obsolete dalle tabelle Tracks_Log e OCR_Log, EventLog
    void DBManager::EraseOldRows() {
        //if (-1 == ExecuteQueryOnAsyncConnection("delete from Tracks_Log where TimeStamp < dateadd(dd,-" + IntToStr(cfmRegCfg.MAX_EVENT_DAYS()) + ",getdate())"))
        //    return;

        //if (-1 == ExecuteQueryOnAsyncConnection("delete from OCR_Log where DatabaseTS < dateadd(dd,-" + IntToStr(cfmRegCfg.MAX_EVENT_DAYS()) + ",getdate())"))
        //    return;

        ////if (-1 == ExecuteQueryOnAsyncConnection("delete from ActionLog where DatabaseTS < dateadd(dd,-" + IntToStr(cfmRegCfg.MAX_EVENT_DAYS()) + ",getdate())"))
        ////    return;

        ////if (-1 == ExecuteQueryOnAsyncConnection("delete from RuleLog where DatabaseTS < dateadd(dd,-" + IntToStr(saraRegCfg.MAX_EVENT_DAYS()) + ",getdate())"))
        ////    return;

        //if (-1 == ExecuteQueryOnAsyncConnection("delete from LogCommand where DatabaseTS < dateadd(dd,-" + IntToStr(cfmRegCfg.MAX_EVENT_DAYS()) + ",getdate())"))
        //    return;

        //if (-1 == ExecuteQueryOnAsyncConnection("delete from EventLog where DatabaseTS < dateadd(dd,-" + IntToStr(cfmRegCfg.MAX_EVENT_DAYS()) + ",getdate())"))
        //    return;

        //if (-1 == ExecuteQueryOnAsyncConnection("delete from EventAndLog where DatabaseTS < dateadd(dd,-" + IntToStr(cfmRegCfg.MAX_EVENT_DAYS()) + ",getdate())"))
        //    return;

        //if (-1 == ExecuteQueryOnAsyncConnection("delete from EventOrLog where DatabaseTS < dateadd(dd,-" + IntToStr(cfmRegCfg.MAX_EVENT_DAYS()) + ",getdate())"))
        //    return;
    }
    //----------------------------------------------------------------------------

    void DBManager::EraseOldRowsRequest() {
        PostThreadMessage(this->GetThreadId(), DB_ERASE_OLD_ROWS, 0, 0);
    }
    //----------------------------------------------------------------------------

    //Nuova per cancellare le righe da una tabella in cui un ts è obsoleto
    //Cancella tutte le devices
    int DBManager::DeleteTable(std::string sTableName, std::string TSFieldName, int days)
    {
        //std::string query = "delete from " + sTableName + " where " + TSFieldName + " < dateadd(dd,-" + IntToStr(days) + ",getdate())";

        //QUERY_ENTER(query, -1)

        //    QUERY_EXIT(query)

        //    return (r) ? 0 : -1;
        return 0;
    }
    //----------------------------------------------------------------------------

    unsigned long DBManager::getULongSyncQuery(std::string query) {
        //QUERY_ENTER(query, 0);

        //if (aStatement.fetch_next())
        //    return aStatement.field(1).as_unsigned_long();

        //QUERY_EXIT(query)
            return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::ExecuteSyncQuery(std::string query, bool bMutex) {
        //if (bMutex)
        //{
        //    LockedMutex lm(mutex);
        //    return ExecuteSyncQueryNoMutex(query);
        //}
        //else
        //    return ExecuteSyncQueryNoMutex(query);
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::ExecuteSyncQueryNoMutex(std::string query) {
        bool r = true;

        if (!IsConnected() || !syncConn->connected())
            return -1;

        //try
        //{
        //    tiodbc::statement aStatement;

        //    if (!aStatement.execute_direct(*syncConn, query))
        //    {
        //        std::string message, code;
        //        ManageSQLError(message, code, aStatement.last_error_status_code(message, code), query);
        //        return false;
        //    }
        //    QUERY_EXIT(query)

        //        return (r) ? 0 : -1;
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::ExecuteQueryOnAsyncConnection(std::string query) {
            bool r = true;

            if (!IsConnected() || !asyncConn->connected())
                return -1;

            //try
            //{
            //    tiodbc::statement aStatement;

            //    if (!aStatement.execute_direct(*asyncConn, query))
            //    {
            //        std::string message, code;
            //        ManageSQLError(message, code, aStatement.last_error_status_code(message, code), query);
            //        return -1;
            //    }
            //    QUERY_EXIT(query)

            //        return (r) ? 0 : -1;
            return 0;
    }
    //----------------------------------------------------------------------------

    void DBManager::DbClean() {
                PostThreadMessage(GetThreadId(), DB_CLEAN, 0, 0);
    }
    //----------------------------------------------------------------------------

    bool DBManager::IsConnected() {
        return (state == STATE_CONNECTED);
    }
    //----------------------------------------------------------------------------
    VOID CALLBACK DBManager::MyTimerProc(
        HWND hwnd,        // handle to window for timer messages 
        UINT message,     // WM_TIMER message 
        UINT_PTR idTimer,     // timer identifier 
        DWORD dwTime)     // current system time 
    {
        PostThreadMessage(DBManager::getInstance()->GetThreadId(), DB_CONNECT_TIMEOUT, 0, 0);
    }
    //----------------------------------------------------------------------------

    VOID CALLBACK DBManager::TimerMinute(
        HWND hwnd,        // handle to window for timer messages 
        UINT message,     // WM_TIMER message 
        UINT_PTR idTimer,     // timer identifier 
        DWORD dwTime)     // current system time 
    {
        PostThreadMessage(DBManager::getInstance()->GetThreadId(), DB_TIMER, 0, 0);
    }
    //----------------------------------------------------------------------------

    int DBManager::SetSiteForDevice(int iId, long iIdSite) {
        //std::string query = "UPDATE devices SET SiteId=" + IntToStr(iIdSite) + " WHERE id=" + IntToStr(iId);

        //return ExecuteSyncQuery(query);
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::SetPresetForCamera(unsigned long DeviceId, std::string presetName, int numPreset, std::string SubId) {
        //SQLINTEGER iIdMax = 0;
        //std::string query;
        //bool update = true;

        //query = "SELECT Id FROM Presets "
        //    "WHERE PresetNumber=" + IntToStr(numPreset) +
        //    " AND DeviceId=" + IntToStr(DeviceId) +
        //    " AND SubId='" + SubId + "'";

        //// Verifico se il record del preset è presente in archivio
        //QUERY_ENTER(query, -1)

        //    if (aStatement.fetch_next())
        //        iIdMax = aStatement.field(1).as_long(iIdMax);

        //QUERY_EXIT(query)

        //    if (!r)
        //        return -1;

        //if (iIdMax <= 0)
        //{
        //    update = false;
        //    // Preset non trovato, calcola Id massimo
        //    query = "SELECT MAX(Id) AS idmax from Presets";

        //    QUERY_ENTER(query, -1)

        //        if (!aStatement.fetch_next())
        //            return -1;

        //    iIdMax = aStatement.field(1).as_long(iIdMax) + 1;

        //    QUERY_EXIT(query)

        //        if (!r)
        //            return -1;
        //}
        //presetName = ValidateStringDB(presetName);

        //if (update)
        //    // Esegue l'update dei campi
        //    query = "UPDATE Presets SET "
        //    "  Name='" + presetName + "'"
        //    "WHERE Id=" + IntToStr(iIdMax);
        //else
        //    // Esegue l'update dei campi
        //    query = "INSERT INTO Presets VALUES( " + IntToStr(iIdMax) + ", '" + presetName +
        //    "', " + IntToStr(numPreset) + ", " + IntToStr(DeviceId) + ", '" + SubId + "')";

        //return ExecuteSyncQuery(query);
        return 0;
    }
    //----------------------------------------------------------------------------
    int DBManager::GetPresetForCamera(unsigned long DeviceId, std::string PresetSubId) {
        //std::stringstream query(std::stringstream::in | std::stringstream::out);
        //query << "SELECT Id from Presets WHERE DeviceId=" << DeviceId;
        //query << " AND SubId='" << PresetSubId;
        //query << "'";

        //QUERY_ENTER(query.str(), -1)

        //    if (aStatement.fetch_next())
        //        return aStatement.field(1).as_unsigned_long();

        //QUERY_EXIT(query.str())

            return -1;
    }
    //----------------------------------------------------------------------------

    int DBManager::SetAttributeForDevice(int iId, std::string AttributeName, std::string AttributeValue) {
        //int id = 0;
        //AttributeName = ValidateStringDB(AttributeName);
        //AttributeValue = ValidateStringDB(AttributeValue);

        //std::stringstream query(std::stringstream::in | std::stringstream::out);
        //query << "select CA.Id from CategoriesAttributes as CA where CA.Name = '";
        //query << AttributeName;
        //query << "'";

        //QUERY_ENTER(query.str(), -1)

        //    if (!aStatement.fetch_next())
        //        return -1;

        //id = aStatement.field(1).as_unsigned_long();

        //query.str("");
        //query << "SELECT DevicesId, AValue FROM DevicesCategoriesAttributes WHERE DevicesId = " << iId;
        //query << " AND CategoriesAttributesId = " << id;

        //if (!aStatement.execute_direct(*syncConn, query.str()))
        //{
        //    std::string message, code;
        //    ManageSQLError(message, code, aStatement.last_error_status_code(message, code), query.str());
        //    return -1;
        //}
        //query.str("");
        //if (aStatement.fetch_next())
        //{
        //    std::string strValue = aStatement.field(2).as_string();
        //    if (strValue != AttributeValue)
        //    {
        //        query << "update DevicesCategoriesAttributes set AValue='" << AttributeValue << "' ";
        //        query << "where DevicesId = " << iId;
        //        query << " AND CategoriesAttributesId = " << id;
        //    }
        //}
        //else
        //{
        //    query << "insert into DevicesCategoriesAttributes(DevicesId,CategoriesAttributesId,AValue) ";
        //    query << "values( " << iId << ", " << id << ", '" << AttributeValue << "')";
        //}
        //if (!query.str().empty())
        //{
        //    if (!aStatement.execute_direct(*syncConn, query.str()))
        //    {
        //        std::string message, code;
        //        ManageSQLError(message, code, aStatement.last_error_status_code(message, code), query.str());
        //        return -1;
        //    }
        //}
        //else
        //    ;

        //QUERY_EXIT(query.str())

        //    return (r) ? 0 : -1;
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::GetRegionFromSIDUID(int SIDUID) {
        //std::map<int, int>::iterator it;
        //if ((it = mapSIDUIDRegion.find(SIDUID)) != mapSIDUIDRegion.end())
        //    return it->second;

        //std::stringstream query(std::stringstream::in | std::stringstream::out);
        //query << "select System.SARARegionId from System where System.Id=" << SIDUID;

        //QUERY_ENTER(query.str(), 0)

        //    if (aStatement.fetch_next())
        //    {
        //        int id = aStatement.field(1).as_unsigned_long();
        //        mapSIDUIDRegion[SIDUID] = id;

        //        return id;
        //    }
        //QUERY_EXIT(query.str())

            return 0;
    }
    //----------------------------------------------------------------------------

    std::vector<domain::CfmCategoriesAttributes_Table> DBManager::GetDeviceParameters(int iId)
    {
        std::stringstream query(std::stringstream::in | std::stringstream::out);
        query << "select CA.Id, CA.Name, CA.Description, DCA.AValue ";
        query << "FROM CategoriesAttributes as CA join DevicesCategoriesAttributes as DCA ";
        query << "on CA.Id = DCA.CategoriesAttributesId ";
        query << "where DCA.DevicesId = " << iId;

        std::vector<domain::CfmCategoriesAttributes_Table> v;

        //QUERY_ENTER(query.str(), v)

        //    while (aStatement.fetch_next())
        //    {
        //        SaraCategoriesAttributes_Table s;

        //        s.Id = aStatement.field(1).as_unsigned_long();
        //        s.Name = aStatement.field(2).as_string();
        //        s.Description = aStatement.field(3).as_string();
        //        s.Value = aStatement.field(4).as_string();

        //        v.push_back(s);
        //    }
        //QUERY_EXIT(query.str())

            return v;
    }
    //----------------------------------------------------------------------------

    std::map<std::string, domain::CfmDevicesCategoriesAttributes_Table> DBManager::GetDevicesCategoriesAttributes()
    {
        std::stringstream query(std::stringstream::in | std::stringstream::out);
        query << " select CA.Name, CategoriesAttributesId, DCA.AValue, DCA.DevicesId ";
        query << " FROM DevicesCategoriesAttributes as DCA join CategoriesAttributes as CA ";
        query << "	on DCA.CategoriesAttributesId = CA.Id ";

        std::map<std::string, domain::CfmDevicesCategoriesAttributes_Table> v;

        //QUERY_ENTER(query.str(), v)

        //    while (aStatement.fetch_next())
        //    {
        //        SaraDevicesCategoriesAttributes_Table s;

        //        s.Name = aStatement.field(1).as_string();
        //        s.CategoriesAttributesId = aStatement.field(2).as_short();
        //        s.AValue = aStatement.field(3).as_string();
        //        s.DevicesId = aStatement.field(4).as_unsigned_long();

        //        std::string key = IntToStr(s.DevicesId) + "|" + s.Name;
        //        v[key] = s;
        //    }
        //QUERY_EXIT(query.str())

            return v;
    }
    //----------------------------------------------------------------------------

    domain::CfmDevices_Table* DBManager::getDevice(int idDevice) {
        std::string query = " SELECT Devices.Id, Devices.LogicalStatus, Devices.PhisicalStatus, Devices.CategoryId, Devices.Description, "
            "	Devices.SystemId, Devices.SubId, Devices.Deleted, Devices.Modified, Devices.SiteId, Devices.Updated, "
            "	Devices.ParentId, Site.Name, Devices.ProcessState, Devices.Executing, Devices.UpdatedUTC "
//            " FROM Devices, Site WHERE Devices.Id = " + IntToStr(idDevice) +
            "	AND Devices.SiteId = Site.Id";

        domain::CfmDevices_Table* sdt = NULL;

        //QUERY_ENTER(query, NULL)

        //    if (aStatement.fetch_next())
        //    {
        //        sdt = new SaraDevices_Table();

        //        sdt->Id = aStatement.field(1).as_unsigned_long();
        //        sdt->LogicalStatus = (bool)aStatement.field(2).as_short();
        //        sdt->PhisicalStatus = aStatement.field(3).as_short();
        //        sdt->catId = aStatement.field(4).as_short();
        //        sdt->Description = aStatement.field(5).as_string();
        //        sdt->SystemId = aStatement.field(6).as_unsigned_long();
        //        sdt->subId = aStatement.field(7).as_string();
        //        sdt->bDeleted = (stRecDelete)aStatement.field(8).as_unsigned_long();;
        //        sdt->sModifiedTS = aStatement.field(9).as_string();
        //        sdt->SiteId = aStatement.field(10).as_unsigned_long();
        //        sdt->sUpdatedTS = aStatement.field(11).as_string();
        //        sdt->ParentId = aStatement.field(12).as_unsigned_long();
        //        sdt->SiteName = aStatement.field(13).as_string();
        //        sdt->ProcessState = aStatement.field(14).as_short();
        //        sdt->Executing = aStatement.field(15).as_short();
        //        sdt->sUpdatedUTCTS = aStatement.field(16).as_string();
        //    }
        //QUERY_EXIT(query)

        //    if (sdt)
        //        sdt->DeviceParameters = GetDeviceParameters(sdt->Id);

        return sdt;
    }
    //----------------------------------------------------------------------------
    std::map<std::string, domain::CfmSite_Table> DBManager::getMapSiteByName() {
        std::map<std::string, domain::CfmSite_Table> mapSite;

        //std::string query("SELECT Id, Name, Description, SubSystemId, SystemId, Watched, Modified FROM Site");

        //QUERY_ENTER(query, mapSite)

        //    while (aStatement.fetch_next())
        //    {
        //        SaraSite_Table p;

        //        p.Id = aStatement.field(1).as_unsigned_long();
        //        p.Name = aStatement.field(2).as_string();
        //        p.Description = aStatement.field(3).as_string();
        //        p.SubSystemId = aStatement.field(4).as_unsigned_long();
        //        p.SystemId = aStatement.field(5).as_unsigned_long();
        //        p.SiteWatched = (bool)aStatement.field(6).as_short();
        //        p.ModifiedTS = aStatement.field(7).as_string();

        //        mapSite[p.Name] = p;
        //    }
        //QUERY_EXIT(query)

        //    // Inizio 08/11/2010
        //    // Carica la lista dei SystemId (SIDID) che gestiscono il sito
        //    std::map<std::string, SaraSite_Table>::iterator itSite = mapSite.begin();

        //for (; itSite != mapSite.end(); itSite++)
        //{
        //    query = "SELECT SystemId, SubSiteId FROM SiteSystem WHERE SiteId = " + IntToStr(itSite->second.Id);
        //    QUERY_ENTER(query, mapSite)

        //        while (aStatement.fetch_next())
        //        {
        //            SaraSiteSystem_Table p;
        //            p.SiteId = itSite->second.Id;
        //            p.SystemId = aStatement.field(1).as_unsigned_long();
        //            p.SubSiteId = aStatement.field(2).as_unsigned_long();
        //            itSite->second.mapSiteSystem[p.SystemId] = p;
        //        }
        //    QUERY_EXIT(query)
        //}
        //// Fine 08/11/2010

        return mapSite;
    }
    //----------------------------------------------------------------------------

    int DBManager::InsertSite(int SystemId, int SubSystemId, bool SiteWatched,
        std::string Name, std::string Description, std::string ModifiedTS) {

        unsigned long Id = 0;
        //if (ModifiedTS.empty())
        //    ModifiedTS = GetLocalTime();

        //Name = ValidateStringDB(Name);
        //Description = ValidateStringDB(Description);

        //std::string query = "INSERT INTO Site( Name, Description, SubSystemId, "
        //    //"		   Modified, Watched, SystemId ) OUTPUT INSERTED.Id "
        //    "		   Modified, Watched, SystemId ) "
        //    " VALUES('" + Name + "','" + Description + "'," + IntToStr(SubSystemId) +
        //    "	   , '" + ModifiedTS + "', " + IntToStr(SiteWatched) + "," + IntToStr(SystemId) + ") ";
        //if (!ExecuteSyncQuery(query))
        //{
        //    query = "SELECT Id FROM Site "
        //        " WHERE Name = '" + Name + "' "
        //        "   AND SubSystemId=" + IntToStr(SubSystemId) +
        //        "   AND SystemId=" + IntToStr(SystemId);

        //    Id = getULongSyncQuery(query);

        //    if (Id > 0)
        //    {
        //        query = "INSERT INTO SiteSystem ( SiteId, SystemId, SubSiteId ) "
        //            "VALUES(" + IntToStr(Id) + ", " + IntToStr(SystemId) + ", " + IntToStr(SubSystemId) + ")";

        //        ExecuteSyncQuery(query);
        //    }
        //}
        return Id;
    }
    //----------------------------------------------------------------------------

    int DBManager::UpdateSite(int SiteId, int SystemId, int SubSystemId, bool SiteWatched,
        std::string Name, std::string Description, std::string ModifiedTS) {

        int ret = -1;

        //if (ModifiedTS.empty())
        //    ModifiedTS = GetLocalTime();

        //std::string query = "UPDATE Site SET Name='" + Name + "'" +
        //    "	, Description='" + Description + "'" +
        //    "	, SubSystemId=" + IntToStr(SubSystemId) +
        //    "	, Watched=" + IntToStr(SiteWatched) +
        //    "	, SystemId= " + IntToStr(SystemId) +
        //    "	, Modified='" + ModifiedTS + "'" +
        //    "	WHERE Name='" + Name + "'";

        //ret = ExecuteSyncQuery(query);
        //if (ret == 0)
        //{
        //    bool found = false;

        //    query = "SELECT SiteId, SystemId, SubSiteId FROM SiteSystem  "
        //        "	WHERE SiteId = " + IntToStr(SiteId) + " AND SystemId = " + IntToStr(SystemId);

        //    QUERY_ENTER(query, ret)
        //        if (aStatement.fetch_next())
        //            found = true;
        //    QUERY_EXIT(query)

        //        if (!found)
        //        {
        //            query = "INSERT INTO SiteSystem ( SiteId, SystemId, SubSiteId ) "
        //                "VALUES(" + IntToStr(SiteId) + ", " + IntToStr(SystemId) + ", " + IntToStr(SubSystemId) + ")";
        //            ExecuteSyncQuery(query);
        //        }
        //}
        return ret;
    }
    //----------------------------------------------------------------------------

    int DBManager::DeleteSiteSystem(unsigned long SiteId, unsigned long SystemId) {
        int ret = -1;
        //std::string query = "UPDATE Site SET Modified='" + GetLocalTime() + "' " +
        //    "	WHERE Id='" + IntToStr(SiteId) + "'";

        //ret = ExecuteSyncQuery(query);

        //query = "DELETE FROM SiteSystem "
        //    "	WHERE SiteId = " + IntToStr(SiteId) + " AND SystemId = " + IntToStr(SystemId);
        //ret |= ExecuteSyncQuery(query);
        return ret;
    }
    //----------------------------------------------------------------------------

    int DBManager::InsertScenarios(std::string Name, int SiteId, int ScenariosTemplateId) {
        Name = ValidateStringDB(Name, 50);
        std::string query;
        int Id = -1;

        //query = "SELECT Id FROM Scenarios "
        //    "WHERE Name='" + Name + "' "
        //    " AND SiteId=" + IntToStr(SiteId) +
        //    " AND ScenariosTemplateId=" + IntToStr(ScenariosTemplateId);

        //Id = getULongSyncQuery(query);
        ///*
        //QUERY_ENTER(query,-1)

        //// Se esiste ritorna Id
        //if (aStatement.fetch_next())
        //    Id = aStatement.field(1).as_unsigned_long();

        //QUERY_EXIT(query)
        //*/

        //// Se non esiste inserisci lo Scenario
        //if (Id == -1)
        //{
        //    //query =	"INSERT INTO Scenarios ( Name, SiteId, ScenariosTemplateId ) OUTPUT INSERTED.Id "
        //    query = "INSERT INTO Scenarios ( Name, SiteId, ScenariosTemplateId ) "
        //        "VALUES('" + Name + "'," + IntToStr(SiteId) + "," + IntToStr(ScenariosTemplateId) + ")";

        //    ExecuteSyncQuery(query);
        //    /*
        //    QUERY_ENTER(query,-1)

        //    if (aStatement.fetch_next())
        //        Id = aStatement.field(1).as_unsigned_long();

        //    QUERY_EXIT(query)
        //    */
        //    query = "SELECT Id FROM Scenarios "
        //        " WHERE Name='" + Name + "' "
        //        "   AND SiteId= " + IntToStr(SiteId) +
        //        "   AND ScenariosTemplateId=" + IntToStr(ScenariosTemplateId);
        //    Id = getULongSyncQuery(query);
        //}
        return Id;
    }
    //----------------------------------------------------------------------------

    int DBManager::InsertScenariosDevice(int id, unsigned long deviceId, int cellPos, int presetId) {

        //std::string query;
        //unsigned long idDevice = 0;
        //int idPreset = 0;

        //query = "SELECT DeviceId, PresetId FROM ScenariosDevices "
        //    "WHERE ScenarioId=" + IntToStr(id) +
        //    " AND CellPos=" + IntToStr(cellPos);

        //QUERY_ENTER(query, -1)

        //    if (aStatement.fetch_next())
        //    {
        //        idDevice = aStatement.field(1).as_unsigned_long();
        //        idPreset = aStatement.field(2).as_long();
        //    }
        //QUERY_EXIT(query)

        //    // Non Trovato - Inseriscilo
        //    if (idDevice == 0)
        //    {
        //        if (presetId > 0)
        //            query = "INSERT INTO ScenariosDevices ( ScenarioId, CellPos, DeviceId, PresetId ) "
        //            "VALUES(" + IntToStr(id) + "," + IntToStr(cellPos) + "," +
        //            IntToStr(deviceId) + "," + IntToStr(presetId) + ")";
        //        else
        //            query = "INSERT INTO ScenariosDevices ( ScenarioId, CellPos, DeviceId, PresetId ) "
        //            "VALUES(" + IntToStr(id) + "," + IntToStr(cellPos) + "," +
        //            IntToStr(deviceId) + ",NULL)";

        //        return ExecuteSyncQuery(query);
        //    }
        //// Trovato - verifica se eseguire UPDATE
        //if (idDevice != deviceId || idPreset != presetId)
        //{
        //    query = "UPDATE ScenariosDevices SET "
        //        "  DeviceId = " + IntToStr(deviceId);

        //    if (presetId > 0)
        //        query += ", presetId = " + IntToStr(presetId);
        //    else
        //        query += ", presetId = NULL";

        //    query += " WHERE ScenarioId=" + IntToStr(id) +
        //        " AND CellPos=" + IntToStr(cellPos);

        //    return ExecuteSyncQuery(query);
        //}
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::InsertDeviceScenario(unsigned long deviceId, int scenarioId) {

        //std::string query;
        //unsigned long idDevice = 0;
        //int idScenario = 0;

        //query = "SELECT DeviceId, ScenarioId FROM DeviceScenario "
        //    "WHERE DeviceId=" + IntToStr(deviceId);

        //QUERY_ENTER(query, -1)

        //    if (aStatement.fetch_next())
        //    {
        //        idDevice = aStatement.field(1).as_unsigned_long();
        //        idScenario = aStatement.field(2).as_long();
        //    }
        //QUERY_EXIT(query)

        //    if (idDevice == 0)
        //    {
        //        query = "INSERT INTO DeviceScenario ( DeviceId, ScenarioId ) "
        //            "VALUES(" + IntToStr(deviceId) + "," + IntToStr(scenarioId) + ")";

        //        return ExecuteSyncQuery(query);
        //    }
        //// Trovato il record ma con scenarioId cambiato
        //if (idScenario != scenarioId)
        //{
        //    query = "UPDATE DeviceScenario SET "
        //        " ScenarioId=" + IntToStr(scenarioId) +
        //        " WHERE DeviceId=" + IntToStr(deviceId);
        //    return ExecuteSyncQuery(query);
        //}
        //// Trovato il record con gli stessi valori
        return 0;
    }
    //----------------------------------------------------------------------------

    unsigned long DBManager::getIdDevice(int SystemId, std::string iSubId) {
        //std::string query = "SELECT Devices.Id from Devices WHERE Devices.SystemId = " + IntToStr(SystemId)
        //    + " and Devices.SubId = '" + iSubId + "'";

        //QUERY_ENTER(query, 0)

        //    if (aStatement.fetch_next())
        //        return aStatement.field(1).as_unsigned_long();

        //QUERY_EXIT(query)

            return 0;
    }
    //----------------------------------------------------------------------------
    std::string	DBManager::ValidateStringDB(std::string strValue, int len) {
        size_t i = strValue.find_first_of('\'');
        while (i != -1)
        {
            strValue[i] = '^';
            i = strValue.find_first_of('\'');
        }
        if (len > 0 && (int)strValue.length() > len)
            strValue.resize(len);

        return strValue;
    }
    //----------------------------------------------------------------------------

    bool DBManager::UpdateSyncPhisicalDeviceStatus(int iDeviceId, short int state, std::string datetime) {
        std::string strDateTime;
        bool r = false;
        //if (datetime.empty())
        //    strDateTime = GetLocalTime();
        //else
        //    strDateTime = datetime;

        //std::string strUTCDateTime = GetUTCTimeByLocalTime(strDateTime);

        //std::string query = "UPDATE Devices SET PhisicalStatus = " + IntToStr(state) +
        //    ", Updated = '" + strDateTime + "' "
        //    ", UpdatedUTC = '" + strUTCDateTime + "' "
        //    "WHERE Id = " + IntToStr(iDeviceId);

        //QUERY_ENTER(query, false)

        //    QUERY_EXIT(query)

        return r;
    }
    //----------------------------------------------------------------------------

    bool DBManager::UpdateSyncLogicalDeviceStatus(int iDeviceId, short int state, std::string datetime) {
        bool r = false;

        std::string strDateTime;

        //if (datetime.empty())
        //    strDateTime = GetLocalTime();
        //else
        //    strDateTime = datetime;

        //std::string strUTCDateTime = GetUTCTimeByLocalTime(strDateTime);
        //std::string query = "UPDATE Devices SET LogicalStatus = " + IntToStr(state) +
        //    ", Updated = '" + strDateTime + "' "
        //    ", UpdatedUTC = '" + strUTCDateTime + "' "
        //    " WHERE Id = " + IntToStr(iDeviceId);

        //QUERY_ENTER(query, false)

        //    QUERY_EXIT(query)

            return r;
    }
    //----------------------------------------------------------------------------

    std::map<int, std::string> DBManager::getSystemStates() {
        if (!mapSystemStates.empty())
            return mapSystemStates;
        try {
            auto result = nanodbc::execute(*syncConn, NANODBC_TEXT("SELECT Id, StateAlias FROM SystemStates;"));
            while (result.next()) {
                mapSystemStates[result.get<short>(0)] = result.get<std::string>(1);
            }
        } catch (nanodbc::database_error& e) {
            ManageSQLError(std::string(e.what()), e.state(), (int)e.native(), "CONNECTION TO DB");
            return mapSystemStates;
        }

        return mapSystemStates;
    }
    //----------------------------------------------------------------------------

    std::string DBManager::StateSIDToString(int state) {
        //std::map<int, std::string>::iterator it;

        //if (mapSystemStates.empty())
        //    mapSystemStates = DBManager::getInstance()->getSystemStates();

        //it = mapSystemStates.find(state);
        //if (it != mapSystemStates.end())
        //    return it->second;

        return "";
    }
    //----------------------------------------------------------------------------

    bool DBManager::setDevicesExecuting(int SystemId) {
        std::string query("UPDATE Devices SET Executing = 0 WHERE SystemId = ");

        //query += IntToStr(SystemId);
        //QUERY_ENTER(query, false)
        //    QUERY_EXIT(query)

            return true;
    }
    //----------------------------------------------------------------------------

    unsigned long DBManager::getMaxDeviceId(int SystemId, int iCat) {
        //std::string query = "SELECT MAX(D.Id) "
        //    " FROM Devices AS D "
        //    " WHERE D.CategoryId=" + IntToStr(iCat) +
        //    "	AND D.SystemId=" + IntToStr(SystemId);

        //QUERY_ENTER(query, 0)
        //    if (aStatement.fetch_next())
        //        return aStatement.field(1).as_unsigned_long();
        //QUERY_EXIT(query)

            return 0;
    }
    //----------------------------------------------------------------------------
    /*
    short DBManager::getSiteIdFromName(std::string SiteName)
    {
        std::map<std::string, SaraSite_Table> mSite = DBManager::getInstance()->getMapSite();
        std::map<std::string, SaraSite_Table>::iterator itSite;

            for ( itSite=mSite.begin(); itSite!=mSite.end(); itSite++)
                if (itSite->second.Name == SiteName)
                    return (itSite->second.Id);
            return 0;
    }
    */
    //----------------------------------------------------------------------------

    int DBManager::DeleteDevicesBySite(int idSid, int category, long siteId) {
 /*       std::string query = "UPDATE Devices SET Deleted = 1 WHERE SystemId = " + IntToStr(idSid) +
            " AND SiteId = " + IntToStr(siteId) + " AND CategoryId = " + IntToStr(category);

        QUERY_ENTER(query, -1)

            QUERY_EXIT(query)

            return (r) ? 0 : -1;*/
        return 0;
    }
    //----------------------------------------------------------------------------

    std::map<unsigned long, std::set<unsigned long>>  DBManager::getDeviceIntrusionZone(int iSubId, long iSiteId) {

        std::map<unsigned long, std::set<unsigned long>> mapZoneDevices;

        //std::string query = "SELECT DinZ.DeviceId, DinZ.IntrusionZoneId "
        //    " FROM [DeviceIntrusionZone] AS DinZ "
        //    "	inner join Devices AS D ON (D.Id=DinZ.DeviceId) "
        //    " WHERE D.SystemId=" + IntToStr(iSubId);
        //if (iSiteId > 0)
        //    query += "	AND D.SiteId=" + IntToStr(iSiteId);

        //QUERY_ENTER(query, mapZoneDevices);

        //while (aStatement.fetch_next())
        //{
        //    unsigned long IdDevice = aStatement.field(1).as_unsigned_long();
        //    unsigned long IdDeviceZone = aStatement.field(2).as_unsigned_long();

        //    mapZoneDevices[IdDeviceZone].insert(IdDevice);
        //}
        //QUERY_EXIT(query)

            return mapZoneDevices;
    }
    //----------------------------------------------------------------------------

    bool DBManager::DeleteDeviceIntrusionZone(unsigned long IdDevice)
    {
        bool retval = false;
        //std::string query = "DELETE FROM DeviceIntrusionZone "
        //    " WHERE DeviceId=" + IntToStr(IdDevice);

        //return !ExecuteSyncQuery(query);
        return retval;
    }
    //----------------------------------------------------------------------------

    bool DBManager::InsertDeviceIntrusionZone(unsigned long IdDevice, unsigned long IdDeviceZone) {
        bool retval = false;
        //std::string query = "SELECT DeviceId, IntrusionZoneId "
        //    " FROM DeviceIntrusionZone "
        //    " WHERE DeviceId=" + IntToStr(IdDevice) +
        //    "	AND IntrusionZoneId=" + IntToStr(IdDeviceZone);

        //QUERY_ENTER(query, retval)

        //    // Se esiste ritorna un record
        //    if (aStatement.fetch_next())
        //        retval = true;

        //QUERY_EXIT(query)

        //    if (!retval)
        //    {
        //        query = "INSERT INTO DeviceIntrusionZone ( DeviceId, IntrusionZoneId ) "
        //            " VALUES(" + IntToStr(IdDevice) + "," + IntToStr(IdDeviceZone) + ")";

        //        QUERY_ENTER(query, retval)
        //            retval = true;
        //        QUERY_EXIT(query)
        //    }

        return retval;
    }
    //----------------------------------------------------------------------------

    int	DBManager::getPersonSubId(domain::CfmPerson_Table& person) {

        return getsTableSubId("PersonSubId", "IdPerson", person.Id, person.mapSubId);
    }
    //----------------------------------------------------------------------------

    int	DBManager::updatePersonSubId(domain::CfmPerson_Table& person) {
        std::string query, strSidId;
        std::map<unsigned int, std::string>::iterator itSubId;
        return updateTableSubId("PersonSubId", "IdPerson", person.Id, person.mapSubId);

    }
    //----------------------------------------------------------------------------

    int	DBManager::deletePersonSubId(domain::CfmPerson_Table& person, unsigned int SidId) {
        //std::string query;
        //std::map<unsigned int, std::string>::iterator itSubId;

        //query = "DELETE FROM PersonSubId "
        //    " WHERE IdPerson=" + IntToStr(person.Id) +
        //    "   AND SystemId IN (" + IntToStr(SidId) + ")";

        //return ExecuteSyncQuery(query);
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::deletePersonById(unsigned long id) {
        //std::string query;

        //query = "DELETE FROM Person "
        //    " WHERE Id=" + IntToStr(id);
        //return ExecuteSyncQuery(query);
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::updatePerson(domain::CfmPerson_Table& person) {
        //bool update = false;
        //std::string query = "SELECT Id "
        //    " FROM [Person] "
        //    " WHERE Id=" + IntToStr(person.Id);

        //QUERY_ENTER(query, -1);

        //if (aStatement.fetch_next())
        //    update = true;

        //QUERY_EXIT(query)

        //    if (update)
        //    {
        //        query = "UPDATE Person "
        //            " SET FirstName	= '" + person.FirstName +
        //            "', LastName	= '" + person.LastName +
        //            "', Address1	= '" + person.Address1 +
        //            "', Address2	= '" + person.Address2 +
        //            "', Telephone1	= '" + person.Telephone1 +
        //            "', Telephone2	= '" + person.Telephone2 +
        //            "',	PersonTypeId= " + IntToStr(person.PersonTypeId) +
        //            ",	Status		= " + IntToStr(person.Status) +
        //            ",	LastModified= '" + person.LastModified + "' "
        //            ",	Modified	= '" + GetLocalTime() + "' "
        //            ",  Deleted = " + IntToStr(person.Deleted) +
        //            " WHERE Id = " + IntToStr(person.Id);
        //    }
        //    else
        //    {
        //        query = "INSERT INTO Person ( Id, FirstName, LastName, "
        //            "	Address1, Address2, Telephone1, Telephone2, "
        //            "	PersonTypeId, Status, " //SubId, "
        //            "   LastModified, Modified, Deleted ) "
        //            "	values (" + IntToStr(person.Id) + ", '" + person.FirstName + "', "
        //            "'" + person.LastName + "', '" + person.Address1 + "', "
        //            "'" + person.Address2 + "', '" + person.Telephone1 + "', "
        //            "'" + person.Telephone2 + "', " + IntToStr(person.PersonTypeId) + "," +
        //            IntToStr(person.Status) + ", '" + person.LastModified + "', "
        //            "'" + GetLocalTime() + "', " + IntToStr(person.Deleted) + " )";
        //    }

        //if (ExecuteSyncQuery(query))
        //    return -1;

        //return updatePersonSubId(person);
        return 0;
    }
    //----------------------------------------------------------------------------

    std::map<unsigned long, domain::CfmPerson_Table> DBManager::getPersons(int mSIDID) {
        std::map<unsigned long, domain::CfmPerson_Table> mapPerson;
        //std::string query;
        //if (mSIDID == 0) //Se query è generale ->tutte le persone 
        //{
        //    query = "SELECT Id, PersonTypeId, Status, "
        //        "	FirstName, LastName, Address1, Address2, "
        //        "	Telephone1, Telephone2, "
        //        "	LastModified, Modified, Deleted "
        //        " FROM [Person] ";
        //}
        //else //Query si limita su utenti di un sid
        //{

        //    query = " SELECT Person.Id, Person.PersonTypeId, Person.Status, Person.FirstName, Person.LastName, Person.Address1, Person.Address2 , Person.Telephone1, Person.Telephone2, Person.LastModified, Person.Modified, Person.Deleted, PersonSubId.SystemId "
        //        " FROM Person, PersonSubId "
        //        " WHERE Person.Id=PersonSubId.IdPerson "
        //        " AND PersonSubId.SystemId= ";
        //    query.append(IntToStr(mSIDID));

        //}
        //QUERY_ENTER(query, mapPerson);

        //while (aStatement.fetch_next())
        //{
        //    SaraPerson_Table person;

        //    person.Id = aStatement.field(1).as_unsigned_long();
        //    person.PersonTypeId = aStatement.field(2).as_short();
        //    person.Status = aStatement.field(3).as_short();
        //    person.FirstName = aStatement.field(4).as_string();
        //    person.LastName = aStatement.field(5).as_string();
        //    person.Address1 = aStatement.field(6).as_string();
        //    person.Address2 = aStatement.field(7).as_string();
        //    person.Telephone1 = aStatement.field(8).as_string();
        //    person.Telephone2 = aStatement.field(9).as_string();
        //    person.LastModified = aStatement.field(10).as_string();
        //    person.DBModified = aStatement.field(11).as_string();
        //    person.Deleted = aStatement.field(12).as_short();

        //    mapPerson[person.Id] = person;
        //}
        //QUERY_EXIT(query)

        //    std::map<unsigned long, SaraPerson_Table>::iterator it;
        //for (it = mapPerson.begin(); it != mapPerson.end(); it++)
        //    getPersonSubId(it->second);

        return mapPerson;
    }
    //----------------------------------------------------------------------------

    domain::CfmPerson_Table DBManager::getPersonById(unsigned long Id) {
        domain::CfmPerson_Table person;

        /*std::string query = "SELECT Id, PersonTypeId, Status, "
            " FirstName, LastName, Address1, Address2, "
            " Telephone1, Telephone2, "
            " LastModified, Modified, Deleted "
            " FROM [Person] "
            " WHERE Id=" + IntToStr(Id);

        QUERY_ENTER(query, person);

        while (aStatement.fetch_next())
        {
            person.Id = aStatement.field(1).as_unsigned_long();
            person.PersonTypeId = aStatement.field(2).as_short();
            person.Status = aStatement.field(3).as_short();
            person.FirstName = aStatement.field(4).as_string();
            person.LastName = aStatement.field(5).as_string();
            person.Address1 = aStatement.field(6).as_string();
            person.Address2 = aStatement.field(7).as_string();
            person.Telephone1 = aStatement.field(8).as_string();
            person.Telephone2 = aStatement.field(9).as_string();
            person.LastModified = aStatement.field(10).as_string();
            person.DBModified = aStatement.field(11).as_string();
            person.Deleted = aStatement.field(12).as_short();
        }
        QUERY_EXIT(query)

            getPersonSubId(person);*/

        return person;
    }
    //----------------------------------------------------------------------------

    unsigned long DBManager::getMaxPersonId() {
        unsigned long id = 0;
        //std::string query = "SELECT MAX(Id)"
        //    " FROM [Person] ";

        //QUERY_ENTER(query, id);

        //if (aStatement.fetch_next())
        //    id = aStatement.field(1).as_unsigned_long();

        //QUERY_EXIT(query)

            return id;
    }
    //----------------------------------------------------------------------------

    int DBManager::getBadgeSubId(domain::CfmBadge_Table& badge) {

        return getsTableSubId("BadgeSubId", "IdBadge", badge.Id, badge.mapSubId);
    }
    //----------------------------------------------------------------------------

    int	DBManager::updateBadgeSubId(domain::CfmBadge_Table& badge) {
        return updateTableSubId("BadgeSubId", "IdBadge", badge.Id, badge.mapSubId);
    }
    //----------------------------------------------------------------------------

    int	DBManager::deleteBadgeSubId(domain::CfmBadge_Table& badge, unsigned int SidId){
        //std::string query;
        //std::map<unsigned int, std::string>::iterator itSubId;

        //query = "DELETE FROM BadgeSubId "
        //    " WHERE IdBadge=" + IntToStr(badge.Id) +
        //    "   AND SystemId IN (" + IntToStr(SidId) + ")";

        //return ExecuteSyncQuery(query);
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::deleteBadgeById(unsigned long id) {
        //std::string query;

        //query = "DELETE FROM Badge "
        //    " WHERE Id=" + IntToStr(id);
        //return ExecuteSyncQuery(query);
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::updateBadge(domain::CfmBadge_Table& badge) {
        /*bool update = false;
        std::string query = "SELECT Id "
            " FROM [badge] "
            " WHERE Id=" + IntToStr(badge.Id);

        QUERY_ENTER(query, -1);

        if (aStatement.fetch_next())
            update = true;

        QUERY_EXIT(query)

            if (update)
            {
                query = "UPDATE Badge "
                    " SET Description = '" + badge.Description +
                    "', EncodedNumber = '" + badge.EncodedNumber +
                    "', AliasNumber = '" + badge.AliasNumber +
                    "', PIN = '" + badge.PIN +
                    "', LastModified = '" + badge.LastModified +
                    "', Modified = '" + GetLocalTime() +
                    "', BeginValidation = '" + badge.BeginValidation +
                    "', EndValidation = '" + badge.EndValidation +
                    "', Status = " + IntToStr(badge.Status) +
                    ",	PersonId = " + IntToStr(badge.PersonId) +
                    ",	Deleted = " + IntToStr(badge.Deleted) +
                    " WHERE Id = " + IntToStr(badge.Id);
            }
            else
            {
                query = "INSERT INTO Badge ( Id, Description, EncodedNumber, "
                    "	AliasNumber, PIN, LastModified, Modified, BeginValidation, "
                    "	EndValidation, Status, PersonId, Deleted ) "
                    " values(" + IntToStr(badge.Id) + ", '" + badge.Description + "', "
                    "'" + badge.EncodedNumber + "', '" + badge.AliasNumber + "', "
                    "'" + badge.PIN + "', '" + badge.LastModified + "', "
                    "'" + GetLocalTime() + "', '" + badge.BeginValidation + "', "
                    "'" + badge.EndValidation + "', " + IntToStr(badge.Status) + ", " +
                    IntToStr(badge.PersonId) + ", " + IntToStr(badge.Deleted) + " )";
            }
        if (ExecuteSyncQuery(query))
            return -1;

        return updateBadgeSubId(badge);*/
        return 0;
    }
    //----------------------------------------------------------------------------

    std::map<unsigned long, domain::CfmBadge_Table> DBManager::getBadges(int sid) {
        std::map<unsigned long, domain::CfmBadge_Table> mapBadge;
        //std::string query;
        //if (sid == 0) //Se query è generale ->tutte le persone 
        //{
        //    query = "SELECT Id, Description, EncodedNumber, "
        //        "		AliasNumber, PIN, LastModified, Modified, "
        //        "		BeginValidation, EndValidation, "
        //        "		Status, PersonId, Deleted "
        //        " FROM Badge ";

        //}
        //else //Query si limita su utenti di un sid
        //{

        //    query = " SELECT Badge.Id, Badge.Description, Badge.EncodedNumber, Badge.AliasNumber, Badge.PIN, Badge.LastModified, Badge.Modified , Badge.BeginValidation, Badge.Telephone2, Badge.LastModified, Badge.Modified, Badge.BeginValidation, Badge.EndValidation, Badge.Status,Badge.PersonId,Badge.Deleted, BadgeSubId.SystemId "
        //        " FROM Person, PersonSubId "
        //        " WHERE Badge.Id=BadgeSubId.IdBadge "
        //        " AND BadgeSubId.SystemId= ";
        //    query.append(IntToStr(sid));
        //}

        //QUERY_ENTER(query, mapBadge);

        //while (aStatement.fetch_next())
        //{
        //    SaraBadge_Table badge;

        //    badge.Id = aStatement.field(1).as_unsigned_long();
        //    badge.Description = aStatement.field(2).as_string();
        //    badge.EncodedNumber = aStatement.field(3).as_string();
        //    badge.AliasNumber = aStatement.field(4).as_string();
        //    badge.PIN = aStatement.field(5).as_string();
        //    badge.LastModified = aStatement.field(6).as_string();
        //    badge.DBModified = aStatement.field(7).as_string();
        //    badge.BeginValidation = aStatement.field(8).as_string();
        //    badge.EndValidation = aStatement.field(9).as_string();
        //    badge.Status = aStatement.field(10).as_short();
        //    badge.PersonId = aStatement.field(11).as_unsigned_long();
        //    badge.Deleted = aStatement.field(12).as_short();

        //    mapBadge[badge.Id] = badge;
        //}
        //QUERY_EXIT(query)

        //    std::map<unsigned long, SaraBadge_Table>::iterator it;
        //for (it = mapBadge.begin(); it != mapBadge.end(); it++)
        //    getBadgeSubId(it->second);

        return mapBadge;
    }
    //----------------------------------------------------------------------------

    domain::CfmBadge_Table DBManager::getBadgeById(unsigned long Id) {
        domain::CfmBadge_Table badge;

        /*std::string query = "SELECT Id, Description, EncodedNumber, "
            "		AliasNumber, PIN, LastModified, Modified, "
            "		BeginValidation, EndValidation "
            "		Status, PersonId, Deleted "
            " FROM Badge "
            " WHERE Id=" + IntToStr(Id);

        QUERY_ENTER(query, badge);

        while (aStatement.fetch_next())
        {
            badge.Id = aStatement.field(1).as_unsigned_long();
            badge.Description = aStatement.field(2).as_string();
            badge.EncodedNumber = aStatement.field(3).as_string();
            badge.AliasNumber = aStatement.field(4).as_string();
            badge.PIN = aStatement.field(5).as_string();
            badge.LastModified = aStatement.field(6).as_string();
            badge.DBModified = aStatement.field(7).as_string();
            badge.BeginValidation = aStatement.field(8).as_string();
            badge.EndValidation = aStatement.field(9).as_string();
            badge.Status = aStatement.field(10).as_short();
            badge.PersonId = aStatement.field(11).as_unsigned_long();
            badge.Deleted = aStatement.field(12).as_short();
        }
        QUERY_EXIT(query)

            getBadgeSubId(badge);*/

        return badge;
    }
    //----------------------------------------------------------------------------

    unsigned long DBManager::getMaxBadgeId() {
        unsigned long id = 0;
        //std::string query = "SELECT MAX(Id)"
        //    " FROM Badge ";

        //QUERY_ENTER(query, id);

        //if (aStatement.fetch_next())
        //    id = aStatement.field(1).as_unsigned_long();

        //QUERY_EXIT(query)

            return id;
    }
    //----------------------------------------------------------------------------

    std::map<std::string, domain::CfmAlarmGroup_Table> DBManager::getAlarmGroups(int SystemId, int SiteId) {
        std::map<std::string, domain::CfmAlarmGroup_Table> mapAlmGr;
       /* std::string query;

        query = "SELECT Id, Name, SiteId, Modified FROM AlarmGroup ";

        if (SiteId > 0)
            query += " WHERE SiteId=" + IntToStr(SiteId);

        QUERY_ENTER(query, mapAlmGr)

            while (aStatement.fetch_next())
            {
                SaraAlarmGroup_Table p;

                p.Id = aStatement.field(1).as_unsigned_long();
                p.Name = aStatement.field(2).as_string();
                p.SiteId = aStatement.field(3).as_unsigned_long();
                p.Modified = aStatement.field(4).as_string();

                std::string key = p.Name + "|" + IntToStr(p.SiteId);
                mapAlmGr[key] = p;
            }

        QUERY_EXIT(query)

            std::map<std::string, SaraAlarmGroup_Table>::iterator it;

        for (it = mapAlmGr.begin(); it != mapAlmGr.end(); it++)
        {
            query = "SELECT DevicesId, SubIdAlarmGroup, SystemId "
                " FROM AlarmGroupDevices "
                " WHERE IdAlarmGroup = " + IntToStr(it->second.Id) + " AND "
                "		SystemId = " + IntToStr(SystemId);

            QUERY_ENTER(query, mapAlmGr)
                while (aStatement.fetch_next())
                {
                    SaraAlarmGroupDevice_Table p;

                    p.IdAlarmGroup = it->second.Id;
                    p.DevicesId = aStatement.field(1).as_unsigned_long();
                    p.SubIdAlarmGroup = aStatement.field(2).as_string();
                    p.SystemId = aStatement.field(3).as_unsigned_long();

                    it->second.mapAlarmGroupDevice[p.DevicesId] = p;
                }
            QUERY_EXIT(query)
        }*/
        return mapAlmGr;
    }
    //----------------------------------------------------------------------------

    int DBManager::InsertAlarmGroup(domain::CfmAlarmGroup_Table& alGrp) {
        unsigned long Id = 0;
        //std::string query;

        //if (alGrp.Modified.empty())
        //    alGrp.Modified = GetLocalTime();

        ////query = "SELECT Id, Name, SiteId, Modified FROM AlarmGroup WHERE Id="+IntToStr(alGrp.Id);
        //query = "SELECT Id, Name, SiteId, Modified FROM AlarmGroup "
        //    " WHERE Name='" + alGrp.Name + "' AND SiteId=" + IntToStr(alGrp.SiteId);
        //QUERY_ENTER(query, -1)

        //    if (aStatement.fetch_next())
        //        Id = aStatement.field(1).as_unsigned_long();

        //QUERY_EXIT(query)

        //    if (Id)
        //    {
        //        query = "UPDATE AlarmGroup SET Name='" + alGrp.Name + "'" +
        //            "	, SiteId=" + IntToStr(alGrp.SiteId) +
        //            "	, Modified='" + alGrp.Modified + "'" +
        //            "	WHERE Id=" + IntToStr(Id);

        //        ExecuteSyncQuery(query);
        //    }
        //    else {
        //        //query = "INSERT INTO AlarmGroup ( Name, SiteId, Modified ) OUTPUT INSERTED.Id "
        //        query = "INSERT INTO AlarmGroup ( Name, SiteId, Modified ) "
        //            " VALUES('" + alGrp.Name + "', " + IntToStr(alGrp.SiteId) + ", '" + alGrp.Modified + "')";

        //        if (!ExecuteSyncQuery(query))
        //        {
        //            query = "SELECT Id FROM AlarmGroup "
        //                " WHERE Name='" + alGrp.Name + "' "
        //                "   AND SiteId=" + IntToStr(alGrp.SiteId);
        //            Id = getULongSyncQuery(query);
        //        }
        //    }

        //std::map<unsigned long, SaraAlarmGroupDevice_Table>::iterator it;

        //alGrp.Id = Id;
        //for (it = alGrp.mapAlarmGroupDevice.begin(); it != alGrp.mapAlarmGroupDevice.end(); it++)
        //{
        //    query = "SELECT SubIdAlarmGroup, SystemId "
        //        " FROM AlarmGroupDevices "
        //        " WHERE IdAlarmGroup = " + IntToStr(Id) + " AND "
        //        "		DevicesId = " + IntToStr(it->second.DevicesId);

        //    bool found = false;

        //    QUERY_ENTER(query, Id)
        //        if (aStatement.fetch_next())
        //            found = true;
        //    QUERY_EXIT(query)

        //        if (!found) {
        //            query = "INSERT INTO AlarmGroupDevices ( IdAlarmGroup,	DevicesId, SubIdAlarmGroup, SystemId ) "
        //                " VALUES( " + IntToStr(Id) + ", " + IntToStr(it->second.DevicesId) + ", '" +
        //                it->second.SubIdAlarmGroup + "', " + IntToStr(it->second.SystemId) + ")";
        //        }
        //        else {
        //            query = "UPDATE AlarmGroupDevices SET "
        //                "	SubIdAlarmGroup = '" + it->second.SubIdAlarmGroup + "', "
        //                "	SystemId = '" + IntToStr(it->second.SystemId) + "' "
        //                " WHERE IdAlarmGroup = " + IntToStr(Id) +
        //                "	AND DevicesId = " + IntToStr(it->second.DevicesId);
        //        }
        //    ExecuteSyncQuery(query);
        //    it->second.IdAlarmGroup = Id;
        //}
        return (int)Id;
    }
    //----------------------------------------------------------------------------

    int DBManager::UpdateAlarmGroup(unsigned long IdAlarmGroup, unsigned long DeviceId,
        int SystemId, int SiteId, domain::CfmAlarmGroup_Table& alGrp) {
        //std::string query;

        //if (alGrp.Modified.empty())
        //    alGrp.Modified = GetLocalTime();

        //query = "UPDATE AlarmGroup SET Name='" + alGrp.Name + "'" +
        //    "	, Site=" + IntToStr(SiteId) +
        //    "	, Modified='" + alGrp.Modified + "'" +
        //    "	WHERE Id=" + IntToStr(IdAlarmGroup);

        //int ret = ExecuteSyncQuery(query);
        //if (ret == 0)
        //{
        //    bool found = false;

        //    std::map<unsigned long, SaraAlarmGroupDevice_Table>::iterator it;

        //    alGrp.Id = IdAlarmGroup;
        //    for (it = alGrp.mapAlarmGroupDevice.begin(); it != alGrp.mapAlarmGroupDevice.end(); it++)
        //    {
        //        query = "SELECT SubIdAlarmGroup "
        //            " WHERE IdAlarmGroup = " + IntToStr(IdAlarmGroup) + " AND "
        //            "		DevicesId = " + IntToStr(it->second.DevicesId);

        //        bool found = false;

        //        QUERY_ENTER(query, IdAlarmGroup)
        //            if (aStatement.fetch_next())
        //                found = true;
        //        QUERY_EXIT(query)

        //            if (!found) {
        //                query = "INSERT INTO AlarmGroupDevices ( IdAlarmGroup,	DevicesId, SubIdAlarmGroup ) "
        //                    " VALUES( " + IntToStr(IdAlarmGroup) + ", " + IntToStr(it->second.DevicesId) + ", '" + it->second.SubIdAlarmGroup + "' )";
        //            }
        //            else {
        //                query = "UPDATE AlarmGroupDevices SET SubIdAlarmGroup = '" + it->second.SubIdAlarmGroup + "' "
        //                    " WHERE IdAlarmGroup = " + IntToStr(IdAlarmGroup) +
        //                    "	AND DevicesId = " + IntToStr(it->second.DevicesId);
        //            }
        //        ExecuteSyncQuery(query);
        //        it->second.IdAlarmGroup = IdAlarmGroup;
        //    }

        //}
        //return ret;
        return 0;
    }
    //----------------------------------------------------------------------------

    int	DBManager::UpdateAccessRightSubId(domain::CfmAccessRight_Table& accessRight) {
        //std::string query, strSidId;
        //std::map<unsigned int, std::string>::iterator itSubId;
        //return updateTableSubId("AccessRightSubId", "IdAccessRight", accessRight.Id, accessRight.mapSubId);
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::getAccessRightSubId(domain::CfmAccessRight_Table& accessRight) {
        //std::string query, strSidId;
        //std::map<unsigned int, std::string>::iterator itSubId;

        //query = "SELECT SystemId, SubId "
        //    " FROM AccessRightSubId  "
        //    " WHERE IdAccessRight=" + IntToStr(accessRight.Id) +
        //    "   AND Deleted <> 1 ";

        //QUERY_ENTER(query, -1)

        //    while (aStatement.fetch_next())
        //    {
        //        unsigned int SidId = aStatement.field(1).as_unsigned_long();
        //        std::string	 SubId = aStatement.field(2).as_string();
        //        accessRight.mapSubId[SidId] = SubId;
        //    }

        //QUERY_EXIT(query)

            return 0;
    }
    //----------------------------------------------------------------------------

    int	DBManager::updateAccessRightSubId(domain::CfmAccessRight_Table& accessRight) {
        std::string query, strSidId;
        //std::map<unsigned int, std::string>::iterator itSubId;

        //for (itSubId = accessRight.mapSubId.begin(); itSubId != accessRight.mapSubId.end(); itSubId++)
        //    query += "INSERT INTO AccessRightSubId ( IdAccessRight, SystemId, SubId ) "
        //    "VALUES ( " + IntToStr(accessRight.Id) + ", " + IntToStr(itSubId->first) +
        //    ", '" + itSubId->second + "'); ";

        //ExecuteSyncQuery(query);

        return 0;
    }
    //----------------------------------------------------------------------------

    int	DBManager::deleteAccessRightSubId(domain::CfmAccessRight_Table& accessRight, unsigned int SidId) {
        //std::string query;
        //std::map<unsigned int, std::string>::iterator itSubId;

        //query = "DELETE FROM AccessRightSubId "
        //    " WHERE IdAccessRight=" + IntToStr(accessRight.Id) +
        //    "   AND SystemId IN (" + IntToStr(SidId) + ")";

        //return ExecuteSyncQuery(query);
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::deleteAccessRightById(unsigned long id) {
        //std::string query;

        //query = "DELETE FROM AccessRight "
        //    " WHERE Id=" + IntToStr(id);
        //return ExecuteSyncQuery(query);
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::UpdateAccessRightIdReader(domain::CfmAccessRight_Table& tAccessRight) {
        int retval = 0;
        /*std::string query, strSidId;
        std::set<unsigned long>::iterator it;
        

        for (it = tAccessRight.listReader.begin(); it != tAccessRight.listReader.end(); it++)
        {
            query = "SELECT IdAccessRight FROM AccessRightIdReader "
                " WHERE IdAccessRight = " + IntToStr(tAccessRight.Id) +
                "   AND IdReader = " + IntToStr(*it);
            unsigned long Id = getULongSyncQuery(query);
            if (Id <= 0)
            {
                query += "INSERT INTO AccessRightIdReader ( IdAccessRight, IdReader, Deleted ) "
                    " VALUES ( " + IntToStr(tAccessRight.Id) + ", " + IntToStr(*it) + ", 0 ); ";
            }
            else
            {
                query = "Update AccessRightIdReader SET "
                    " Deleted = 0 "
                    " WHERE IdAccessRight = " + IntToStr(tAccessRight.Id) +
                    "   AND IdReader = " + IntToStr(*it);
            }

            retval |= ExecuteSyncQuery(query);
        }*/
        return retval;
    }
    //----------------------------------------------------------------------------

    int DBManager::UpdateAccessRightIdBadge(domain::CfmAccessRight_Table& tAccessRight) {
        int retval = 0;

        /*std::string query, strSidId;
        std::set<unsigned long>::iterator it;
        

        for (it = tAccessRight.listBadge.begin(); it != tAccessRight.listBadge.end(); it++)
        {
            query = "SELECT IdAccessRight FROM AccessRightIdBadge "
                " WHERE IdAccessRight = " + IntToStr(tAccessRight.Id) +
                "   AND IdBadge = " + IntToStr(*it);
            unsigned long Id = getULongSyncQuery(query);
            if (Id <= 0)
            {
                query = "INSERT INTO AccessRightIdBadge ( IdAccessRight, IdBadge, Deleted ) "
                    " VALUES ( " + IntToStr(tAccessRight.Id) + ", " + IntToStr(*it) + ", 0 ); ";
            }
            else
            {
                query = "Update AccessRightIdBadge SET "
                    " Deleted = 0 "
                    " WHERE IdAccessRight = " + IntToStr(tAccessRight.Id) +
                    "   AND IdBadge = " + IntToStr(*it);
            }

            retval |= ExecuteSyncQuery(query);
        }*/
        return retval;
    }
    //----------------------------------------------------------------------------

    int DBManager::updateAccessRight(domain::CfmAccessRight_Table& accessRight) {
        bool update = false;
        //std::string query = "SELECT Id "
        //    " FROM AccessRight "
        //    " WHERE Id=" + IntToStr(accessRight.Id);

        //QUERY_ENTER(query, -1);

        //if (aStatement.fetch_next())
        //    update = true;

        //QUERY_EXIT(query)

        //    if (update)
        //    {
        //        query = "UPDATE AccessRight "
        //            " SET Description = '" + accessRight.Description + "'"
        //            ",	IdTimeSchedule = " + IntToStr(accessRight.IdTimeSchedule) +
        //            ",	StartDate = '" + accessRight.StartDate + "'"
        //            ",	EndDate	= '" + accessRight.EndDate + "'"
        //            ",	LastModified = '" + accessRight.LastModified + "'"
        //            ",	Modified = '" + accessRight.DBModified + "' "
        //            ",  Deleted = " + IntToStr(accessRight.Deleted) +
        //            " WHERE Id = " + IntToStr(accessRight.Id);
        //    }
        //    else
        //    {
        //        query = "INSERT INTO AccessRight "
        //            "( Id, Description, "
        //            " IdTimeSchedule, StartDate, "
        //            " EndDate, "
        //            " LastModified, Modified, Deleted ) "
        //            " VALUES ( " + IntToStr(accessRight.Id) + ", '" + accessRight.Description + "', " +
        //            IntToStr(accessRight.IdTimeSchedule) + ", '" + accessRight.StartDate + "', "
        //            "'" + accessRight.EndDate + "', "
        //            "'" + accessRight.LastModified + "', '" + GetLocalTime() + "', " +
        //            IntToStr(accessRight.Deleted) + " )";
        //    }

        //if (ExecuteSyncQuery(query))
        //    return -1;

        //UpdateAccessRightSubId(accessRight);
        //UpdateAccessRightIdBadge(accessRight);
        //UpdateAccessRightIdReader(accessRight);
        return 0;
    }
    //----------------------------------------------------------------------------

    std::map<unsigned long, domain::CfmAccessRight_Table> DBManager::getAccessRight() {
        std::map<unsigned long, domain::CfmAccessRight_Table> mapAccessRight;

        /*std::string query = "SELECT Id, Description, "
            " IdTimeSchedule, StartDate, "
            " EndDate, LastModified, "
            " Modified, Deleted "
            " FROM AccessRight ";

        QUERY_ENTER(query, mapAccessRight);

        while (aStatement.fetch_next())
        {
            SaraAccessRight_Table accessRight;

            accessRight.Id = aStatement.field(1).as_unsigned_long();
            accessRight.Description = aStatement.field(2).as_string();
            accessRight.IdTimeSchedule = aStatement.field(3).as_unsigned_long();
            accessRight.StartDate = aStatement.field(4).as_string();
            accessRight.EndDate = aStatement.field(5).as_string();
            accessRight.LastModified = aStatement.field(6).as_string();
            accessRight.DBModified = aStatement.field(7).as_string();
            accessRight.Deleted = aStatement.field(8).as_short();

            mapAccessRight[accessRight.Id] = accessRight;
        }

        QUERY_EXIT(query)

            std::map<unsigned long, SaraAccessRight_Table>::iterator it;
        for (it = mapAccessRight.begin(); it != mapAccessRight.end(); it++)
        {
            getAccessRightSubId(it->second);
            getAccessRightIdReader(it->second);
            getAccessRightIdBadge(it->second);
        }*/
        return mapAccessRight;
    }
    //----------------------------------------------------------------------------

    domain::CfmAccessRight_Table DBManager::getAccessRightById(unsigned long Id) {
        domain::CfmAccessRight_Table accessRight;

        /*std::string query = "SELECT Id, Description, "
            " IdTimeSchedule, StartDate, "
            " EndDate, LastModified, "
            " DBModified, Deleted "
            " FROM AccessRight ";
        " WHERE Id=" + IntToStr(Id);

        QUERY_ENTER(query, accessRight);

        if (aStatement.fetch_next())
        {
            accessRight.Id = aStatement.field(1).as_unsigned_long();
            accessRight.Description = aStatement.field(2).as_string();
            accessRight.IdTimeSchedule = aStatement.field(3).as_unsigned_long();
            accessRight.StartDate = aStatement.field(4).as_string();
            accessRight.EndDate = aStatement.field(5).as_string();
            accessRight.LastModified = aStatement.field(6).as_string();
            accessRight.DBModified = aStatement.field(7).as_string();
            accessRight.Deleted = aStatement.field(8).as_short();
        }
        QUERY_EXIT(query)

            getAccessRightSubId(accessRight);
        getAccessRightIdReader(accessRight);
        getAccessRightIdBadge(accessRight);*/

        return accessRight;
    }
    //----------------------------------------------------------------------------

    unsigned long DBManager::getMaxAccessRightId() {
        unsigned long id = 0;
        //std::string query = "SELECT MAX(Id)"
        //    " FROM AccessRight ";

        //QUERY_ENTER(query, id);

        //if (aStatement.fetch_next())
        //    id = aStatement.field(1).as_unsigned_long();

        //QUERY_EXIT(query)

            return id;
    }
    //----------------------------------------------------------------------------

    int DBManager::getAccessRightIdReader(domain::CfmAccessRight_Table& tAccessRight) {
        std::string query;

        //query = "SELECT IdReader FROM AccessRightIdReader "
        //    " WHERE IdAccessRight=" + IntToStr(tAccessRight.Id) +
        //    "   AND Deleted <> 1 ";

        //QUERY_ENTER(query, -1);

        //while (aStatement.fetch_next())
        //    tAccessRight.listReader.insert(aStatement.field(1).as_unsigned_long());

        //QUERY_EXIT(query)

            return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::getAccessRightIdBadge(domain::CfmAccessRight_Table& tAccessRight) {
        std::string query;

        //query = "SELECT IdBadge FROM AccessRightIdBadge "
        //    " WHERE IdAccessRight=" + IntToStr(tAccessRight.Id) +
        //    "   AND Deleted <> 1 ";

        //QUERY_ENTER(query, -1);

        //while (aStatement.fetch_next())
        //    tAccessRight.listBadge.insert(aStatement.field(1).as_unsigned_long());

        //QUERY_EXIT(query)

            return 0;
    }
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    // TimeSchedule
    //----------------------------------------------------------------------------

    int DBManager::updateTimeSchedule(domain::CfmTimeSchedule_Table& tTimeSchedule) {
        bool update = false;
        //std::string query = "SELECT Id, Name "
        //    " FROM TimeSchedule "
        //    " WHERE Id=" + IntToStr(tTimeSchedule.Id);

        //QUERY_ENTER(query, -1);

        //if (aStatement.fetch_next())
        //    update = true;

        //QUERY_EXIT(query)

        //    if (update)
        //    {
        //        query = "UPDATE TimeSchedule "
        //            " SET Name = '" + tTimeSchedule.Description + "'"
        //            " , LastModified = '" + tTimeSchedule.LastModified + "'"
        //            " , Modified = '" + GetLocalTime() + "' "
        //            " , Deleted = " + IntToStr(tTimeSchedule.Deleted) +
        //            " WHERE Id = " + IntToStr(tTimeSchedule.Id);
        //    }
        //    else
        //    {
        //        query = "INSERT INTO TimeSchedule (	Id, Name, LastModified, Modified, Deleted ) "
        //            " values( " + IntToStr(tTimeSchedule.Id) + ", '" + tTimeSchedule.Description + "', "
        //            "'" + tTimeSchedule.LastModified + "', '" + GetLocalTime() + "', " +
        //            IntToStr(tTimeSchedule.Deleted) + " )";
        //    }

        //if (ExecuteSyncQuery(query))
        //    return -1;

        //updateTimeRange(tTimeSchedule.Id, tTimeSchedule.mapTimeRange);
        //updateTableSubId("TimeScheduleSubId", "IdTimeSchedule", tTimeSchedule.Id, tTimeSchedule.mapSubId);
        return 0;
    }
    //----------------------------------------------------------------------------

    std::map<unsigned long, domain::CfmTimeSchedule_Table> DBManager::getTimeSchedule() {
        std::map<unsigned long, domain::CfmTimeSchedule_Table> mapTimeSchedule;

        /*std::string query = "SELECT Id, Name, LastModified, Modified, Deleted "
            " FROM TimeSchedule ";

        QUERY_ENTER(query, mapTimeSchedule);

        while (aStatement.fetch_next())
        {
            SaraTimeSchedule_Table tTimeSchedule;

            tTimeSchedule.Id = aStatement.field(1).as_unsigned_long();
            tTimeSchedule.Description = aStatement.field(2).as_string();
            tTimeSchedule.LastModified = aStatement.field(3).as_string();
            tTimeSchedule.DBModified = aStatement.field(4).as_string();
            tTimeSchedule.Deleted = aStatement.field(5).as_short();

            mapTimeSchedule[tTimeSchedule.Id] = tTimeSchedule;
        }

        QUERY_EXIT(query)

            std::map<unsigned long, SaraTimeSchedule_Table>::iterator it;
        for (it = mapTimeSchedule.begin(); it != mapTimeSchedule.end(); it++)
        {
            getsTableSubId("TimeScheduleSubId", "IdTimeSchedule", it->first, it->second.mapSubId);
            getsTimeRange(it->first, it->second.mapTimeRange);
        }*/

        return mapTimeSchedule;
    }
    //----------------------------------------------------------------------------

    domain::CfmTimeSchedule_Table DBManager::getTimeScheduleById(unsigned long Id) {
        domain::CfmTimeSchedule_Table tTimeSchedule;

        /*std::string query = "SELECT Id, Name, LastModified, Modified "
            " FROM TimeSchedule "
            " WHERE Id=" + IntToStr(Id);

        QUERY_ENTER(query, tTimeSchedule);

        if (aStatement.fetch_next())
        {
            tTimeSchedule.Id = aStatement.field(1).as_unsigned_long();
            tTimeSchedule.Description = aStatement.field(2).as_string();
            tTimeSchedule.LastModified = aStatement.field(3).as_string();
            tTimeSchedule.DBModified = aStatement.field(4).as_string();
        }

        QUERY_EXIT(query)

            getsTableSubId("TimeScheduleSubId", "IdTimeSchedule", tTimeSchedule.Id, tTimeSchedule.mapSubId);
        getsTimeRange(tTimeSchedule.Id, tTimeSchedule.mapTimeRange);*/

        return tTimeSchedule;
    }
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    // TimeRange
    //----------------------------------------------------------------------------

    int DBManager::getsTimeRange(unsigned long Id, std::map<unsigned long, domain::CfmTimeRange_Table>& mapTimeRange) {
        std::string query;

        //query = "SELECT Id, Day, StartTime, EndTime, IdTimeSchedule, "
        //    " Holiday, LastModified, Modified, Deleted "
        //    " FROM TimeRange "
        //    " WHERE IdTimeSchedule=" + IntToStr(Id) + " AND Deleted <> 1";

        //QUERY_ENTER(query, -1);

        //while (aStatement.fetch_next())
        //{
        //    SaraTimeRange_Table tTimeRange;

        //    tTimeRange.Id = aStatement.field(1).as_unsigned_long();
        //    tTimeRange.Day = aStatement.field(2).as_unsigned_long();
        //    tTimeRange.StartTime = aStatement.field(3).as_string();
        //    tTimeRange.EndTime = aStatement.field(4).as_string();
        //    tTimeRange.IdTimeSchedule = aStatement.field(5).as_unsigned_long();
        //    tTimeRange.Holiday = aStatement.field(6).as_long();
        //    tTimeRange.LastModified = aStatement.field(7).as_string();
        //    tTimeRange.DBModified = aStatement.field(8).as_string();
        //    tTimeRange.Deleted = aStatement.field(9).as_short();

        //    mapTimeRange[Id] = tTimeRange;
        //}

        //QUERY_EXIT(query)

        //    std::map<unsigned long, SaraTimeRange_Table>::iterator it;
        //for (it = mapTimeRange.begin(); it != mapTimeRange.end(); it++)
        //    getsTableSubId("TimeRangeSubId", "IdTimeRange", it->first, it->second.mapSubId);

        return 0;
    }
    //----------------------------------------------------------------------------

    int	DBManager::updateTimeRange(unsigned long IdTS, std::map<unsigned long, domain::CfmTimeRange_Table>& mapTimeRange) {
        std::string query;
        std::map<unsigned long, domain::CfmTimeRange_Table>::iterator it;

        //for (it = mapTimeRange.begin(); it != mapTimeRange.end(); it++) {

        //    it->second.IdTimeSchedule = IdTS;

        //    query = "SELECT Id "
        //        " FROM TimeRange "
        //        " WHERE Id = " + IntToStr(it->second.Id);

        //    unsigned long TRId = getULongSyncQuery(query);

        //    if (TRId > 0)
        //    {
        //        query = "UPDATE TimeRange "
        //            " SET Id = " + IntToStr(it->second.Id) +
        //            "	, Day = " + IntToStr(it->second.Day) +
        //            "	, StartTime = '" + it->second.StartTime + "'"
        //            "	, EndTime = '" + it->second.EndTime + "'"
        //            "	, IdTimeSchedule = " + IntToStr(it->second.IdTimeSchedule) +
        //            "	, Holiday = " + IntToStr(it->second.Holiday) +
        //            "	, LastModified = '" + it->second.LastModified + "'"
        //            "	, Modified = '" + GetLocalTime() + "'"
        //            "	, Deleted = " + IntToStr(it->second.Deleted) +
        //            " WHERE Id = " + IntToStr(it->second.Id);
        //    }
        //    else
        //    {
        //        query = "INSERT INTO TimeRange "
        //            " ( Id, Day, StartTime, EndTime, IdTimeSchedule, "
        //            "   Holiday, LastModified, Modified, Deleted ) "
        //            " VALUES ( " + IntToStr(it->second.Id) + ", " + IntToStr(it->second.Day) + ", "
        //            "'" + it->second.StartTime + "', '" + it->second.EndTime + "', " +
        //            IntToStr(it->second.IdTimeSchedule) + ", " + IntToStr(it->second.Holiday) + ", "
        //            "'" + it->second.LastModified + "', '" + GetLocalTime() + "', " +
        //            IntToStr(it->second.Deleted) + " );";
        //    }

        //    ExecuteSyncQuery(query);

        //    updateTableSubId("TimeRangeSubId", "IdTimeRange", it->first, it->second.mapSubId);
        //}
        return 0;
    }
    //----------------------------------------------------------------------------

    //----------------------------------------------------------------------------
    // Global Table 
    //----------------------------------------------------------------------------

    unsigned long DBManager::getTableMaxId(std::string tableName) {
        unsigned long id = 0;
        //std::string query = "SELECT MAX(Id)"
        //    " FROM  " + tableName;

        //QUERY_ENTER(query, id);

        //if (aStatement.fetch_next())
        //    id = aStatement.field(1).as_unsigned_long();

        //QUERY_EXIT(query)

            return id;
    }
    //----------------------------------------------------------------------------

    int	DBManager::updateTableSubId(std::string tableName, std::string fieldName,
        unsigned int Id, std::map<unsigned int, std::string>& mapSubId) {
        std::string query, strSidId;
        std::map<unsigned int, std::string>::iterator itSubId;

        //for (itSubId = mapSubId.begin(); itSubId != mapSubId.end(); itSubId++)
        //{
        //    unsigned long TRSubId = 0;

        //    query = "SELECT " + fieldName +
        //        " FROM " + tableName +
        //        " WHERE " + fieldName + " = " + IntToStr(Id) +
        //        "   AND SystemId = " + IntToStr(itSubId->first) +
        //        "   AND SubId = '" + itSubId->second + "';";

        //    TRSubId = getULongSyncQuery(query);
        //    if (TRSubId > 0)
        //    {
        //        query = "UPDATE " + tableName +
        //            " SET Deleted = 0 "
        //            " WHERE " + fieldName + " = " + IntToStr(Id) +
        //            "    AND SystemId = " + IntToStr(itSubId->first) +
        //            "    AND SubId = '" + itSubId->second + "';";
        //    }
        //    else
        //    {
        //        query = "INSERT INTO " + tableName + " ( " + fieldName + ", SystemId, SubId, Deleted ) "
        //            "VALUES ( " + IntToStr(Id) + ", " + IntToStr(itSubId->first) +
        //            ", '" + itSubId->second + "', 0 ); ";
        //    }
        //    ExecuteSyncQuery(query);
        //}
        return 0;
    }
    //----------------------------------------------------------------------------

    int DBManager::getsTableSubId(std::string tableName, std::string fieldName,
        unsigned int Id, std::map<unsigned int, std::string>& mapSubId) {
        std::string query;
        //std::map<unsigned int, std::string>::iterator itSubId;

        //query = "SELECT SystemId, SubId "
        //    " FROM " + tableName +
        //    " WHERE " + fieldName + " = " + IntToStr(Id) + " AND Deleted <> 1";

        //QUERY_ENTER(query, -1)

        //    while (aStatement.fetch_next())
        //    {
        //        unsigned int SidId = aStatement.field(1).as_unsigned_long();
        //        std::string	 SubId = aStatement.field(2).as_string();
        //        mapSubId[SidId] = SubId;
        //    }

        //QUERY_EXIT(query)

            return 0;
    }
    //----------------------------------------------------------------------------

} //end namespace