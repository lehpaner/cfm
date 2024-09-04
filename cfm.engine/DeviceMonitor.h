/****************************** Module Header ******************************\
* Module Name:  DeviceMonitor.h
* Project:      S000
*
*
* Device monitor is thread watching the state of devices managed with engine
*
\***************************************************************************/
#pragma once
#include "framework.h"
#include "Timers.h"
#include "CfmHandlesMaps.h"

#include "CallbackTypes.h"
#include "StateMachine.h"
#include "Thread.h"

#define SM_LOG_BUFLEN				1024            /**< Massima lunghezza delle stringhe di logging */

namespace cfm::application {
	//Struttura che contiene lo stato di salute del CFM
	enum CfmStates { ALIVE = 0, CRITICAL };

	struct CfmStatus {
		CfmStates status;
		std::vector<int> vSid;
	};

	typedef enum enumOperationMode { 
		opConfig, 
		opRunning, 
		opUpdate 
	} tpOperationMode;

	//void notifyStatusCallbak(int SIDUID, SIDExecStates status) {
	//	return;
	//};

#define DEVICERANGE		50000  //DA GESTIRE NEL DATABASE (analizzare con Luca)

	enum RangeCategory {
		rcDI = DEVICERANGE,
		rcDO = DEVICERANGE * 2,
		rcTRK = DEVICERANGE * 3,
		rcALMMGR = DEVICERANGE * 4,
		rcREADER = DEVICERANGE * 5,
		rcDVR = DEVICERANGE * 6,
		rcVS = DEVICERANGE * 7,
		rcPTZ = DEVICERANGE * 8,
		rcZONE = DEVICERANGE * 9,
		rcCAMERA = DEVICERANGE * 10,
		rcCDC = DEVICERANGE * 11,
		//.... per le altre categorie 
	  // OSC 16/09/2009 Fine
		rcULTIMO = DEVICERANGE * 20
	};

	//#define SIDRANGE 	rcULTIMO*DEVICERANGE
#define SIDRANGE 	rcULTIMO

/**
  * @class  CDeviceMonitor
  * @brief  Monitorizza lo stato dei driver installati come moduli del kernel
  *
  */
	class CDeviceMonitor : public CThread, public CStateMachine <CDeviceMonitor> {
		typedef enum {
			IDLE,
			WAIT_CLOSE_ALL,
			WAIT_ONE_PING,
			WAIT_DATA_ACQ,
			ACTIVE,
			CLOSING,

			SID_MONITOR_STATES_NO
		} SID_MONITOR_STATES;

		/*! \enum SID_MONITOR_TIMERS
		 *  Elenco dei timers utilizzati dal processo.
		 */
		typedef enum {
			TIMER_WATCHDOG = 0,             /**< ID del TIMER utilizzato per il polling */
			TIMER_VERIFY,                   /**< ID del TIMER utilizzato per avviare la procedura di verifica dello stato del SID */
			TIMER_ONE_PING,

			SID_MONITOR_TIMERS_NO
		} SID_MONITOR_TIMERS;

		TTimersThread* timer;
		/* --------------------------------------------------------------------------------------- */
		/* State Machine Actions: BEGIN ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
		/* --------------------------------------------------------------------------------------- */
		void idleStart();
		void anyNotIdleStart();
		void idleStartAll();
		void idleAutosense();
		void idleUpdate();
		void anyUpdateSid();
		void startUpdateData();
		void idleStopAll();
		void idleStop();
		void waitCloseAllCloseAll();
		void activeNotifyStatus();
		void activeMonTimer();
		void activeMonVerTimer();
		void activePingResp();
		void activeStopAll();
		void activeStop();
		void waitOnePingPingResp();
		void waitOnePingTimeout();
		void closingNotifyStatus();
		/* --------------------------------------------------------------------------------------- */
		/* State Machine Actions: END ------------------------------------------------------------ */
		/* --------------------------------------------------------------------------------------- */
		bool InitAllSID(tpOperationMode operationMode);
		void RegisterSID(int SIDUID, CfmDeviceHandleMap* SIDData);
		bool InitSID(tpOperationMode operationMode, int SIDUID, std::string SiteName = "");
		bool CloseSID(int SIDUID);
		void StartSID(int SIDUID);
		void StopSID(int SIDUID);
		//Avvia tutti i SID registrati
		void StartAllSID();
		//Arresta tutti i SID registrati
		void StopAllSID();
		void PingAllSID();
		void VerifyAllSID();
		void CloseAllSID();
		bool SIDalive();
		bool SIDalive(int SIDUID);
		bool SIDexists();
		bool SIDexists(int SIDUID);
		static void sidMonTo(unsigned int timer, unsigned int param, void* ptrParam);

