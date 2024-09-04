/****************************** Module Header ******************************\
* Module Name:  CfmTypes.h
*
* Base application types...
*
\***************************************************************************/
#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

namespace cfm::application::domain {

	enum CfmAction {
		PT_CALLPRESET = 1,
		PT_SETPRESET = 2,
		DO_ON = 3,
		DO_OFF = 4,
		DEV_REWIND = 5,
		DVR_PLAY = 6,
		DVR_SEARCH = 7,
		DVR_LIVE = 8,
		ALARM_SET = 9,
		ALARM_RESET = 10,
		PT_CALLABSPOSITION = 11,
		PT_MOVERIGHT = 12,
		PT_MOVELEFT = 13,
		PT_MOVEUP = 14,
		PT_MOVEDOWN = 15,
		PT_MOVEUPRIGHT = 16,
		PT_MOVEDOWNRIGHT = 17,
		PT_MOVEUPLEFT = 18,
		PT_MOVEDOWNLEFT = 19,
		PT_STOP = 20,
		ZONE_ARM = 21,
		ZONE_DISARM = 22,
		DVR_ESTRACTREC = 23,
		DVR_STARTREC = 24,
		DVR_STOPREC = 25,
		ALARM_ACK = 26,
		ALARM_ENABLE = 27,
		ALARM_DISABLE = 28,
		CAM_CALLABSPOSITION = 29,
		CAM_MOVERIGHT = 30,
		CAM_MOVELEFT = 31,
		CAM_MOVEUP = 32,
		CAM_MOVEDOWN = 33,
		CAM_MOVEUPRIGHT = 34,
		CAM_MOVEDOWNRIGHT = 35,
		CAM_MOVEUPLEFT = 36,
		CAM_MOVEDOWNLEFT = 37,
		CAM_STOP = 38,
		CAM_CALLPRESET = 39,
		CAM_SETPRESET = 40,
		CAM_SETBACKLIGHTON = 41,
		CAM_SETBACKLIGHTOFF = 42,
		DO_ENABLE = 43,
		DO_DISABLE = 44,
		CAM_ENABLE = 45,
		CAM_DISABLE = 46,
		PTZ_ENABLE = 47,
		PTZ_DISABLE = 48,
		ALARM_PURGE = 49,
		CAM_HOME = 50,
		CAM_CONNECT = 51,
		CAM_DISCONNECT = 52,
		PTZ_CONNECT = 53,
		PTZ_DISCONNECT = 54
	};

	enum CfmEvent {
		OCR_SENT = 1,
		DI_ON = 3,
		DI_OFF = 4,
		TRK_SENT = 5,
		BDG_SWIPE = 8,
		ALARM_ON = 9,
		ALARM_OFF = 10,
		BDG_AUTH = 11,
		BDG_NOAUTH = 12,
		DI_CUT = 13,
		DI_SHORT = 14,
		DI_ENABLED = 15,
		DI_DISABLED = 16,
		ZONE_ARMED = 17,
		ZONE_DISARMED = 18,
		ZONE_ARMFAILED = 19,
		ZONE_DISARMFAILED = 20,
		ZONE_DISARMNORIGHT = 21,
		ALARM_ENABLED = 22,
		ALARM_DISABLED = 23,
		ALARM_ACKED = 24,
		DOON = 25,
		DOOFF = 26,
		DOENABLED = 27,
		DODISABLED = 28,
		READER_ENABLED = 29,
		READER_DISABLED = 30,
		READER_BDGUNKNOWN = 31,
		EXECUTING_EVENT = 32,
		ALARM_PURGED = 33,
		ALARM_DELETED = 34,
		CAMERA_ENABLED = 35,
		CAMERA_DISABLED = 36,
		CAMERA_VLOSS = 37,
		CAMERA_READY = 38,
		CDC_ENABLED = 39,
		CDC_DISABLED = 40,
		CDC_OFFLINE = 41,
		CDC_ONLINE = 42

	};



