/****************************** Module Header ******************************\
* Module Name:  Logger.cpp
* Project:      S000
*
* Provides a implementation class for a service that will exist as main of a service
* application. CFMService is derived from base service.
*
\***************************************************************************/
#include "framework.h"
#include "Logger.h"
//#include "DatabaseManager.h"
//#include "BaseEngine.h"
#include "Messages.h"
#include "CategoryTypes.h"
#include <cassert>

#define MAX_BUFFER_LEN	2048

namespace cfm::application {
	CLogger* CLogger::smInstance = NULL;

	CLogger::CLogger(HWND hw, int debugLevel) {
		bLogSQLOnFile = false;//(saraRegCfg.LOG_ON_FILE()) ? true : false;
		bLogOnFile = true;//(saraRegCfg.LOG_ON_FILE()) ? true : false;
		bLogOnDB = false;//(saraRegCfg.LOG_ON_DB()) ? true : false;
		bLogOnGUI = false;// (saraRegCfg.LOG_ON_GUI()) ? true : false;
		EventManagerId = 0;
		fpLog = NULL;
		fpSQL = NULL;

		hDlgW = hw;

		sFilePath = "C:\\work\\cfmLog.log";//std::string(saraRegCfg.LOG_FILE_PATH());
		sSQLFilePath = "C:\\work\\cfmSqlLog.log";//std::string(saraRegCfg.LOG_FILE_PATH());

		NewLogFile();

		NewSQLLogFile();

		//Impostazione del debug level
		if (debugLevel <= DEBUG_LEVEL_VERY_HIGH && debugLevel >= DEBUG_LEVEL_VERY_LOW)
			iDebugLevel = debugLevel;
		else
			iDebugLevel = DEBUG_LEVEL_VERY_HIGH;
	}

	CLogger::~CLogger()
	{
		if (fpSQL != NULL)
			fclose(fpSQL);

		if (fpLog != NULL)
			fclose(fpLog);
	}

	CLogger* CLogger::createInstance(HWND hw, int debugLevel) {

		if (smInstance == NULL)
			smInstance = new CLogger(hw, debugLevel);

		return smInstance;
	}

	CLogger* CLogger::getInstance() {
		return smInstance;
	}

	//Critical Message
	void CLogger::LogCriticalMessage(std::string msg) {
		if (!msg.empty()) {
			char* logBuffer = new char[msg.size() + 1];
			memset(logBuffer, 0, msg.size() + 1);
			strncpy(logBuffer, msg.c_str(), msg.size());
			PostThreadMessage(this->m_ThreadCtx.m_dwTID, SL_LOG_CRITICAL_MESSAGE, (WPARAM)logBuffer, NULL);
		}
	}

	//Pubblica (LOG di stringhe con debug level - Utilizzata dal ThEventManager)
	void CLogger::Log(char* msg, int debugLevel, int sidId) {
		if (debugLevel > iDebugLevel)
			return;

		assert(msg);
		size_t msgLen = strlen(msg);
		char* logBuffer = new char[msgLen + 1];
		memset(logBuffer, 0, msgLen + 1);
		strncpy(logBuffer, msg, msgLen);
		PostThreadMessage(this->m_ThreadCtx.m_dwTID, SL_LOG_KRN_MESSAGE, (WPARAM)logBuffer, (LPARAM)sidId);
	}

	//Pubblica (LOG di stringhe con debug level - Utilizzata dal ThEventManager)
	void CLogger::Log(wchar_t* msg, int debugLevel, int sidId) {
		if (debugLevel > iDebugLevel)
			return;

		assert(msg);

		//wchar_t* logBuffer = new char[_wstrlen(msg) + 1];
		//wstrncpy_s(logBuffer, msg);
		//PostThreadMessage(this->m_ThreadCtx.m_dwTID, SL_LOG_KRN_MESSAGE, (WPARAM)logBuffer, (LPARAM)sidId);
	}