	public:
		//Metodi per il caricamento e l'istanziemnto dei driver SID
		unsigned long getParentId(int sourceCatId, std::string sourceId);
		bool LoadLocalSid(tpOperationMode operationMode, domain::CfmSystem_Table* System, bool& bInitOk, std::string SiteName = "");
		bool LoadRemoteSid(tpOperationMode operationMode, domain::CfmSystem_Table* System, bool& bInitOk);
		bool RegisterSID(domain::CfmSystem_Table* System, bool bInitOk);
		bool CheckLicense(domain::CfmSystem_Table* System);
		//-------------------

		MSG msg;

		static CDeviceMonitor* getInstance(int regionId, bool create = false);
		static void destroyInstances();
		static bool existsInstance(int regionId);
		static void destroyInstance(int id);
		static void PingCallback(int SIDUID);

		DWORD Run(LPVOID /* arg */);

		void req_start_all();
		void req_stop_all();
		void req_autosense();
		void req_update_sid(UPDATE_MESSAGE* pCom, void* p);	   //Aggiorna configurazione devices SID su DB
		void req_update();								       //Aggiorna configurazione di tutti i SID sul DB
		void req_close_all();							       //Richiesta di terminazione del processo SARA (srk.exe) o stop del servizio
		void notify_region_restored(int SARARegionId);
		void req_start_sid(int SIDUID, void* p);
		void req_stop_sid(int SIDUID, void* p);
		//SASA GestionePersone
		void req_modify_data(int SIDUID, void* p);

		//DIEGO 06 04 2010
		CfmStatus req_status(); //richiesta di stato (metodo bloccante)

		int GetThreadId(); //Id del thread di gestione del DeviceMonitor

		std::map<int, CfmDeviceHandleMap> GetSIDMap() {
			std::map<int, CfmDeviceHandleMap> hMap;
			std::map<int, CfmDeviceHandleMap*>::iterator it;
			CfmDeviceHandleMap h;

			h.Status = StatusInitializing;

			for (it = hSIDMap.begin(); it != hSIDMap.end(); it++)
				if (it->second)
					hMap[it->first] = *it->second;
				else
					hMap[it->first] = h;

			return hMap;
		}

		void req_sidm_terminate();

		// Serve a nascondere quella di CThread, altrimenti si rischia di chiamarla più volte o mai.
		DWORD Start(void* arg = NULL) {
			return 0;
		}
		std::string getState() {
			switch (state) {
			case IDLE:
				return "IDLE";
			case WAIT_CLOSE_ALL:
				return "WAIT_CLOSE_ALL";
			case WAIT_ONE_PING:
				return "WAIT_ONE_PING";
			case ACTIVE:
				return "ACTIVE";
			case WAIT_DATA_ACQ:
				return "WAIT DATA ACQ";
			case CLOSING:
				return "CLOSING";

			default:
				return "";
			}
		}

	private:

		bool bTerminated;
		bool terminated;
		/**
		* Map of the instances Device Monitor 
		*/
		static std::map <int, CDeviceMonitor*> smInstance;  
		int myRegionId;
		//Hashtable (Chiave: SIDUID)
		std::map<int, CfmDeviceHandleMap*> hSIDMap;	/**< Mappa dei puntatori agli oggetti SID (indice: SIDUID configurato sul DB) */
		std::map<int, std::string> mapSystemStates;	/**< Mappa degli alias degli stati dei SID (indice: stato del SID) */
		bool updating;

		CDeviceMonitor(int id);
		~CDeviceMonitor();

		char logBuffer[SM_LOG_BUFLEN];  /**< Array di caratteri utilizzati per il logging */

		//Questo contatore serve per calcolare quando eliminare le righe obsolete dal database
		int OldRowCycleCounter;

		//Buffer utilizzato per i messaggi di log
		char logBuf[512];			  /**< Char array used for the message logging */

		char* cMachineHardwareKey;     /**< Contains the SARA Kernel host hardware key */
		std::map<std::string, CfmDLLHandleMap*> hSystemMap;

		std::set <int> sidsToStopSet;

		bool CaricaScenari(tpOperationMode operationMode, CSID* ifSID, std::string NameSite);
	};

} //end namsepace
