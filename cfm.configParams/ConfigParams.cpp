/****************************** Module Header ******************************\
* Module Name:  ConfigParams.cpp
* Project:      S000
*
*
* Configuration params 4 engine run
*
\***************************************************************************/
#include "ConfigParams.h"
#include "SimpleIni.h"
#define SI_CONVERT_GENERIC 1
int str2int(std::string sNumber) {
	int iRetValue = 0;
	try {
		iRetValue = atoi(sNumber.c_str());
	}
	catch (...) {
		return -1;
	}
	return iRetValue;
}

namespace cfm::application {
	//--------------------------------------------------------

	CConfig::CConfig(const std::string& where) {
		//Caricamento dati dal file INI
		char fullPathName[512]; strcpy(fullPathName, "");
		char* enginePath = getenv("ENGINE_PATH");
		if (enginePath) strcat(fullPathName, enginePath);
		else strcat(fullPathName, where.c_str());
		strcat(fullPathName, "/config/cfm.ini");

		this->iMAX_EVENT_DAYS = 20; //Default value
		this->bDISPATCHSYSMESSAGES = false;
		this->iMAXPOSTBUFFER = 1000000;
		this->iHOUR = 1;
		this->iMINUTE = 0;

		this->iACTION_MONITOR_TIMEOUT = 60000;


		CSimpleIniA si(true, true, true);
		int r = si.LoadFile(fullPathName);
		if (!r) {
			//CFM
			if (si.GetValue("CFM", "CFM_ID"))
				iCfmRegionId = str2int(si.GetValue("CFM", "CFM_ID"));
			if (si.GetValue("CFM", "CFM_NAME"))
				sCfmRegionName = si.GetValue("CFM", "CFM_NAME");

			//DATABASE
			if (si.GetValue("DATABASE", "DSN"))
				sDSN = si.GetValue("DATABASE", "DSN");
			if (si.GetValue("DATABASE", "DBUSER"))
				sDBUSER = si.GetValue("DATABASE", "DBUSER");
			if (si.GetValue("DATABASE", "DBPASSWORD"))
				sDBPASSWORD = si.GetValue("DATABASE", "DBPASSWORD");

			//DEBUG
			if (si.GetValue("DEBUG", "DEBUG_LEVEL"))
				iDEBUG_LEVEL = str2int(si.GetValue("DEBUG", "DEBUG_LEVEL"));

			//LOGGER
			if (si.GetValue("LOGGER", "LOG_FILE_PATH"))
				sLOG_FILE_PATH = si.GetValue("LOGGER", "LOG_FILE_PATH");
			if (si.GetValue("LOGGER", "LOG_ON_DB"))
				bLOG_ON_DB = str2int(si.GetValue("LOGGER", "LOG_ON_DB"));
			if (si.GetValue("LOGGER", "LOG_ON_FILE"))
				bLOG_ON_FILE = str2int(si.GetValue("LOGGER", "LOG_ON_FILE"));
			if (si.GetValue("LOGGER", "LOG_ON_GUI"))
				bLOG_ON_GUI = str2int(si.GetValue("LOGGER", "LOG_ON_GUI"));

			//ACTION MONITOR
			if (si.GetValue("ACTION_MONITOR", "ACTION_MONITOR_TIME"))
				iACTION_MONITOR_TIME = str2int(si.GetValue("ACTION_MONITOR", "ACTION_MONITOR_TIME"));
			if (si.GetValue("ACTION_MONITOR", "ACTION_MONITOR_TIMEOUT"))
				iACTION_MONITOR_TIMEOUT = str2int(si.GetValue("ACTION_MONITOR", "ACTION_MONITOR_TIMEOUT"));

			// SONO DEL SID_MONITOR
			if (si.GetValue("ACTION_MONITOR", "SID_PING_TIME"))
				iSID_PING_TIME = str2int(si.GetValue("ACTION_MONITOR", "SID_PING_TIME"));
			if (si.GetValue("ACTION_MONITOR", "SID_VERIFY_TIME"))
				iSID_VERIFY_TIME = str2int(si.GetValue("ACTION_MONITOR", "SID_VERIFY_TIME"));
			if (si.GetValue("ACTION_MONITOR", "RESTORE_AFTER_SCAN"))
				bRESTORE_AFTER_SCAN = str2int(si.GetValue("ACTION_MONITOR", "RESTORE_AFTER_SCAN"));

			//SERVICE
			if (si.GetValue("SERVICE", "CHECKLOGIN_TIME"))
				iCHECKLOGIN_TIME = str2int(si.GetValue("SERVICE", "CHECKLOGIN_TIME"));
			if (si.GetValue("SERVICE", "TRYICONBEEP"))
				bTRAYICONBEEP = str2int(si.GetValue("SERVICE", "TRYICONBEEP"));
			if (si.GetValue("SERVICE", "MAX_EVENT_DAYS"))
				iMAX_EVENT_DAYS = str2int(si.GetValue("SERVICE", "MAX_EVENT_DAYS"));
			if (si.GetValue("SERVICE", "MAX_POST_BUFFER"))
				iMAXPOSTBUFFER = str2int(si.GetValue("SERVICE", "MAX_POST_BUFFER"));
			if (si.GetValue("SERVICE", "TIME_HOUR"))
				iHOUR = str2int(si.GetValue("SERVICE", "TIME_HOUR"));
			if (si.GetValue("SERVICE", "TIME_MINUTE"))
				iMINUTE = str2int(si.GetValue("SERVICE", "TIME_MINUTE"));

			//MAIN
			if (si.GetValue("MAIN", "SIDMON_TIME"))
				iSIDMON_TIME = str2int(si.GetValue("MAIN", "SIDMON_TIME"));

			//REGION_MONITOR
			if (si.GetValue("REGION_MONITOR", "FAILOVER_TIME"))
				iFAILOVER_TIME = str2int(si.GetValue("REGION_MONITOR", "FAILOVER_TIME"));
			if (si.GetValue("REGION_MONITOR", "POLLING_TIME"))
				iPOLLING_TIME = str2int(si.GetValue("REGION_MONITOR", "POLLING_TIME"));
			if (si.GetValue("REGION_MONITOR", "RESPONSE_TIME"))
				iRESPONSE_TIME = str2int(si.GetValue("REGION_MONITOR", "RESPONSE_TIME"));
			if (si.GetValue("REGION_MONITOR", "FAILOVER_PASSWORD"))
				sFAILOVER_PASSWORD = Decrypt(si.GetValue("REGION_MONITOR", "FAILOVER_PASSWORD"));
			//SID_MONITOR
			if (si.GetValue("SID_MONITOR", "ONE_PING_TIME"))
				iONE_PING_TIME = str2int(si.GetValue("SID_MONITOR", "ONE_PING_TIME"));

			if (si.GetValue("LOGGER", "DISPATCH_SYS_MESSAGES"))
				bDISPATCHSYSMESSAGES = str2int(si.GetValue("LOGGER", "DISPATCH_SYS_MESSAGES"));
		}
	}
	//----------------------------------------------------------

	CConfig::~CConfig(void) { }
	//----------------------------------------------------------

	std::string CConfig::Decrypt(std::string sEncryptedString) {
		char out[512];
		unsigned long len = sizeof(out) - 1;

		//if (!dbStringDataDecrypt((char*)sEncryptedString.c_str(), sEncryptedString.size(), out, &len))
			return "";

		sEncryptedString = ((char*)out);

		return sEncryptedString;
	}
	//----------------------------------------------------------
}