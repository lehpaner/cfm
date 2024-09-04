/****************************** Module Header ******************************\
* Module Name:  BaseSID
*
* Base system interfaces for devices...
*
\***************************************************************************/
#include "BaseSID.h"
namespace cfm::application {
	/**
	  * @fn		CSID::CSID()
	  * @brief	Costruttore oggetto Common SID
	  * @author Diego Gangale
	  * @date	02/04/2007	Created
	  * @date	05/04/2007	Modified
	  * @date	22/07/2007	Remarked
	  *
	  * In fase di costruzione dell'oggetto vengono posti a NULL tutti i puntatori alla callbacks
	  *
	  */
	  // Costruttore CSID
	CSID::CSID() {
		//CatList = NULL;
		bTerminated = false;
		terminated = false;
		PingCallback = NULL;
		RaiseErrorCallback = NULL;
		RaiseMessageCallback = NULL;
		NotifyStatusCallback = NULL;
		SIDState = StatusUndefined;
		// InitSID();  
	} 
	// Distruttore
	CSID::~CSID() {
		// OSC 06-05-2010
		// La CloseSID() deve essere avviata in caso di 
		//CloseSID();
	}
	//----------------------------------------------------------------------------

	std::vector<Site> CSID::GetSiteList(long idSite) {
		std::vector<Site> v;
		return v;
	}
	//----------------------------------------------------------------------------

	std::vector<Scenarios> CSID::GetScenariosList(long IdSite)
	{
		std::vector<Scenarios> v;
		return v;
	}
	//----------------------------------------------------------------------------

	std::map<std::string, TimeSchedule> CSID::GetTimeScheduleList() {
		std::map<std::string, TimeSchedule> v;
		return v;
	}
	//----------------------------------------------------------------------------

	/*
	//Ritorna la lista delle categorie
	Categories *CSID::GetCategoriesList()
	{
		return CatList;
	}

	//Ritorna il numero di categorie a cui appartiene il SID
	unsigned int CSID::GetCategoriesCount()
	{
		return CatCount;
	}
	*/
	// Termina l'esecuzione del SID
	int CSID::Terminate() {
		//Si invia a se stessi (main loop) il messaggio di terminazione
		if (terminated)
		{
			//Notifico lo stato di esecuzione al kernel
			if (NotifyStatusCallback != NULL)
				NotifyStatusCallback(GetUID(), StatusTerminated);
			SIDState = StatusTerminated;
		}
		else
			PostThreadMessage(GetThId(), 777777/*ID_EVENT_TERMINATE*/ , 0, 0);
		return 0;
	}
	//----------------------------------------------------------------------------
	// Termina l'esecuzione del SID ora!
	void CSID::TerminateNow()              
	{
		if (terminated)
			return;

		PostThreadMessage(GetThId(), SID_EVENT_TERMINATE_NOW, 0, 0);

		int cnt = 200; // 20 secondi di attesa

		while (!terminated || --cnt)
			Sleep(100);

		if (!cnt)
		{
			// Killa il thread
			Stop(true);
		}
	}
	//----------------------------------------------------------------------------

	bool CSID::IsTerminated()    //Permette la verifica delo stato di funzionamento del SID
	{
		return SIDState == StatusUndefined || SIDState == StatusTerminated;
	}
	//----------------------------------------------------------------------------

	int CSID::Crash()              // Termina l'esecuzione del SID
	{
		//Si invia a se stessi (main loop) il messaggio di terminazione
		if (terminated)
		{
			//Notifico lo stato di esecuzione al kernel
			if (NotifyStatusCallback != NULL)
				NotifyStatusCallback(GetUID(), StatusCrashed);
			SIDState = StatusTerminated;
		}
		else
			PostThreadMessage(GetThId(), SID_EVENT_CRASH, 0, 0);
		return 0;
	}
	//----------------------------------------------------------------------------

