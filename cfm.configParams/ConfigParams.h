/****************************** Module Header ******************************\
* Module Name:  ConfigParams.h
* Project:      S000
*
*
* Configuration params 4 engine run
*
\***************************************************************************/
#pragma once

#include "framework.h"
#include "WinReg.h"
#include "singleton.h"

namespace cfm::application {

	class CConfig: public common::Singleton<CConfig> {

	private:

		std::string sDSN;
		std::string sDBUSER;
		std::string sDBPASSWORD;
		std::string sDBCONNECTION;
		int			iDEBUG_LEVEL;
		bool		bLOG_ON_DB;
		bool		bLOG_ON_FILE;
		bool		bLOG_ON_GUI;
		bool		bRESTORE_AFTER_SCAN;
		std::string	sLOG_FILE_PATH;
		int			iACTION_MONITOR_TIME;
		int			iACTION_MONITOR_TIMEOUT;
		int			iSID_PING_TIME;
		int			iSID_VERIFY_TIME;
		int			iMAX_GUI_LOG_LINES;
		int			iCHECKLOGIN_TIME;
		bool		bTRAYICONBEEP;
		bool		bIsMaster;
		int			iCfmRegionId;
		std::string sCfmRegionName;
		int         iSIDMON_TIME;
		int			iFAILOVER_TIME;
		int			iPOLLING_TIME;
		int			iRESPONSE_TIME;
		int			iONE_PING_TIME;
		std::string sFAILOVER_PASSWORD;
		int			iMAX_EVENT_DAYS;
		bool		bDISPATCHSYSMESSAGES;
		int			iMAXPOSTBUFFER;
		int			iHOUR;
		int			iMINUTE;
		HKEY		HKEY_ROOT;	 /**< registry root, e.g.HKEY_LOCAL_MACHINE					*/
		std::string	sParentKey;  /**< registry subfolder path, e.g. "/SOFTWARE/CFM/CFM/"	*/
		CWindowsRegister* saraCfg;

		std::string Decrypt(std::string sEncryptedString);
	public:
		CConfig(const std::string& home);
		~CConfig(void);
		std::string DSN() { return sDSN; };
		std::string DBUSER() { return sDBUSER; };
		std::string DBPASSWORD() { return sDBPASSWORD; };
		std::string DBCONNECTION() { return "Driver={Sql Server Native Client 11.0};Server=(localdb)\\MSSQLLocalDB;Trusted_Connection=yes;Database=SCHAFFHAUSEN"; };
		int			DBG_LEVEL() { return iDEBUG_LEVEL; };
		bool		LOG_ON_DB() { return bLOG_ON_DB; };
		bool		LOG_ON_FILE() { return bLOG_ON_FILE; };
		bool		LOG_ON_GUI() { return bLOG_ON_GUI; };
		std::string LOG_FILE_PATH() { return sLOG_FILE_PATH; };
		int			ACTION_MONITOR_TIME() { return iACTION_MONITOR_TIME; };
		int			ACTION_MONITOR_TIMEOUT() { return iACTION_MONITOR_TIMEOUT; };
		int			SID_PING_TIME() { return iSID_PING_TIME; };
		int			SID_VERIFY_TIME() { return iSID_VERIFY_TIME; };
		int			MAX_GUI_LOG_LINES() { return iMAX_GUI_LOG_LINES; };
		int			CHECKLOGIN_TIME() { return iCHECKLOGIN_TIME; };
		bool		RESTORE_AFTER_SCAN() { return bRESTORE_AFTER_SCAN; };
		bool		TRAY_ICON_BEEP() { return bTRAYICONBEEP; };
		std::string CFM_REGION_NAME() { return sCfmRegionName; };
		int			CFM_REGION_ID() { return iCfmRegionId; };
		int			SIDMON_TIME() { return iSIDMON_TIME; };
		int			FAILOVER_TIME() { return iFAILOVER_TIME; };
		int			POLLING_TIME() { return iPOLLING_TIME; };
		int			RESPONSE_TIME() { return iRESPONSE_TIME; };
		int			ONE_PING_TIME() { return iONE_PING_TIME; };
		std::string FAILOVER_PASSWORD() { return sFAILOVER_PASSWORD; };
		int			MAX_EVENT_DAYS() { return iMAX_EVENT_DAYS; };
		bool		DISPATCHSYSMESSAGES() { return bDISPATCHSYSMESSAGES; };
		int			MAXPOSTBUFFER() { return iMAXPOSTBUFFER; };
		int			HOUR() { return iHOUR; };
		int			MINUTE() { return iMINUTE; };
	};

} //end namespace