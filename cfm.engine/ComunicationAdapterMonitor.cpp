/****************************** Module Header ******************************\
* Module Name:  ComunicationAdapterMonitor.cpp
* Project:      S000
*
*
* Thread class implements monitor to device adapters...
*
\***************************************************************************/
#include "ComunicationAdapterMonitor.h"
#include "ConfigParams.h"
#include "DatabaseManager.h"
#include "Timers.h"
#include "Messages.h"
#include "Logger.h"


cfm::application::CComunicationAdapterMonitor* cfm::application::CComunicationAdapterMonitor::smInstance = nullptr;
namespace cfm::application {


	/**
	* Costruttore base
	*/
	CComunicationAdapterMonitor::CComunicationAdapterMonitor() {

		DBManager::getInstance()->getAdapterList();

		typedef std::vector<CfmAdapter_Table*> ALIST;
		using  adapterIt=std::vector<CfmAdapter_Table>::iterator;

		for (adapterIt it= DBManager::getInstance()->vAdapterList.begin(); it != DBManager::getInstance()->vAdapterList.end(); it++) {
			createObj COFun = NULL;
			destroyObj DEFun = NULL;

			//Caricamento DLL
			HMODULE hMod = LoadLibraryA((*it).DllPath.c_str());
			if (hMod) {
				//Costruttore oggetto (SID)
				COFun = (createObj)GetProcAddress(hMod, "CreateObject");
				//Distruttore oggetto (SID)
				DEFun = (destroyObj)GetProcAddress(hMod, "DestroyObject");
			}
			if (COFun) {
				CComm* myAdapter = (CComm*)COFun();

				std::map<std::string, CfmAdapterParameters_Table> mappaParametri = DBManager::getInstance()->getAdapterParameters((*it).Id);
				std::map<std::string, CfmAdapterParameters_Table>::iterator iParameter;
				for (iParameter = mappaParametri.begin(); iParameter != mappaParametri.end(); iParameter++) {
					myAdapter->SetCfgParameter(iParameter->second.Section, iParameter->second.Name, iParameter->second.Value);
				}
				myAdapter->SetUID((*it).Id);
				myAdapter->SetAlias((*it).Name);

//Pekmez				myAdapter->RegisterErrorCallback(errorCallback);
//Pekmez				myAdapter->RegisterMessageCallback(messageCallback);
				myAdapter->RegisterCmdCallback(ProcessAction);
				myAdapter->RegisterPingCallback(PingCallback);

				DWORD a = myAdapter->Start();

				//Registrazione dell'adapter
				bool watchdog = atoi(myAdapter->GetCfgParameter("WATCHDOG", "Enable").c_str());
				CfmCommAdapterHandleMap* sca = new CfmCommAdapterHandleMap(watchdog);
				sca->bInitOk = true;
				sca->CommAdapterIf = myAdapter;
				sca->Status = 1;
				RegisterAdapter((*it).Id, sca);

				if (watchdog)
					sca->timer->setTimeout(0, atoi(myAdapter->GetCfgParameter("WATCHDOG", "Seconds").c_str()) * SECS, adapterMonTo, (unsigned int)this, sca);
			}
		}
	}
	/**
	* @fn		ThCommAdapterMonitor::Run(LPVOID)
	* @brief	Crea (se la prima volta che viene chiamato) o ritorna l'istanza del singleton
	* @author Diego Gangale
	* @date	15 04 2010 Created
	* @return Il puntatore all'istanza del singleton
	*
	*/
	CComunicationAdapterMonitor* CComunicationAdapterMonitor::getInstance() {

		if (smInstance == NULL)
			smInstance = new CComunicationAdapterMonitor();

		return smInstance;
	}

#define PREPARE_DATA\
    if(!msg.wParam) break;\
    CfmDevices_Table sdt;\
    CfmDevices_Table *p =(CfmDevices_Table *) msg.wParam;\
    sdt = *p; //Copia dei dati....