	/**
	  * @struct CfmRuleLog_Table
	  * @brief  Struttura dati tabella di log delle regole di reazione attivate dal sistema
	  * @author Diego Gangale
	  * @date	14 06 2007 Created
	  *
	  */
	struct CfmRuleLog_Table
	{
		int IdRuleLog;		 /**< Id del record nella tabella */
		std::string StartTS;    /**< Timestamp ricezione evento dal Kernel in formato ISO SQL */
		std::string DatabaseTS;  /**< Timestamp scrittura evento su database in formato ISO SQL */
		int IdRule;              /**< Regola avviata */
		int IdEventLog;          /**< Riferimento all'envento che la ha avviata */
		std::string ExecutedTS;  /**< Timestamp di completata esecuzione  */
		bool Succeded;			 /**< Tutte le azioni sono state eseguite con successo */
		int ExecutionTime;        /**< Millisecondi utilizzati per l'esecuzione */

	};

	/**
	  * @struct CfmActionLog_Table
	  * @brief  Struttura dati tabella di log delle azioni attivate dal sistema
	  * @author Diego Gangale
	  * @date	14 06 2007 Created
	  *
	  */
	struct CfmActionLog_Table
	{
		int IdActionLog;		 /**< Id del record nella tabella */
		std::string KernelTS;    /**< Timestamp ricezione evento dal Kernel in formato ISO SQL */
		std::string DatabaseTS;  /**< Timestamp scrittura evento su database in formato ISO SQL */
		int IdActionRule;            /**< Azione avviata */
		int IdRuleLog;           /**< Riferimento alla regola che la ha avviata */
		std::string ExecutedTS;  /**< Timestamp dio completata esecuzione  */
		int ExecutionTime;        /**< Millisecondi utilizzati per l'esecuzione */

	};


	/**
	  * @struct CfmEventLog_Table
	  * @brief  Struttura dati tabella di log degli eventi arrivati al sistema
	  * @author Diego Gangale
	  * @date	14 06 2007 Created
	  *
	  */
	struct CfmEventLog_Table
	{
		int IdEventLog;		       /**< Id del record nella tabella */
		std::string KernelTS;      /**< Timestamp ricezione evento dal Kernel in formato ISO SQL */
		std::string DatabaseTS;    /**< Timestamp scrittura evento su database in formato ISO SQL */
		std::string SourceTS;	   /**< Timestamp invio evento dal SID in formato ISO SQL */
		int DeviceId;			   /**< Identificativo della device che ha generato l'evento */
		//std::string Description; /**< Descrizione dell'evento */
		int EventId;			   /**< Identificativo dell'evento (reference: tabells Events ) */
		std::string Details;       /**< Dettagli relativi all'evento */
	};

	/**
	  * @struct CfmCommandLog_Table
	  * @brief  Struttura dati tabella di log dei comandi arrivati al sistema
	  * @author Diego Gangale
	  * @date	14 06 2010 Created
	  *
	  */
	struct CfmCommandLog_Table
	{
		std::string SourceTS;	   /**< Timestamp invio evento dal SID in formato ISO SQL */
		long DeviceId;			   /**< Identificativo della device che ha generato l'evento */
		int ActionId;			   /**< Identificativo dell'evento (reference: tabells Events ) */
		int UserId;				   /**< Id user del WebMonitor */
		std::string Message;       /**< Dettagli relativi all'evento */
	};

	/**
	  * @struct CfmEventAndLog_Table
	  * @brief  Struttura dati tabella di log degli eventiAnd arrivati al sistema
	  * @author Oscar Casale
	  * @date	03 06 2009 Created
	  *
	  */
	struct CfmEventAndLog_Table
	{
		int IdEventAndLog;		       /**< Id del record nella tabella */
		std::string KernelTS;      /**< Timestamp ricezione evento dal Kernel in formato ISO SQL */
		std::string DatabaseTS;    /**< Timestamp scrittura evento su database in formato ISO SQL */
		std::string SourceTS;	   /**< Timestamp invio evento dal SID in formato ISO SQL */
		int IdEventAnd;			   /**< Identificativo della device che ha generato l'evento */
		int DeviceId;			   /**< Identificativo della device che ha generato l'evento */
		//std::string Description; /**< Descrizione dell'evento */
		int EventId;			   /**< Identificativo dell'evento (reference: tabells Events ) */
		std::string Details;       /**< Dettagli relativi all'evento */
	};

