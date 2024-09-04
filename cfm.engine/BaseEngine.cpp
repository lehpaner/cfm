/****************************** Module Header ******************************\
* Module Name:  BaseEngine.cpp
* Project:      S000
*
*
* Base Engine class that is base class for different types of engine
*
\***************************************************************************/
#include "BaseEngine.h"
#include "Messages.h"
#include "shellapi.h"
#include "tchar.h"
#include "ConfigParams.h"

#include "ApplicationEngine.h"
#include "ConsoleEngine.h"
#include "UserEngine.h"
#include "ComunicationAdapterMonitor.h"

void _MBOX(wchar_t* sMessage, wchar_t* sCaption) {
#ifdef _MBOX_ 
	MessageBox(NULL, sMessage, sCaption, MB_OK);
#endif
}
//---------------------------------------------------------------------
static bool ParseStandardArgs(int argc, char* argv) {
	return false;
}
extern cfm::application::CConfigParms  cfmRegCfg;
//---------------------------------------------------------------------

namespace cfm::application {

	bool CBaseEngine::terminate = false;
	std::map<int, CBaseEngine*> CBaseEngine::smInstance;
	std::set<int> CBaseEngine::regionIdSet;
	std::set<CBaseEngine::MSG_FOR_OTHER*> CBaseEngine::regionSet;
	int CBaseEngine::tid = 0;
	MSG CBaseEngine::msg;
	CLogger* CBaseEngine::sLogger = nullptr;
	DBManager* CBaseEngine::DBM = nullptr;
	CEventManager* CBaseEngine::srk = nullptr;
	CConfigParms saraRegCfg;

	CBaseEngine::CBaseEngine(int id) {
		std::map<int, CBaseEngine*>::iterator it;
		it = smInstance.find(id);
		assert(it == smInstance.end());
		smInstance[id] = this;
		regionId = id;
		regionIdSet.insert(id);
		pSidMon = nullptr;
	}

	CBaseEngine::~CBaseEngine() {
		regionIdSet.erase(regionId);
	}

	CBaseEngine* CBaseEngine::getInstance(int id) {
		std::map<int, CBaseEngine*>::iterator it;
		it = smInstance.find(id);
		assert(it != smInstance.end());
		return it->second;
	}

	CBaseEngine* CBaseEngine::getInstance() {
		return getInstance(cfmRegCfg.SARA_REGION_ID());
	}

	int CBaseEngine::getInstancesSize() {
		return smInstance.size();
	}

	void CBaseEngine::destroyInstance(int id) {
		std::map<int, CBaseEngine*>::iterator it;

		if ((it = smInstance.find(id)) != smInstance.end()) {
			delete it->second;
			smInstance.erase(it);
		}
	}

	void CBaseEngine::destroyInstances() {
		std::map<int, CBaseEngine*>::iterator it;

		while ((it = smInstance.begin()) != smInstance.end()) {
			delete it->second;
			smInstance.erase(it);
		}
	}

