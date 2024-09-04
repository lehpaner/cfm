/****************************** Module Header ******************************\
* Module Name:  BaseComm
*
* Base system comminication interfaces...
*
\***************************************************************************/
#pragma once
#include "framework.h"
#include "CfmDomainObjects.h"
#include "CallbackTypes.h"
#include "Thread.h"
//-------------------------------------------------------------
//SYSTEM EVENTS - USED BY CComm BASE
//-------------------------------------------------------------
//(API_MESSAGES_BASE + APIMESSAGERANGE * 15)
#define CCOMM_EVENT_BASE				BASECOMM_MESSAGES_BASE
#define CCOMM_EVENT_TERMINATE			CCOMM_EVENT_BASE + 1
#define CCOMM_EVENT_PING				CCOMM_EVENT_BASE + 2
#define CCOMM_PUSH_DEVICE_EVENT			CCOMM_EVENT_BASE + 3
#define CCOMM_PUSH_SYS_MESSAGE			CCOMM_EVENT_BASE + 4  //Invio messaggi di sistema

namespace cfm::application {

	//Tipi funzioni di accesso a SID nelle DLL
	typedef void* (*createObj)		();
	typedef void	(*destroyObj)		(void*);

	typedef struct {
		long idEvent;
		domain::CfmDevices_Table deviceData;
		std::string MethodName;
	} pushDeviceData;

	typedef struct {
		SystemMessage sysMessage;
		std::string MethodName;
	} pushSysMessageData;


	/*
	   - Include ed escludere allarme
	   - Attivazione / Disattivazione sensore
	   - Ack dell'alarme
	   - Purge allarme
	   - Arma/Disarma Zona
	   - Apertura chiusura DO
	*/

	//-------------------------------------------------------------------------------
	//INTERFACCIA INTERFACCIA GENERALE Communication Adapter (DA ESTENDERE)
	//-------------------------------------------------------------------------------
	extern "C"
	{
		// OSC 12-04-2010
#pragma warning(disable:4275)
		class __declspec(dllexport) CComm : public CThread
		{
			// OSC 12-04-2010
#pragma warning(default:4275)
		public:
			bool PushDeviceEvent(long idEvent, domain::CfmDevices_Table deviceData, std::string MethodName);
			bool PushSysMessage(SystemMessage sysMessage, std::string MethodName);

			//Usate dal Kernel in inizializzazione
			void RegisterMessageCallback(RaiseMessage pCallback); //Invio messaggio al kernel
			void RegisterErrorCallback(RaiseError pCallback);   //Invio messaggio di errore al kernel
			//Ad uso interno del Kernel
			void RegisterPingCallback(PingRespCallback p);		//invio risposte al Communication Adapter Monitor
			//Per i comandi
			void RegisterCmdCallback(CommAdapterCommandCallback p);  //Invio comando da Gui (ack, purge...)


			//Per i parametri di configurazione
			void SetCfgParameter(std::string sSection, std::string sName, std::string sValue);  //La usa il kernel per settare i parametri
			//*************************************************************************
			// USED by SID Developers 
			//*************************************************************************
			std::string GetCfgParameter(std::string sSection, std::string sName);

			//Da invocare per inviare informazioni al Kernel
			CommAdapterCommandCallback commandCallback;


			//***************************************
			// MUST be overridden by CommAdapter developer
			// ret values:
			//		SUCCESS		0
			//     FAILURE	   -1
			//
			//***************************************
			virtual int InitAdapter() { return -1; };
			virtual int CloseAdapter() { return -1; };

			CComm();
			int GetThId(); //Ritorna l'id del thread base
			void SetUID(unsigned int id);
			unsigned int GetUID();
			int Terminate();
			DWORD Restart();
			void AsyncPing();
			std::string GetAlias();
			void SetAlias(std::string alias);

		private:

			//************************************************************************
			// CALLBACKS (Risposta alle richieste del Communication Adapter Monitor)
			//************************************************************************
			PingRespCallback	PingCallback;

			//*******************************
			// For internal use
			//*******************************
			virtual DWORD Run(LPVOID /* arg */);
			bool bTerminated;			//Indica al Main Loop di terminare
			unsigned int uID;			//Id numerico univoco assegnato dal ThKernel in fase 
			//di inizializzazione (impostato su DB - ID)
			std::string sAlias;		//Id alfanumerico univoco assegnato dal ThKernel in fase 
			//di inizializzazione (impostato su DB - ALIAS) 

		protected:
			//*******************************************************
			//MUST be overridden by Communication Adapter developer
			//*******************************************************
			virtual void BeforeRun() = 0;
			virtual void ProcessCustomMessages(MSG& msg) = 0;
			virtual void AfterRun() = 0;
			virtual bool OnPushDeviceEvent(long idEvent, domain::CfmDevices_Table deviceData, std::string MethodName) = 0;
			virtual bool OnPushSysMessage(SystemMessage sysMessage, std::string MethodName) = 0;

			//*************************************
			// CALLBACKS to send event to KERNEL 
			//*************************************
			RaiseError			RaiseErrorCallback;
			RaiseMessage		RaiseMessageCallback;
			//Queste vanno invocate dall'adapter all'arrivo di un comando dal WebMonitor (xmlrpc server )
			//lo devo  fare
			//lo devo da fa

#pragma warning(disable:4251)
			std::map <std::string, std::string> mapCfgKeys; // hash table contenente la configurazione del SID
#pragma warning(default:4251)


		};

	}
}
