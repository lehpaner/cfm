/****************************** Module Header ******************************\
* Module Name:  CallbackTypes.h
*
* Base system interfaces...
*
\***************************************************************************/

#pragma once

#include "framework.h"
#include "CategoryTypes.h"

namespace cfm::application {
	//Risposta base ad un comando
	struct PayloadRespBase
	{
		//28 09 2007
		std::string	id; //SubId - Indica da quale dispositivo in particolare sta arrivando la risposta
		int SIDUID;
		unsigned int idCommand;
		std::string description;
		std::string sSourceTS;

		/**
		 * returns the former datetime for tagging as formatted string "YYYYMMDD hh:mm:ss.xxx"
		 * xxx = milliseconds
		 */
		std::string	GetTime() {
			char sRetVal[255];
			SYSTEMTIME    timeNow;
			GetLocalTime(&timeNow);
			sprintf_s(sRetVal, sizeof(sRetVal), "%04i%02i%02i %02i:%02i:%02i.%03i", timeNow.wYear, timeNow.wMonth, timeNow.wDay,
				timeNow.wHour, timeNow.wMinute, timeNow.wSecond, timeNow.wMilliseconds);
			return sRetVal;
		}

		PayloadRespBase() {
			SIDUID = -1;
			id = "-1";
			idCommand = 0;
			description = "";
			sSourceTS = GetTime();
		};
	};


	/**
	 * DEFINIZIONE DELLA CALLBAKS DI SISTEMA E DI CATEGORIA
	 */


	 //CALLBACK PER INVIO MESSAGGI CHE I SID RICHIEDONO DI LOGGARE AL KERNEL

	 /**
	   * @class	   SystemMessage
	   * @author    Gangale, Abbruzzese
	   * @copyright I&SI S.p.A.
	   * @date      12/06/2007
	   * @brief     contains the last system message logged from SID to Kernel
	   * @version   1.0
	   *
	   */
	class SystemMessage {
	public:
		int				SIDUID;			/**< SID Unique Identifier					*/
		int				AppCode;		/**< message/error application code         */
		std::string		AppDescription; /**< message/error application description	*/
		std::string		Source;			/**< who the sender is						*/
		int				Severity;		/**< message severity/debug level			*/
		unsigned long	msgCount;		/**< subsystem progressive message count ID */
		std::string     sysTime;		/**< message creation date and time			*/

		/**
		 * returns the former datetime for tagging as formatted string "YYYYMMDD hh:mm:ss.xxx"
		 * xxx = milliseconds
		 */
		std::string	GetTime()
		{
			char sRetVal[255];
			SYSTEMTIME    timeNow;
			GetLocalTime(&timeNow);
			sprintf_s(sRetVal, sizeof(sRetVal), "%04i%02i%02i %02i:%02i:%02i.%03i", timeNow.wYear, timeNow.wMonth, timeNow.wDay,
				timeNow.wHour, timeNow.wMinute, timeNow.wSecond, timeNow.wMilliseconds);
			return sRetVal;
		};

		SystemMessage() {
			SIDUID = 0;
			AppCode = 0;
			AppDescription = "";
			Source = "";
			Severity = 0;
			msgCount = 0;
			sysTime = GetTime();
		};

		SystemMessage(SystemMessage& p) {
			SIDUID = p.SIDUID;
			AppCode = p.AppCode;
			AppDescription = p.AppDescription;
			Source = p.Source;
			Severity = p.Severity;
			msgCount = p.msgCount;
			sysTime = p.sysTime;
		};
	};

	/**
	  * @class	   SystemError
	  * @author    Gangale, Abbruzzese
	  * @copyright I&SI S.p.A.
	  * @date      12/06/2007
	  * @brief     Contains the last system error logged from SID to Kernel.
	  *			   Inherhits from SystemMessage and adds further info
	  * @version   1.0
	  *
	  */
	class SystemError : public SystemMessage
	{
	public:
		int				NativeCode;			/**< error native code				*/
		std::string		NativeDescription;  /**< error native description		*/
		bool			Unexpected;			/**< true if it is a system bug		*/

		/** inherited properties need to be initialized */
		SystemError() : SystemMessage()  {
			NativeCode = 0;
			NativeDescription = "";
			Unexpected = false;
		};
	};
	typedef void (*RaiseError)   (int           UID,
		int	        AppCode,
		std::string	AppDescription,
		int			Severity,
		int			NativeCode,
		std::string	NativeDescription,
		std::string	Source,
		bool			Unexpected,
		std::string   sysTime);	   /** Callback prototype */

	typedef void (*RaiseMessage) (int           UID,
		int	        AppCode,
		std::string	AppDescription,
		int			Severity,
		std::string	Source,
		std::string   sysTime);		/** Callback prototype */

