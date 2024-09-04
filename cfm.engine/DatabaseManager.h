/****************************** Module Header ******************************\
* Module Name:  DatabaseManager.h
* Project:      S000
*
*
* Thread class implements runnable relative to database operations...
*
\***************************************************************************/
#pragma once
#include <sqlext.h>		/**< Per l'accesso a ODBC */
#include <vector>
#include <mutex>
#include "CfmDomainObjects.h"
#include "StateMachine.h"
//#include "TinyOdbc.h"
#include "nanodbc.h"
#include "Thread.h"

using namespace cfm::application::domain;
namespace cfm::application {
    
    struct CListaAzioni {
        int a;
        std::vector<domain::CfmActionRules_Table> lista;
        void PushAction(domain::CfmActionRules_Table action);
    };
  /**
  * @class DBManager
  * @brief Oggetto singleton gestore delle letture / scritture su database
  * @author Diego Gangale
  * @date 04/05/2007 Created
  *
  * Il gestore del database viene realizzato sia come singleton che come thread.
  * Tale componente mette a disposizione del Kernel un potente strumento di accesso ai dati	\n
  * memorizzati sul DBMS e che di fatto rappresentano la realta di interesse del sistema SARA. \n
  * L'oggetto supporta comandi di lettura e scrittura sincroni e comandi di scrittura e lettura asincroni.	\n
  *
  * L'oggetto espone 2 connessioni verso il dbms (tramite ODBC): \n
  *																 \n
  * - Connessione per accesso sincrono
  * - Connessione per accesso asincrono
  *
  *
  */
    class DBManager : public CThread, public CStateMachine <DBManager> {

        //Stati possibili in cui si può trovare l'oggetto DBManager
        typedef enum {
            STATE_NOT_CONNECTED,    /**< Stato del DB Manager: NON CONNESSO */
            STATE_CONNECTING,       /**< Stato del DB Manager: IN CONNESSIONE */
            STATE_CONNECTED,        /**< Stato del DB Manager: CONNESSO */
            DB_MANAGER_STATES_NO
        } DB_MANAGER_STATES;

        /* State Machine Actions: BEGIN ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
        void notConnectedDbConnect();
        void notConnectedDbReConnect();
        void connectingConnectTimeout();
        void connectedDbRelease();
        void connectedDbClean();
        void connectedInsertOcr();
        void connectedInsertTrack();
        void connectedInsertLogMess();
        void connectedInsertLogEventAnd();
        void connectedInsertLogEventOr();
        void connectedDbExecuteOp();
        void connectedInsertLogEvent();
        void connectedInsertLogCommand();
        void connectedInsertLogAction();
        void connectedInsertLogRule();
        void connectedUpdateLogAction();
        void connectedUpdateLogRule();
        void connectedUpdateSystemStatus();
        void connectedUpdateMaintenanceReq();
        void connectedUpdatePhisicalStatus();
        void connectedUpdateLogicalStatus();
        void connectedUpdateDeleteDevice();
        void connectedEraseOldRows();
        void connectedTimer();
        /* State Machine Actions: END ------------------------------------------------------------ */

        MSG msg;
        int timer;
    public:
        //----------------------
        //  METODI PUBBLICI
        //----------------------
        bool IsConnected();

        //Ritorna il puntatore al singleton
        static DBManager* getInstance();

        //La connessione viene effettuata verso ODBC
        void SetConnectionParams(const std::string& DSN);
        void Connect();
        void Release();

        //Metodi del thread
        DWORD Run(LPVOID /* arg */);

        bool isMaster() { return bIsMaster; }
        //Riempe la lista dei sistemi gestiti da SARA
        bool getSystemList();
        bool getSystemList(int sid);

        //Lista dei sistemi configurati su DB
        std::vector<domain::CfmSystem_Table> vSystemList = {}; //Pekmez

        //Riempe la lista delle Categorie gestiti da SARA
        bool getCategoriesPriority();

