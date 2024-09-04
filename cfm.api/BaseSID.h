/****************************** Module Header ******************************\
* Module Name:  BaseSID
*
* Base system interfaces...
*
\***************************************************************************/
#pragma once
#include "framework.h"

#include <set>
#include <vector>

#include "CallbackTypes.h"
#include "Thread.h"
#include "CfmApiMessages.h"

#define SUCCESS		 0	
#define FAILURE		-1

#define	DEVICE_ENABLE		1
#define	DEVICE_DISABLE		0
#define	STATUS_UNDEFINED	-99

//-------------------------------------------------------------
//SYSTEM EVENTS - USED BY SID BASE
//-------------------------------------------------------------
#define SID_EVENT_BASE				BASESID_MESSAGES_BASE	
#define SID_EVENT_TERMINATE			SID_EVENT_BASE + 1
#define SID_EVENT_PING				SID_EVENT_BASE + 2
#define SID_EVENT_CRASH				SID_EVENT_BASE + 3
#define SID_EVENT_TERMINATE_NOW		SID_EVENT_BASE + 4
//-------------------------------------------------------------
// EXPORT DEI SIMBOLI DEFINITI NELLA DLL
//-------------------------------------------------------------
#define CFM_API_DLL				__declspec(dllexport)

namespace cfm::application {
	// OSC 04-05-2010	
	// La Caterory è stata ridefinita in base ai valori del campo Id della tabella Categories.
	// In caso di aggiunta o variazione di categorie, occorre intervenire sulla tabella Categories 
	// e sulla enum Category
	//enum Category		// Veccia definizione
	//{
	//	DI = 1, DO = 2, TRK = 3, ALMMGR = 5, READER = 11, DVR = 6, PTZ, ZONE, VS, SERIAL = 4
	//};
	enum Category {
		catUndefined = 0,
		DI = 1,
		DO = 2,
		TRK = 3,
		SERIAL = 4,
		ALMMGR = 5,
		DVR = 6,
		OCR = 8,
		PTZ = 9,
		READER = 11,
		CTLPTZ = 12,
		ZONE = 13,
		CAMERA = 14,
		VIDEOSRV = 15,
		CDC = 16,
		SDI = 17,
		SDO = 18,
		VCH = 19,
		CTRLACCESS = 20
	};

	class Device {
	protected:
		char NAME[32];
	public:
		std::string id;
		std::string description;
		int			enabled;
		int			status;
		Category	parentCatId;	// Categoria del dispositivo sorgente dell'allarme
		std::string	parentId;		// Dispositivo sorgente dell'allarme
		int			siteId;
		std::string datetime;
		std::set<std::string> setZone;

		Device() {
			memset(NAME, 0, sizeof(NAME));
			strncpy(NAME, "Device", sizeof(NAME));

			enabled = DEVICE_ENABLE;
			status = STATUS_UNDEFINED;
			parentCatId = catUndefined;	//Categoria del dispositivo sorgente dell'allarme
			siteId = 0;
		}
	};

	//----------------------------------------------
	// Pacchetto parametri per comandi asincroni
	//----------------------------------------------
	class SIDCommandParameters {
	protected:
		char NAME[32];

	public:
		unsigned int	idCommand;		//Identificativo univoco del comando
		unsigned long	idDevice;		//Identificativo di SARA del dispositivo su cui effettuare l'azione
		std::string		subSysId;		//Identificativo del dispositivo (del sistema controllato) su cui effettuare l'azione

		struct sDO {
			unsigned int iDuration;

			sDO() {
				iDuration = 0;
			};
		};

		sDO DO;

		struct sPTZ {
			unsigned int iPreset;
			unsigned int iSpeed;  //0..100
			int iAbsPan;    //-180..180 centesimi di grado
			int iAbsTilt;   //-90..90   centesimi di grado 
			int iAbsZoom;   //0..100    fattore di zoom
			std::string  sPresetTable; //nome del file di configuazione (da valutare se serve davvero)

			sPTZ() {
				iPreset = 0;
				iSpeed = 50;
				iAbsPan = 0;
				iAbsTilt = 0;
				iAbsZoom = 0;
				sPresetTable = "";
			};
		};

		sPTZ PTZ;