	CBaseEngine* CBaseEngine::init(HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow) {
		tid = GetCurrentThreadId();
		int regionId = saraRegCfg.SARA_REGION_ID();

		if (strcmp((char*)lpCmdLine, "-s") == 0) {
			PostThreadMessage(tid, MAIN_START, 0, regionId);
			return CAEngine::getInstance(regionId);
		}
		// CAEngine::getInstance(id);
		// Create the service object
		int argc;
		LPWSTR* argv;
		argv = CommandLineToArgvW(GetCommandLineW(), &argc);

		// Parse for standard arguments (install, uninstall, version etc.)
		//Pekmez _MBOX(L"Opening", L"SRM::start()");

		if (!ParseStandardArgs(argc, lpCmdLine)) {
			if ((argc > 2) && wcscmp((wchar_t*)argv[1], L"-env") == 0) {
				bool r = (bool)::SetEnvironmentVariableW(L"SARA_PATH2", argv[2]);
				exit(!r);
			}
			// if you want to start SARA as an application
			if (argc > 1 && ((strcmp((char*)lpCmdLine, "-a") == 0))) {
				AllocConsole();
				SetConsoleTitle("CFM 1.0.0 Console");
				freopen("CONOUT$", "wb", stdout);  // reopen stout handle as console window output
				freopen("CONOUT$", "wb", stderr); // reopen stderr handle as console window output
				//Per disabilitare la chiuisura della finestra console
				//EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE),SC_CLOSE ,MF_GRAYED);
				//DrawMenuBar(GetConsoleWindow());

				PostThreadMessage(tid, MAIN_START, 0, regionId);
				//Pekmez return getAModeInstance(regionId);
				return CAEngine::getInstance(regionId);
			} else if (argc > 1 && strcmp((char*)lpCmdLine, "-c") == 0) {
				AllocConsole();
				freopen("CONOUT$", "wb", stdout);  // reopen stout handle as console window output
				freopen("CONOUT$", "wb", stderr); // reopen stderr handle as console window output

				//Per disabilitare la chiuisura della finestra console
				//EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE),SC_CLOSE ,MF_GRAYED);
				//DrawMenuBar(GetConsoleWindow());

				SetConsoleTitle("CFM 1.0.0 Console");
				printf("BASE ENGINE SENDING START EVENT [%i] TO MACHINE... \n", (int)MAIN_START);
				PostThreadMessage(tid, MAIN_START, 0, regionId);
				auto application_engine= CConsoleEngine::getInstance(regionId);
				return application_engine;
			} else if (argc > 1 && strcmp((char*)lpCmdLine, "-update") == 0) {
				AllocConsole();
				freopen("CONOUT$", "wb", stdout);  // reopen stout handle as console window output
				freopen("CONOUT$", "wb", stderr); // reopen stderr handle as console window output

				//Per disabilitare la chiuisura della finestra console
				//EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE),SC_CLOSE ,MF_GRAYED);
				//DrawMenuBar(GetConsoleWindow());

				SetConsoleTitle("SARA 2.0.0 Console");

				PostThreadMessage(tid, MAIN_START, 0, regionId);
				return CUserEngine::getInstance(regionId);
				// return getUModeInstance(regionId);
			} else if (argc > 1 && strcmp((char*)lpCmdLine, "-k") == 0) {
				//Generazione della chiave hardware per l'installazione del sistema SARA
				const wchar_t* s = L"778guigiu445566";// GetSARAHWKey();

				FILE* fp = fopen("SaraHWKey.txt", "w");
				fprintf(fp, "%s", s);
				fclose(fp);

				MessageBoxW(NULL, s, L"SARA HW Code", MB_OK);
			} else if (argc == 6 && wcscmp((wchar_t*)argv[1], L"-GL") == 0) {
				//Generazione licenza
				char sDllNameMb[200]; memset(sDllNameMb, 0, 200);
				char sInstances[20]; memset(sInstances, 0, 20);
				char sExpirationDateMb[10]; int cbExpDate; memset(sExpirationDateMb, 0, 10);
				int cb = wcstombs(sDllNameMb, argv[2], 200);
				if (cb != -1) {
					std::string sDllName = std::string(sDllNameMb);
					cb = wcstombs(sInstances, argv[3], 20);
					cbExpDate = wcstombs(sExpirationDateMb, argv[5], 8);
					std::string sExpirationDate = std::string(sExpirationDateMb);
					if (cb != -1 && cbExpDate != -1) {
						int iInstances = atoi(sInstances);
						char cHWKey[200];

						cb = wcstombs(cHWKey, argv[4], 20);
						std::string sLicense = ""; // GenerateLicenseKey(sDllName, iInstances, cHWKey);

						std::wstring sLicenseAndDate = L""; // AddExpirationDate(sLicense, sExpirationDate);

						FILE* fp = fopen("SaraLicense.txt", "w");
						fprintf(fp, "%s", sLicenseAndDate.c_str());
						fclose(fp);

						MessageBoxW(NULL, sLicenseAndDate.c_str(), L"License", MB_OK);
					}
				}
			}
			else  // otherwise you wish to start it as a windows service
			{
				//srvSARA.applInstance = hInstance;
				//srvSARA.StartService();
			}
		}
		// When we get here, the service has been stopped
		//Pekmez  _MBOX(L"Closing", L"SRM::start()");

		return NULL;
	}

	void CBaseEngine::startDB() {
		// Creo il DBManager e lo avvio
		DBM = DBManager::getInstance();
		DBM->SetConnectionParams(saraRegCfg.DBCONNECTION());
		DBM->Start();
		Sleep(1000); // Forse non serve...provare
		DBM->Connect();
	}

	int CBaseEngine::getEvent() {
		return msg.message;
	}