        //Lista delle categorie configurate su DB
        std::map<int, domain::CfmCategory_Table> mapCategoriesList;
        std::map<int, domain::CfmCategory_Table> mapCategoriesPriority;

        //Riempe la lista degli Adapter gestiti da SARA
        bool getAdapterList();

        //Riempe la lista delle regioni gestite da SARA
        bool getRegionList(int regionId = 0);

        //Lista dei sistemi configurati su DB
        std::vector<domain::CfmAdapter_Table>                       vAdapterList;

        //Lista delle regioni configurati su DB
        std::vector<domain::CfmRegion_Table>                        vRegionList;

        //Metodi per la lettura completa di tabelle
        //Chiave: LocalDll
        std::map<std::string, domain::CfmLicense_Table>				getLicenses();
        //Chiave EventId | DeviceId
        std::map<std::string, domain::CfmEventRules_Table>			getEventRules();
        //Chiave RuleId
        std::map<int, CListaAzioni>						    getActionRules();
        //Chiave Event Id
        std::map<int, domain::CfmEvent_Table>						getEvents();
        //Chiave Action idAction
        std::map<int, domain::CfmAction_Table>						getActions();
        //Chiave Action Name|Cat
        std::map<std::string, domain::CfmAction_Table>				getActionsByNameCat();
        //Chiave SIDUID | SubId
        std::map<std::string, domain::CfmDevices_Table>			    getDevices();
        //chiave Id
        std::map<int, domain::CfmRules_Table>						getRules();

        //Chiave Alias
        std::map<std::string, domain::CfmCategory_Table>			getCategories();

        //Chiave Section|Name
        std::map<std::string, domain::CfmSystemParameters_Table>	getSystemParameters(int iSysId);
        //Chiave Section|Name
        std::map<std::string, domain::CfmAdapterParameters_Table>	getAdapterParameters(int id);
        //Chiave IdRule|IdAction|ParameterAlias
        std::map<std::string, domain::CfmActionParametersJoin_Table*>  getRulesParameters();

        // Chiave StatoSid
        std::map<int, std::string> getSystemStates();
        std::string StateSIDToString(int StatoSid);

        //Chiave IdAction (Tabella anagrafica dei parametri)
        std::map<int, domain::CfmActionParameters_Table*> getActionParameters();

        //Chiave EventAndId
        std::map<std::string, domain::CfmEventAndRules_Table> getEventAndRules();
        //Chiave EventOrId
        std::map<std::string, domain::CfmEventOrRules_Table>  getEventOrRules();

        std::vector<domain::CfmEventAnd_Table>                getEventAnd();
        std::vector<domain::CfmEventOr_Table>                 getEventOr();

        //Recupera le devices in base a sottosistema e categoria
        std::vector<domain::CfmDevices_Table>				getDevices(int iSubId, std::string sCat);
        //std::vector<CfmDevices_Table>			getDevices(int iSubId, int iCat);
        std::vector<domain::CfmDevices_Table>				getDevices(int iSubId, int iCat, long iSiteId = 0);
        //std::map<std::string, CfmDevices_Table>	getDevices(int iSystemId, int iCat, int iSiteId);
        domain::CfmDevices_Table* getDevice(int idDevice);
        unsigned long								getIdDevice(int SystemId, std::string iSubId);
        bool										setDevicesExecuting(int SystemId);
        unsigned long								getMaxDeviceId(int SystemId, int iCat);

        //std::map<int, CfmSite_Table *>				getSite(bool forceFromDB=true);
        //std::map<std::string, CfmSite_Table>		getMapSite(bool forceFromDB=false);

        //std::map<std::string, CfmSite_Table>::iterator findMapSite(int SystemId, int SubSystemId);
        //std::map<std::string, CfmSite_Table>::iterator endMapSite();
        //short										getSiteIdFromName(std::string SiteName);
        std::map<std::string, domain::CfmSite_Table>		getMapSiteByName();
        int 										DeleteDevicesBySite(int idSid, int category, long siteId);