	/**
	  * @struct CfmEventOrLog_Table
	  * @brief  Struttura dati tabella di log degli EventiOr arrivati al sistema
	  * @author Oscar Casale
	  * @date	03 06 2009 Created
	  *
	  */
	struct CfmEventOrLog_Table
	{
		int IdEventOrLog;		       /**< Id del record nella tabella */
		std::string KernelTS;      /**< Timestamp ricezione evento dal Kernel in formato ISO SQL */
		std::string DatabaseTS;    /**< Timestamp scrittura evento su database in formato ISO SQL */
		std::string SourceTS;	   /**< Timestamp invio evento dal SID in formato ISO SQL */
		int IdEventOr;			   /**< Identificativo della device che ha generato l'evento */
		int DeviceId;			   /**< Identificativo della device che ha generato l'evento */
		//std::string Description; /**< Descrizione dell'evento */
		int EventId;			   /**< Identificativo dell'evento (reference: tabells Events ) */
		std::string Details;       /**< Dettagli relativi all'evento */
	};


	struct CfmActionParameters_Table
	{
		int Id;
		int IdAction;
		std::string Alias;
		std::string Type;
	};


	struct CfmActionParametersJoin_Table
	{
		int Id; //24 01 2008
		int IdRule;
		int IdAction;
		int IdActionRule;
		int IdParameter;
		std::string Alias;
		std::string Type;
		std::string Value;
	};

	struct CfmLog_Table
	{
		long Id;
		std::string SourceTS;
		std::string HostTS;
		std::string DBTS;
		int iSourceId;
		int iSeverity;
		std::string sMsgType;
		std::string sMsg;
	};


	/**
	  * @struct CfmSystemParameters_Table
	  * @brief  Struttura dati tabella paramatri di configurazione e inizializzazione sottosistemi
	  * @author Diego Gangale
	  * @date	31 05 2007 Created
	  *
	  */
	struct CfmSystemParameters_Table
	{
		int Id;
		std::string Section;
		std::string Name;
		std::string Value;
		std::string MasterValue;
		bool Encrypted;
		int SystemId;
	};


	/**
	  * @struct CfmAdapterParameters_Table
	  * @brief  Struttura dati tabella paramatri di configurazione Adapter
	  * @author Mladen Seget
	  * @date	29 04 2010 Created
	  *
	  */
	struct CfmAdapterParameters_Table
	{
		int Id;
		std::string Section;
		std::string Name;
		std::string Value;
		bool Encrypted;
		int AdapterId;
		int SARARegionId;
	};

	struct CfmCategory_Table
	{
		int Id;						//Identificativo nomerico della categoria
		std::string Description;	//Descrizione testuale della categoria di sensori/attuatori
		std::string Alias;			//Nome mnemonico per la categoria
		int Actuator;				//
		int Priority;				//Ordine di caricamento dal DB
	};

	/**
	  * @struct SaraEvent_Table
	  * @brief  Struttura dati tabella eventi gestibili dall'Action Logic
	  * @author Diego Gangale
	  * @date	23 04 2007 Created
	  *
	  */
	struct CfmEvent_Table
	{
		int Id;
		std::string alias;
		std::string description;
		int CategoryId;
	};

	/**
	  * @struct CfmAction_Table
	  * @brief  Struttura dati tabella azioni gestibili dall'Action Logic
	  * @author Diego Gangale
	  * @date	23 04 2007 Created
	  *
	  */
	struct CfmAction_Table
	{
		int Id;
		std::string alias;
		std::string description;
		int CategoryId;
	};