	/*! \brief Attende messaggio e restituisce oggetto
	* relativo ad lParam (id della regione).
	* Se lParam == 0, usa come default l'id della regione
	* della macchina locale. In tale modo chi deve inviare
	* messaggi all'automa principale (MainAMode) non ha
	* necessita' di impostare il parametro.
	*
	* @return      puntatore ad oggetto automa, altrimenti NULL
	*/
	CBaseEngine* CBaseEngine::receive() {
		terminate = !GetMessage(&msg, NULL, 0, 0);

		if (terminate)
			return NULL;

		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (!msg.lParam)
			msg.lParam = saraRegCfg.SARA_REGION_ID();

		return getInstance(msg.lParam);
	}

	/*! \brief Ritorna l'eventuale automa da far girare
	* inserito nel set.
	*
	* Verifica se il set contiene un MSG_FOR_OTHER ed in caso
	* positivo copia i parametri nel messaggio MSG msg.
	* Successivamente effettua il "delete" dell'elemento
	* MSG_FOR_OTHER e lo elimina dal set.
	*
	* @return puntatore ad oggetto automa, altrimenti NULL
	*/
	CBaseEngine* CBaseEngine::forOtherStateMachines() {
		std::set<MSG_FOR_OTHER*>::iterator it;

		if ((it = CBaseEngine::regionSet.begin()) == regionSet.end())
			return NULL;

		msg.message = (*it)->message;
		msg.lParam = (*it)->lParam;
		msg.wParam = (*it)->wParam;

		delete* it;

		regionSet.erase(it);

		return getInstance(msg.lParam);
	}

	bool CBaseEngine::sendToOtherStateMachine(int message, WPARAM wParam, LPARAM lParam) {
		std::pair<std::set<MSG_FOR_OTHER*>::iterator, bool> ret;
		ret = regionSet.insert(new MSG_FOR_OTHER(message, wParam, lParam));
		return ret.second;
	}

	void CBaseEngine::sendToAllOtherStateMachines(int message, WPARAM wParam, int id) {
		std::map<int, CBaseEngine*>::iterator it;

		for (it = smInstance.begin(); it != smInstance.end(); it++) {
			if (it->second->regionId != id)
				sendToOtherStateMachine(message, wParam, it->second->regionId);
		}
	}