		SIDCommandParameters() {
			memset(NAME, 0, sizeof(NAME));
			strncpy(NAME, "SIDCommandParameters", sizeof(NAME));

			idCommand = 0;
			idDevice = 0;
			subSysId = "";
		};
	};

	//-----------------------------------
	// SID Messages
	//-----------------------------------
	struct SIDMessage {
		unsigned int	idCommand;
		time_t			timestamp;
		void* payload;	//Il tipo differisce se il messaggio è utilizzato per CMD o per RESP
		//ed in base alla richiesta / risposta effettuata
	};

	//Oggetto zona
	//struct DZone
	class DZone : public Device {
	public:
		DZone() {
			memset(NAME, 0, sizeof(NAME));
			strncpy(NAME, "DZone", sizeof(NAME));
		}
	};
	struct PayloadGetZoneList {
		std::vector<DZone> zoneList;
	};

	// Payload per la RSP_ALMMGR_GETALMLIST_OK
	// struct Alarm
	class Alarm : public Device {
	public:
		int count;
		int priority;
		int processState;
		Alarm() {
			memset(NAME, 0, sizeof(NAME));
			strncpy(NAME, "Alarm", sizeof(NAME));
			priority = 0;
			count = 0;
			processState = 0;
		}
	};

	struct PayloadGetAlmList {
		std::vector<Alarm> alarmList;
	};

	class AlarmGroup {
	public:
		std::string		id;
		std::string		Name;
		std::string		SiteId;
		std::string		ModifiedTS;

		std::set<std::string> setIdAlarm;
	};

	//Payload per la RSP_READER_GETRDRLIST_OK
	// struct Reader 
	class Reader : public Device {
	public:
		Reader() {
			memset(NAME, 0, sizeof(NAME));
			strncpy(NAME, "Reader", sizeof(NAME));
		}
	};
	struct PayloadGetRdrList {
		std::vector<Reader> readerList;
	};

	//Payload per la RSP_DI_GETDILIST_OK
	//struct DIn
	class DIn : public Device {
	public:
		DIn() {
			memset(NAME, 0, sizeof(NAME));
			strncpy(NAME, "DIn", sizeof(NAME));
		}
	};

	struct PayloadGetDIList {
		std::vector<DIn> DIList;
	};

	//Payload per la RSP_DO_GETDOLIST_OK
	//struct DOut
	class DOut : public Device{
	public:
		DOut() {
			memset(NAME, 0, sizeof(NAME));
			strncpy(NAME, "DOut", sizeof(NAME));
		}
	};
	struct PayloadGetDOList {
		std::vector<DOut> DOList;
	};

	//Payload per la GetPTZList
	class Positioner : public Device {
	public:
		Positioner() {
			memset(NAME, 0, sizeof(NAME));
			strncpy(NAME, "Positioner", sizeof(NAME));
		}
	};
	struct PayloadGetPTZList {
		std::vector<Positioner> PTZList;
	};

	class Preset {
	public:
		int id;
		std::string Name;
		std::string CameraId;
		int preset;

		Preset() {
			id = 0; Name = "";
		};
	};

	//Payload per la GetCameraList
	class Camera : public Device {
	public:
		int			type;
		std::string	ipAddress;
		int			sockPort;
		int			controlPort;
		std::string model;
		int			CameraAddress;
		int			VideoLoss;

		std::vector<Preset>  vPresetList;

		enum enumVideo { eVideoUndefined = -1, eVideoConnect, eVideoLoss };

		Camera() {
			memset(NAME, 0, sizeof(NAME));
			strncpy(NAME, "Camera", sizeof(NAME));
			type = 0;
			sockPort = 0;
			controlPort = 0;
			CameraAddress = 0;
			VideoLoss = eVideoUndefined;
		}
	};

	struct PayloadGetCameraList {
		std::vector<Camera> CameraList;
	};

	//Payload per la RSP_DVR_GETDILIST_OK
	class Dvr : public Device {
	public:
		int			type;
		std::string	ipAddress;
		int			sockPort;
		int			DvrAddress;
		int			controlPort;
		int			iTimeZone;
		int			numChannel;