        //Gestione tabelle di sistema
        int DeleteTable(std::string sTableName);
        int DeleteTable(std::string sTableName, std::string TSFieldName, int days); //19 03 2008
        int ExecuteSyncQuery(std::string query, bool mutex = true); //19 03 2008
        int ExecuteQueryOnAsyncConnection(std::string query);

        unsigned long getULongSyncQuery(std::string query);

        int InsertDevice(int iId, int iLogicalStatus, int iPhisicalStatus, int idCategory, std::string sDescription,
            int iIdSystemId, std::string sSubId, int iParentId, int siteId, int iProcessState = 3,
            std::string sDateTime = "");
        int UpdateDevice(int iId, int iPhisicalStatus, int iLogicalStatus, int idCategory,
            std::string sDescription, int iIdSystemId,
            std::string sSubId, int iParentId, int iSiteId, int iProcessState = 3,
            std::string sDateTime = "", domain::stRecDelete bDelete = recNoDeleted, int iExecuting = 0);

        int UpdateExecuting(int IdDevice, int iExecuting);
        int SetAttributeForDevice(int iId, std::string AttributeName, std::string AttributeValue);

        bool InsertDeviceIntrusionZone(unsigned long IdDevice, unsigned long IdDeviceZone);
        bool DeleteDeviceIntrusionZone(unsigned long IdDevice);
        std::map<unsigned long, std::set<unsigned long>>  getDeviceIntrusionZone(int iSubId, long iSiteId);

        int SetPresetForCamera(unsigned long DeviceId, std::string presetName, int numPreset, std::string SubId);
        int GetPresetForCamera(unsigned long DeviceId, std::string PresetSubId);

        std::vector<domain::CfmCategoriesAttributes_Table> GetDeviceParameters(int iId);
        std::map<std::string, domain::CfmDevicesCategoriesAttributes_Table> GetDevicesCategoriesAttributes();

        int SetSiteForDevice(int iId, long iIdSite);
        int InsertSite(int SystemId, int SubSystemId, bool SiteWatched, std::string Name, std::string Description, std::string ModifiedTS = "");
        int UpdateSite(int SiteId, int SystemId, int SubSystemId, bool SiteWatched, std::string Name, std::string Description, std::string ModifiedTS = "");
        int DeleteSiteSystem(unsigned long SiteId, unsigned long SystemId);

        std::map<std::string, domain::CfmAlarmGroup_Table> getAlarmGroups(int SystemId, int SiteId);
        int InsertAlarmGroup(domain::CfmAlarmGroup_Table& alGrp);
        int UpdateAlarmGroup(unsigned long IdAlarmGroup, unsigned long DeviceId,
            int SystemId, int SiteId, domain::CfmAlarmGroup_Table& alGrp);

        int InsertScenarios(std::string Name, int SiteId, int ScenariosTemplateId);
        int InsertScenariosDevice(int id, unsigned long deviceId, int cellPos, int presetId = 0);
        int InsertDeviceScenario(unsigned long deviceId, int scenarioId);

        int											updatePerson(domain::CfmPerson_Table& person);
        int											updatePersonSubId(domain::CfmPerson_Table& person);
        std::map<unsigned long, domain::CfmPerson_Table>	getPersons(int SIDID = 0);
        domain::CfmPerson_Table							getPersonById(unsigned long Id);
        int											getPersonSubId(domain::CfmPerson_Table& person);
        unsigned long								getMaxPersonId();
        int											deletePersonSubId(domain::CfmPerson_Table& person, unsigned int SidId);
        int											deletePersonById(unsigned long id);

        int											updateBadge(domain::CfmBadge_Table& badge);
        int											updateBadgeSubId(domain::CfmBadge_Table& badge);
        std::map<unsigned long, domain::CfmBadge_Table>		getBadges(int SIDID = 0);
        domain::CfmBadge_Table								getBadgeById(unsigned long Id);
        int											getBadgeSubId(domain::CfmBadge_Table& badge);
        unsigned long								getMaxBadgeId();
        int											deleteBadgeSubId(domain::CfmBadge_Table& badge, unsigned int SidId);
        int											deleteBadgeById(unsigned long id);