	/**
	  * @fn ThCommAdapterMonitor::Run(LPVOID)
	  * @brief Codice eseguito dal thread CommAdapter Monitor
	  * @author Diego Gangale
	  * @date	15 04 2010 Created
	  *
	  *
	  */
	DWORD CComunicationAdapterMonitor::Run(LPVOID /* arg */) {
		//Ogni SID_PING_TIME si richiede il keepalive dai SID
		SetTimer(NULL, TIMER_CCOMM_WATCHDOG, CConfig::GetInstance()->SID_PING_TIME(), CComunicationAdapterMonitor::PingTimerProc);
		//Ogni SID_VERIFY_TIME si verifica quali sono i SID realmente in runnign
		SetTimer(NULL, TIMER_CCOMM_VERIFY, CConfig::GetInstance()->SID_VERIFY_TIME(), CComunicationAdapterMonitor::VerifyTimerProc);

		int res;
		std::map<int, CfmCommAdapterHandleMap*>::iterator it;
		//x ROSALBA while sempre
		while (true) {
			try {
				MSG msg;
				GetMessage(&msg, NULL, 0, 0);

				switch (msg.message)
				{
				case EVENT_CCOMM_TIMER:
					sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] State: RUNNING");
					CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW);
					break;

				case EVENT_CCOMM_VERIFY_TIMER:
					sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] Verifing all CComm");
					CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW);
					this->VerifyAllAdapters();
					break;