	void CLogger::Log(std::string msg, int debugLevel, int sidId) {
		if (debugLevel > iDebugLevel)
			return;
		size_t msgS = msg.size();
		if (!msg.empty()){
			char* logBuffer = new char[msgS + 1];
			memset(logBuffer, 0, msgS + 1);
			strncpy(logBuffer, msg.c_str(), msgS);
			PostThreadMessage(this->m_ThreadCtx.m_dwTID, SL_LOG_KRN_MESSAGE, (WPARAM)logBuffer, (LPARAM)sidId);
		}
	}

	void CLogger::LogPair(char* p1, int p2) {
		char* logBuffer = new char[MAX_BUFFER_LEN];
		memset(logBuffer, 0, MAX_BUFFER_LEN);
		sprintf(logBuffer, "%s = %i", p1, p2);
		PostThreadMessage(this->m_ThreadCtx.m_dwTID, SL_LOG_KRN_MESSAGE, (WPARAM)logBuffer, 0);
	}

	void CLogger::LogPair(char* p1, char* p2) {
		char* logBuffer = new char[MAX_BUFFER_LEN];
		memset(logBuffer, 0, MAX_BUFFER_LEN);
		sprintf(logBuffer, "%s = %s", p1, p2);
		PostThreadMessage(this->m_ThreadCtx.m_dwTID, SL_LOG_KRN_MESSAGE, (WPARAM)logBuffer, 0);
	}

	//Pubblica (OVERLOAD del metodo Log, LOG di messaggi dai SID)
	void CLogger::Log(SystemMessage* sm, bool critical) {
		if ((sm->Severity > iDebugLevel) && !critical) {
			delete sm;
			return;
		}
		try {
			char* logBuffer = new char[MAX_BUFFER_LEN];
			memset(logBuffer, 0, MAX_BUFFER_LEN);
			sprintf_s(logBuffer, MAX_BUFFER_LEN, "UID:[%i] CNT:[%i] SRC:[%s] DBG:[%i] TSTAMP:[%s] SIDMSG:[%s]",
				sm->SIDUID,
				sm->msgCount,
				sm->Source.c_str(),
				sm->Severity,
				sm->sysTime.c_str(),
				sm->AppDescription.c_str());

			if (!critical)
				PostThreadMessage(this->m_ThreadCtx.m_dwTID, SL_LOG_SID_MESSAGE, (WPARAM)logBuffer, sm->SIDUID);
			else
				PostThreadMessage(this->m_ThreadCtx.m_dwTID, SL_LOG_CRITICAL_MESSAGE, (WPARAM)logBuffer, sm->SIDUID);
			delete sm; //allocato dalla callback legata al SID
		} catch (std::exception& e) {
			e.what();
			if (sm)
				delete sm;
		}
	}
				


	//Pubblica (OVERLOAD del metodo Log, LOG di errori dai SID)
	void CLogger::Log(SystemError* se) {
		try {
			char* logBuffer = new char[MAX_BUFFER_LEN];
			memset(logBuffer, 0, MAX_BUFFER_LEN);
			sprintf_s(logBuffer, MAX_BUFFER_LEN, "SYSERR: UID:[%i] CNT:[%i] SRC:[%s] DBG:[%i] TSTAMP:[%s] SIDERR:[%s] - [%s]",
				se->SIDUID,
				se->msgCount,
				se->Source.c_str(),
				se->Severity,
				se->sysTime.c_str(),
				se->AppDescription.c_str(),
				se->NativeDescription.c_str());
			PostThreadMessage(this->m_ThreadCtx.m_dwTID, SL_LOG_SID_ERROR, (WPARAM)logBuffer, se->SIDUID);
			delete se; //allocato dalla callback legata al SID
		} catch (std::exception& e) {
			//e.what();
			if (se)
				delete se;
		}
	}


	//Privata
	void CLogger::LogMessage(char* msg, bool critical) {
		LogMessage(msg, 0, critical);
	}

	//Privata
	void CLogger::LogMessage(char* msg, int sidId, bool critical) {
		//Timestamp is
		SYSTEMTIME st; // = new SYSTEMTIME(); 
		GetLocalTime(&st);
		char szLocalDate[255], szLocalTime[255], szDateTime[255];
		GetDateFormatA(LOCALE_USER_DEFAULT, 0, &st, "dd-MM-yyyy", szLocalDate, 255);
		GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &st, "HH:mm:ss", szLocalTime, 255);
		sprintf(szDateTime, "%s %s.%03i", szLocalDate, szLocalTime, st.wMilliseconds);