	/**
	 * CALLBACK PER RISPOSTA AL PING ASINCRONO
	 */
	typedef void (*PingRespCallback)(int SIDUID);

	/*
	 * CALLBACK UTILIZZATA DAL SID PER NOTIFICARE IL PROPRIO STATO DI ESECUZIONE (AFTER RUN, RUN, BEFORE RUN)
	 */
	enum SIDExecStates {
		StatusUndefined = 0,
		StatusNotRunning = 1,
		StatusRun = 2,
		StatusNotResponding = 4,
		StatusEnteringMaintenance = 6,
		StatusUnderMaintenance = 7,
		StatusExitingMaintenance = 8,
		StatusInitializing = 9,
		StatusBeforeRun = 10,
		StatusAfterRun = 11,
		StatusCrashed = 13,
		StatusTerminated = 14
	};


	typedef void (*NotifySIDStatusCallback) (int SIDUID, SIDExecStates status);


	//CALLBACKS CATEGORIA TRK

	// CALLBACK per i dati di traccia (ad ex Sonar, Radar)
	typedef void (*TrackEventCallback)(TRKData* data, int iEvt, int SIDUID);

	// CALLBACK per i dati di lettura ocr (ad ex Lettori accesso veicoli, casse)
	typedef void (*OCREventCallback)(OCRData* data, int iEvt, int SIDUID);


	//CALLBACKS CATEGORIA READER
	typedef void (*BadgeEventCallback) (READERData* data, int Evt, int SIDUID);


	//-----------------------------------------------
	//	THO.24.06.2009:		CALLBACKS CATEGORIA ZONA
	typedef void (*ZoneEventCallback)(ZONEData* data, int iEvt, int SIDUID);
	typedef void (*ZoneRespCallback) (PayloadRespBase* data, int iEvt, int SIDUID);
	//-----------------------------------------------

	// CALLBACKS CATEGORIA ALARM MANAGER
	typedef void (*AlmManagerEventCallback)(ALMMGRData* data, int iEvt, int SIDUID);
	typedef void (*AlmManagerRespCallback) (PayloadRespBase* data, int iEvt, int SIDUID);

	// ON COMMAND CATEGORIA ALARM MANAGER

	// CALLBACKS CATEGORIA DI
	typedef void (*DiEventCallback)(DIData* data, int iEvt, int SIDUID);

	// CALLBACKS CATEGORIA DO
	typedef void (*DoEventCallback)(DOData* data, int iEvt, int SIDUID);
	typedef void (*DoRespCallback) (PayloadRespBase* data, int iEvt, int SIDUID);

	// CALLBACKS CATEGORIA DVR
	typedef void (*DvrEventCallback)(void* data, int iEvt, int SIDUID);
	typedef void (*DvrRespCallback) (PayloadRespBase* data, int iEvt, int SIDUID);

	// CALLBACKS CATEGORIA VIDEO SERVER
	typedef void (*VsEventCallback)(void* data, int iEvt, int SIDUID);

	// CALLBACKS CATEGORIA SERIAL
	typedef void (*SerialEventCallback)(void* data, int iEvt, int SIDUID);

	// CALLBACKS CATEGORIA PTZ
	typedef void (*PtzEventCallback)(void* data, int iEvt, int SIDUID);
	typedef void (*PtzRespCallback) (PayloadRespBase* data, int iEvt, int SIDUID);

	// CALLBACKS CATEGORIA CAMERA
	typedef void (*CameraEventCallback)(void* data, int iEvt, int SIDUID);
	typedef void (*CameraRespCallback) (PayloadRespBase* data, int iEvt, int SIDUID);

	// CALLBACKS CATEGORIA CTLPTZ
	typedef void (*CtlPtzEventCallback)(void* data, int iEvt, int SIDUID);

	// CALLBACKS CATEGORIA CTRLACCESS
	typedef void (*CABadgeEventCallback)(void* data, int iEvt, int SIDUID);
	typedef void (*CABadgeRespCallback) (PayloadRespBase* data, int iEvt, int SIDUID);
	typedef void (*CAPersonEventCallback)(void* data, int iEvt, int SIDUID);
	typedef void (*CAPersonRespCallback) (PayloadRespBase* data, int iEvt, int SIDUID);

	// CALLBACKS CATEGORIA CDC
	typedef void (*CdcEventCallback)(CDCData* data, int iEvt, int SIDUID);

	//Tipologia di stato di punti di input digitali
	enum DStatus { Off, On, Short, Cut };



	/**
	 * CALLBACKS NECESSARIE AI COMMUNICATION ADAPTER
	 */
	typedef bool (*CommAdapterCommandCallback) (int tipo, int device, std::string action, std::map<std::string, std::string> parametri);

}