				case EVENT_CCOMM_PING_RESP:
					sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] [RECEIVED RESP] [CComm]: %i", (int)msg.wParam);
					CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW);
					hCommAdapterMap[(int)msg.wParam]->AliveTimestamp = clock();
					break;

				case EVENT_CCOMM_UNDER_MAINTENANCE:
					//19 12 2007 (Il DBManager ha appena verificato lo stato dei sotto sistemi (SIDs) )
					// E si è trovato uno o più sotto sistemi per cui è stata richiesta la manutenzione o l'uscita dalla manutenzione
					break;

					//Messaggi che arrivano dal kernel
				case EVENT_PUSH_DI:
				{
					PREPARE_DATA

						for (it = hCommAdapterMap.begin(); it != hCommAdapterMap.end(); it++)
							res = pushDeviceEventWithLog(it->second->CommAdapterIf, msg.lParam, sdt, (char*)"sara.PushDIEvent");
					delete (CfmDevices_Table*)msg.wParam;
				}
				break;

				case EVENT_PUSH_DO:
				{
					PREPARE_DATA

						for (it = hCommAdapterMap.begin(); it != hCommAdapterMap.end(); it++)
							res = pushDeviceEventWithLog(it->second->CommAdapterIf, msg.lParam, sdt, (char*)"sara.PushDOEvent");
					delete (CfmDevices_Table*)msg.wParam;
				}
				break;

				case EVENT_PUSH_TRK:
				{
					PREPARE_DATA

						//for (it = hCommAdapterMap.begin(); it != hCommAdapterMap.end(); it++) 
						//	res = it->second->CommAdapterIf->PushTRKEvent(msg.lParam, &sdt);

						delete (CfmDevices_Table*)msg.wParam;
				}
				break;

				case EVENT_PUSH_ZONE:
				{
					PREPARE_DATA

						for (it = hCommAdapterMap.begin(); it != hCommAdapterMap.end(); it++)
							res = pushDeviceEventWithLog(it->second->CommAdapterIf, msg.lParam, sdt, (char*)"sara.PushZoneEvent");
					delete (CfmDevices_Table*)msg.wParam;
				}
				break;

				case EVENT_PUSH_ALARM:
				{
					PREPARE_DATA

						for (it = hCommAdapterMap.begin(); it != hCommAdapterMap.end(); it++)
							res = pushDeviceEventWithLog(it->second->CommAdapterIf, msg.lParam, sdt, (char*)"sara.PushAlarm");
					delete (CfmDevices_Table*)msg.wParam;
				}
				break;

				case EVENT_PUSH_READER:
				{
					PREPARE_DATA

						for (it = hCommAdapterMap.begin(); it != hCommAdapterMap.end(); it++)
							res = it->second->CommAdapterIf->PushDeviceEvent(msg.lParam, sdt, "sara.PushReaderEvent");
					delete (CfmDevices_Table*)msg.wParam;
				}
				break;

				case EVENT_PUSH_EXECUTING:
				{
					PREPARE_DATA

						for (it = hCommAdapterMap.begin(); it != hCommAdapterMap.end(); it++)
							res = pushDeviceEventWithLog(it->second->CommAdapterIf, msg.lParam, sdt, (char*)"sara.PushExecuting");
					delete (CfmDevices_Table*)msg.wParam;
				}
				break;

				case EVENT_PUSH_MESSAGE:
				{
					if (!msg.wParam) break;
					SystemMessage sm;
					SystemMessage* p = (SystemMessage*)msg.wParam;
					sm = *p;
					for (it = hCommAdapterMap.begin(); it != hCommAdapterMap.end(); it++)
						res = it->second->CommAdapterIf->PushSysMessage(sm, "sara.PushMessage");
					delete (SystemMessage*)msg.wParam;
				}
				break;

				case EVENT_PUSH_CAMERA:
				{
					PREPARE_DATA

						for (it = hCommAdapterMap.begin(); it != hCommAdapterMap.end(); it++)
							res = pushDeviceEventWithLog(it->second->CommAdapterIf, msg.lParam, sdt, (char*)"sara.PushCameraEvent");
					delete (CfmDevices_Table*)msg.wParam;
				}
				break;

				case EVENT_PUSH_CDC:
				{
					PREPARE_DATA

						for (it = hCommAdapterMap.begin(); it != hCommAdapterMap.end(); it++)
							res = pushDeviceEventWithLog(it->second->CommAdapterIf, msg.lParam, sdt, (char*)"sara.PushCDCEvent");
					delete (CfmDevices_Table*)msg.wParam;
				}
				break;

				default:
					TranslateMessage(&msg);  //Dispatch dei messaggi
					DispatchMessage(&msg);
					break;
				}
				Sleep(100);
			}
			catch (std::exception& e)
			{
				//Logging dell'errore
				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] [Error] %s", e.what());
				CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW);
			}

		}//Chiude il ciclo principale del thread
		try {
			this->StopAllAdapters();
		}
		catch (std::exception& e) {}
	}
	/**
	* @fn		ThCommAdapterMonitor::RegisterAdapter(int CommUID, SaraSIDHandleMap* SIDData)
	* @brief	Registra sulla tabella interna il riferimento al SID identifcato da SIDUID
	* @author Diego Gangale
	* @date	15 04 2010 Created
	* @param	SIDUID	Identificativo univoco del SID
	* @param	SIDData	Dettagli del SID che si sta registrando
	* @see	ThEventManager
	* @see	SaraSIDHandleMap
	*
	* La funzione viene utilizzata dal ThEventManager per registrare un SID sul SID monitor.
	* Se il SID è locale (quindi realizzato tramite dll) allora su di esso viene registrata
	* la callback necessaria per rispondere al polling (ping)
	*
	*
	*
	*/
	void CComunicationAdapterMonitor::RegisterAdapter(int CCommUID, CfmCommAdapterHandleMap* CCommData)
	{
		hCommAdapterMap.insert(std::pair<int, CfmCommAdapterHandleMap*>(CCommUID, CCommData));

		//Si registra la callback solo se SID locale
		CCommData->CommAdapterIf->RegisterPingCallback(CComunicationAdapterMonitor::PingCallback);
	}

	/*! \brief Funzione di timeout lanciata dal timer mediante il thread timerThread
	 *
	 * Invia timeout a se stesso.
	 *
	 * \param timer indice del timer
	 * \param parameter parametro intero eventuale passato
	 * \param ptrParam parametro puntatore eventuale passato
	 */
	void CComunicationAdapterMonitor::adapterMonTo(unsigned int timer, unsigned int param, void* ptrParam) {
		CfmCommAdapterHandleMap* p = (CfmCommAdapterHandleMap*)ptrParam;
		if (p) {
			//SaraDevices_Table sdt;
			//int res = ((ThCommAdapterMonitor *) param)->pushDeviceEventWithLog(p->CommAdapterIf, 0, sdt, "sara.Watchdog");
			SystemMessage sm; sm.Source = "XMLRPCADAPTER"; sm.AppDescription = "Watchdog"; sm.AppCode = 5000; sm.msgCount = 1; sm.Severity = 1; sm.SIDUID = 0;
			int res = p->CommAdapterIf->PushSysMessage(sm, "sara.Watchdog");
		}

		p->timer->refreshTimeout(0);
	}

	/**
	  * @fn		ThCommAdapterMonitor::StartAllAdapters()
	  * @brief	Avvia tutti i Communication Adapter registrati
	  * @author	Diego Gangale
	  * @date	16 04 2010 Created
	  *
	  * Per il thread base di ogni CComm viene impostata la priorità di esecuzione così
	  * come impostata sul database. \n
	  * Se per un CComm nella tabella System viene impostato a true il cmapo RealTimeContext
	  * allora l'intero processo Kernel.exe viene impostato alla esecuzione realtime
	  *
	  */
	void CComunicationAdapterMonitor::StartAllAdapters() {
		std::map<int, CfmCommAdapterHandleMap*>::iterator i;

		for (i = hCommAdapterMap.begin(); i != hCommAdapterMap.end(); i++) {
			if (i->second->bInitOk) {
				Sleep(200);

				i->second->CommAdapterIf->Start();

				bool r = false;

				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] - CComm: [%i].... STARTED", i->first);
				CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW);

				if (i->second->timer)
					i->second->timer->refreshTimeout(0);
			} //Chiude la if bInitOk
		}
	}

	void CComunicationAdapterMonitor::VerifyAllAdapters() {

		clock_t cNow = clock();

		std::map<int, CfmCommAdapterHandleMap*>::iterator i;
		for (i = hCommAdapterMap.begin(); i != hCommAdapterMap.end(); i++) {

			clock_t elapsedTime = cNow - i->second->AliveTimestamp;
			long secondi = elapsedTime / CLOCKS_PER_SEC;

			if (secondi > 5) {
				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] CComm [%i %s] not responding", i->first, i->second->CommAdapterIf->GetAlias().c_str());
				CLogger::getInstance()->Log(logBuffer);

				//Memorizzo su DB lo stato del sotto sistema
				DBManager::getInstance()->UpdateSystemStatus(i->first, 4);
				i->second->Status = 4;
			}

			if (secondi > 15) {
				//i->second->NotRespondingCycles++;
				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] CComm [%i %s] not running", i->first, i->second->CommAdapterIf->GetAlias().c_str());
				CLogger::getInstance()->Log(logBuffer);

				//Memorizzo su DB lo stato del sotto sistema
				DBManager::getInstance()->UpdateSystemStatus(i->first, 1);
				i->second->Status = 1;

				//Riavvio di un SID che non risponde
				//if(i->second->NotRespondingCycles > 5)
				//{
				//	i->second->NotRespondingCycles = 0;
				Beep(200, 200);
				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] Trying to restart CComm [%i %s]", i->first, i->second->CommAdapterIf->GetAlias().c_str());
				CLogger::getInstance()->Log(logBuffer);
				Sleep(100);
				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] Close CComm [%i %s]", i->first, i->second->CommAdapterIf->GetAlias().c_str());
				CLogger::getInstance()->Log(logBuffer);
				i->second->CommAdapterIf->CloseAdapter(); //Il SID rilascia tutte le sue risorse e si disinizializza
				Sleep(1000);
				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] Init CComm [%i %s]", i->first, i->second->CommAdapterIf->GetAlias().c_str());
				CLogger::getInstance()->Log(logBuffer);

				//i->second->SIDIf->InitSID();  //Reinizializzo il SID
				Sleep(1000);

				//Chiusura
				i->second->CommAdapterIf->CloseAdapter();
				//Reinizializzazione
				//Inizializzazione del SID
				if (i->second->CommAdapterIf->InitAdapter() != FAILURE)   {
					sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] Start CComm [%i %s]", i->first, i->second->CommAdapterIf->GetAlias().c_str());
					CLogger::getInstance()->Log(logBuffer);

					//Riavvio
					i->second->CommAdapterIf->Restart();  //Lo riavvio
					i->second->AliveTimestamp = clock();

					i->second->bInitOk = true;
				} else {
					sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] Cannot initializing CComm [%i %s]", i->first, i->second->CommAdapterIf->GetAlias().c_str());
					CLogger::getInstance()->Log(logBuffer);
					i->second->bInitOk = false;
				}
				//}
			} else {
				sprintf_s(logBuffer, SM_LOG_BUFLEN, "CComm [%i %s] running", i->first, i->second->CommAdapterIf->GetAlias().c_str());
				CLogger::getInstance()->Log(logBuffer);
				i->second->Status = 0;
			}
		}

	}

	//Arresta tutti i SID registrati
	void CComunicationAdapterMonitor::StopAllAdapters() {

		std::map<int, CfmCommAdapterHandleMap*>::iterator i;

		for (i = hCommAdapterMap.begin(); i != hCommAdapterMap.end(); i++) {
			if (i->second->CommAdapterIf != NULL) {

				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] Stop CComm [%i %s]", i->first, i->second->CommAdapterIf->GetAlias().c_str());
				CLogger::getInstance()->Log(logBuffer);
				i->second->CommAdapterIf->CloseAdapter(); //Il SID rilascia tutte le sue risorse e si disinizializza
				i->second->CommAdapterIf->Terminate(); //Il SID rilascia tutte le sue risorse e si disinizializza
				// OSC 19/06/2009

				i->second->CommAdapterIf->Stop();

				if (i->second->timer)
					i->second->timer->resTimeout(0);
			}
			//Memorizzo su DB lo stato del sotto sistema
			DBManager::getInstance()->UpdateSystemStatus(i->first, 1);
		}

	}

	int CComunicationAdapterMonitor::GetThreadId() {
		return m_ThreadCtx.m_dwTID;
	}

	bool CComunicationAdapterMonitor::pushDeviceEventWithLog(CComm* ccom, long idEvent, domain::CfmDevices_Table& p, char* MethodName) {
		bool res;

		CLogger::getInstance()->Log(std::string(MethodName) + " BEGIN", CLogger::DEBUG_LEVEL_VERY_HIGH);

		res = ccom->PushDeviceEvent(idEvent, p, MethodName);

		CLogger::getInstance()->Log(std::string(MethodName) + " END", CLogger::DEBUG_LEVEL_VERY_HIGH);

		return res;
	}

	//Invio messaggio di risposta al PING sul SID
	void CComunicationAdapterMonitor::PingCallback(int SIDUID) {
		PostThreadMessage(CComunicationAdapterMonitor::getInstance()->m_ThreadCtx.m_dwTID, EVENT_CCOMM_PING_RESP, (WPARAM)SIDUID, 0);
	}

	/**
	* @fn		CComunicationAdapterMonitor::MyTimerProc(HWND hwnd,UINT message, UINT idTimer, DWORD dwTime)
	* @brief	Invia il messaggio di polling al main thread allo scattare del timer
	* @author Diego Gangale
	* @date   18 04 2010 Created
	* @param	hwnd	Handle alla fiestra di riferimento o NULL
	* @param	message	Idenetificativo del messaggio inviato dal timer
	* @param	idTimer	Identificativo univoco del timer
	* @param  dwTime  ora di sistema corrente
	*
	*/
	VOID CALLBACK CComunicationAdapterMonitor::PingTimerProc(
		HWND hwnd,        // handle to window for timer messages 
		UINT message,     // WM_TIMER message 
		UINT_PTR idTimer,     // timer identifier 
		DWORD dwTime)     // current system time 
	{

		CComunicationAdapterMonitor::getInstance()->PingAllAdapters();
		PostThreadMessage(CComunicationAdapterMonitor::getInstance()->m_ThreadCtx.m_dwTID, EVENT_CCOMM_TIMER, 0, 0);
	}

	/**
	  * @fn		CComunicationAdapterMonitor::MyTimerVerifyProc(HWND hwnd,UINT message, UINT idTimer, DWORD dwTime)
	  * @brief	Invia il messaggio di avvio della verifica delle risposte al polling al main thread allo scattare del timer
	  * @author Diego Gangale
	  * @date   18 04 2010 Created
	  * @param	hwnd	Handle alla fiestra di riferimento o NULL
	  * @param	message	Idenetificativo del messaggio inviato dal timer
	  * @param	idTimer	Identificativo univoco del timer
	  * @param  dwTime  ora di sistema corrente
	  *
	  */
	VOID CALLBACK CComunicationAdapterMonitor::VerifyTimerProc(
		HWND hwnd,        // handle to window for timer messages 
		UINT message,     // WM_TIMER message 
		UINT_PTR idTimer,     // timer identifier 
		DWORD dwTime)     // current system time 
	{

		PostThreadMessage(CComunicationAdapterMonitor::getInstance()->m_ThreadCtx.m_dwTID, EVENT_CCOMM_VERIFY_TIMER, 0, 0);

	}

	/**
  * @fn		ThCommAdapterMonitor::PingAllAdapters()
  * @brief	Invia un messaggio di polling a tutti i SID installati come moduli del Kernel
  * @author	Diego Gangale
  * @date	15 04 2010 Created
  *
  *
  */
	void CComunicationAdapterMonitor::PingAllAdapters() {
		try {
			std::map<int, CfmCommAdapterHandleMap*>::iterator i;

			for (i = hCommAdapterMap.begin(); i != hCommAdapterMap.end(); i++)
			{
				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] [Send Ping REQ] %i", i->second->CommAdapterIf->GetUID());
				CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW);
				i->second->CommAdapterIf->AsyncPing();
			}


		}
		catch (std::exception& e)
		{
			sprintf_s(logBuffer, SM_LOG_BUFLEN, "[CommAdapterMonitor] [Error] %s", e.what());
			CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW);
		}

	}

	void CComunicationAdapterMonitor::pushExecuting(int iDev, bool executing) {
		DBManager::getInstance()->UpdateExecuting(iDev, executing);
		CfmDevices_Table* sdt = DBManager::getInstance()->getDevice(iDev);

		if (sdt) {
			PostThreadMessage(CComunicationAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_EXECUTING, (WPARAM)sdt, (LPARAM)EXECUTING_EVENT);
		}
	}

	bool CComunicationAdapterMonitor::ProcessAction(int tipo, int device, std::string action, std::map<std::string, std::string> m)
	{
		unsigned int iActionTicket = 0;
		bool ret = true;
		/****************************************
		iActionTicket = ActionMonitor::getInstance()->InsertAction(device);

		TMapParametriAzione tpa; //Cross fucking correlation con parametri
		Parameter* p = NULL;

		//qui va inserito il codice per la gestione dell'inserimnto della riga nella
		//tabella CommandLog
		//INSERT INTO CommandLog(Id, StartTS, DatabaseTS, ActionId, DeviceId, UserId) 
		std::string sKey = action + "|" + IntToStr(tipo);
		std::map<std::string, CfmAction_Table> mapAction = DBManager::getInstance()->getActionsByNameCat();
		if (mapAction.find(sKey) != mapAction.end())
			DBManager::getInstance()->LogCommand(device, 1, mapAction[sKey].Id, m["Message"].c_str());

		switch (tipo)
		{
		case 1: // DI
			pushExecuting(device);

			if (action == "DI_ENABLE") {
				CatDI::getInstance()->SetLogicalStatus(device, iActionTicket, true);
				break;
			}
			if (action == "DI_DISABLE") {
				CatDI::getInstance()->SetLogicalStatus(device, iActionTicket, false);
				break;
			}
			ret = false;
			break;

		case 2: // DO
			pushExecuting(device);

			p = new Parameter();
			p->sAlias = "DURATION";
			p->sType = "INT";
			p->iValue = atoi(m["Duration"].c_str());
			tpa["DURATION"] = p;

			if (action == "DO_ON")
			{
				CatDO::getInstance()->OpenDO(device, iActionTicket, tpa);
				break;
			}
			if (action == "DO_OFF")
			{
				CatDO::getInstance()->CloseDO(device, iActionTicket, tpa);
				break;
			}
			if (action == "DO_ENABLE")
			{
				CatDO::getInstance()->SetLogicalStatus(device, iActionTicket, true);
				break;
			}
			if (action == "DO_DISABLE")
			{
				CatDO::getInstance()->SetLogicalStatus(device, iActionTicket, false);
				break;
			}
			ret = false;
			break;

		case 5: // ALLARMI
			pushExecuting(device);

			if (action == "ALARM_SET")
			{
				CatAlmMgr::getInstance()->SetAlarm(device, iActionTicket, tpa);
				break;
			}
			if (action == "ALARM_RESET")
			{
				CatAlmMgr::getInstance()->ResetAlarm(device, iActionTicket, tpa);
				break;
			}
			if (action == "ALARM_ENABLE")
			{
				CatAlmMgr::getInstance()->SetLogicalStatus(device, iActionTicket, true);
				break;
			}
			if (action == "ALARM_DISABLE")
			{
				CatAlmMgr::getInstance()->SetLogicalStatus(device, iActionTicket, false);
				break;
			}
			if (action == "ALARM_ACK")
			{
				CatAlmMgr::getInstance()->AckAlarm(device, iActionTicket, tpa);
				break;
			}
			if (action == "ALARM_PURGE")
			{
				CatAlmMgr::getInstance()->PurgeAlarm(device, iActionTicket, tpa);
				break;
			}
			ret = false;
			break;

		case 14: // CAMERA
			p = new Parameter();
			p->sAlias = "PRESETNUMBER";
			p->sType = "INT";
			p->iValue = atoi(m["PresetNumber"].c_str());
			tpa["PRESETNUMBER"] = p;

			p = new Parameter();
			p->sAlias = "PAN";
			p->sType = "INT";
			p->iValue = atoi(m["Pan"].c_str());
			tpa["PAN"] = p;

			p = new Parameter();
			p->sAlias = "TILT";
			p->sType = "INT";
			p->iValue = atoi(m["Tilt"].c_str());
			tpa["TILT"] = p;

			p = new Parameter();
			p->sAlias = "ZOOM";
			p->sType = "INT";
			p->iValue = atoi(m["Zoom"].c_str());
			tpa["ZOOM"] = p;

			p = new Parameter();
			p->sAlias = "SPEED";
			p->sType = "INT";
			p->iValue = atoi(m["Speed"].c_str());
			tpa["SPEED"] = p;

			if (action == "CAM_MOVEUP")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->MoveUp(device, iActionTicket, tpa);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->MoveUp(device, iActionTicket, tpa);
				break;
			}
			if (action == "CAM_MOVEDOWN")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->MoveDown(device, iActionTicket, tpa);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->MoveDown(device, iActionTicket, tpa);
				break;
			}

			if (action == "CAM_MOVELEFT")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->MoveLeft(device, iActionTicket, tpa);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->MoveLeft(device, iActionTicket, tpa);
				break;
			}

			if (action == "CAM_MOVERIGHT")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->MoveRight(device, iActionTicket, tpa);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->MoveRight(device, iActionTicket, tpa);
				break;
			}

			if (action == "CAM_MOVEUPLEFT")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->MoveUpLeft(device, iActionTicket, tpa);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->MoveUpLeft(device, iActionTicket, tpa);
				break;
			}

			if (action == "CAM_MOVEUPRIGHT")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->MoveUpRight(device, iActionTicket, tpa);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->MoveUpRight(device, iActionTicket, tpa);
				break;
			}

			if (action == "CAM_MOVEDOWNLEFT")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->MoveDownLeft(device, iActionTicket, tpa);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->MoveDownLeft(device, iActionTicket, tpa);
				break;
			}

			if (action == "CAM_MOVEDOWNRIGHT")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->MoveDownRight(device, iActionTicket, tpa);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->MoveDownRight(device, iActionTicket, tpa);
				break;
			}

			if (action == "CAM_ZOOMIN")
			{
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->ZoomIn(device, iActionTicket, tpa);
				break;
			}

			if (action == "CAM_ZOOMOUT")
			{
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->ZoomOut(device, iActionTicket, tpa);
				break;
			}

			if (action == "CAM_STOP")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->Stop(device, iActionTicket, tpa);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->Stop(device, iActionTicket, tpa);
				break;
			}

			if (action == "CAM_CALLPRESET")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->GotoPreset(device, iActionTicket, tpa);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->GotoPreset(device, iActionTicket, tpa);
				break;
			}
			if (action == "CAM_CALLABSPOSITION")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->GotoAbsPosition(device, iActionTicket, tpa);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->GotoAbsPosition(device, iActionTicket, tpa);
				break;
			}
			if (action == "CAM_ENABLE")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->SetLogicalStatus(device, true);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->SetLogicalStatus(device, true);
				break;
			}
			if (action == "CAM_DISABLE")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->SetLogicalStatus(device, false);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->SetLogicalStatus(device, false);
				break;
			}
			if (action == "CAM_CONNECT")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->Connect(device, iActionTicket, tpa);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->Connect(device, iActionTicket, tpa);
				break;
			}
			if (action == "CAM_DISCONNECT")
			{
				if (CatPTZ::getInstance()->IsPTZ(device))
					CatPTZ::getInstance()->Disconnect(device, iActionTicket, tpa);
				if (CatCAMERA::getInstance()->IsCAMERA(device))
					CatCAMERA::getInstance()->Disconnect(device, iActionTicket, tpa);
				break;
			}
			ret = false;
			break;

		case 13: // ZONE
			pushExecuting(device);
			if (action == "ZONE_ARM")
			{
				CatZone::getInstance()->SetZoneArmed(device, iActionTicket, tpa);
				break;
			}
			if (action == "ZONE_DISARM")
			{
				CatZone::getInstance()->SetZoneDisarmed(device, iActionTicket, tpa);
				break;
			}
			if (action == "ZONE_ENABLE")
			{
				CatZone::getInstance()->SetLogicalStatus(device, true);
				break;
			}
			if (action == "ZONE_DISABLE")
			{
				CatZone::getInstance()->SetLogicalStatus(device, false);
				break;
			}
			ret = false;
			break;

		default:
			ret = false;
			break;
		}

		//DIEGO 30 09 2010 Secondo me la deve terminare solo il category manager
		//ActionMonitor::getInstance()->EndAction(iActionTicket);
		************************************/
		return ret;
	}

}