		char* guiMsg = new char[MAX_BUFFER_LEN];
		sprintf(guiMsg,  "[%s] %s", szDateTime, msg);

		//FILE
		if (bLogOnFile || critical) {
#ifdef DEBUG_LOG
			if (st.wMinute != logDate.wMinute)
#else
			if (st.wDay != logDate.wDay)
#endif
				NewLogFile();

			if (fpLog)
			{
				fprintf(fpLog, "%s \n", guiMsg);
				fflush(fpLog);
			}
		}
		//DATABASE
		if (bLogOnDB || critical) {
			GetDateFormatA(LOCALE_USER_DEFAULT, 0, &st, "yyyyMMdd", szLocalDate, 255);
			GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &st, "HH:mm:ss", szLocalTime, 255);
			sprintf(szDateTime, "%s %s.%i", szLocalDate, szLocalTime, st.wMilliseconds);
			//DBManager::getInstance()->LogMessage(szDateTime, 0, 10, "SYSMSG", std::string(msg));
		}
		//La memoria va liberata qui per ottimizzare le allocazioni in questa parte di codice
		delete[] msg;

		//GUI
		/*if (bLogOnGUI || critical)  {
			printf("%s \n", guiMsg);
			PostThreadMessage(idThSaraMonitor, SARA_MONITOR_LOG, (WPARAM)guiMsg, sidId);
		}
		else
			delete[] guiMsg;*/
	}

	//Pubblica
	//Pubblica (LOG di stringhe con debug level - Utilizzata dal ThEventManager)
	void CLogger::LogSQL(char* msg, int debugLevel, int sidId) {
		if (debugLevel > iDebugLevel)
			return;

		assert(msg);

		char* logBuffer = new char[strlen(msg) + 1];
		strcpy(logBuffer, msg);
		PostThreadMessage(this->m_ThreadCtx.m_dwTID, SL_LOG_SQL_ERROR, (WPARAM)logBuffer, (LPARAM)sidId);
	}

	//Privata
	void CLogger::LogMessageSQL(char* msg, int sidId, bool critical) {
		//Timestamp is
		SYSTEMTIME st; // = new SYSTEMTIME(); 
		GetLocalTime(&st);
		char szLocalDate[255], szLocalTime[255], szDateTime[255];
		GetDateFormatA(LOCALE_USER_DEFAULT, 0, &st, "dd-MM-yyyy", szLocalDate, 255);
		GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &st, "HH:mm:ss", szLocalTime, 255);
		sprintf_s(szDateTime, sizeof(szDateTime), "%s %s.%03i", szLocalDate, szLocalTime, st.wMilliseconds);

		////FILE
		if (bLogSQLOnFile || critical) {
			if (st.wDay != logSQLDate.wDay)
				NewSQLLogFile();

			if (fpSQL) {
				fprintf(fpSQL, "--- %s \n", szDateTime);
				fprintf(fpSQL, "%s \n", msg);
				fflush(fpSQL);
			}
		}
		//La memoria va liberata qui per ottimizzare le allocazioni in questa parte di codice
		delete[] msg;
	}

	//---------------------------------------------
	// RUN
	//---------------------------------------------
	DWORD CLogger::Run(LPVOID /* arg */) {
		MSG msg;

		//Main Loop
		for (;;) {
			GetMessage(&msg, NULL, 0, 0);

			switch (msg.message) {
			//QUERY SQL MESSAGE
			case SL_LOG_SQL_ERROR:
			{
				char* strBuffer = (char*)msg.wParam;
				assert(strBuffer);

				LogMessageSQL(strBuffer, msg.lParam);
			}
			break;

			//KERNEL MESSAGE
			case SL_LOG_KRN_MESSAGE:
			{
				char* strBuffer = (char*)msg.wParam;
				assert(strBuffer);
				LogMessage(strBuffer, 0, msg.lParam);
			}
			break;

			case SL_LOG_CRITICAL_MESSAGE:
			{
				char* strBuffer = (char*)msg.wParam;
				assert(strBuffer);
				LogMessage(strBuffer, 0, true);
			}
			break;

			//SID MESSAGE
			case SL_LOG_SID_MESSAGE:
			{
				char* strBuffer = (char*)msg.wParam;
				assert(strBuffer);
				LogMessage(strBuffer, 0, msg.lParam);
				//Pekmez  LogMessage(strBuffer, msg.lParam);
			}
			break;

			//SID MESSAGE
			case SL_LOG_SID_ERROR:
			{
				char* strBuffer = (char*)msg.wParam;
				assert(strBuffer);
				LogMessage(strBuffer, 0, msg.lParam);
				//LogMessage(strBuffer, msg.lParam);
			}
			break;

			//LOG FLUSH
			case SL_FLUSH:
			{
				if (fpSQL)
					fflush(fpSQL);

				fflush(fpLog);
//Pekmez				PostThreadMessage(CBaseEngine::getInstance()->GetThreadId(), TERMINATE_REQ, 0, 0);
			}
			break;
			}
		}
	}

	void CLogger::BindEventManagerId(int iEventManagerId) {
		EventManagerId = iEventManagerId;
	}


	bool CLogger::NewLogFile() {
		std::string logFile;
		char szLocalDate[255];

		GetLocalTime(&logDate);
		GetDateFormat(LOCALE_USER_DEFAULT, 0, &logDate, "dd-MM-yyyy", szLocalDate, 255);

		int pos = sFilePath.rfind('.');
		if (pos > 0)
#ifdef DEBUG_LOG
		{
			char szLocalTime[255], szDateTime[255];
			GetTimeFormat(LOCALE_USER_DEFAULT, 0, logDate, L"HH-mm-ss", szLocalTime, 255);
			sprintf(szDateTime, "%s_%s", szLocalDate, szLocalTime);
			logFile = sFilePath.substr(0, pos) + "_" + std::string(szDateTime) + sFilePath.substr(pos);
		}
#else
			logFile = sFilePath.substr(0, pos) + "_" + std::string(szLocalDate) + sFilePath.substr(pos);
#endif
		else
			logFile = sFilePath.substr(0, pos) + "_" + std::string(szLocalDate) + ".log";

		if (fpLog && (sLogMsgFileName != logFile)) {
			fflush(fpLog);
			fclose(fpLog);
			fpLog = NULL;
		}

		if (!fpLog)
			if (!(fpLog = fopen(logFile.c_str(), "a+"))){
				std::string msg = std::string("Invalid path for log file :") + logFile;
				MessageBox(NULL, msg.c_str(), "ERROR", MB_OK);
				LogMessage((char*)msg.c_str());
				return false;
			}
			else
				sLogMsgFileName = logFile;

		return true;
	}

	bool CLogger::NewSQLLogFile() {
		std::string logFile;
		char szLocalDate[255];

		GetLocalTime(&logSQLDate);
		GetDateFormat(LOCALE_USER_DEFAULT, 0, &logSQLDate, "dd-MM-yyyy", szLocalDate, 255);

		int pos = sSQLFilePath.rfind('.');
		if (pos > 0)
			logFile = sSQLFilePath.substr(0, pos) + "_" + std::string(szLocalDate) + sSQLFilePath.substr(pos);
		else
			logFile = sSQLFilePath.substr(0, pos) + "_" + std::string(szLocalDate) + ".sql";

		if (fpSQL && (sLogSQLFileName != logFile)) {
			fflush(fpSQL);
			fclose(fpSQL);
			fpSQL = NULL;
		}

		if (!fpSQL)
			if (!(fpSQL = fopen(logFile.c_str(), "a+"))) {
				std::string msg = std::string("Invalid path for log SQL-Error file :") + logFile;
				MessageBox(NULL, msg.c_str(), "ERROR", MB_OK);
				LogMessage((char*)msg.c_str());
				return false;
			}
			else
				sLogSQLFileName = logFile;

		return true;
	}


} //end namespace