	/**
	  * @struct CfmEventRules_Table
	  * @brief  Struttura dati tabella regole di reazione gestibili dall'Action Logic
	  * @author Diego Gangale
	  * @date	23 04 2007 Created
	  *
	  *  Struttura dati tabella regole di reazione gestibili dall'Action Logic \n
	  *  La ricerca verrà effettuata per "Event" quindi la Hash table che conterrà i record
	  *  verrà istanziata con quell'indice
	  *
	  */
	struct CfmEventRules_Table
	{
		int RuleId;
		int EventId;
		int DeviceId;
	};

	struct CfmActionRules_Table
	{
		int Id;
		int RuleId;
		int ActionId;
		int DeviceId;
	};

	struct CfmLicense_Table
	{
		std::string LocalDll;
		short Instances;
		std::string LicenseKey;
		std::string ExpirationDate;
	};

	struct CfmCategoriesAttributes_Table
	{
		int Id;
		std::string Name;
		std::string Description;
		std::string Value;
	};

	struct CfmDevicesCategoriesAttributes_Table
	{
		unsigned long	DevicesId;
		short			CategoriesAttributesId;
		std::string		AValue;
		std::string		Name;
	};

	typedef enum stRecDelete { recNoDeleted = 0, recDeleted = 1 } stateRecDelete;

	struct CfmDevices_Table
	{
		int Id;
		int SystemId;
		bool LogicalStatus;
		short int PhisicalStatus; /** 0 OFF, 1 ON, 2 UNDEFINED*/
		std::string Description;
		stateRecDelete bDeleted;
		int SiteId;
		std::string SiteName;
		bool SiteWatched;
		int ParentId;
		std::string subId;
		int catId;
		std::string sUpdatedTS;
		std::string sModifiedTS;
		bool initEvent;
		short int ProcessState; /** 1 Unack, 2 Ack, 3 Inactive */
		short int Executing;
		std::string sUpdatedUTCTS;

		std::vector<CfmCategoriesAttributes_Table> DeviceParameters;
		CfmDevices_Table() { initEvent = false; Executing = 0; };
	};

	struct CfmAlarmGroupDevice_Table
	{
		unsigned long	IdAlarmGroup;
		unsigned long	DevicesId;
		int				SystemId;
		std::string		SubIdAlarmGroup;

		CfmAlarmGroupDevice_Table() { IdAlarmGroup = DevicesId = SystemId = 0; };
	};

	struct CfmAlarmGroup_Table
	{
		unsigned long	Id;
		std::string		Name;
		long			SiteId;
		std::string		Modified;

		std::map<unsigned long, CfmAlarmGroupDevice_Table> mapAlarmGroupDevice;

		CfmAlarmGroup_Table() { Id = 0; SiteId = 0; };
	};

	struct CfmSiteSystem_Table
	{
		int		SiteId;
		int		SystemId;
		long	SubSiteId;
	};

	struct CfmSite_Table {
		int Id;
		int SystemId;
		int SubSystemId;
		bool SiteWatched;
		std::string Name;
		std::string Description;
		std::string ModifiedTS;

		std::map<unsigned long, CfmSiteSystem_Table> mapSiteSystem;
	};

	struct CfmTimeRange_Table {
		unsigned long	Id;
		unsigned long	IdTimeSchedule;
		int				Day;
		std::string		StartTime;
		std::string		EndTime;
		int				Holiday;
		std::string		LastModified;
		std::string		DBModified;
		int				Deleted;
		std::map<unsigned int, std::string> mapSubId;

		CfmTimeRange_Table() {
			Id = 0;
			IdTimeSchedule = 0;
			Day = 0;
			Holiday = 0;
			Deleted = 0;
		};
	};

	struct CfmTimeSchedule_Table {
		unsigned long	Id;
		std::string		Description;
		std::string		LastModified;
		std::string		DBModified;
		int				Deleted;