		Dvr() {
			memset(NAME, 0, sizeof(NAME));
			strncpy(NAME, "Dvr", sizeof(NAME));
			type = 0;
			sockPort = 0;
			DvrAddress = 0;
			controlPort = 0;
			iTimeZone = 0;
			numChannel = 0;
		}
	};
	struct PayloadGetDvrList {
		std::vector<Dvr> DvrList;
	};

	struct Site {
		std::string  id;
		std::string  description;
	};

	struct PayloadGetSiteList {
		std::vector<Site> siteList;
	};

	class ScenarioDevice {
	public:
		std::string id;
		std::string cameraId;
		int presetId;
		int CellPos;

		ScenarioDevice() {
			presetId = 0; CellPos = 1;
		};
	};

	class Scenarios {
	public:
		std::string id;
		std::string name;
		int			siteId;
		int			scenariosTemplate;
		std::vector<std::string> vAlarmList;
		std::vector<ScenarioDevice> vDevicesList;

		Scenarios() {
			siteId = 1;	scenariosTemplate = 2;
		};
	};

	//------------------------------
	//INTERFACCIA TIME SCHEDULE
	//------------------------------
	class TimeRange {
	protected:
		char NAME[32];
	public:
		std::string id;
		int			Day;
		std::string	StartTime;
		std::string	EndTime;
		int			Holiday;
		std::string	LastModified;

		TimeRange() {
			memset(NAME, 0, sizeof(NAME));
			strncpy(NAME, "TimeRange", sizeof(NAME));
		};
	};

	class TimeSchedule {
	protected:
		char NAME[32];
	public:
		std::string						id;
		std::string						description;
		std::string						LastModified;
		std::map<std::string, TimeRange>	mapTimeRange;

		TimeSchedule() {
			memset(NAME, 0, sizeof(NAME));
			strncpy(NAME, "TimeSchedule", sizeof(NAME));
		};
	};

	//------------------------------
//INTERFACCIA CATEGORIA CTLPTZ
//------------------------------
	extern "C"
	{
		class CFM_API_DLL SIDCtlPtzInterface
		{
		public:

			//Comandi SYNC
			virtual  std::vector<Device> GetCTLPTZList() = 0;

			//Registrazione delle callbacks
			void CTLPTZRespRegisterCallback(CtlPtzEventCallback pCallback);

			//Utilizzata dal ThKernel
			//void PTZEventRegisterCallback(void *pCallBack) {}; //TBD
		protected:
			CtlPtzEventCallback CtlPtzDataCallback;
		};
	}