	void CBaseEngine::printConf(enumOperationMode opMode) {
		char logBuf[512];

		//creo l'istanza del log
		sLogger = CLogger::createInstance(NULL, 0/*saraRegCfg.DBG_LEVEL()*/);
		if (sLogger)
			sLogger->Start(); //Avvio il thread del logger

		//Intestazione del Log
		sLogger->Log("**************************", CLogger::DEBUG_LEVEL_VERY_LOW); //Lo logga sempre
		sLogger->Log("SARA MAIN - RUNNING ", CLogger::DEBUG_LEVEL_VERY_LOW); //Lo logga sempre
		sLogger->Log("**************************", CLogger::DEBUG_LEVEL_VERY_LOW); //Lo logga sempre
		sLogger->Log("", CLogger::DEBUG_LEVEL_VERY_LOW); //Lo logga sempre

		/*sLogger->LogPair("DSN", (char*)saraRegCfg.DSN().c_str());
		sLogger->LogPair("DBUSER", (char*)saraRegCfg.DBUSER().c_str());
		sLogger->LogPair("DBPASSWORD", (char*)saraRegCfg.DBPASSWORD().c_str());
		sLogger->LogPair("DBG_LEVEL", saraRegCfg.DBG_LEVEL());
		sLogger->LogPair("LOG_ON_DB", saraRegCfg.LOG_ON_DB());
		sLogger->LogPair("LOG_ON_FILE", saraRegCfg.LOG_ON_FILE());
		sLogger->LogPair("LOG_ON_GUI", saraRegCfg.LOG_ON_GUI());
		sLogger->LogPair("LOG_FILE_PATH", (char*)saraRegCfg.LOG_FILE_PATH().c_str());
		sLogger->LogPair("ACTION_MONITOR_TIME", saraRegCfg.ACTION_MONITOR_TIME());
		sLogger->LogPair("ACTION_MONITOR_TIMEOUT", saraRegCfg.ACTION_MONITOR_TIMEOUT());
		sLogger->LogPair("SID_PING_TIME", saraRegCfg.SID_PING_TIME());
		sLogger->LogPair("SID_VERIFY_TIME", saraRegCfg.SID_VERIFY_TIME());
		sLogger->LogPair("MAX_GUI_LOG_LINES", saraRegCfg.MAX_GUI_LOG_LINES());
		sLogger->LogPair("RESTORE_AFTER_SCAN", saraRegCfg.RESTORE_AFTER_SCAN());*/

		if (opMode == 1/*opConfig*/) {
			sLogger->Log("", CLogger::DEBUG_LEVEL_VERY_LOW); //Lo logga sempre
			sLogger->Log("--------------------------", CLogger::DEBUG_LEVEL_VERY_LOW); //Lo logga sempre
			sLogger->Log("       CONFIG MODE ", CLogger::DEBUG_LEVEL_VERY_LOW); //Lo logga sempre
			sLogger->Log("--------------------------", CLogger::DEBUG_LEVEL_VERY_LOW); //Lo logga sempre
			sLogger->Log("", CLogger::DEBUG_LEVEL_VERY_LOW); //Lo logga sempre

			int rBox = MessageBox(NULL, "Are yor sure to start config mode?", "CFM Main [SRM]", MB_YESNO | MB_SYSTEMMODAL);
			if (rBox == IDNO)
			{
				Beep(500, 2000);
				sLogger->Log("[SRM] Config mode cancelled by the user", CLogger::DEBUG_LEVEL_VERY_LOW);
				exit(0);
			}
			else
			{
				sLogger->Log("[SRM] Config mode accepted by the user", CLogger::DEBUG_LEVEL_VERY_LOW);
			}
		} else if (opMode == 2 /*opUpdate*/) {
			sLogger->Log("", CLogger::DEBUG_LEVEL_VERY_LOW); //Lo logga sempre
			sLogger->Log("--------------------------", CLogger::DEBUG_LEVEL_VERY_LOW); //Lo logga sempre
			sLogger->Log("       UPDATE MODE ", CLogger::DEBUG_LEVEL_VERY_LOW); //Lo logga sempre
			sLogger->Log("--------------------------", CLogger::DEBUG_LEVEL_VERY_LOW); //Lo logga sempre
			sLogger->Log("", CLogger::DEBUG_LEVEL_VERY_LOW); //Lo logga sempre

			int rBox = MessageBox(NULL, "Are yor sure to start update mode?", "CFM Main [SRM]", MB_YESNO | MB_SYSTEMMODAL);
			if (rBox == IDNO)
			{
				Beep(500, 2000);
				sLogger->Log("[SRM] Update mode cancelled by the user", CLogger::DEBUG_LEVEL_VERY_LOW);
				exit(0);
			}
			else
			{
				sLogger->Log("[SRM] Update mode accepted by the user", CLogger::DEBUG_LEVEL_VERY_LOW);
			}
		}

	}

	void CBaseEngine::cleanDB() {
		int r = DBM->DeleteTable("RuleParameters");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting RuleParameters table"); //Lo logga sempre
		else sLogger->Log("[SRM] RuleParameters table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("ScenariosDevices");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting ScenariosDevices table"); //Lo logga sempre
		else sLogger->Log("[SRM] ScenariosDevices table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("EventRules");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting EventRules table"); //Lo logga sempre
		else sLogger->Log("[SRM] EventRules table deleted"); //Lo logga sempre

		// OSC 10/07/2009
		r = DBM->DeleteTable("EventAnd");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting EventAnd table"); //Lo logga sempre
		else sLogger->Log("[SRM] EventAnd table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("EventAndRules");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting EventAndRules table"); //Lo logga sempre
		else sLogger->Log("[SRM] EventAndRules table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("EventOr");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting EventOr table"); //Lo logga sempre
		else sLogger->Log("[SRM] EventOr table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("EventOrRules");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting EventOrRules table"); //Lo logga sempre
		else sLogger->Log("[SRM] EventOrRules table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("ActionLog");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting ActionLog table"); //Lo logga sempre
		else sLogger->Log("[SRM] ActionLog table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("ActionRules");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting ActionRules table"); //Lo logga sempre
		else sLogger->Log("[SRM] ActionRules table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("Rules");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting Rules table"); //Lo logga sempre
		else sLogger->Log("[SRM] Rules table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("AlarmGroupDevices");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting AlarmGroupDevices table"); //Lo logga sempre
		else sLogger->Log("[SRM] AlarmGroupDevices table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("AlarmGroup");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting AlarmGroup table"); //Lo logga sempre
		else sLogger->Log("[SRM] AlarmGroup table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("DevicesCategoriesAttributes");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting DevicesCategoriesAttributes table"); //Lo logga sempre
		else sLogger->Log("[SRM] DevicesCategoriesAttributes table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("Devices");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting Devices table"); //Lo logga sempre
		else sLogger->Log("[SRM] Devices table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("ViewSite");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting ViewSite table"); //Lo logga sempre
		else sLogger->Log("[SRM] ViewSite table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("MapsLayers");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting MapsLayers table"); //Lo logga sempre
		else sLogger->Log("[SRM] MapsLayers table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("Maps");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting Maps table"); //Lo logga sempre
		else sLogger->Log("[SRM] Maps table deleted"); //Lo logga sempre

		r = DBM->ExecuteSyncQuery("DELETE FROM SiteSystem");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting SiteSystem table"); //Lo logga sempre
		else sLogger->Log("[SRM] SiteSystem table deleted"); //Lo logga sempre

		r = DBM->ExecuteSyncQuery("DELETE FROM Site WHERE Id!=1");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting Site table"); //Lo logga sempre
		else sLogger->Log("[SRM] Site table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("DeviceScenario");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting DeviceScenario table"); //Lo logga sempre
		else sLogger->Log("[SRM] DeviceScenario table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("Presets");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting Presets table"); //Lo logga sempre
		else sLogger->Log("[SRM] Presets table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("ScenariosDevices");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting ScenariosDevices table"); //Lo logga sempre
		else sLogger->Log("[SRM] ScenariosDevices table deleted"); //Lo logga sempre

		r = DBM->DeleteTable("Scenarios");
		if (r == -1)
			sLogger->Log("[SRM] Error deleting Scenarios table"); //Lo logga sempre
		else sLogger->Log("[SRM] Scenarios table deleted"); //Lo logga sempre
	}