		std::map<unsigned long, CfmTimeRange_Table> mapTimeRange;
		std::map<unsigned int, std::string> mapSubId;

		CfmTimeSchedule_Table() {
			Id = 0;
			Deleted = 0;
		};
	};

	struct CfmRules_Table {
		int Id;
		std::string Alias;
	};

	struct CfmSystem_Table {
		int Id;				/**< (SIDUID) Identificativo univoco del SID */
		std::string alias;		/**< Nome mnemonico del sotto sistema */
		std::string description;	/**< Descrizione del sottosistema */
		std::string deployment;  /**< Local o Remote */
		std::string localDll;	/**< Dll per il deploy locale */
		bool realTimeContext;
		int ThreadPriority;
		int Status;
		//18 07 2007
		std::string RemoteIPAddress;
		int RemoteIPPort;
		int MaintenanceReq;
		// OSC 03-05-2010 Eliminata la gestione introdotta per SimGuard
		// -- NEW --
		//24 01 2008 Come richiesto per compatibilità con SimGuard
		//int AlarmMaster;   /**< (SIDUID) Invia verso il sottosistema slave tutti gli allarmi ricevuti */
		//int AlarmSlave;    /**< (SIDUID) Riceve tramite comando di attuazione allarme gli allarmi ricevuti dal master */
		// -- END NEW --
		int SIDId;
		int SARARegionId;
		bool enabled;
	};

	struct CfmLogOCR_Table {
		int DeviceId;
		std::string Timestamp;
		std::string LicensePlate;
		std::string TrailerPlate;
		std::string Container1;
		std::string Container2;
		std::string Container3;
		std::string Container4;
		int EventId; //01 10 2007 Per correlazione con l'evento
		std::string img_path_1;
		std::string img_path_2;
		std::string img_path_3;
		std::string img_path_4;
		std::string img_path_5;
		std::string img_path_6;
	};

	/**
	  * @struct SaraEventAndRules_Table
	  * @brief  Struttura dati tabella regole di reazione gestibili dall'Action Logic
	  * @author Oscar Casale
	  * @date	27 02 2009 Created
	  *
	  *  Struttura dati tabella regole di reazione gestibili dall'Action Logic \n
	  *  La ricerca verrà effettuata per "EventAndId" quindi la Hash table che conterrà i record
	  *  verrà istanziata con quell'indice
	  *
	  */
	struct CfmEventAndRules_Table {
		int EventOpId;
		int DeltaT;
		int RuleId;
	};

	/**
	  * @struct SaraEventOrRules_Table
	  * @brief  Struttura dati tabella regole di reazione gestibili dall'Action Logic
	  * @author Oscar Casale
	  * @date	27 02 2009 Created
	  *
	  *  Struttura dati tabella regole di reazione gestibili dall'Action Logic \n
	  *  La ricerca verrà effettuata per "EventOrId" quindi la Hash table che conterrà i record
	  *  verrà istanziata con quell'indice
	  *
	  */
	typedef CfmEventAndRules_Table CfmEventOrRules_Table;

	/**
	  * @struct SaraEventAnd_Table
	  * @brief  Struttura dati tabella regole di reazione gestibili dall'Action Logic
	  * @author Oscar Casale
	  * @date	27 02 2009 Created
	  *
	  *  Struttura dati tabella regole di reazione gestibili dall'Action Logic \n
	  *  La ricerca verrà effettuata per "EventId-DeviceId" quindi la Hash table che conterrà i record
	  *  verrà istanziata con quell'indice
	  *
	  */
	struct CfmEventAnd_Table {
		int EventId;
		int DeviceId;
		int EventOpId;
		CfmEventAnd_Table(): EventId(0), DeviceId(0), EventOpId(0) {}
	};

