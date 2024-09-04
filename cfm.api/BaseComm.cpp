/****************************** Module Header ******************************\
* Module Name:  BaseComm
*
* Base system comminication interfaces...
*
\***************************************************************************/
#include "BaseComm.h"

namespace cfm::application {
	/**
	* @fn		CComm::CComm()
	* @brief	Costruttore oggetto Common Adapter
	*
	* In fase di costruzione dell'oggetto vengono posti a NULL tutti i puntatori alla callbacks
	*
	*/
	// Costruttore CComm
	CComm::CComm() {
		bTerminated = false;
	}
	//----------------------------------------------------------------------------

	//Callbacks registration

	//Registrazione della callback per l'evento Ping
	void CComm::RegisterPingCallback(PingRespCallback p) {
		PingCallback = p;
	}
	//----------------------------------------------------------------------------

	void CComm::RegisterMessageCallback(RaiseMessage pCallback) {
		RaiseMessageCallback = pCallback;
	}
	//----------------------------------------------------------------------------

	void CComm::RegisterErrorCallback(RaiseError pCallback) {
		RaiseErrorCallback = pCallback;
	}
	//----------------------------------------------------------------------------


	//---------------------------------------------------------------
	// RUN: Who develops Communication Adapter MUST override the following methods:
	//
	// - Init();
	// - ProcessCustomMessages();
	// - Close();
	//
	//---------------------------------------------------------------
	DWORD CComm::Run(LPVOID /* arg */) {
		MSG msg;

		BeforeRun();
		// Waiting for system messages
		while (!bTerminated) {
			try {
				bool r = (bool)GetMessage(&msg, NULL, 0, 0);
				//TranslateMessage(&msg);

				//Messages management
				switch (msg.message) {
				case CCOMM_EVENT_TERMINATE:
					bTerminated = true;
					break;

				case CCOMM_EVENT_PING:
					if (PingCallback != NULL)
						PingCallback(GetUID());
					break;

				case CCOMM_PUSH_DEVICE_EVENT: 
				{
					pushDeviceData* p = (pushDeviceData*)msg.wParam;
					if (p)
					{
						OnPushDeviceEvent(p->idEvent, p->deviceData, p->MethodName);
						delete p;
					}
				}
				break;

				case CCOMM_PUSH_SYS_MESSAGE:
				{
					pushSysMessageData* p = (pushSysMessageData*)msg.wParam;
					if (p)
					{
						OnPushSysMessage(p->sysMessage, p->MethodName);
						delete p;
					}
				}

				default:
					//Manage your application messages here
					ProcessCustomMessages(msg);
					//Dispatch dei messaggi
					//DispatchMessage(&msg);  // ?????  
					break;
				}
			}
			catch (...)
			{
				printf("utch...");
			}
		}

		AfterRun();

		return m_ThreadCtx.m_dwExitCode;
	}

	/**
	  * @fn		CComm::AsyncPing()
	  * @brief	Ping asincrono del thread base
	  * @author Diego Gangale
	  * @date	02/04/2010	Created
	  *
	  * Viene inviato il messaggio CCOMM_EVENT_PING al thread base
	  *
	  */
	void CComm::AsyncPing() {
		PostThreadMessage(GetThId(), CCOMM_EVENT_PING, 0, 0);
	}
	//----------------------------------------------------------------------------

	int CComm::GetThId() {
		return this->m_ThreadCtx.m_dwTID;
	}


	//Base methods
	void CComm::SetUID(unsigned int id) {
		uID = id;
	}


	/**
	  * @fn		CComm::GetUID()
	  * @brief	Ritorna l'identificativo univoco del Communication Adapter
	  * @author Diego Gangale
	  * @date	02/04/2010	Created
	  * @return	Identificativo numerico univoco del Communication Adapter
	  *
	  */
	unsigned int CComm::GetUID() {
		return uID;
	}
	// Termina l'esecuzione del SID
	int CComm::Terminate() {
		//Si invia a se stessi (main loop) il messaggio di terminazione
		PostThreadMessage(GetThId(), CCOMM_EVENT_TERMINATE, 0, 0);
		return 0;
	}

	/*
	void CComm::RegisterMessageCallback(RaiseMessage pCallback) //Invio messaggio al kernel
	{
		RaiseMessageCallback = pCallback;
	}
	void CComm::RegisterErrorCallback(RaiseError pCallback)   //Invio messaggio di errore al kernel
	{
		RaiseErrorCallback = pCallback;
	}

	void CComm::RegisterPingCallback(PingRespCallback p)		//invio risposte al Communication Adapter Monitor
	{
		PingCallback = p;
	}
	*/
	//Invio comando su allarme da Gui (ack, purge...)
	void CComm::RegisterCmdCallback(CommAdapterCommandCallback p) {
		commandCallback = p;
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
	void CComm::SetAlias(std::string alias) {
		sAlias = alias;
	}
	//----------------------------------------------------------------------------

	std::string CComm::GetAlias() {
		return sAlias;
	}

	/**
	  * @fn		CComm::Restart()
	  * @brief	Riavvio del thread base
	  * @author Diego Gangale
	  * @date	02/04/2010	Created
	  *
	  * L'attributo bTerminated viene posto a false. Viene richiamato il metodo Start();
	  *
	  */
	DWORD CComm::Restart() {
		bTerminated = false;
		return (Start());
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
	void CComm::SetCfgParameter(std::string sSection, std::string sName, std::string sValue) {
		try {
			std::string sKey = sSection + "|" + sName;
			mapCfgKeys.insert(std::pair<std::string, std::string>(sKey, sValue));
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
	std::string CComm::GetCfgParameter(std::string sSection, std::string sName) {
		std::string sRetValue;
		try {
			sRetValue = mapCfgKeys[sSection + "|" + sName];
		}
		catch (...)
		{
			;
		}
		return sRetValue;
	}
	//----------------------------------------------------------------------------

	bool CComm::PushDeviceEvent(long idEvent, domain::CfmDevices_Table deviceData, std::string MethodName) {
		pushDeviceData* p = new pushDeviceData;
		p->idEvent = idEvent;
		p->deviceData = deviceData;
		p->MethodName = MethodName;

		PostThreadMessage(GetThId(), CCOMM_PUSH_DEVICE_EVENT, (WPARAM)p, 0);
		return true;
	}
	//----------------------------------------------------------------------------

	bool CComm::PushSysMessage(SystemMessage sysMessage, std::string MethodName) {
		pushSysMessageData* p = new pushSysMessageData;
		p->MethodName = MethodName;
		p->sysMessage = sysMessage;

		PostThreadMessage(GetThId(), CCOMM_PUSH_SYS_MESSAGE, (WPARAM)p, 0);
		return true;
	}


}