        int											updateAccessRight(domain::CfmAccessRight_Table& accessRight);
        int											updateAccessRightSubId(domain::CfmAccessRight_Table& accessRight);
        std::map<unsigned long, domain::CfmAccessRight_Table> getAccessRight();
        domain::CfmAccessRight_Table						getAccessRightById(unsigned long Id);
        int											getAccessRightSubId(domain::CfmAccessRight_Table& accessRight);
        unsigned long								getMaxAccessRightId();
        int											deleteAccessRightSubId(domain::CfmAccessRight_Table& accessRight, unsigned int SidId);
        int											deleteAccessRightById(unsigned long id);
        int											UpdateAccessRightIdReader(domain::CfmAccessRight_Table& tAccessRight);
        int											UpdateAccessRightIdBadge(domain::CfmAccessRight_Table& tAccessRight);
        int											UpdateAccessRightSubId(domain::CfmAccessRight_Table& accessRight);
        int											getAccessRightIdBadge(domain::CfmAccessRight_Table& tAccessRight);
        int											getAccessRightIdReader(domain::CfmAccessRight_Table& tAccessRight);

        int											updateTimeSchedule(domain::CfmTimeSchedule_Table& tTimeSchedule);
        //int											updateTimeScheduleSubId(SaraTimeSchedule_Table &tTS);
        std::map<unsigned long, domain::CfmTimeSchedule_Table> getTimeSchedule();
        //int											getsTimeScheduleSubId(unsigned int Id, std::map<unsigned int,std::string> &mapSubId);
        domain::CfmTimeSchedule_Table						getTimeScheduleById(unsigned long Id);
        unsigned long								getMaxTimeScheduleId() { return getTableMaxId("TimeSchedule"); };

        int											updateTimeRange(unsigned long IdTS, std::map<unsigned long, domain::CfmTimeRange_Table>& mapTimeRange);
        int											getsTimeRange(unsigned long Id, std::map<unsigned long, domain::CfmTimeRange_Table>& mapTimeRange);
        unsigned long								getMaxTimeRangeId() { return getTableMaxId("TimeRange"); }

        //----------------------------------------------------------------------------
        // Global Table 
        //----------------------------------------------------------------------------
        unsigned long		getTableMaxId(std::string tableName);
        int					updateTableSubId(std::string tableName, std::string fieldName,
            unsigned int Id, std::map<unsigned int, std::string>& mapSubId);
        int					getsTableSubId(std::string tableName, std::string fieldName,
            unsigned int Id, std::map<unsigned int, std::string>& mapSubId);
        //----------------------------------------------------------------------------


        int  InsertRule(int iId, std::string sAlias);
        int  SetFailover(int idRegion, bool failover = true);
        bool GetUserPassword(std::string& user, std::string& password);
        int  InsertEventRule(int iEventId, int iDeviceId, int iRuleId);
        int  InsertActionRule(int iId, int iActionId, int iDeviceId, int iRuleId);
        int  InsertRuleParameter(int Id, int iIdRule, int iIdActionRule, int iIdParameter, std::string sValue);

        //Log di tracce e letture OCR
        void LogTrack(int DeviceId, std::string SystemTrackId, float x, float y, float z, float Azimuth, float Range, float Elevation, int EventId);
        void LogOCR(int DeviceId, std::string LicensePlate, std::string TrailerPlate, std::string Container1, std::string Container2, std::string Container3, std::string Container4, int EventId, std::string img_path_1, std::string img_path_2, std::string img_path_3, std::string img_path_4, std::string img_path_5, std::string img_path_6);
        //Log messaggi ed errori di sistema
        void LogMessage(std::string SourceTS, int iSourceId, int iSeverity, std::string msgType, std::string msg);
        //Log event
        void LogEvent(unsigned int& idEventLog, int idEvent/*std::string sEventAlias*/, int iDevice, std::string Details, std::string sSourceTS = "20070101 00:00:00.000");
        //Log Command
        void LogCommand(long iDevice, int iUser, int iAction, std::string sMessage);

