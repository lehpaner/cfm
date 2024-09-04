/****************************** Module Header ******************************\
* Module Name:  ComunicationAdapterMonitor.h
* Project:      S000
*
*
* Thread class implements monitor to device adapters...
*
\***************************************************************************/

#pragma once
#include "framework.h"
#include "Thread.h"
#include "CfmHandlesMaps.h"

#define SM_LOG_BUFLEN					1024            /**< Massima lunghezza delle stringhe di logging */

namespace cfm::application {
	class CComunicationAdapterMonitor : public CThread {
	public:
		static CComunicationAdapterMonitor* getInstance(); /**< Ritorna il puntatore la singleton */
		static void PingCallback(int CCommUID);

		static VOID CALLBACK PingTimerProc(
			HWND hwnd,        // handle to window for timer messages 
			UINT message,     // WM_TIMER message 
			UINT_PTR idTimer,     // timer identifier 
			DWORD dwTime);
		static VOID CALLBACK VerifyTimerProc(
			HWND hwnd,        // handle to window for timer messages 
			UINT message,     // WM_TIMER message 
			UINT_PTR idTimer,     // timer identifier 
			DWORD dwTime);

		DWORD Run(LPVOID /* arg */);

		//Avvia tutti i SID registrati
		void StartAllAdapters();
		//Arresta tutti i SID registrati
		void StopAllAdapters();
		void PingAllAdapters();
		void VerifyAllAdapters();
		int GetThreadId();

		//Hashtable di tutti gli adapter configurati sul database
		std::map<int, CfmCommAdapterHandleMap*> GetAdapterMap() { return hCommAdapterMap; }

		static bool ProcessAction(int tipo, int device, std::string action, std::map<std::string, std::string> parametri);
		//Pubblica perchè la deve usare anche l'ActionMonitor
		static void pushExecuting(int iDev, bool executing = true);

	private:
		CComunicationAdapterMonitor();                 /**< Costruttore */

		static CComunicationAdapterMonitor* smInstance;  /**< Puntatore all'instanza del singleton SID Monitor */
		//Hashtable (Chiave: SIDUID)
		std::map<int, CfmCommAdapterHandleMap*> hCommAdapterMap; /**< Mappa dei puntatori agli oggetti SID (indice: SIDUID configurato sul DB) */
		char logBuffer[SM_LOG_BUFLEN];            /**< Array di caratteri utilizzati per il logging */

		//Prima integrazione adapter MLD (da far diventare una map)
		void RegisterAdapter(int CCommUID, CfmCommAdapterHandleMap* CCommData);

		static void adapterMonTo(unsigned int timer, unsigned int param, void* ptrParam);

		bool pushDeviceEventWithLog(CComm* ccom, long idEvent, domain::CfmDevices_Table& p, char* methodName);

	};

} //end namespace
