/****************************** Module Header ******************************\
* Module Name:  Logger.h
* Project:      S000
*
*
* Thread class implements runnable with message queue...
*
\***************************************************************************/
#pragma once

#include <string>
#include "Thread.h"
#include "CallbackTypes.h"



namespace cfm::application {

	class CLogger : public CThread {
	public:

		static const int DEBUG_LEVEL_VERY_HIGH = 5;
		static const int DEBUG_LEVEL_HIGH = 4;
		static const int DEBUG_LEVEL_MEDIUM = 3;
		static const int DEBUG_LEVEL_LOW = 2;
		static const int DEBUG_LEVEL_VERY_LOW = 1;

		//Ritorna il puntatore al singleton
		static CLogger* createInstance(HWND hw, int debugLevel);
		static CLogger* getInstance();

		HWND hDlgW;

		void Log(char* msg, int debugLevel = DEBUG_LEVEL_VERY_LOW, int sidId = 0);
		void Log(wchar_t* msg, int debugLevel = DEBUG_LEVEL_VERY_LOW, int sidId = 0);
		void Log(std::string msg, int debugLevel = DEBUG_LEVEL_VERY_LOW, int sidId = 0);
		void LogCriticalMessage(std::string msg);
		void Log(SystemMessage* sm, bool critical = false);

		void Log(SystemError* se);
		void LogPair(char* p1, char* p2);
		void LogPair(char* p1, int   p2);

		void BindEventManagerId(int iEventManagerId);

		bool bLogOnFile;
		bool bLogOnDB;
		bool bLogOnGUI;

		// Gestione msg SQL error 
		bool bLogSQLOnFile;
		void LogSQL(char* msg, int debugLevel = DEBUG_LEVEL_VERY_LOW, int sidId = 0);

	private:
		CLogger(HWND hw, int debugLevel);
		~CLogger();
		static CLogger* smInstance;
		bool NewLogFile();
		SYSTEMTIME logDate;

		std::string sFilePath;
		int EventManagerId;

		int iDebugLevel;
		FILE* fpLog;
		std::string sLogMsgFileName;

		// Gestione msg SQL error 
		SYSTEMTIME logSQLDate;
		FILE* fpSQL;
		std::string sLogSQLFileName;
		std::string sSQLFilePath;
		bool NewSQLLogFile();
		void LogMessageSQL(char* msg, int sidId = 0, bool critical = false);


		//Interfaccia Thread
		DWORD Run(LPVOID /* arg */);

		void LogMessage(char* msg, bool critical = false);
		void LogMessage(char* msg, int sidId, bool critical = false);
		void LogMessage(wchar_t* msg, bool critical = false);
		void LogMessage(wchar_t* msg, int sidId, bool critical = false);

	};
}