        void LogEventAnd(int idEventAndLog, int iEventOpId, int iEvent, int iDevice, std::string Details, std::string sSourceTS = "20070101 00:00:00.000");
        void LogEventOr(int idEventOrLog, int iEventOpId, int iEvent, int iDevice, std::string Details, std::string sSourceTS = "20070101 00:00:00.000");

        void DeleteLogEventAnd(int EventOpId, int DeviceId, int EventId);
        void DeleteLogEventAnd(int EventOpId, std::string sTS);
        void DeleteLogEventOr(int EventOpId, int DeviceId, int EventId);
        void DeleteLogEventOr(int EventOpId, std::string sTS);

        //LogAction
        void LogAction(int idActionLog, std::string sKernelTS, int idAction, int idRuleLog);
        //LogRule
        void LogRule(int idRuleLog, std::string sStartTS, int idRule, int idEventLog);
        //Update Action Log
        void UpdateAction(int idActionLog, int iExecutionTime, std::string sExecutedTS);
        //UpdateRule Log
        void UpdateRule(int idRuleLog, std::string ExecutedTS, int ExecutionTime, bool Succeded);

        //Update System stauts (0 Running, 1 Not Running)
        void UpdateSystemStatus(int SIDUID, int Status);

        //Update Phisical Device Status
        void UpdatePhisicalDeviceStatus(int iDeviceId, short int stat, std::string datetime, short int processState = 3);
        //Update Logical Device Status
        void UpdateLogicalDeviceStatus(int iDeviceId, short int state, std::string datetime);

        //Update Delete Device Status
        void UpdateDeleteDeviceStatus(int iDeviceId, domain::stateRecDelete state);

        bool UpdateSyncPhisicalDeviceStatus(int iDeviceId, short int state, std::string datetime);
        bool UpdateSyncLogicalDeviceStatus(int iDeviceId, short int state, std::string datetime);

        void EraseOldRowsRequest();

        //Per l'inizializzazione dei contatori
        unsigned int GetMaxEventLogId();
        unsigned int GetMaxRuleLogId();
        unsigned int GetMaxActionLogId();

        unsigned int GetMaxEventOrLogId();
        unsigned int GetMaxEventAndLogId();
        //unsigned int GetMaxEventOpLogId(std::string NameTableEvent, std::string NameIdEvent );
        unsigned int GetMaxEventLogId(std::string NameTableEvent, std::string NameIdEvent, std::string DatetimeTS);

        void DbClean();

        int  GetThreadId() { return m_ThreadCtx.m_dwTID; }
        int  GetRegionFromSIDUID(int SIDUID);
        bool setIsMaster(int id);

    private:
        //----------------------
        //  METODI PRIVATI
        //----------------------
        DBManager();
        ~DBManager();

        static std::atomic<DBManager*>  smInstance;
        static std::mutex               m_mutex;
        bool                            bTerminated;
        bool                            bIsMaster;

        std::unique_ptr<nanodbc::connection> syncConn;
        std::unique_ptr<nanodbc::connection> asyncConn;

        char logBuffer[4096];
        std::string	ValidateStringDB(std::string strValue, int len = 0);

        int ExecuteSyncQueryNoMutex(std::string query);

        //Gestione errori Database
        bool ManageSQLError(SQLRETURN retcode, int SQLHandleType, SQLHSTMT hstmt, SQLHDBC hdbc = NULL);

        void ManageSQLError(const std::string& message, const std::string& code, int res, const std::string& query);

        //void ManageSQLError(std::wstring& message, std::wstring& code, int res, std::wstring query = L"");
        bool ManageAsyncSQLError(SQLRETURN retcode, int SQLHandleType, SQLHSTMT asyncHstmt);

