/****************************** Module Header ******************************\
* Module Name:  DeviceMonitor.cpp
* Project:      S000
*
*
* Device monitor is thread watching the state of devices managed with engine
*
\***************************************************************************/
#include "DeviceMonitor.h"
#include "Logger.h"
#include "Messages.h"
#include "DatabaseManager.h"

#include "CfmHandlesMaps.h"
#include "CategoryTypes.h"

namespace cfm::application {
    std::map<int, CDeviceMonitor*>CDeviceMonitor::smInstance;

    CDeviceMonitor::CDeviceMonitor(int id) : myRegionId(id) {
        /**
        addEvent(IDLE, SIDMON_START, &ThSIDMonitor::idleStart);
        addEvent(IDLE, SIDMON_START_ALL, &ThSIDMonitor::idleStartAll);
        addEvent(IDLE, SIDMON_AUTOSENSE, &ThSIDMonitor::idleAutosense);
        addEvent(IDLE, SIDMON_UPDATE, &ThSIDMonitor::idleUpdate);
        addEvent(IDLE, SIDMON_STOP_ALL, &ThSIDMonitor::idleStopAll);
        addEvent(IDLE, SIDMON_STOP, &ThSIDMonitor::idleStop);



        addEvent(WAIT_CLOSE_ALL, SIDMON_CLOSE_ALL, &ThSIDMonitor::waitCloseAllCloseAll);

        addEvent(WAIT_ONE_PING, EVENT_PING_RESP, &ThSIDMonitor::waitOnePingPingResp);
        //    addEvent(WAIT_ONE_PING,     SIDMON_ONE_PING_TIMEOUT,        &ThSIDMonitor::waitOnePingTimeout);
        addEvent(WAIT_ONE_PING, SIDMON_ONE_PING_TIMEOUT, &ThSIDMonitor::activeStopAll);
        addEvent(WAIT_ONE_PING, SID_NOTIFY_STATUS, &ThSIDMonitor::activeNotifyStatus);
        addEvent(WAIT_ONE_PING, SIDMON_STOP_ALL, &ThSIDMonitor::activeStopAll);
        addEvent(WAIT_ONE_PING, SIDMON_START, &ThSIDMonitor::anyNotIdleStart);
        addEvent(WAIT_ONE_PING, SIDMON_STOP, &ThSIDMonitor::activeStop);

        addEvent(ACTIVE, SID_NOTIFY_STATUS, &ThSIDMonitor::activeNotifyStatus);
        addEvent(ACTIVE, EVENT_SIDMON_TIMER, &ThSIDMonitor::activeMonTimer);
        addEvent(ACTIVE, EVENT_SIDMON_VERIFY_TIMER, &ThSIDMonitor::activeMonVerTimer);
        addEvent(ACTIVE, EVENT_PING_RESP, &ThSIDMonitor::activePingResp);
        addEvent(ACTIVE, SIDMON_STOP_ALL, &ThSIDMonitor::activeStopAll);
        addEvent(ACTIVE, SIDMON_START, &ThSIDMonitor::anyNotIdleStart);
        addEvent(ACTIVE, SIDMON_STOP, &ThSIDMonitor::activeStop);
        //  Data Update delegates
        //	addEvent(WAIT_DATA_ACQ,      EVENT_SIDMON_TIMER,             &ThSIDMonitor::activeMonTimer);
        addEvent(ACTIVE, SIDMON_UPDATE_DATA, &ThSIDMonitor::startUpdateData);
        addEvent(WAIT_ONE_PING, SIDMON_UPDATE_DATA, &ThSIDMonitor::startUpdateData);

        addEvent(CLOSING, SID_NOTIFY_STATUS, &ThSIDMonitor::closingNotifyStatus);

        addEvent(SIDMON_UPDATE_SID, &ThSIDMonitor::anyUpdateSid);
        ******/
        timer = new TTimersThread(SID_MONITOR_TIMERS_NO, 1000);
        std::string secret_word{"secret"};
       // cMachineHardwareKey = GetSARAHWKey();

        updating = false;
    }

    CDeviceMonitor::~CDeviceMonitor() {
        delete timer;

        terminated = false;
        bTerminated = true;

        PostThreadMessage(GetThreadId(), 0, 0, 0);

        while (!terminated)
            Sleep(100);
    }

    CDeviceMonitor* CDeviceMonitor::getInstance(int regionId, bool create) {
        std::map<int, CDeviceMonitor*>::iterator it;

        if ((it = smInstance.find(regionId)) == smInstance.end()) {
            assert(create);

            smInstance[regionId] = new CDeviceMonitor(regionId);
            smInstance[regionId]->CThread::Start();
        }
        return smInstance[regionId];
    }

    bool CDeviceMonitor::existsInstance(int regionId) {
        return smInstance.find(regionId) != smInstance.end();
    }

    void CDeviceMonitor::destroyInstances() {
        std::map<int, CDeviceMonitor*>::iterator it;

        while ((it = smInstance.begin()) != smInstance.end()) {
            delete it->second;
            smInstance.erase(it);
        }
    }

    void CDeviceMonitor::destroyInstance(int regionId) {
        std::map<int, CDeviceMonitor*>::iterator it;

        if ((it = smInstance.find(regionId)) != smInstance.end()) {
            delete it->second;
            smInstance.erase(it);
        }
    }