	bool CBaseEngine::startOther(enumOperationMode opMode) {
		bool failover = false;

		//Recupero la lista dei SID dal database (tabella: Systems)
		DBM->getSystemList();
		//Recupero la lista delle Categories dal database (tabella: Categories)
		DBM->getCategoriesPriority();

		//Cancellazione di tutte le regole e di tutte le devices dal database
		//if (opMode == opConfig)
		//	cleanDB();

		CComunicationAdapterMonitor::getInstance()->Start();

		int regionId = saraRegCfg.SARA_REGION_ID();

		//// Avvio del ThSIDMonitor
		CDeviceMonitor* thSidMon = CDeviceMonitor::getInstance(regionId, true);

		switch (opMode) {
		case opRunning:
		//	// Il thKernel deve essere il primo thread dato che è il dispatcher
			srk = new CEventManager();
			srk->Start();

//Pekmez			SaraMonitor::getInstance()->Start(NULL);

		//	//Action Logic
		//	actionLogic = new ActionLogic(); //Istanzio l'oggetto
		//	actionLogic->Init();             //Lo inizializzo ora che il dbmanager è pronto

		//	//Avvio l'Action Monitor
		//	ActionMonitor::getInstance()->Start();

		//	//Recupero la lista delle regioni dal database (tabella: SARARegion)
			DBM->getRegionList();

			for (int i = 0, id; i < DBM->vRegionList.size(); i++)
				if ((id = DBM->vRegionList[i].Id) != regionId) {
					if (DBM->isMaster()) {
//Pekmez						getMasterInstance(id);
						sendToOtherStateMachine(MAIN_START, (WPARAM)&DBM->vRegionList[i], (LPARAM)id);
					}
				}
				else
					failover = DBM->vRegionList[i].Failover;

			if (!failover)
				thSidMon->req_start_all();
			break;

		case opConfig:
			thSidMon->req_autosense();
			break;

		case opUpdate:
			thSidMon->req_update();
			break;
		}
		return !failover;
	}


} //end namespace


/***

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	CSaraMain *p;

	if (!(p = CSaraMain::init(hInstance, lpCmdLine, nCmdShow)))
		return 0;

	while (!CSaraMain::terminate)
	{
		if (CSaraMain::terminate || !(p = CSaraMain::receive()))
			continue ;

		p->executeStateMachine(p->getEvent());

		while (p = CSaraMain::forOtherStateMachines())
		{
			p->executeStateMachine(p->getEvent());
		}
	}
	CSaraMain::destroyInstances();
	//std::cout << "terminato main !!!"<<std::endl;
	//std::cout.flush();
	return 0;
}

**/