        //Varibili globali per la connessione ODBC (letture e scritture sincrone)
        SQLHENV henv;
        SQLHDBC hdbc;

        //Varibili globali per la connessione ODBC (scritture asincrone)
        SQLHENV asyncHenv;
        SQLHDBC asyncHdbc;

        //Statement utilizzata per i metodi asincroni
        SQLRETURN  asyncRetcode;
        SQLPOINTER asyncRgbValue;

        //Connection string for ODBC
        std::string sConnectionString;
        std::string sDSNString;
        std::string sUser;
        std::string sPwd;

        //class LockedMutex {
        //    void* mtx;

        //public:
        //    LockedMutex(void* m);
        //    ~LockedMutex();
        //};

        // Mappa dei siti controllati dal SARA (costruita dai siti ritornati dai vari SID)
        //std::map<int, SaraSite_Table *>			mapSiteList;
        //std::map<std::string, SaraSite_Table>	mapSiteSearch;

        // Mappa SIDUID / RegionId
        std::map<int, int> mapSIDUIDRegion;

        //Metodi utilizzati per la scrittura dal thread
        void SaveOCR(domain::CfmLogOCR_Table* record);
        void SaveTrack(domain::CfmLogTrack_Table* record);
        //Metodo utilizzato per la schittura dal thread
        void SaveLogMessage(domain::CfmLog_Table* record);
        //Metodo utilizzato per la scrittura sulla EventLog
        void SaveEventLog(domain::CfmEventLog_Table* record); //Ritorna il numero di riga

        //Metodo utilizzato per la scrittura sulla EventAndLog
        void SaveEventAndLog(domain::CfmEventAndLog_Table* record); //Ritorna il numero di riga

        //Metodo utilizzato per la scrittura sulla EventOrLog
        void SaveEventOrLog(domain::CfmEventOrLog_Table* record); //Ritorna il numero di riga

        //Metodo utilizzato per la scrittura sulla ActionLog
        void SaveActionLog(domain::CfmActionLog_Table* record); //Ritorna il numero di riga
        //Metodo utilizzato per la scrittura sulla RuleLog
        void SaveRuleLog(domain::CfmRuleLog_Table* record);
        //
        void SaveCommandLog(domain::CfmCommandLog_Table* record);
        //Update action log
        void UpdateActionLog(domain::CfmActionLog_Table* record);
        //Update rule log
        void UpdateRuleLog(domain::CfmRuleLog_Table* record);
        //Update System
        void SaveSystemStatus(domain::CfmSystem_Table* record);
        void SavePhisicalDeviceStatus(domain::CfmDevices_Table* record);
        void SaveLogicalDeviceStatus(domain::CfmDevices_Table* record);
        void SaveSystemMaintenanceReq(domain::CfmSystem_Table* record);
        void EraseOldRows();

        void SaveDeleteDeviceStatus(domain::CfmDevices_Table* record);

        //Tables cache
        std::map<int, domain::CfmEvent_Table>           mapEvents;
        std::map<int, domain::CfmAction_Table>			mappaAzioni;
        std::map<std::string, domain::CfmAction_Table>	mappaAzioniByNameCat;
        std::map<int, std::string>				mapSystemStates;

        int asyncErrCounter;
        bool ConnectDb();
        void ReleaseDb();

        //typedef VOID (CALLBACK* TIMERPROC)(HWND, UINT, UINT_PTR, DWORD);
        static VOID CALLBACK MyTimerProc(
            HWND hwnd,        // handle to window for timer messages 
            UINT message,     // WM_TIMER message 
            UINT_PTR idTimer,     // timer identifier 
            DWORD dwTime);

        static VOID CALLBACK TimerMinute(
            HWND hwnd,        // handle to window for timer messages 
            UINT message,     // WM_TIMER message 
            UINT_PTR idTimer,     // timer identifier 
            DWORD dwTime);

    };

}
