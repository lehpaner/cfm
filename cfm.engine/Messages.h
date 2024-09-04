/****************************** Module Header ******************************\
* Module Name:  Messages.h
* Project:      S000
*
*
* Operational messages definition 
*
\***************************************************************************/
#pragma once

#define CFM_MESSAGES_BASE                  0x7FFF //(/*WM_USER + */ )

#define CFMMESSAGERANGE                    1000

#define CFMMAIN_MESSAGES_BASE               (CFM_MESSAGES_BASE + CFMMESSAGERANGE * 0)
#define SIDMONITOR_MESSAGES_BASE            (CFM_MESSAGES_BASE + CFMMESSAGERANGE * 1)
#define COMMADAPTERMONITOR_MESSAGES_BASE    (CFM_MESSAGES_BASE + CFMMESSAGERANGE * 2)
#define CFMTYPES_MESSAGES_BASE              (CFM_MESSAGES_BASE + CFMMESSAGERANGE * 3)
#define ACTIONMONITOR_MESSAGES_BASE         (CFM_MESSAGES_BASE + CFMMESSAGERANGE * 4)
#define DBMANAGER_MESSAGES_BASE             (CFM_MESSAGES_BASE + CFMMESSAGERANGE * 5)
#define CFMLOG_MESSAGES_BASE                (CFM_MESSAGES_BASE + CFMMESSAGERANGE * 6)
#define REGMONITOR_MESSAGES_BASE            (CFM_MESSAGES_BASE + CFMMESSAGERANGE * 7)

namespace cfm::application {
    /*! \enum SARAMAIN_MESSAGES
 *  Elenco messaggi ricevuti dagli automi.
 */
    typedef enum
    {
        DUMMY_BOY = CFMMAIN_MESSAGES_BASE,
        MAIN_START,
        DB_READY,
        DB_NOT_READY,
        SIDMON_READY,
        SIDMON_NOT_READY,
        SIDMON_TIMEOUT,
        SIDMON_OFF,
        UPDATE_REQ,
        UPDATE_DONE,
        SUSPEND,
        RESUME,
        POLLING,
        TERMINATE_REQ,
    } CFMMAIN_MESSAGES;


    /*! \enum SIDMONITOR_MESSAGES
     *  Elenco messaggi ricevuti dall'automa.
     */
    typedef enum
    {
        SIDMON_START = SIDMONITOR_MESSAGES_BASE,    //WPARAM = SIDUID
        SIDMON_START_ALL,
        SIDMON_AUTOSENSE,
        SIDMON_CLOSE_ALL,
        SIDMON_STOP,                    //WPARAM = SIDUID
        SIDMON_STOP_ALL,
        SIDMON_UPDATE_SID,              //WPARAM = SIDUID
        SIDMON_ONE_PING_TIMEOUT,
        SIDMON_SET_PERSON,

        EVENT_PING_RESP,                /**< Evento di risposta al PING */
        EVENT_SIDMON_TIMER,             /**< Inviato dal timer che controlla il polling */
        EVENT_SIDMON_VERIFY_TIMER,      /**< Inviato dal timer che controlla la procedura la procedura di verifica dello stato del SID */
        EVENT_SYSTEM_UNDER_MAINTENANCE,
        REQ_MANAGE_REGION,              //WPARAM = REGIONID
        REQ_RESTORE_REGION,             //WPARAM = REGIONID
        NOT_REGION_RESTORED,            //WPARAM = REGIONID
        REQ_UPDATE_RESTART_SID,         //WPARAM = SIDUID
        REQ_CLOSE_SID,                  //WPARAM = NULL	
        SIDMON_UPDATE

    } SIDMONITOR_MESSAGES;