	/**
	  * @fn		CSID::Restart()
	  * @brief	Riavvio del thread base
	  * @author Diego Gangale
	  * @date	02/04/2007	Created
	  * @date	05/04/2007	Modified
	  * @date	22/07/2007	Remarked
	  *
	  * L'attributo bTerminated viene posto a false. Viene richiamato il metodo Start();
	  *
	  */
	DWORD CSID::Restart()
	{
		bTerminated = false;

		// OSC 06-05-2010
		//Verificare se va aggiunta la Stop(), dato che è una restart

		return (Start());
	}

	/**
	  * @fn		CSID::AsyncPing()
	  * @brief	Ping asincrono del thread base
	  * @author Diego Gangale
	  * @date	02/04/2007	Created
	  * @date	05/04/2007	Modified
	  * @date	22/07/2007	Remarked
	  *
	  * Viene inviato il messaggio SID_EVENT_PING al thread base
	  *
	  */
	void CSID::AsyncPing()
	{
		int appo = GetThId();
		PostThreadMessage(GetThId(), SID_EVENT_PING, 0, 0);
	}
	//----------------------------------------------------------------------------

	/**
	  * @fn		CSID::AttachInterface(void *itf, Category cat)
	  * @brief	Inserisce un riferimento all'interfaccia di categoria specificata dal parametro cat
	  * @author Diego Gangale
	  * @date	02/04/2007	Created
	  * @date	05/04/2007	Modified
	  * @date	22/07/2007	Remarked
	  * @param	itf			Puntatore all'interfaccia di categoria specificata dal parametro cat
	  * @param	cat			Indica per quale tipo di inetrfaccia di categoria è richiesto l'attach
	  *
	  * Viene inviato il messaggio SID_EVENT_PING al thread base	\n
	  *
	  */
	void CSID::AttachInterface(void* itf, Category cat)
	{
		switch (cat)
		{
		case DI:
			sidIfMap.diIf = (SIDDIInterface*)itf;
			break;
		case DO:
			sidIfMap.doIf = (SIDDOInterface*)itf;
			break;
		case TRK:
			sidIfMap.trkIf = (SIDTrkInterface*)itf;
			break;
		case ALMMGR:
			sidIfMap.almMgrIf = (SIDAlmMgrInterface*)itf;
			break;
		case READER:
			sidIfMap.badgeIf = (SIDBadgeInterface*)itf;
			break;
		case PTZ:
			sidIfMap.ptzIf = (SIDPtzInterface*)itf;
			break;
			// OSC 03/07/2009
		case ZONE:
			sidIfMap.zoneIf = (SIDZoneInterface*)itf;
			break;
			// OSC 03/07/2009 Fine
		case CAMERA:
			sidIfMap.cameraIf = (SIDCameraInterface*)itf;
			break;
		case CDC:
			sidIfMap.cdcIf = (SIDCDCInterface*)itf;
			break;

		case CTRLACCESS:
			sidIfMap.ctrlAccessIf = (SIDCtrlAccessInterface*)itf;
			break;
		}
	}
	//----------------------------------------------------------------------------

	//Callbacks registration

	//Registrazione della callback per l'evento Ping
	void CSID::RegisterPingCallback(PingRespCallback p)
	{
		PingCallback = p;
	}
	//----------------------------------------------------------------------------

	void CSID::RegisterNotifyStatusCallback(NotifySIDStatusCallback p)
	{
		NotifyStatusCallback = p;
	}

	void CSID::RegisterMessageCallback(RaiseMessage pCallback)
	{
		RaiseMessageCallback = pCallback;
	}
	//----------------------------------------------------------------------------

	void CSID::RegisterErrorCallback(RaiseError pCallback)
	{
		RaiseErrorCallback = pCallback;
	}
	//----------------------------------------------------------------------------

	//Base methods
	void CSID::SetUID(unsigned int id)
	{
		uID = id;
	}

	/**
	  * @fn		CSID::GetUID()
	  * @brief	Ritorna l'identificativo univoco del driver
	  * @author Diego Gangale
	  * @date	02/04/2007	Created
	  * @date	05/04/2007	Modified
	  * @date	22/07/2007	Remarked
	  * @return	Identificativo numerico univoco del SID
	  *
	  */
	unsigned int CSID::GetUID()
	{
		return uID;
	}

