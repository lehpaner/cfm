/****************************** Module Header ******************************\
* Module Name:  CfmHandlesMaps.h
* Project:      S000
*
*
* Complex domain structures
*
\***************************************************************************/
#pragma once

#include "framework.h"
#include "CallbackTypes.h"
#include "BaseSID.h"
#include "BaseComm.h"
#include "Timers.h"

namespace cfm::application {
	//record relativo alla mappa usata dal DeviceMonitor
	struct CfmDeviceHandleMap {
		void* obj;
		CSID* SIDIf;
		clock_t AliveTimestamp;
		bool RealTimeContext;
		int  ThreadPriority;
		bool bInitOk;

		//Remoting
		bool Remote;
		void* RSIDIf; //Network adapter object

		bool UnderMaintenance;

		int Status;
		int SIDId;

		CfmDeviceHandleMap() {
			obj = NULL;
			SIDIf = NULL;
			RealTimeContext = false;
			ThreadPriority = 0;
			AliveTimestamp = clock();

			//Remoting
			Remote = false;
			RSIDIf = NULL;

			UnderMaintenance = false;

			Status = StatusUndefined;
			SIDId = 0;
		};
	};

	//record relativo alla mappa usata dal ThSIDMonitor
	struct CfmCommAdapterHandleMap {
		CComm* CommAdapterIf;    /**< Puntatore all'istanza dell'adapter */
		clock_t AliveTimestamp;  /**< Timestamp dell'ultimo KeepAlive andato a buon fine */
		bool bInitOk;			 /**< Inizializzazione andata a buon fine? */
		int Status;				 /**< Mo decido gli stati possibili (int-->enum).....  */

		TTimersThread* timer;

		CfmCommAdapterHandleMap(bool watchdog) {
			CommAdapterIf = NULL;
			AliveTimestamp = clock();
			Status = 1; //NOT RUNNING

			//timer = (watchdog) ? new TTimersThread(1, SECS) : NULL;
			timer = (watchdog) ? new TTimersThread(1, 1000) : nullptr;
		}
		~CfmCommAdapterHandleMap() {
			if (timer)
				delete timer;
		}
	};

	//Record relativo alla mappa usata dal Kernel per memorizzare gli HANDLE alle DLL
	struct  CfmDLLHandleMap {
		//SaraSystem_Table *data;
		HMODULE DllSIDHandle;
		bool bLoaded;
		// OSC 15-04-2010
		std::map<int, void*> objMap; //l'indice è il SystemId
		std::map<int, domain::CfmSystem_Table*> dataMap; //idem

		CfmDLLHandleMap() {
			//data = NULL;
			DllSIDHandle = NULL;
			bLoaded = false;
		};

		void UnregisterSystem(int idSystem)
		{
			std::map<int, void*>::iterator it;
			if ((it = objMap.find(idSystem)) != objMap.end())
				objMap.erase(it);

			std::map<int, domain::CfmSystem_Table*>::iterator it2;
			if ((it2 = dataMap.find(idSystem)) != dataMap.end()) {
				delete it2->second;
				dataMap.erase(it2);
			}
		}
	};
} //end namespace