    /*! \enum COMMADAPTERMONITOR_MESSAGES
     *  Elenco messaggi ricevuti dall'automa.
     */
    typedef enum
    {
        EVENT_CCOMM_PING_RESP = COMMADAPTERMONITOR_MESSAGES_BASE,
        /**< Evento di risposta al PING */
        EVENT_CCOMM_TIMER,              /**< Inviato dal timer che controlla il polling */
        EVENT_CCOMM_VERIFY_TIMER,       /**< Inviato dal timer che controlla la procedura la procedura di verifica dello stato del SID */
        EVENT_CCOMM_UNDER_MAINTENANCE,
        EVENT_PUSH_TRK,
        EVENT_PUSH_ALARM,
        EVENT_PUSH_DI,
        EVENT_PUSH_DO,
        EVENT_PUSH_ZONE,
        EVENT_PUSH_READER,
        TIMER_CCOMM_WATCHDOG,           /**< ID del TIMER utilizzato per il polling */
        TIMER_CCOMM_VERIFY,             /**< ID del TIMER utilizzato per avviare la procedura di verifica dello stato del SID */
        EVENT_PUSH_EXECUTING,
        EVENT_PUSH_MESSAGE,
        EVENT_PUSH_CAMERA,
        EVENT_PUSH_CDC
    } COMMADAPTERMONITOR_MESSAGES;


    /*! \enum ACTIONMONITOR_MESSAGES
     *  Elenco messaggi ricevuti dall'automa.
     */
    typedef enum
    {
        EVENT_ACTMON_TIMER = ACTIONMONITOR_MESSAGES_BASE,
        TIMER_ACT_WATCHDOG,
    } ACTIONMONITOR_MESSAGES;


    /*! \enum DBMANAGER_MESSAGES
     *  Elenco messaggi ricevuti dall'automa.
     */
    typedef enum
    {
        DB_INSERT_TRACK = DBMANAGER_MESSAGES_BASE,
        /**< Comando di inserimento nuova traccia nella tabella Tracks_Log */
        DB_INSERT_OCR,                  /**< Comando di inserimento nuovo record nella tabella OCR_Log */
        DB_INSERT_LOGMESSAGE,
        DB_INSERT_LOGEVENT,
        DB_INSERT_LOGACTION,
        DB_INSERT_LOGRULE,
        DB_UPDATE_LOGACTION,
        DB_UPDATE_LOGRULE,
        DB_INSERT_LOGCOMMAND,
        DB_UPDATE_SYSTEMSTATUS,
        DB_CONNECT,                     /**< Richiesta di connessione */
        DB_RECONNECT,                   /**< Richiesta di riconnessione */
        DB_RELEASE,                     /**< Richiesta di disconnessione */
        DB_CONNECT_TIMEOUT,             /**< Timeout per richiesta di connessione */
        DB_CLEAN,                       /**< Cancella DB */
        DB_UPDATE_PHISICALDEVICESTATUS,
        DB_UPDATE_MAINTENANCEREQ,
        DB_UPDATE_LOGICALDEVICESTATUS,
        DB_ERASE_OLD_ROWS,
        DB_INSERT_LOGEVENTAND,
        DB_INSERT_LOGEVENTOR,
        DB_EXECUTE_OPERATION,
        DB_UPDATE_DELETEDEVICE,
        DB_TIMER,
    } DBMANAGER_MESSAGES;


    /*! \enum SARALOG_MESSAGES
     *  Elenco messaggi ricevuti dall'automa.
     */
    typedef enum
    {
        WM_UPDATE_LOG = CFMLOG_MESSAGES_BASE,
        SL_LOG_KRN_MESSAGE,
        SL_LOG_SID_MESSAGE,
        SL_LOG_SID_ERROR,
        SL_FLUSH,
        SL_LOG_CRITICAL_MESSAGE, //Viene scritto sempre a prescindere dalla configurazione
        SL_LOG_SQL_ERROR,
    } CFMLOG_MESSAGES;


    /*! \enum REGMONITOR_MESSAGES
     *  Elenco messaggi ricevuti dall'automa.
     */
    typedef enum
    {
        REGMON_START = REGMONITOR_MESSAGES_BASE,
        REGMON_FAILOVER_TIMEOUT,
        REGMON_CONNECT,
        REGMON_CONNECT_FAILED,
        REGMON_POLLING_TIMEOUT,
        REGMON_SLAVE_RESPONSE,
        REGMON_RESPONSE_TIMEOUT,
        REGMON_MASTER_STOPPED,

    } REGMONITOR_MESSAGES;

}//end namespace