	//------------------------------
	//INTERFACCIA CATEGORIA PTZ
	//------------------------------
	extern "C"
	{
		class CFM_API_DLL SIDCameraInterface
		{
		public:

			//Comandi SYNC
			virtual  std::vector<Camera>			GetCameraList(long IdSite = 0) = 0;

			//PTZ Category Interface (ASYNC)
			virtual void OnCameraSetMoveSpeed(unsigned int cmdId, unsigned int ptzId, unsigned short uSpeed) = 0;
			virtual void OnCameraMoveRight(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnCameraMoveUp(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnCameraMoveDown(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnCameraMoveLeft(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnCameraMoveUpRight(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnCameraMoveUpLeft(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnCameraMoveDownRight(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnCameraMoveDownLeft(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnCameraStop(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnCameraSetPreset(unsigned int cmdId, unsigned int ptzId, unsigned int presetNumber) = 0;
			virtual void OnCameraGotoPreset(unsigned int cmdId, unsigned int ptzId, unsigned int presetNumber) = 0;
			virtual void OnCameraGotoAbsPosition(unsigned int cmdId, unsigned int ptzId, int uP, int uT, int uZ) = 0;
			virtual void OnCameraLoadPresetTable(unsigned int cmdId, unsigned int ptzId, std::string sFilePath) = 0;
			virtual void OnCameraSetBacklight(unsigned int cmdId, unsigned int ptzId, bool active) = 0;
			virtual void OnCameraZoomIn(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnCameraZoomOut(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnCameraConnect(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnCameraDisconnect(unsigned int cmdId, unsigned int ptzId) = 0;

			//Registrazione delle callbacks
			void CameraRespRegisterCallback(CameraRespCallback pCallback);
			void CameraEvtRegisterCallback(CameraEventCallback pCallback);

			//Utilizzata dal ThKernel
			//void PTZEventRegisterCallback(void *pCallBack) {}; //TBD
		protected:
			CameraRespCallback CameraResponseCallback;
			CameraEventCallback  CameraDataCallback;

		};
	}



	//------------------------------
	//INTERFACCIA CATEGORIA PTZ
	//------------------------------
	extern "C"
	{
		class CFM_API_DLL SIDPtzInterface
		{
		public:

			//Comandi SYNC
			virtual  std::vector<Positioner> GetPTZList() = 0;

			//PTZ Category Interface (ASYNC)
			virtual void OnPTZSetMoveSpeed(unsigned int cmdId, unsigned int ptzId, unsigned short uSpeed) = 0;
			virtual void OnPTZMoveRight(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnPTZMoveUp(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnPTZMoveDown(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnPTZMoveLeft(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnPTZMoveUpRight(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnPTZMoveUpLeft(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnPTZMoveDownRight(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnPTZMoveDownLeft(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnPTZStop(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnPTZSetPreset(unsigned int cmdId, unsigned int ptzId, unsigned int presetNumber) = 0;
			virtual void OnPTZGotoPreset(unsigned int cmdId, unsigned int ptzId, unsigned int presetNumber) = 0;

			// OSC 24/09/2009
			//virtual void OnPTZGotoAbsPosition	(unsigned int cmdId, unsigned int ptzId, unsigned int uP, unsigned int uT, unsigned int uZ) = 0;
			virtual void OnPTZGotoAbsPosition(unsigned int cmdId, unsigned int ptzId, int uP, int uT, int uZ) = 0;

			virtual void OnPTZLoadPresetTable(unsigned int cmdId, unsigned int ptzId, std::string sFilePath) = 0;
			virtual void OnPTZConnect(unsigned int cmdId, unsigned int ptzId) = 0;
			virtual void OnPTZDisconnect(unsigned int cmdId, unsigned int ptzId) = 0;

			//Registrazione delle callbacks
			void PTZRespRegisterCallback(PtzRespCallback pCallback);

			//Utilizzata dal ThKernel
			//void PTZEventRegisterCallback(void *pCallBack) {}; //TBD
		protected:
			PtzRespCallback PtzResponseCallback;
		};
	}


	//------------------------------
	//INTERFACCIA CATEGORIA TRK
	//------------------------------
	extern "C"
	{
		class CFM_API_DLL SIDTrkInterface
		{
		protected:
			TrackEventCallback TrkDataCallback;
		public:
			void TRKDataRegisterCallback(TrackEventCallback pCallback);
			virtual std::vector<Device> GetDeviceList() = 0;
		};
	}

	//------------------------------
	//INTERFACCIA CATEGORIA OCR
	//------------------------------
	extern "C"
	{
		class CFM_API_DLL SIDOcrInterface
		{
		protected:
			OCREventCallback OcrDataCallback;
		public:
			void OCRDataRegisterCallback(OCREventCallback pCallback);
			virtual std::vector<Device> GetReadersList() = 0;
		};
	}


	//-------------------------------------------
	//INTERFACCIA CATEGORIA DVR (DVMRe, NVR...)
	//-------------------------------------------
	extern "C"
	{
		class CFM_API_DLL SIDDvrInterface
		{
		protected:
			DvrEventCallback DVRDataCallback;
		public:
			//DVR Category Interface
			virtual void StartRec(unsigned long channel) {}; // = 0;
			virtual void StopRec(unsigned long channel) {};// = 0;
			virtual void ExtractVideo(unsigned long channel) {};// = 0;

			//Per le notifiche di esecuzione
			virtual void DVRRespRegisterCallBack(void* pCallBack) {};// = 0;

			void DVRDataRegisterCallback(DvrEventCallback pCallback);
			virtual  std::vector<Dvr> GetDvrList(long IsSite = 0) = 0;

		};
	}

	//-------------------------------------------
	//INTERFACCIA CATEGORIA VIDEOSERVER (IV VideoBridge, ACTI VideoServer ...)
	//-------------------------------------------
	extern "C"
	{
		class CFM_API_DLL SIDVsInterface
		{
		public:

			// Video Server Category Interface
			virtual void ShowLive(unsigned int channel) = 0;
			virtual void StopLive(unsigned int channel) = 0;
			virtual void VSRespRegisterCallBack(void* pCallBack) = 0;
		};
	}

	//-------------------------------------------
	//INTERFACCIA CATEGORIA SERIAL
	//-------------------------------------------
	extern "C"
	{
		class CFM_API_DLL SIDSerialInterface
		{
		public:
			//Serial Category Interface
			virtual void SendPacket(char* p, unsigned int size) = 0;
			virtual void SerialChunkRegisterCallback(void* pCallBack) = 0;
			virtual void SerialRespRegisterCallBack(void* pCallBack) = 0;
		};
	}

	//------------------------------
	//INTERFACCIA CATEGORIA READER
	//------------------------------
	extern "C"
	{
		class CFM_API_DLL SIDBadgeInterface
		{
		protected:
			BadgeEventCallback  BadgeDataCallback;
		public:
			//Comandi SYNC
			virtual  std::vector<Reader> GetReaderList(long IdSite = 0) = 0;
			//TRK Category Interface
			void BadgeDataRegisterCallback(BadgeEventCallback pCallback);

			SIDBadgeInterface() { BadgeDataCallback = NULL; };
		};
	}

	//--------------------------------------------
	//INTERFACCIA CATEGORIA DIGITAL INPUT (DI)
	//--------------------------------------------
	extern "C"
	{
		class CFM_API_DLL SIDDIInterface
		{
		protected:
			DiEventCallback DIDataCallback;
		public:
			//DI Category Interface
			virtual unsigned int GetDIPointCount() = 0;
			virtual DStatus* GetAllStatus() = 0;
			virtual DStatus  GetStatus(char* cPointId) = 0;
			void DIDataRegisterCallback(DiEventCallback pCallBack);
			// void DIRespRegisterCallBack(void *pCallBack) = 0;	  
			virtual  std::vector<DIn> GetDIList(long IdSite = 0) = 0;

			virtual void OnEnableDI(unsigned int cmdId, std::string strPoint) = 0;
			virtual void OnDisableDI(unsigned int cmdId, std::string strPoint) = 0;
		};
	}
	//----------------------------------------------------------------------------

	//--------------------------------------------
	//INTERFACCIA CATEGORIA DIGITAL OUTPUT (DO)
	//--------------------------------------------
	extern "C"
	{
		class CFM_API_DLL SIDDOInterface
		{
		protected:
			DoEventCallback DODataCallback;
			DoRespCallback  DOResponseCallback;

		public:
			// METODI SYNC KERNEL FUNZIONALITA' DI BASE GIA' IMPLEMENTATE NEL BASE SID
			void DORespRegisterCallBack(DoRespCallback pCallBack);
			void DODataRegisterCallback(DoEventCallback pCallBack);
			// METODI SYNC KERNEL: DA REALIZZARE NEL SID
			virtual unsigned int GetDOPointCount() = 0;
			virtual  std::vector<DOut> GetDOList(long IdSite = 0) = 0;

			// HANDLER DEI COMANDI
			virtual void OnOpenDO(unsigned int cmdId, std::string strPoint, long mseconds = 0) = 0;
			virtual void OnCloseDO(unsigned int cmdId, std::string strPoint) = 0;
			virtual void OnEnableDO(unsigned int cmdId, std::string strPoint) = 0;
			virtual void OnDisableDO(unsigned int cmdId, std::string strPoint) = 0;
			//virtual void DOEventRegisterCallback(DoEventCallback *pCallBack)		= 0;       
		};
	}
	//----------------------------------------------------------------------------


	//-----------------------------------------------
	//	THO.24.06.2009: INTERFACCIA CATEGORIA ZONA
	//-----------------------------------------------
	extern "C"
	{

		class CFM_API_DLL SIDZoneInterface
		{
		private:

		protected:
			//Callback per il rise degli eventi (La utilizza il SID)
			ZoneEventCallback	ZoneDataCallback;
			ZoneRespCallback	ZoneResponseCallback;

		public:
			//ZONE Category Interface - Handler dei comandi ASYNC
			virtual void OnArmZone(unsigned int cmdId, std::string strPoint) = 0;
			virtual void OnDisarmZone(unsigned int cmdId, std::string strPoint) = 0;
			virtual void OnEnableZone(unsigned int cmdId, std::string strPoint) = 0;
			virtual void OnDisableZone(unsigned int cmdId, std::string strPoint) = 0;

			//void ZoneDataRegisterCallback(ZoneEventCallback pCallback);			
			void ZoneDataRespCallback(ZoneRespCallback pCallback);
			void ZoneDataEventCallback(ZoneEventCallback pCallback);
			virtual  std::vector<DZone> GetZoneList(long IdSite = 0) = 0;
		};
	}

	//-------------------------------------------
	//INTERFACCIA CATEGORIA ALMMGR
	//-------------------------------------------
	extern "C"
	{
		class CFM_API_DLL SIDAlmMgrInterface
		{
		private:

		protected:
			//Callback per il rise degli eventi (La utilizza il SID)
			AlmManagerEventCallback AlmMgrDataCallback;
			AlmManagerRespCallback  AlmMgrResponseCallback;

			// OSC 12-04-2010
#pragma warning(disable:4251)
			std::vector<Alarm> vMasterAlarmList;  //24 01 2008 (SimGuard)
			std::map<int, Alarm*> mMasterAlarmMap; //In questo caso si usa int cone subId
			// OSC 12-04-2010
#pragma warning(default:4251)

		public:
			SIDAlmMgrInterface();

			//Comandi SYNC
			virtual  std::vector<Alarm> GetAlarmList(long IdSite = 0) = 0;
			virtual  std::map<std::string, AlarmGroup> GetAlarmGroupList(long IdSite = 0) = 0;
			//ALMMGR Category Interface - Handler dei comandi ASYNC
			virtual void OnSetAlarm(unsigned int cmdId, std::string strAlarm) = 0;
			virtual void OnResetAlarm(unsigned int cmdId, std::string strAlarm) = 0;
			virtual void OnEnableAlarm(unsigned int cmdId, std::string strAlarm) = 0;	//Da far diventare virtuali puri    
			virtual void OnDisableAlarm(unsigned int cmdId, std::string strAlarm) = 0;	//appena pronti per la modifica massiva
			virtual void OnAckAlarm(unsigned int cmdId, std::string strAlarm) = 0;	//idem
			virtual void OnPurgeAlarm(unsigned int cmdId, std::string strAlarm) = 0;	//idem
			virtual void OnDeleteAlarm(unsigned int cmdId, std::string strAlarm) = 0;	//idem
			//Utilizzati dal ThKernel
			void AlarmMgrEventRegisterCallback(AlmManagerEventCallback pCallback);
			void AlarmMgrRespRegisterCallBack(AlmManagerRespCallback pCallBack);
			//24 01 2008 (SimGuard)
			void SaveSlaveAlarmList(std::vector<Alarm>);

		};
	}
	//----------------------------------------------------------------------------
	//------------------------------
	//INTERFACCIA CATEGORIA CONTROLACCESS
	//------------------------------
	class CtrlAccess
	{
	protected:
		char NAME[32];
	public:
		std::string id;
		std::string description;
		int			status;

		CtrlAccess() {
			memset(NAME, 0, sizeof(NAME));
			strncpy(NAME, "CtrlAccess", sizeof(NAME));

			status = STATUS_UNDEFINED;
		};
	};

	class Badge : public CtrlAccess
	{
	public:
		std::string		PersonId;
		std::string		EncodedNumber;
		std::string		AliasNumber;
		std::string		PIN;
		std::string		BeginValidation;
		std::string		EndValidation;
		std::string		LastModified;
	};

	class Person : public CtrlAccess
	{
	public:
		std::string		EmployeeNumber;
		std::string		PersonTypeId;
		std::string		FirstName;
		std::string		LastName;
		std::string		Address1;
		std::string		Address2;
		std::string		Telephone1;
		std::string		Telephone2;
		std::string		LastModified;
	};

	class AccessRight : public CtrlAccess
	{
	public:
		std::string				IdTimeSchedule;
		std::string				StartDate;
		std::string				EndDate;
		std::string				LastModified;
		std::set<std::string>	listBadge;
		std::set<std::string>	listReader;
	};

	extern "C"
	{
		class CFM_API_DLL SIDCtrlAccessInterface
		{
		protected:
			/*
					CABadgeEventCallback	CABadgeDataCallback;
					CABadgeRespCallback		CABadgeResponseCallback;
					CAPersonEventCallback	CAPersonDataCallback;
					CAPersonRespCallback	CAPersonResponseCallback;
			*/
		public:

			SIDCtrlAccessInterface();
			/*
					void CABadgeEventRegisterCallback	(CABadgeEventCallback	pCallback);
					void CABadgeRespRegisterCallBack	(CABadgeRespCallback	pCallBack);
					void CAPersonEventRegisterCallback	(CAPersonEventCallback	pCallback);
					void CAPersonRespRegisterCallBack	(CAPersonRespCallback	pCallBack);
			*/
			//Comandi Badge
			virtual  std::map<std::string, Badge>	GetBadgeList() = 0;
			virtual  Badge							GetBadge(std::string id) = 0;
			virtual  bool							SaveBadge(Badge badge) = 0;

			virtual  std::map<std::string, Person>	GetPersonList() = 0;
			virtual  Person							GetPerson(std::string id) = 0;
			virtual  bool							SavePerson(Person person) = 0;

			virtual std::map<std::string, AccessRight>	GetAccessRightList() = 0;
			virtual AccessRight						GetAccessRight(std::string id) = 0;
			virtual bool 							SaveAccessRight(AccessRight ar) = 0;

		};
	}
	//----------------------------------------------------------------------------

	//--------------------------------------------
	//INTERFACCIA CATEGORIA CONCENTRATORI DI CAMPO (CDC)
	//--------------------------------------------

	//Payload per la RSP_CDC_GETDILIST_OK
	//struct Cdc
	class Cdc : public Device
	{
	public:
		Cdc()
		{
			memset(NAME, 0, sizeof(NAME));
			strncpy(NAME, "CDC", sizeof(NAME));
		}
	};
	struct PayloadGetCDCList
	{
		std::vector<Cdc> DIList;
	};


	extern "C"
	{
		class CFM_API_DLL SIDCDCInterface {
		protected:
			CdcEventCallback CdcDataCallback;
		public:
			SIDCDCInterface() { CdcDataCallback = NULL; }
			//CDC Category Interface
			void CdcDataRegisterCallback(CdcEventCallback pCallBack);
			virtual  std::vector<Cdc> GetCDCList(long IdSite = 0) = 0;

			//virtual void OnEnableCDC(unsigned int cmdId,	std::string strPoint)	= 0; 
			//virtual void OnDisableCDC(unsigned int cmdId,	std::string strPoint)	= 0;        
		};
	}
	//----------------------------------------------------------------------------

	//Riferimentoi incrociati CSID --> Altre interfacce
	class SidInterfaceMap
	{
	public:
		SIDDIInterface* diIf;
		SIDDOInterface* doIf;
		SIDTrkInterface* trkIf;
		SIDAlmMgrInterface* almMgrIf;
		SIDBadgeInterface* badgeIf;
		SIDPtzInterface* ptzIf;
		SIDZoneInterface* zoneIf;
		SIDCameraInterface* cameraIf;
		SIDCDCInterface* cdcIf;
		SIDCtrlAccessInterface* ctrlAccessIf;
	};

	//-------------------------------------------------------------
	//INTERFACCIA INTERFACCIA GENERALE SID (DA ESTENDERE SEMPRE)
	//-------------------------------------------------------------
	extern "C"
	{
		// OSC 12-04-2010
#pragma warning(disable:4275)
		class CFM_API_DLL CSID : public CThread {
			// OSC 12-04-2010
#pragma warning(default:4275)

		private:

			//*******************************
			// CALLBACKS
			//*******************************
			PingRespCallback	PingCallback;

			//*******************************
			// For internal use
			//*******************************
			virtual DWORD Run(LPVOID /* arg */);
			bool bTerminated;			//Indica al Main Loop di terminare
			bool terminated;			//Indica che il thread è terminato
			unsigned int uID;			//Id numerico univoco assegnato dal ThKernel in fase 
			//di inizializzazione (impostato su DB - ID)
			std::string sAlias;		//Id alfanumerico univoco assegnato dal ThKernel in fase 
			//di inizializzazione (impostato su DB - ALIAS) 
			SIDExecStates SIDState;

			// OSC 12-04-2010
#pragma warning(disable:4251)
			std::map <std::string, std::string> mapCfgKeys; // hash table contenente la configurazione del SID
			// OSC 12-04-2010
#pragma warning(default:4251)

		protected:

			SidInterfaceMap sidIfMap;

			//***************************************
			//MUST be overridden by SID developer
			//***************************************
			virtual void BeforeRun() = 0;
			virtual void ProcessCustomMessages(MSG& msg) = 0;
			virtual void AfterRun() = 0;

			//*************************************
			// CALLBACKS to send event to KERNEL 
			//*************************************
			RaiseError					RaiseErrorCallback;
			RaiseMessage				RaiseMessageCallback;
			NotifySIDStatusCallback	    NotifyStatusCallback;

		public:
			//***************************************
			// MUST be overridden by SID developer
			// ret values:
			//		SUCCESS		0
			//     FAILURE	   -1
			//
			//***************************************
			virtual int InitSID() { return FAILURE; };
			virtual int CloseSID() { return FAILURE; };

			//Import dei SITI
			virtual std::vector<Site> GetSiteList(long IdSite = 0); //Da fare l'override durante lo sviluppo del SID

			virtual std::vector<Scenarios> GetScenariosList(long IdSite = 0);

			virtual std::map<std::string, TimeSchedule> GetTimeScheduleList(); //Da fare l'override durante lo sviluppo del SID

			//**********************************
			//Constructor and deconstructor
			//**********************************
			CSID();
			~CSID();

			//*************************************************************************
			// USED by Kernel and SID Monitor (init, reg callbacks,  and monitoring)
			//*************************************************************************
			void SetUID(unsigned int id);
			unsigned int GetUID();
			void SetAlias(std::string alias);
			std::string GetAlias();
			int GetThId();
			//For monitoring
			virtual void AsyncPing();
			int Terminate();
			void TerminateNow();
			int Crash();
			DWORD Restart();
			bool IsTerminated();
			SIDExecStates GetSIDState() { return SIDState; };
			void AttachInterface(void* itf, Category cat);
			//registrazione delle callback
			void RegisterPingCallback(PingRespCallback pCallback);
			void RegisterNotifyStatusCallback(NotifySIDStatusCallback pCallback);
			void RegisterErrorCallback(RaiseError pCallback);
			void RegisterMessageCallback(RaiseMessage pCallback);
			void SetCfgParameter(std::string sSection, std::string sName, std::string sValue);
			//*************************************************************************
			// USED by SID Developers 
			//*************************************************************************
			std::string GetCfgParameter(std::string sSection, std::string sName);

			virtual long GetSiteIdFromName(std::string siteName) { return 0; };
		};
	}
	//Tipi funzioni di accesso a SID nelle DLL
	typedef void*	(*createObj)			();
	typedef void	(*destroyObj)		(void*);
	//Tipo funzione per interfaccia comune SID
	typedef CSID*	(*getSIDInterface)	(void*);
	//Tipi funzioni per interfaccia di categorie
	typedef SIDTrkInterface* (*getTrkInterface)		(void*);
	typedef SIDDIInterface* (*getDIInterface)		(void*);
	typedef SIDDOInterface* (*getDOInterface)		(void*);
	typedef SIDPtzInterface* (*getPTZInterface)		(void*);
	typedef SIDCameraInterface* (*getCAMERAInterface)	(void*);
	typedef SIDAlmMgrInterface* (*getALMMGRInterface)	(void*);
	typedef SIDDvrInterface* (*getDVRInterface)      (void*);
	typedef SIDVsInterface* (*getVSInterface)		(void*);
	typedef SIDSerialInterface* (*getSerialInterface)	(void*);
	typedef SIDBadgeInterface* (*getREADERInterface)   (void*);
	typedef SIDOcrInterface* (*getOCRInterface)      (void*);
	typedef SIDZoneInterface* (*getZONEInterface)		(void*);
	typedef SIDCtrlAccessInterface* (*getCAInterface)	(void*);
	typedef SIDCDCInterface* (*getCDCInterface)		(void*);
} //end namespace