	typedef CfmEventAnd_Table CfmEventOr_Table;
	/**
	  * @struct SaraEventOr_Table
	  * @brief  Struttura dati tabella regole di reazione gestibili dall'Action Logic
	  * @author Oscar Casale
	  * @date	27 02 2009 Created
	  *
	  *  Struttura dati tabella regole di reazione gestibili dall'Action Logic \n
	  *  La ricerca verrà effettuata per "EventId-DeviceId" quindi la Hash table che conterrà i record
	  *  verrà istanziata con quell'indice
	  *
	  */
	typedef CfmEventAnd_Table CfmEventOr_Table;


	struct CfmLogTrack_Table {
		int DeviceId;
		float Azimuth;
		float Range;
		float Elevation;
		float x;
		float y;
		float z;
		std::string SystemTrackId;
		int EventId;
	};

	//....... da definire per tutte le tabelle del database di SARA ......

	struct CfmAdapter_Table {
		int Id;				        /**< (SIDUID) Identificativo univoco dell'Adapter */
		std::string Name;		    /**< Nome mnemonico dell'Adapter */
		std::string Description;	/**< Descrizione dell'Adapter */
		std::string DllPath;	    /**< Dll per il deploy locale */
		int SARARegionId;		    /**< Identificativo della regione */
		int AdapterStatesId;
	};

	struct CfmRegion_Table {
		int Id;				        /**< Identificativo della regione */
		std::string Name;		    /**< Nome mnemonico della regione */
		std::string Description;	/**< Descrizione della regione */
		std::string HostName;	    /**< Nome Host */
		std::string IpAddress;	    /**< Indirizzo IP */
		std::string IpGateway;	    /**< Indirizzo IP Gateway */
		int PortNumber;	            /**< Numero Porta */
		bool Failover;	            /**< Failover */
		bool AutoRestore;	        /**< Politica di rientro da Failover (automatica o manuale) */
		bool EnableSSL;	            /**< Abilitazione SSL */
		std::string CAFile;	        /**< Path del CA file (certificato) */
		std::string SSLPassword;	/**< SSL Password */
		std::string SessionID;	    /**< SSL Session ID */
	};

	struct CfmPerson_Table {
		unsigned long	Id;
		short			PersonTypeId;
		short			Status;
		std::string		FirstName;
		std::string		LastName;
		std::string		Address1;
		std::string		Address2;
		std::string		Telephone1;
		std::string		Telephone2;
		std::string		LastModified;
		std::string		DBModified;
		int				Deleted;
		std::map<unsigned int, std::string> mapSubId;

		CfmPerson_Table() {
			Id = 0;
			PersonTypeId = 0;
			Status = 0;
			Deleted = 0;
		}
	};

	struct CfmBadge_Table {
		unsigned long	Id;
		std::string		Description;
		unsigned long	PersonId;
		std::string		EncodedNumber;
		std::string		AliasNumber;
		short			Status;
		std::string		PIN;
		std::string		LastModified;
		std::string		BeginValidation;
		std::string		EndValidation;
		std::string		DBModified;
		int				Deleted;

		std::map<unsigned int, std::string> mapSubId;

		CfmBadge_Table() {
			Id = 0;
			PersonId = 0;
			Status = 0;
			Deleted = 0;
		}
	};

	struct CfmAccessRight_Table {
		unsigned long	Id;
		std::string		Description;
		unsigned long	IdTimeSchedule;
		std::string		StartDate;
		std::string		EndDate;
		std::string		LastModified;
		std::string		DBModified;
		int				Deleted;
		std::set<unsigned long> listBadge;
		std::set<unsigned long> listReader;
		std::map<unsigned int, std::string> mapSubId;

		CfmAccessRight_Table() {
			Id = 0;
			IdTimeSchedule = 0;
			Deleted = 0;
		}
	};

	typedef struct data_change_event {
		int fSID;
		int fObjectID;
		std::string fObjectType;
		void* data;
		data_change_event()
		{
			fSID = 0;
			fObjectID = 0;
			fObjectType = "";
			data = NULL;
		}
	} ChangeEvent;

} //end namespace