    /**
  * @fn ThSIDMonitor::Run(LPVOID)
  * @brief Codice eseguito dal thread SID Monitor
  * @author Diego Gangale
  * @date 10/05/2007 Created
  * @date 20/06/2007 Modifyed (try.... catch)
  *
  *
  */
    DWORD CDeviceMonitor::Run(LPVOID /* arg */) {
        //Si inizializza il contatore
        OldRowCycleCounter = 0;

        bTerminated = false;

        while (!bTerminated) {
            try {
                GetMessage(&msg, NULL, 0, 0);

                if (!execute(msg.message)) {
                    //Dispatch dei messaggi
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                Sleep(100);
            }  catch (std::exception& e) {
                //Logging dell'errore
                sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] [Error] %s", e.what());
                CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_HIGH);
            } catch (...) {
                //Logging dell'errore
                sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] [Error]");
                CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_HIGH);
            }
        }//Chiude il ciclo principale del thread
        terminated = true;

        return this->m_ThreadCtx.m_dwExitCode;
    }


    void CDeviceMonitor::req_close_all() {
        PostThreadMessage(GetThreadId(), SIDMON_CLOSE_ALL, NULL, NULL);
    }

	/**
  * @fn		ThSIDMonitor::RegisterSID(int SIDUID, SaraSIDHandleMap* SIDData)
  * @brief	Registra sulla tabella interna il riferimento al SID identifcato da SIDUID
  * @author Diego Gangale
  * @date	10/05/2007 Created
  * @date	06/07/2007 Gestito bInitOk in SaraSIDHandleMap per segnalare lo stato di inizializzazione del SID
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
	void CDeviceMonitor::RegisterSID(int SIDUID, CfmDeviceHandleMap* SIDData) {
		assert(hSIDMap.find(SIDUID) != hSIDMap.end());
		assert(!hSIDMap[SIDUID]);

		hSIDMap[SIDUID] = SIDData;

		//Si registra la callback solo se SID locale
		if (!SIDData->Remote)
			SIDData->SIDIf->RegisterPingCallback(CDeviceMonitor::PingCallback);
	}

	bool CDeviceMonitor::CloseSID(int SIDUID) {
		std::map<int, CfmDeviceHandleMap*>::iterator it;

		if ((it = hSIDMap.find(SIDUID)) != hSIDMap.end()) {
			if (!it->second) {
				hSIDMap.erase(it);
				return true;
			}
			if (SUCCESS == it->second->SIDIf->CloseSID()) {
				delete it->second;
				hSIDMap.erase(it);
				return true;
			}
		}
		return false;
	}

	/**
	  * @fn		ThSIDMonitor::StartAllSID()
	  * @brief	Avvia tutti i SID registrati come SID locali (dll)
	  * @author	Diego Gangale
	  * @date	24 05 2007 Created
	  * @date	22 07 2007 Remarked
	  *
	  * Per il thread base di ogni SID viene impostata la priorità di esecuzione così
	  * come impostata sul database. \n
	  * Se per un SID nella tabella System viene impostato a true il cmapo RealTimeContext
	  * allora l'intero processo Kernel.exe viene impostato alla esecuzione realtime
	  *
	  */
	void CDeviceMonitor::StartAllSID() {
		std::map<int, CfmDeviceHandleMap*>::iterator i;

		for (i = hSIDMap.begin(); i != hSIDMap.end(); i++) {
			if (!i->second)
				continue;

			//Si esegue il codice solo se è un sid locale
			if (i->second->Remote == true)
				continue;

			if ((i->second->SIDIf->GetSIDState() != StatusUndefined)
				&& (i->second->SIDIf->GetSIDState() != StatusTerminated))
				continue;

			if (i->second->bInitOk) {
				i->second->SIDIf->Start();

				bool r = false;
				//Settaggio della classe di priorità del thread
				if (i->second->RealTimeContext) {
					if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
					{
						DWORD dwError = GetLastError();
						sprintf_s(logBuffer, SM_LOG_BUFLEN, "SID MONITOR - Changing priority class error (%d)\n", dwError);
						CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, i->second->SIDIf->GetUID());

						LPTSTR s;
						if (::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
							NULL,
							dwError,
							0,
							(LPTSTR)&s,
							0,
							NULL))
						{
							sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] - Changing priority class error (%s)", s);
							CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, i->second->SIDIf->GetUID());
							::LocalFree(s);
						}

					}
					else
					{
						sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] - SARA PROCESS SWITCHED TO REALTIME");
						CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, i->second->SIDIf->GetUID());
					}
				}
				//Priorità del thread
				r = i->second->SIDIf->SetPriority(i->second->ThreadPriority);

				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] - SID: [%i].... STARTED", i->first);
				CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, i->first);
			} //Chiude la if bInitOk
		}
	}

	void CDeviceMonitor::StartSID(int SIDUID) {
		std::map<int, CfmDeviceHandleMap*>::iterator i;

		if ((i = hSIDMap.find(SIDUID)) != hSIDMap.end()) {
			if (!i->second)
				return;

			//Si esegue il codice solo se è un sid locale
			if (i->second->Remote == true)
				return;

			if ((i->second->SIDIf->GetSIDState() != StatusUndefined)
				&& (i->second->SIDIf->GetSIDState() != StatusTerminated))
				return;

			if (i->second->bInitOk) {
				i->second->SIDIf->Start();

				bool r = false;
				//Settaggio della classe di priorità del thread
				if (i->second->RealTimeContext) {
					if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS)) {
						DWORD dwError = GetLastError();
						sprintf_s(logBuffer, SM_LOG_BUFLEN, "SID MONITOR - Changing priority class error (%d)\n", dwError);
						CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, i->second->SIDIf->GetUID());

						LPTSTR s;
						if (::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
							NULL,
							dwError,
							0,
							(LPTSTR)&s,
							0,
							NULL))
						{
							sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] - Changing priority class error (%s)", s);
							CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, i->second->SIDIf->GetUID());
							::LocalFree(s);
						}

					}
					else
					{
						sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] - SARA PROCESS SWITCHED TO REALTIME");
						CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, i->second->SIDIf->GetUID());
					}
				}
				//Priorità del thread
				r = i->second->SIDIf->SetPriority(i->second->ThreadPriority);

				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] - SID: [%i].... STARTED", i->first);
				CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, i->first);
			} //Chiude la if bInitOk
		}
	}

	bool CDeviceMonitor::InitAllSID(tpOperationMode operationMode) {
		DBManager* DBM = DBManager::getInstance();

		//Main loop per il caricamento dei SID
		for (unsigned int i = 0; i < DBM->vSystemList.size(); i++) {
			if (!DBM->isMaster() && DBM->vSystemList[i].SARARegionId != myRegionId)
				continue;

			if (DBM->isMaster() && operationMode == opRunning && DBM->vSystemList[i].SARARegionId != myRegionId)
				continue;

			if (operationMode == opRunning && !DBM->vSystemList[i].enabled)
				continue;

			char logBuf[512];
			//Display data on standard output
			sprintf_s(logBuf, sizeof(logBuf), "%i  %s  %s  %s",
				DBM->vSystemList.at(i).Id,
				DBM->vSystemList.at(i).alias.c_str(),
				DBM->vSystemList.at(i).deployment.c_str(),
				DBM->vSystemList.at(i).localDll.c_str());
			CLogger::getInstance()->Log(logBuf, CLogger::DEBUG_LEVEL_VERY_LOW);

			/*
			*
			* Se il SID è deployato come remoto allora si deve allocare un oggetto di tipo
			* network adapter sulla porta indicata in configurazione
			*
			*/
			if (DBM->vSystemList.at(i).deployment == "Remote") {
				bool bInitOk = false;
				LoadRemoteSid(operationMode, &DBM->vSystemList.at(i), bInitOk);

				std::string ipAddress = DBM->vSystemList.at(i).RemoteIPAddress;
				int         ipPort = DBM->vSystemList.at(i).RemoteIPPort;

				//NetworkAdapter *na = new NetworkAdapter();
				//na->AddRemoteSID(DBM->vSystemList.at(i)->Id,ipAddress,ipPort);
				//na->Start();

				//-------------------------------------------
				//Registrazione del SID Remoto sul SID Monitor
				//-------------------------------------------
				//SaraSIDHandleMap *sshm	= new SaraSIDHandleMap();
				//sshm->obj					= (void *) na;
				//sshm->SIDIf				= NULL;
				//sshm->RSIDIf				= NULL;
				//sshm->bInitOk				= true;
				//sshm->Remote				= true;
				//sshm->RealTimeContext		= DBM->vSystemList.at(i)->realTimeContext;
				//sshm->ThreadPriority		= DBM->vSystemList.at(i)->ThreadPriority;
				//thSidMon->RegisterSID(DBM->vSystemList.at(i)->Id, sshm);

				// Per ora che i SID Remoti non sono supportati
			} else {
				bool bInitOk = false;
				if (LoadLocalSid(operationMode, &DBM->vSystemList.at(i), bInitOk))
				{
					if (bInitOk) CLogger::getInstance()->Log("So inizializzato");
					RegisterSID(&DBM->vSystemList.at(i), bInitOk);
				}
				else
					if (operationMode == opConfig)
						return false;
			}
		} //Chiude il loop principale del caricamento della configurazione dei sottosistemi
		return true;
	}

	bool CDeviceMonitor::InitSID(tpOperationMode operationMode, int SIDUID, std::string SiteName) {
		DBManager* DBM = DBManager::getInstance();
		bool ret = false;

		if (SIDexists(SIDUID)) return false;

		DBM->getSystemList(SIDUID);

		//Main loop per il caricamento dei SID
		for (unsigned int i = 0; i < DBM->vSystemList.size(); i++) {
			if (DBM->vSystemList[i].Id != SIDUID || DBM->vSystemList[i].SARARegionId != myRegionId)
				continue;

			if (operationMode == opRunning && !DBM->vSystemList[i].enabled)
				continue;

			char logBuf[512];
			//Display data on standard output
			sprintf_s(logBuf, sizeof(logBuf), "%i  %s  %s  %s",
				DBM->vSystemList.at(i).Id,
				DBM->vSystemList.at(i).alias.c_str(),
				DBM->vSystemList.at(i).deployment.c_str(),
				DBM->vSystemList.at(i).localDll.c_str());
			CLogger::getInstance()->Log(logBuf, CLogger::DEBUG_LEVEL_VERY_LOW, SIDUID);

			/*
			*
			* Se il SID è deployato come remoto allora si deve allocare un oggetto di tipo
			* network adapter sulla porta indicata in configurazione
			*
			*/
			if (DBM->vSystemList.at(i).deployment == "Remote") {
				bool bInitOk = false;
				LoadRemoteSid(operationMode, &DBM->vSystemList.at(i), bInitOk);

				std::string ipAddress = DBM->vSystemList.at(i).RemoteIPAddress;
				int         ipPort = DBM->vSystemList.at(i).RemoteIPPort;

				//NetworkAdapter *na = new NetworkAdapter();
				//na->AddRemoteSID(DBM->vSystemList.at(i)->Id,ipAddress,ipPort);
				//na->Start();

				//-------------------------------------------
				//Registrazione del SID Remoto sul SID Monitor
				//-------------------------------------------
				//SaraSIDHandleMap *sshm	= new SaraSIDHandleMap();
				//sshm->obj					= (void *) na;
				//sshm->SIDIf				= NULL;
				//sshm->RSIDIf				= NULL;
				//sshm->bInitOk				= true;
				//sshm->Remote				= true;
				//sshm->RealTimeContext		= DBM->vSystemList.at(i)->realTimeContext;
				//sshm->ThreadPriority		= DBM->vSystemList.at(i)->ThreadPriority;
				//thSidMon->RegisterSID(DBM->vSystemList.at(i)->Id, sshm);

				// Per ora che i SID Remoti non sono supportati
			} else {
				bool bInitOk = false;
				if (LoadLocalSid(operationMode, &DBM->vSystemList.at(i), bInitOk, SiteName)) {
					if (bInitOk) CLogger::getInstance()->Log("SID inizializzato", CLogger::DEBUG_LEVEL_VERY_LOW, SIDUID);
					RegisterSID(&DBM->vSystemList.at(i), bInitOk);
					ret = true;
				}
				else
					if (operationMode == opConfig)
						return false;
			}
		}
		return ret;
	}

	//Invio messaggio di risposta al PING sul SID
	void CDeviceMonitor::PingCallback(int SIDUID) {
		int region = DBManager::getInstance()->GetRegionFromSIDUID(SIDUID);

		if (CDeviceMonitor::existsInstance(region)) {
			PostThreadMessage(CDeviceMonitor::getInstance(region)->GetThreadId(), EVENT_PING_RESP, 0, (LPARAM)SIDUID);
		}
	}


	void CDeviceMonitor::VerifyAllSID() {
		clock_t cNow = clock();

		std::map<int, CfmDeviceHandleMap*>::iterator it;
		std::vector<int> localSID;

		for (it = hSIDMap.begin(); it != hSIDMap.end(); it++)
			localSID.push_back(it->first);

		while (localSID.size() > 0) {
			int id = *localSID.begin();
			localSID.erase(localSID.begin());

			// OSC 02/03/2011 Penso che la riga sottostante va cambiata cn la seguente
			//if( hSIDMap.find(id) != hSIDMap.end() && !hSIDMap[id])  // nuova
			// originale
			if (!hSIDMap[id])  {
				CloseSID(id);
				//Inizializzazione del SID
				if (InitSID(opRunning, id))  {
					//Si esegue il codice solo se è un sid locale
					if (hSIDMap[id]->Remote == true)
						continue;

					if (hSIDMap[id]->UnderMaintenance == true)
						continue;

					sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] Start SID [%i %s]", id, hSIDMap[id]->SIDIf->GetAlias().c_str());
					CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, id);

					//Riavvio
					StartSID(id);

					hSIDMap[id]->AliveTimestamp = clock();
					hSIDMap[id]->bInitOk = true;
				} else {
					sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] Cannot initializing SID [%i]", id);
					CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, id);

					if (hSIDMap.find(id) != hSIDMap.end() && hSIDMap[id])
						hSIDMap[id]->bInitOk = false; // Ma a che c... serve ???
				}
				continue;
			}
			//Si esegue il codice solo se è un sid locale
			if (hSIDMap[id]->Remote == true)
				continue;

			if (hSIDMap[id]->UnderMaintenance == true)
				continue;

			clock_t elapsedTime = cNow - hSIDMap[id]->AliveTimestamp;
			unsigned long secondi = elapsedTime / CLOCKS_PER_SEC;
			int pingTime = 10000; // saraRegCfg.SID_PING_TIME() / 1000;

			if (secondi > pingTime && secondi < 60) {
				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] SID [%i %s] not responding", id, hSIDMap[id]->SIDIf->GetAlias().c_str());
				CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, id);

				//Memorizzo su DB lo stato del sotto sistema
				DBManager::getInstance()->UpdateSystemStatus(id, StatusNotResponding);
				hSIDMap[id]->Status = StatusNotResponding;
			}
			else
				if (secondi >= 60) {
					//hSIDMap[i]->NotRespondingCycles++;
					sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] SID [%i %s] not running", id, hSIDMap[id]->SIDIf->GetAlias().c_str());
					CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, id);

					//Memorizzo su DB lo stato del sotto sistema
					DBManager::getInstance()->UpdateSystemStatus(id, StatusNotRunning);
					hSIDMap[id]->Status = StatusNotRunning;

					Beep(200, 200);
					sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] Trying to restart SID [%i %s]", id, hSIDMap[id]->SIDIf->GetAlias().c_str());
					CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, id);
					Sleep(100);
					sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] Close SID [%i %s]", id, hSIDMap[id]->SIDIf->GetAlias().c_str());
					CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, id);

					hSIDMap[id]->SIDIf->TerminateNow();

					sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] Init SID [%i %s]", id, hSIDMap[id]->SIDIf->GetAlias().c_str());
					CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, id);

					//Reinizializzazione
					CloseSID(id);
					//Inizializzazione del SID
					if (InitSID(opRunning, id))  {
						sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] Start SID [%i %s]", id, hSIDMap[id]->SIDIf->GetAlias().c_str());
						CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, id);

						//Riavvio
						StartSID(id);

						hSIDMap[id]->AliveTimestamp = clock();
						hSIDMap[id]->bInitOk = true;
					} else {
						sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] Cannot initializing SID [%i]", id);
						CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, id);

						if (hSIDMap.find(id) != hSIDMap.end() && hSIDMap[id])
							hSIDMap[id]->bInitOk = false; // Ma a che c... serve ???
					}
				}
		}
	}

	bool CDeviceMonitor::SIDalive() {
		if (!SIDexists())
			return false;

		for (std::map<int, CfmDeviceHandleMap*>::iterator it = hSIDMap.begin(); it != hSIDMap.end(); it++)
			if (it->second)
				return true;

		return false;
	}

	bool CDeviceMonitor::SIDalive(int SIDUID) {
		return SIDexists(SIDUID) && hSIDMap[SIDUID];
	}

	bool CDeviceMonitor::SIDexists() {
		return !hSIDMap.empty();
	}

	bool CDeviceMonitor::SIDexists(int SIDUID) {
		return hSIDMap.find(SIDUID) != hSIDMap.end();
	}

	//Arresta tutti i SID registrati
	void CDeviceMonitor::StopAllSID() {
		std::map<int, CfmDeviceHandleMap*>::iterator i;

		for (i = hSIDMap.begin(); i != hSIDMap.end(); i++) {
			if (!i->second)
				continue;

			//Si esegue il codice solo se è un sid locale
			if (i->second->Remote == true)
				continue;

			if (i->second->SIDIf != NULL)
			{
				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] Stop SID [%i %s]", i->first, i->second->SIDIf->GetAlias().c_str());
				CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, i->first);
				i->second->SIDIf->Terminate(); //Il SID rilascia tutte le sue risorse e si disinizializza
			}
		}
	}

	void CDeviceMonitor::StopSID(int SIDUID) {
		std::map<int, CfmDeviceHandleMap*>::iterator i;

		if ((i = hSIDMap.find(SIDUID)) != hSIDMap.end()) {
			if (!i->second)
				return;

			//Si esegue il codice solo se è un sid locale
			if (i->second->Remote == true)
				return;

			if (i->second->SIDIf != NULL) {
				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] Stop SID [%i %s]", i->first, i->second->SIDIf->GetAlias().c_str());
				CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, i->first);
				i->second->SIDIf->Terminate(); //Il SID rilascia tutte le sue risorse e si disinizializza
			}
		}
	}

	//Chiude tutti i SID registrati
	void CDeviceMonitor::CloseAllSID() {
		std::map<int, CfmDeviceHandleMap*>::iterator i;
		int sid;

		while ((i = hSIDMap.begin()) != hSIDMap.end()) {
			sid = i->first;

			if (!i->second) {
				if (CloseSID(sid))
					DBManager::getInstance()->UpdateSystemStatus(sid, StatusTerminated);
				continue;
			}
			//Si esegue il codice solo se è un sid locale
			if (i->second->Remote == true)
				continue;

			if (i->second->SIDIf != NULL) {
				sprintf_s(logBuffer, SM_LOG_BUFLEN, "[SID Monitor] Close SID [%i %s]", i->first, i->second->SIDIf->GetAlias().c_str());
				CLogger::getInstance()->Log(logBuffer, CLogger::DEBUG_LEVEL_VERY_LOW, i->first);
				if (CloseSID(sid)) //Il SID rilascia tutte le sue risorse e si disinizializza
					DBManager::getInstance()->UpdateSystemStatus(sid, StatusTerminated);
			}
		}
	}


	int CDeviceMonitor::GetThreadId() {
		return m_ThreadCtx.m_dwTID;
	}


	void CDeviceMonitor::req_update_sid(UPDATE_MESSAGE* pCommandUpdate, void* p) {
		if (!updating)
			PostThreadMessage(this->GetThreadId(), SIDMON_UPDATE_SID, (WPARAM)pCommandUpdate, (LPARAM)p);
		//else
		//	MessageSender::getInstance()->Send("Updating SID " + IntToStr(pCommandUpdate->SIDUID) + ", please wait!\r\n", p);
	}
	//Aggiorna configurazione di tutti i SID
	void CDeviceMonitor::req_update() {
		PostThreadMessage(this->GetThreadId(), SIDMON_UPDATE, NULL, NULL);
	}
	//---------------------------------------------------------------------
	void CDeviceMonitor::notify_region_restored(int SARARegionId) {
	}

	//---------------------------------------------------------------------
	void CDeviceMonitor::req_start_sid(int SIDUID, void* p) {
		PostThreadMessage(this->GetThreadId(), SIDMON_START, SIDUID, (LPARAM)p);
	}

	void CDeviceMonitor::req_stop_sid(int SIDUID, void* p) {
		PostThreadMessage(this->GetThreadId(), SIDMON_STOP, SIDUID, (LPARAM)p);
	}
	//SASA Gestione Persone...
	void CDeviceMonitor::req_modify_data(int SIDUID, void* p) {
		PostThreadMessage(this->GetThreadId(), 36000/*SIDMON_UPDATE_DATA*/, SIDUID, (LPARAM)p);
	}


	//DIEGO 06 04 2010
	//richiesta di stato (metodo bloccante)
	CfmStatus CDeviceMonitor::req_status()  {
		CfmStatus ss;
		ss.status = ALIVE;
		std::map<int, CfmDeviceHandleMap*>::iterator i;
		for (i = hSIDMap.begin(); i != hSIDMap.end() && i->second; i++) {
			ss.vSid.push_back(i->second->SIDIf->GetUID());
		}
		return ss;
	}

	void CDeviceMonitor::req_start_all() {
		PostThreadMessage(this->GetThreadId(), SIDMON_START_ALL, NULL, NULL);
	}

	void CDeviceMonitor::req_stop_all() {
		PostThreadMessage(this->GetThreadId(), SIDMON_STOP_ALL, NULL, NULL);
	}

	void CDeviceMonitor::req_autosense() {
		PostThreadMessage(this->GetThreadId(), SIDMON_AUTOSENSE, NULL, NULL);
	}

	bool CDeviceMonitor::LoadRemoteSid(tpOperationMode operationMode, domain::CfmSystem_Table* System, bool& bInitOk) {
		return true;
	}
	//----------------------------------------------------------------------------

	bool CDeviceMonitor::LoadLocalSid(tpOperationMode operationMode, domain::CfmSystem_Table* System, bool& bInitOk, std::string SiteName) {
		bool retval = false;
		LPCSTR p = NULL;
		HMODULE hMod = NULL;

		//Puntatore all'oggetto SID
		void* obj = NULL;

		//Interfaccia comune
		CSID* ifSID = NULL;
		hSIDMap[System->Id] = NULL;

		bInitOk = false;

		if (!CheckLicense(System))
			return false;

		if (operationMode != opConfig && operationMode != opRunning && operationMode != opUpdate) {
			CLogger::getInstance()->Log("Tipo di caricamento del SID non riconoscito");
			return false;
		}
		//Tentativo di caricamento Mappa delle DLL
		p = System->localDll.c_str();

		//Caricamento DLL andato a buon fine
		if (hSystemMap.find(System->localDll) == hSystemMap.end()) {
			//Caricamento DLL
			//-----------------------------
			// hMod = LoadLibrary(p);
			//-----------------------------
			CLogger::getInstance()->Log("Richiesto caricamento della DLL ");
			CfmDLLHandleMap* hmSaraDll = new CfmDLLHandleMap();
			//hmSaraDll->data = System;
			hmSaraDll->DllSIDHandle = hMod;
			hmSaraDll->bLoaded = true; //true??????
			hmSaraDll->dataMap.insert(std::pair<int, domain::CfmSystem_Table*>(System->Id, System));
			hSystemMap.insert(std::pair<std::string, CfmDLLHandleMap*>(System->localDll, hmSaraDll));
		} else {
			hMod = hSystemMap[System->localDll]->DllSIDHandle;
			hSystemMap[System->localDll]->dataMap.insert(std::pair<int, domain::CfmSystem_Table*>(System->Id, System));
			CLogger::getInstance()->Log("DLL già presente nella mappa ");
		}
		if (hMod) {
			CLogger::getInstance()->Log("Binding alla DLL andato a buon fine ");
			CLogger::getInstance()->Log("Verifica consistenza degli entry point.... ");

			//Recupero le funzioni di utilità dalla DLL
			//Costruttore oggetto (SID)
			createObj COFun = (createObj)GetProcAddress(hMod, "CreateObject");
			//Distruttore oggetto (SID)
			destroyObj DEFun = (destroyObj)GetProcAddress(hMod, "DestroyObject");
			//Interfaccia Base
			getSIDInterface GetIfSID = (getSIDInterface)GetProcAddress(hMod, "GetSIDInterface");

			if (COFun && DEFun && GetIfSID) {
				CLogger::getInstance()->Log("Entry points e interfaccia base caricati correttamente ");

				if (hSystemMap[System->localDll]->objMap.find(System->Id) != hSystemMap[System->localDll]->objMap.end())
					obj = hSystemMap[System->localDll]->objMap[System->Id];
				else
				{
					//Allocazione del SID
					obj = COFun();
					hSystemMap[System->localDll]->objMap.insert(std::pair<int, void*>(System->Id, obj));
				}
				//Recupero l'interfaccia base del SID
				ifSID = GetIfSID(obj);
				if (ifSID) {
					CLogger::getInstance()->Log("Trovata l'interfaccia base del SID");

					//Memorizzo su DB lo stato del sotto sistema (Initializing....)
					DBManager::getInstance()->UpdateSystemStatus(ifSID->GetUID(), StatusInitializing);

					ifSID->SetUID(System->Id);
					ifSID->SetAlias(System->alias);
//Pekmez					ifSID->RegisterMessageCallback(messageCallback);
//Pekmez					ifSID->RegisterErrorCallback(errorCallback);
//Pekmez					ifSID->RegisterNotifyStatusCallback(notifyStatusCallbak);

					std::map<std::string, CfmSystemParameters_Table> mappaParametri = DBManager::getInstance()->getSystemParameters(ifSID->GetUID());
					std::map<std::string, CfmSystemParameters_Table>::iterator iParameter;
					for (iParameter = mappaParametri.begin(); iParameter != mappaParametri.end(); iParameter++)
					{
						if (!DBManager::getInstance()->isMaster())
							ifSID->SetCfgParameter(iParameter->second.Section, iParameter->second.Name, iParameter->second.Value);
						else
							ifSID->SetCfgParameter(iParameter->second.Section, iParameter->second.Name, iParameter->second.MasterValue);
					}
					//Questa va portata nel ThSIDMonitor DIEGO 06 05 2010
					//Inizializzazione del SID
					if (SUCCESS != ifSID->InitSID()) {
						//Loggo l'errore di inizializzazione
						CLogger::getInstance()->Log("[SID Monitor] INIT SID FAILURE");
						bInitOk = false;
					}
					//else
					//	bInitOk = CatSite::getInstance()->CaricaSiti(operationMode, ifSID, SiteName);

					//if (bInitOk)
					//	bInitOk = clTimeSchedule::getInstance()->LoadTimeSchedule(operationMode, ifSID, SiteName);

					//Binding interfacce di categoria sui Category Manager
					std::map<int, CfmCategory_Table>::iterator itCategories;
					for (itCategories = DBManager::getInstance()->mapCategoriesPriority.begin();
						bInitOk == true && itCategories != DBManager::getInstance()->mapCategoriesPriority.end();
						itCategories++)
					{
						switch (itCategories->second.Id)
						{
							//TRACKERS
						case CAMERA:
							//bInitOk = CatCAMERA::getInstance()->LoadIf(operationMode, hMod, obj, SiteName);
							break;

							//TRACKERS
						case TRK:
							//bInitOk = CatTrk::getInstance()->LoadIf(operationMode, hMod, obj);
							break;

							//DIGITAL INPUT
						case DI:
							//bInitOk = CatDI::getInstance()->LoadIf(operationMode, hMod, obj, SiteName);
							break;

							//DIGITAL OUTPUT
						case DO:
							//bInitOk = CatDO::getInstance()->LoadIf(operationMode, hMod, obj, SiteName);
							break;

							// ALARM MANAGER
						case ALMMGR:
							//bInitOk = CatAlmMgr::getInstance()->LoadIf(operationMode, hMod, obj, System, SiteName);
							break;

							//PTZ
						case PTZ:
							//bInitOk = CatPTZ::getInstance()->LoadIf(operationMode, hMod, obj, SiteName);
							break;

							//DVR
						case DVR:
							//bInitOk = CatDVR::getInstance()->LoadIf(operationMode, hMod, obj, SiteName);
							break;

							//VIDEOSRV
						case VIDEOSRV:
							//bInitOk = CatVIDEOSRV::getInstance()->LoadIf( hMod, obj);
							break;

							//READER
						//case READER:
						//	bInitOk = CatReader::getInstance()->LoadIf(operationMode, hMod, obj, SiteName);
						//	break;

							//ZONE
						//case ZONE:
						//	bInitOk = CatZone::getInstance()->LoadIf(operationMode, hMod, obj, SiteName);
						//	break;

							//CONCENTRATORI DI CAMPO
						//case CDC:
						//	bInitOk = CatCDC::getInstance()->LoadIf(operationMode, hMod, obj, SiteName);
						//	break;

#ifdef CONTROLLOACCESSO
						case CTRLACCESS:
							bInitOk = CatCtrlAccess::getInstance()->LoadIf(operationMode, hMod, obj, System);
							break;
#endif
						}
					}
					if (operationMode == opRunning && bInitOk)
						DBManager::getInstance()->setDevicesExecuting(ifSID->GetUID());

					if (bInitOk)
					{
						bInitOk = CaricaScenari(operationMode, ifSID, SiteName);
					}
					retval = bInitOk;

					if (!bInitOk)
						ifSID->CloseSID();
				} //Chiude la verifica sull'interfaccia base del SID
				else
				{
					//Memorizzo su DB lo stato del sotto sistema
					DBManager::getInstance()->UpdateSystemStatus(System->Id, StatusNotRunning);
					if (hSIDMap.find(System->Id) != hSIDMap.end() && hSIDMap[System->Id])
						hSIDMap[System->Id]->Status = StatusNotRunning;
				}
			} //Chiude la verifica sugli entry point della DLL	
		}  //Chiude la verifica sulla disponibilità della DLL nel path dell'eseguibile
		else
		{
			sprintf_s(logBuf, sizeof(logBuf), "[%s] DLL NOT FOUND", System->localDll.c_str());
			CLogger::getInstance()->Log(logBuf);
		}
		return retval;
	}
	//----------------------------------------------------------------------------

	bool CDeviceMonitor::CheckLicense(domain::CfmSystem_Table* System) {
		bool validLicense = false;

		try {
			//CARICAMENTO DELLE LICENSE
			std::map<std::string, CfmLicense_Table> hLicenseMap = DBManager::getInstance()->getLicenses();

			//VERIFICA LICENZA ED EVENTUALE CARICAMENTO DELL'OGGETTO
			//E DELLA DLL
			if (hLicenseMap.find(System->localDll.c_str()) != hLicenseMap.end()) {
				int iInstances = hLicenseMap[System->localDll.c_str()].Instances;
				std::string sLicense = hLicenseMap[System->localDll.c_str()].LicenseKey;
				std::string sDllName = hLicenseMap[System->localDll.c_str()].LocalDll;
				std::string sExpirationDate = hLicenseMap[System->localDll.c_str()].ExpirationDate;

				sprintf_s(logBuf, sizeof(logBuf), "SID License FOUND for %i instances", iInstances);
				CLogger::getInstance()->Log(logBuf);
				sprintf_s(logBuf, sizeof(logBuf), "LICENSE: %s", sLicense.c_str());
				CLogger::getInstance()->Log(logBuf);

				std::string sComputedLicense = ""; // GenerateLicenseKey(sDllName, iInstances, cMachineHardwareKey);
				//if (sComputedLicense == sLicense)
				//{
				//	if (!sExpirationDate.empty() && CheckExpirationDate(sExpirationDate))
				//	{
				//		std::map<int, SaraSIDHandleMap*>::iterator i;

				//		try
				//		{
				//			int SidId = 0;
				//			for (i = hSIDMap.begin(); i != hSIDMap.end(); i++)
				//			{
				//				if (i->second && i->second->SIDId == System->SIDId)
				//					SidId++;
				//			}
				//			if (SidId < iInstances)
				//			{
				//				sprintf_s(logBuf, sizeof(logBuf), "VALID LICENSE FOUND [%s] STARTING DLL INIT", sDllName.c_str());
				//				CLogger::getInstance()->Log(logBuf);
				//				validLicense = true;
				//			}
				//		}
				//		catch (std::exception& e)
				//		{
				//		}

				//	}
				//}

				//if (!validLicense)
				//{
				//	sprintf_s(logBuf, sizeof(logBuf), "NO VALID LICENSE FOUND [%s] SKIPPING DLL INIT", sDllName.c_str());
				//	CLogger::getInstance()->Log(logBuf);
				//	Beep(400, 2000);
				//}
			}
			else
				CLogger::getInstance()->Log("SID License NOT FOUND");
		}
		catch (std::exception) {
			CLogger::getInstance()->Log("SID License NOT FOUND");
		}
		return validLicense;
	}
	unsigned long CDeviceMonitor::getParentId(int sourceCatId, std::string sourceId) {
		/*try
		{
			switch (sourceCatId)
			{
			case DI:
				return CatDI::getInstance()->getId(sourceId);
				break;
			case READER:
				return CatReader::getInstance()->getId(sourceId);
				break;
			case ALMMGR:
				return CatAlmMgr::getInstance()->getId(sourceId);
				break;
			}
		}
		catch (std::exception& e)
		{
		}*/
		return 0;
	}

	bool CDeviceMonitor::RegisterSID(CfmSystem_Table* System, bool bInitOk) {
		try {
			//-------------------------------------------
			//Registrazione del SID sul SID Monitor
			if (hSystemMap.find(System->localDll) != hSystemMap.end())
			{
				CSID* ifSID = NULL;
				getSIDInterface GetIfSID = (getSIDInterface)GetProcAddress(hSystemMap[System->localDll]->DllSIDHandle, "GetSIDInterface");

				if (GetIfSID)
					ifSID = GetIfSID(hSystemMap[System->localDll]->objMap[System->Id]);

				if (ifSID) {
					CfmDeviceHandleMap* sshm = new CfmDeviceHandleMap();
					sshm->obj = hSystemMap[System->localDll]->objMap[System->Id];
					sshm->SIDIf = ifSID;
					sshm->bInitOk = bInitOk;
					sshm->Remote = false;
					sshm->RealTimeContext = System->realTimeContext;
					sshm->ThreadPriority = System->ThreadPriority;
					sshm->SIDId = System->SIDId;
					RegisterSID(System->Id, sshm);
				}
			}
		} catch (std::exception& e) {
			e.what();
			return false;
		}
		return true;
	}

	bool CDeviceMonitor::CaricaScenari(tpOperationMode operationMode, CSID* ifSID, std::string SiteName) {

		bool retval = false;

		//switch (operationMode)
		//{
		//case opRunning:
		//	retval = true;
		//	break;

		//case opConfig:
		//{
		//	std::vector<Scenarios> vScenarios = ifSID->GetScenariosList();
		//	std::vector<Scenarios>::iterator itScenarios;

		//	for (itScenarios = vScenarios.begin(); itScenarios != vScenarios.end(); itScenarios++)
		//	{
		//		// CategorySite
		//		int siteId = CatSite::getInstance()->getSiteIdFromLocalSite(ifSID->GetUID(), itScenarios->siteId);

		//		//int siteId = getSiteIdFromLocalSite(ifSID->GetUID(), itScenarios->siteId);
		//		//std::string logMsg = "New IdSite: "+IntToStr(siteId1)+" Old IdSite = "+IntToStr(siteId);
		//		//SaraLogger::getInstance()->Log(logMsg);

		//		int idScenario = DBManager::getInstance()->InsertScenarios(itScenarios->name, siteId,
		//			itScenarios->scenariosTemplate);
		//		if (idScenario > 0) {
		//			std::vector<ScenarioDevice>::iterator itScDevice;
		//			int CellPos = 1;
		//			for (itScDevice = itScenarios->vDevicesList.begin();
		//				// Al massimo carica 4 telecamere
		//				CellPos < 5 && itScDevice != itScenarios->vDevicesList.end();
		//				itScDevice++)
		//			{
		//				unsigned long idCamera = CatCAMERA::getInstance()->getIdDevice(ifSID->GetUID(),
		//					itScDevice->cameraId);
		//				if (idCamera > 0)
		//				{
		//					int presetId = DBManager::getInstance()->GetPresetForCamera(idCamera,
		//						IntToStr(itScDevice->presetId));
		//					DBManager::getInstance()->InsertScenariosDevice(idScenario, idCamera, CellPos++, presetId);
		//				}
		//			}

		//			std::vector<std::string>::iterator itAlarmId;
		//			for (itAlarmId = itScenarios->vAlarmList.begin();
		//				itAlarmId != itScenarios->vAlarmList.end();
		//				itAlarmId++)
		//			{
		//				unsigned long idAllarme = CatAlmMgr::getInstance()->getIdDevice(ifSID->GetUID(),
		//					*itAlarmId);
		//				if (idAllarme > 0)
		//					DBManager::getInstance()->InsertDeviceScenario(idAllarme, idScenario);
		//			}
		//		}
		//		std::string strMsg = "Inserito Scenario " + itScenarios->name + " da SID[" +
		//			IntToStr(ifSID->GetUID()) + "]";
		//		SaraLogger::getInstance()->Log(strMsg, SaraLogger::DEBUG_LEVEL_VERY_LOW, ifSID->GetUID());
		//	}
		//	vScenarios.clear();
		//	retval = true;
		//} break;

		//case opUpdate:
		//{
		//	//std::vector<Scenarios> vScenarios = ifSID->GetScenariosList();
		//	std::vector<Scenarios>::iterator itScenarios;
		//	std::vector<Scenarios> vScenarios;

		//	if (operationMode == opConfig)
		//		//Recupero i dati dal SID
		//		vScenarios = ifSID->GetScenariosList();
		//	else
		//	{
		//		long idSite = 0, IdSiteDB = 0;

		//		if (!SiteName.empty())
		//		{
		//			// CategorySite
		//			idSite = CatSite::getInstance()->getSiteIdFromName(ifSID, SiteName, IdSiteDB);

		//			//idSite = getSiteIdFromName(ifSID, SiteName, IdSiteDB);
		//			//std::string logMsg = "New IdSite: "+IntToStr(siteId1)+" Old IdSite = "+IntToStr(idSite);
		//			//SaraLogger::getInstance()->Log(logMsg);

		//			if (idSite == 0 && IdSiteDB == 0)
		//				idSite = -1;

		//			if (idSite < 0)
		//				return false;
		//		}

		//		if (idSite >= 0)
		//			//Recupero i dati dal SID
		//			vScenarios = ifSID->GetScenariosList(idSite);
		//	}

		//	for (itScenarios = vScenarios.begin(); itScenarios != vScenarios.end(); itScenarios++)
		//	{
		//		// CategorySite
		//		int siteId = CatSite::getInstance()->getSiteIdFromLocalSite(ifSID->GetUID(), itScenarios->siteId);

		//		//int siteId = getSiteIdFromLocalSite(ifSID->GetUID(), itScenarios->siteId);
		//		//std::string logMsg = "New IdSite: "+IntToStr(siteId1)+" Old IdSite = "+IntToStr(siteId);
		//		//SaraLogger::getInstance()->Log(logMsg);

		//		int idScenario = DBManager::getInstance()->InsertScenarios(itScenarios->name, siteId,
		//			itScenarios->scenariosTemplate);
		//		if (idScenario > 0) {
		//			std::vector<ScenarioDevice>::iterator itScDevice;
		//			int CellPos = 1;
		//			for (itScDevice = itScenarios->vDevicesList.begin();
		//				CellPos < 5 && itScDevice != itScenarios->vDevicesList.end();
		//				itScDevice++)
		//			{
		//				unsigned long idCamera = CatCAMERA::getInstance()->getIdDevice(ifSID->GetUID(),
		//					itScDevice->cameraId);
		//				if (idCamera > 0)
		//				{
		//					int presetId = DBManager::getInstance()->GetPresetForCamera(idCamera,
		//						IntToStr(itScDevice->presetId));
		//					DBManager::getInstance()->InsertScenariosDevice(idScenario, idCamera, CellPos++, presetId);
		//				}
		//			}

		//			// Per cancellare gli allarmi non più validi bisogna legare la tabella 
		//			// DeviceScenario-IdDevice ai device (quando vengono cancellati i device 
		//			// devono essere cancellati anche gli scenari associati
		//			std::vector<std::string>::iterator itAlarmId;
		//			for (itAlarmId = itScenarios->vAlarmList.begin();
		//				itAlarmId != itScenarios->vAlarmList.end();
		//				itAlarmId++)
		//			{
		//				unsigned long idAllarme = CatAlmMgr::getInstance()->getIdDevice(ifSID->GetUID(),
		//					*itAlarmId);
		//				if (idAllarme > 0)
		//					DBManager::getInstance()->InsertDeviceScenario(idAllarme, idScenario);
		//			}
		//		}
		//		std::string strMsg = "Inserito Scenario " + itScenarios->name + " da SID[" +
		//			IntToStr(ifSID->GetUID()) + "]";
		//		SaraLogger::getInstance()->Log(strMsg, SaraLogger::DEBUG_LEVEL_VERY_LOW, ifSID->GetUID());
		//	}
		//	vScenarios.clear();
		//	retval = true;
		//} break;
		//}
		return retval;
	}
}