	/**
	  * @fn		CSID::SetAlias(std::string alias)
	  * @brief	Imposta l'alias del driver
	  * @author Diego Gangale
	  * @date	02/04/2007	Created
	  * @date	05/04/2007	Modified
	  * @date	22/07/2007	Remarked
	  * @param	alias		Nome da assegnare al SID
	  *
	  * Utilizzato dal ThKernel per assegnare un nome al SID		\n
	  * Viene impostato lo stesso nome configurato sul database		\n
	  *
	  */
	void CSID::SetAlias(std::string alias)
	{
		sAlias = alias;
	}
	//----------------------------------------------------------------------------

	std::string CSID::GetAlias()
	{
		return sAlias;
	}
	//----------------------------------------------------------------------------

	int CSID::GetThId()
	{
		return this->m_ThreadCtx.m_dwTID;
	}
	//---------------------------------------------------------------

	//---------------------------------------------------------------
	// RUN: Who develops SID MUST override the following methods:
	//
	// - Init();
	// - ProcessCustomMessages();
	// - Close();
	//
	//---------------------------------------------------------------
	DWORD CSID::Run(LPVOID /* arg */)
	{
		MSG msg;

		terminated = false;

		if (NotifyStatusCallback != NULL)
			NotifyStatusCallback((int)GetUID(), StatusBeforeRun);
		SIDState = StatusBeforeRun;

		bool bCrashed = false;
		bTerminated = false;
		BeforeRun();

		//Notifico lo stato di esecuzione al kernel
		if (NotifyStatusCallback != NULL)
			NotifyStatusCallback(GetUID(), StatusRun);
		SIDState = StatusRun;

		while (!bTerminated)               // Waiting for system messages
		{
			try
			{
				GetMessage(&msg, NULL, 0, 0);
				TranslateMessage(&msg);

				//Messages management
				switch (msg.message)
				{

				case SID_EVENT_CRASH:
					bCrashed = true; // NO BREAK!!!
				case SID_EVENT_TERMINATE:
				case SID_EVENT_TERMINATE_NOW:
					bTerminated = true;
					break;

				case SID_EVENT_PING:
					if (PingCallback != NULL)
						PingCallback(GetUID());
					break;

					//DI
				case CMD_DI_ENABLE:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.diIf->OnEnableDI(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				case CMD_DI_DISABLE:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.diIf->OnDisableDI(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				//ALMMGR
				case CMD_ALMMGR_SET:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.almMgrIf->OnSetAlarm(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				case CMD_ALMMGR_RESET:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.almMgrIf->OnResetAlarm(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				case CMD_ALMMGR_ACK:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.almMgrIf->OnAckAlarm(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				case CMD_ALMMGR_ENABLE:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.almMgrIf->OnEnableAlarm(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				case CMD_ALMMGR_DISABLE:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.almMgrIf->OnDisableAlarm(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				case CMD_ALMMGR_PURGE:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.almMgrIf->OnPurgeAlarm(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				case CMD_ALMMGR_DELETE:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.almMgrIf->OnDeleteAlarm(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				//DO
				case CMD_DO_ON_UNDEFINED:
				case CMD_DO_ON_DURATION:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.doIf->OnOpenDO(cmdParameters->idCommand, cmdParameters->subSysId,
						cmdParameters->DO.iDuration);
					delete cmdParameters;
				}
				break;

				case CMD_DO_OFF:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.doIf->OnCloseDO(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				case CMD_DO_ENABLE:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.doIf->OnEnableDO(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				case CMD_DO_DISABLE:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.doIf->OnDisableDO(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				//PTZ
				case CMD_PTZ_SETMOVESPEED:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZSetMoveSpeed(cmdParameters->idCommand, cmdParameters->idDevice, cmdParameters->PTZ.iSpeed);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_MOVEUP:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZMoveUp(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_MOVEDOWN:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZMoveDown(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_MOVELEFT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZMoveLeft(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_MOVERIGHT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZMoveRight(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_MOVEUPLEFT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZMoveUpLeft(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_MOVEUPRIGHT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZMoveUpRight(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_MOVEDOWNLEFT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZMoveDownLeft(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_MOVEDOWNRIGHT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZMoveDownRight(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_STOP:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZStop(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_GOTOPRESET:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZGotoPreset(cmdParameters->idCommand, cmdParameters->idDevice, cmdParameters->PTZ.iPreset);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_SETPRESET:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZSetPreset(cmdParameters->idCommand, cmdParameters->idDevice, cmdParameters->PTZ.iPreset);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_LOADPRESETTABLE:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZLoadPresetTable(cmdParameters->idCommand, cmdParameters->idDevice, cmdParameters->PTZ.sPresetTable);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_GOTOABSPOSITION:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZGotoAbsPosition(cmdParameters->idCommand, cmdParameters->idDevice, cmdParameters->PTZ.iAbsPan, cmdParameters->PTZ.iAbsTilt, cmdParameters->PTZ.iAbsZoom);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_CONNECT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZConnect(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_PTZ_DISCONNECT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.ptzIf->OnPTZDisconnect(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				//CAMERA
				case CMD_CAMERA_SETMOVESPEED:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraSetMoveSpeed(cmdParameters->idCommand, cmdParameters->idDevice, cmdParameters->PTZ.iSpeed);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_MOVEUP:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraMoveUp(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_MOVEDOWN:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraMoveDown(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_MOVELEFT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraMoveLeft(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_MOVERIGHT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraMoveRight(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_MOVEUPLEFT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraMoveUpLeft(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_MOVEUPRIGHT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraMoveUpRight(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_MOVEDOWNLEFT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraMoveDownLeft(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_MOVEDOWNRIGHT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraMoveDownRight(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_STOP:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraStop(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_GOTOPRESET:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraGotoPreset(cmdParameters->idCommand, cmdParameters->idDevice, cmdParameters->PTZ.iPreset);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_SETPRESET:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraSetPreset(cmdParameters->idCommand, cmdParameters->idDevice, cmdParameters->PTZ.iPreset);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_LOADPRESETTABLE:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraLoadPresetTable(cmdParameters->idCommand, cmdParameters->idDevice, cmdParameters->PTZ.sPresetTable);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_GOTOABSPOSITION:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraGotoAbsPosition(cmdParameters->idCommand, cmdParameters->idDevice, cmdParameters->PTZ.iAbsPan, cmdParameters->PTZ.iAbsTilt, cmdParameters->PTZ.iAbsZoom);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_SETBACKLIGHTON:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraSetBacklight(cmdParameters->idCommand, cmdParameters->idDevice, true);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_SETBACKLIGHTOFF:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraSetBacklight(cmdParameters->idCommand, cmdParameters->idDevice, false);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_ZOOMIN:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraZoomIn(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_ZOOMOUT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraZoomOut(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_CONNECT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraConnect(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_CAMERA_DISCONNECT:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.cameraIf->OnCameraDisconnect(cmdParameters->idCommand, cmdParameters->idDevice);
					delete cmdParameters;
				}
				break;

				case CMD_ZONE_ARMED:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.zoneIf->OnArmZone(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				case CMD_ZONE_DISARMED:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.zoneIf->OnDisarmZone(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				case CMD_ZONE_ENABLE:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.zoneIf->OnEnableZone(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;

				case CMD_ZONE_DISABLE:
				{
					SIDCommandParameters* cmdParameters = (SIDCommandParameters*)msg.wParam;
					sidIfMap.zoneIf->OnDisableZone(cmdParameters->idCommand, cmdParameters->subSysId);
					delete cmdParameters;
				}
				break;
				case SIDMON_UPDATE_DATA:
				//{
				//	ChangeEvent* pUpdateEvent = (ChangeEvent*)msg.lParam;
				//	std::string oType = pUpdateEvent->fObjectType;
				//	if (oType == "Person")
				//	{
				//		Person* p = (Person*)pUpdateEvent->data;
				//		sidIfMap.ctrlAccessIf->SavePerson(*p);
				//		delete p;
				//	}
				//	if (oType == "Badge")
				//	{
				//		Badge* b = (Badge*)pUpdateEvent->data;
				//		sidIfMap.ctrlAccessIf->SaveBadge(*b);
				//		delete b;
				//	}
				//	delete pUpdateEvent;
				//}
				break;

				default:
					//Manage your application messages here
					ProcessCustomMessages(msg);
					//Dispatch dei messaggi
					//DispatchMessage(&msg);  // ?????  
					break;
				}
			}
			catch (std::exception& e)
			{
				//t.b.d.
				//Che si verifichi un errore qui è grave
				e.what();
			}
		}

		//Notifico lo stato di esecuzione al kernel
		if (NotifyStatusCallback != NULL)
			NotifyStatusCallback(this->GetUID(), StatusAfterRun);
		SIDState = StatusAfterRun;

		AfterRun();

		//Notifico lo stato di esecuzione al kernel
		if (NotifyStatusCallback != NULL && msg.message != SID_EVENT_TERMINATE_NOW)
			NotifyStatusCallback(GetUID(), (bCrashed) ? StatusCrashed : StatusTerminated);
		SIDState = StatusTerminated;

		terminated = true;

		return m_ThreadCtx.m_dwExitCode;
	}


	/**
	  * @fn		CSID::SetCfgParameter (std::string sSection, std::string sName, std::string sValue)
	  * @brief	Inserisce un nuovo parametro (sezione, nome, valore) nella mappa dei parametri di configurazione
	  * @author Diego Gangale
	  * @date	02/04/2007	Created
	  * @date	05/04/2007	Modified
	  * @date	22/07/2007	Remarked
	  * @param	sSection	Sezione che raggruppa i parametri (stile .ini file)
	  * @param	sName		Nome del parametro
	  * @param	sValue		Valore del parametro (stringa)
	  * @see	GetCfgParameter
	  *
	  * Utilizzato dal ThKernel per inserire un nuovo valore nella mappa dei parametri utilizzabile da ogni istanza di driver \n
	  * Sul database vengono impostati i parametri per ogni SID (gestore del sottosistema) a cui viene associato un identificativo univoco \n
	  * Il ThKernel per per ogni istanza di SID utilizza questa funzione per riempire la tabella dei parametri di configurazione \n
	  * Il SID può utilizzare direttamente la mappa o la funzione GetCfgParameter per recuperare i valori configurati \n
	  *
	  */
	void CSID::SetCfgParameter(std::string sSection, std::string sName, std::string sValue)
	{
		try
		{
			mapCfgKeys[sSection + "|" + sName] = sValue;
		}
		catch (...)
		{
			;
		}
	}

	/**
	  * @fn		CSID::GetCfgParameter (std::string sSection, std::string sName)
	  * @brief	Recupera il valore di un parametro di configurazione in base alla coppia (sezione, nome)
	  * @author Diego Gangale
	  * @date	02/04/2007	Created
	  * @date	05/04/2007	Modified
	  * @date	22/07/2007	Remarked
	  * @param	sSection	Sezione che raggruppa i parametri (stile .ini file)
	  * @param	sName		Nome del parametro
	  * @return	Valore del parametro (stringa)
	  *
	  * Viene utilizzato dallo sviluppatore del SID per ottenere le informazioni necessarie alla inizializzazione del driver
	  *
	  */
	std::string CSID::GetCfgParameter(std::string sSection, std::string sName)
	{
		std::string sRetValue;
		try
		{
			if (mapCfgKeys.find(sSection + "|" + sName) != mapCfgKeys.end())
				sRetValue = mapCfgKeys[sSection + "|" + sName];
		}
		catch (...)
		{
			;
		}
		return sRetValue;
	}
	//----------------------------------------------------------------------------
}
//----------------------------------------------------------------------------