/****************************** Module Header ******************************\
* Module Name:  CallbackTypes.h
*
* Base system interfaces...
*
\***************************************************************************/
#pragma once
#include <string>
#include <map>
#include "CfmApiMessages.h"
//-------------------------------------------------------
//Messaggi di Kernel
//-------------------------------------------------------
#define MSG_TO_LOG					EVENTMANAGER_MESSAGES_BASE + 0 /**< Indica al ThKernel che il messaggio contiene un messaggio di testo da loggare */
#define ERR_TO_LOG					EVENTMANAGER_MESSAGES_BASE + 1 /**< Indica al ThKernel che il messaggio contiene un errore da loggare */
#define SID_NOTIFY_STATUS			EVENTMANAGER_MESSAGES_BASE + 2 /**< Viene segnalato al Kernel il punto di esecuzione del main thread del SID */
#define RULES_RELOAD_REQ            EVENTMANAGER_MESSAGES_BASE + 3 /**< Viene richiesto all'EventManager di effettuare l'aggiornamento delle regole */

#define SARA_MONITOR_LOG			EVENTMANAGER_MESSAGES_BASE + 100 /**< Indica al SaraMonitor che deve loggare un messaggio sulla sessione telnet */


//-----------------------------------------------------
//Indice base eventi per tutte le categorie
//-----------------------------------------------------
#define EVT_DI_BASE					CATTYPES_MESSAGES_BASE	/**< Messaggio base per spontanee categoria DI */
#define RSP_DI_BASE					EVT_DI_BASE + 100		/**< Messaggio base per risposte categoria DI */
#define CMD_DI_BASE					RSP_DI_BASE + 100		/**< Messaggio base per comandi categoria DI */

#define EVT_DO_BASE					CATTYPES_MESSAGES_BASE + 1000 /**< Messaggio base per spontanee categoria DO */
#define RSP_DO_BASE					EVT_DO_BASE + 100		/**< Messaggio base per risposte categoria DO */
#define CMD_DO_BASE					RSP_DO_BASE + 100		/**< Messaggio base per comandi categoria DO */

#define EVT_TRK_BASE				CATTYPES_MESSAGES_BASE + 2000 /**< Messaggio base per spontanee categoria TRK */
#define RSP_TRK_BASE				EVT_TRK_BASE + 100		/**< Messaggio base per risposte categoria TRK */
#define CMD_TRK_BASE				RSP_TRK_BASE + 100		/**< Messaggio base per comandi categoria TRK */

#define EVT_PTZ_BASE				CATTYPES_MESSAGES_BASE + 3000 /**< Messaggio base per spontanee categoria PTZ */
#define RSP_PTZ_BASE				EVT_PTZ_BASE + 100		/**< Messaggio base per risposte categoria PTZ */
#define CMD_PTZ_BASE				RSP_PTZ_BASE + 100		/**< Messaggio base per comandi categoria PTZ */

#define EVT_ALMDTC_BASE				CATTYPES_MESSAGES_BASE + 4000 /**< Messaggio base per spontanee categoria ALMDTC */
#define RSP_ALMDTC_BASE				EVT_ALMDTC_BASE + 100	/**< Messaggio base per risposte categoria ALMDTC */
#define CMD_ALMDTC_BASE				RSP_ALMDTC_BASE + 100	/**< Messaggio base per comandi categoria ALMDTC */

#define EVT_ALMMGR_BASE				CATTYPES_MESSAGES_BASE + 5000 /**< Messaggio base per spontanee categoria ALMMGR */
#define RSP_ALMMGR_BASE				EVT_ALMMGR_BASE + 100	/**< Messaggio base per risposte categoria ALMMGR */
#define CMD_ALMMGR_BASE				RSP_ALMMGR_BASE + 100	/**< Messaggio base per comandi categoria ALMMGR */

#define EVT_DVR_BASE				CATTYPES_MESSAGES_BASE + 6000 /**< Messaggio base per spontanee categoria DVR */
#define RSP_DVR_BASE				EVT_DVR_BASE + 100		/**< Messaggio base per risposte categoria DVR */
#define CMD_DVR_BASE				RSP_DVR_BASE + 100		/**< Messaggio base per comandi categoria DVR */

#define EVT_VIDEOSRV_BASE			CATTYPES_MESSAGES_BASE + 7000 /**< Messaggio base per spontanee categoria VIDEOSRV */
#define RSP_VIDEOSRV_BASE			EVT_VS_BASE + 100		/**< Messaggio base per risposte categoria VIDEOSRV */
#define CMD_VIDEOSRV_BASE			RSP_VS_BASE + 100		/**< Messaggio base per comandi categoria VIDEOSRV */

#define EVT_SERIAL_BASE				CATTYPES_MESSAGES_BASE + 8000 /**< Messaggio base per spontanee categoria SERIAL */
#define RSP_SERIAL_BASE				EVT_SERIAL_BASE + 100	/**< Messaggio base per risposte categoria SERIAL */
#define CMD_SERIAL_BASE				RSP_SERIAL_BASE + 100	/**< Messaggio base per comandi categoria SERIAL */

#define EVT_READER_BASE				CATTYPES_MESSAGES_BASE + 9000 /**< Messaggio base per spontanee categoria READER */
#define RSP_READER_BASE				EVT_READER_BASE + 100	/**< Messaggio base per risposte categoria READER */
#define CMD_READER_BASE				RSP_READER_BASE + 100	/**< Messaggio base per comandi categoria READER */

#define EVT_CTLPTZ_BASE				CATTYPES_MESSAGES_BASE + 10000 /**< Messaggio base per spontanee categoria CTLPTZ */
#define RSP_CTLPTZ_BASE				EVT_CTLPTZ_BASE + 100	/**< Messaggio base per risposte categoria CTLPTZ */
#define CMD_CTLPTZ_BASE				RSP_CTLPTZ_BASE + 100	/**< Messaggio base per comandi categoria CTLPTZ */

#define EVT_OCR_BASE				CATTYPES_MESSAGES_BASE + 11000 /**< Messaggio base per spontanee categoria OCR */
#define RSP_OCR_BASE				EVT_OCR_BASE + 100		/**< Messaggio base per risposte categoria OCR */
#define CMD_OCR_BASE				RSP_OCR_BASE + 100		/**< Messaggio base per comandi categoria OCR */

#define EVT_ZONE_BASE				CATTYPES_MESSAGES_BASE + 12000 // Messaggio base per spontanee categoria ZONA
#define RSP_ZONE_BASE				EVT_ZONE_BASE + 100		// Messaggio base per risposte categoria ZONA
#define CMD_ZONE_BASE				RSP_ZONE_BASE + 100		// Messaggio base per comandi categoria ZONA

#define EVT_CAMERA_BASE				CATTYPES_MESSAGES_BASE + 13000 /**< Messaggio base per spontanee categoria CAMERA */
#define RSP_CAMERA_BASE				EVT_CAMERA_BASE + 100		/**< Messaggio base per risposte categoria CAMERA */
#define CMD_CAMERA_BASE				RSP_CAMERA_BASE + 100		/**< Messaggio base per comandi categoria CAMERA */

#define EVT_ACCESSCTRL_BASE			CATTYPES_MESSAGES_BASE + 14000	/**< Messaggio base per spontanee categoria ACCESSCTRL */
#define RSP_ACCESSCTRL_BASE			EVT_ACCESSCTRL_BASE + 100		/**< Messaggio base per risposte categoria ACCESSCTRL */
#define CMD_ACCESSCTRL_BASE			RSP_ACCESSCTRL_BASE + 100		/**< Messaggio base per comandi categoria ACCESSCTRL */

#define EVT_CDC_BASE				CATTYPES_MESSAGES_BASE + 15000	/**< Messaggio base per spontanee categoria CDC */
#define RSP_CDC_BASE				EVT_CDC_BASE + 100				/**< Messaggio base per risposte categoria CDC */
#define CMD_CDC_BASE				RSP_CDC_BASE + 100				/**< Messaggio base per comandi categoria CDC */

//-----------------------------------------------------------------------
//		THO.26.06.2009: Eventi relativi alla categoria Zona
//-----------------------------------------------------------------------
#define EVT_ZONE_ARMED				EVT_ZONE_BASE + 1
#define EVT_ZONE_DISARMED			EVT_ZONE_BASE + 2
#define EVT_ZONE_ARM_FAILED			EVT_ZONE_BASE + 3
#define EVT_ZONE_DISARM_FAILED		EVT_ZONE_BASE + 4
#define EVT_ZONE_DISARM_NO_RIGHT	EVT_ZONE_BASE + 5


#define CMD_ZONE_ARMED				CMD_ZONE_BASE + 1
#define CMD_ZONE_DISARMED			CMD_ZONE_BASE + 2
#define CMD_ZONE_ENABLE				CMD_ZONE_BASE + 3
#define CMD_ZONE_DISABLE			CMD_ZONE_BASE + 4

#define RSP_ZONE_ARMED_OK			RSP_ZONE_BASE + 1
#define RSP_ZONE_ARMED_KO			RSP_ZONE_BASE + 2
#define RSP_ZONE_DISARMED_OK		RSP_ZONE_BASE + 3
#define RSP_ZONE_DISARMED_KO		RSP_ZONE_BASE + 4
#define RSP_ZONE_ENABLE_OK			RSP_ZONE_BASE + 5
#define RSP_ZONE_ENABLE_KO			RSP_ZONE_BASE + 6
#define RSP_ZONE_DISABLE_OK			RSP_ZONE_BASE + 7
#define RSP_ZONE_DISABLE_KO			RSP_ZONE_BASE + 8



//-----------------------------------------------------
// Eventi relativi alla categoria DI
//-----------------------------------------------------
#define EVT_DI_ON 					EVT_DI_BASE + 1			/**< DI - Passaggio dallo stato OFF allo stato ON */	
#define EVT_DI_OFF					EVT_DI_BASE + 2			/**< DI - Passaggio dallo stato ON allo stato OFF */	
#define EVT_DI_ENABLED				EVT_DI_BASE + 3			/**< DI - Abilitazione di un punto */ 	
#define EVT_DI_DISABLED				EVT_DI_BASE + 4			/**< DI - Disabilitazione di un punto */ 	 	
#define EVT_DI_CONF_CHANGED			EVT_DI_BASE + 5			/**< DI - Segnalazione del cambiamento di configurazione del driver */ 
#define EVT_DI_SHORT				EVT_DI_BASE + 6			/**< DI - Passaggio allo stato di corto */
#define EVT_DI_CUT					EVT_DI_BASE + 7			/**< DI - Passaggio allo stato di taglio */
#define EVT_DI_UNDEFINED			EVT_DI_BASE + 8			/**< DI - DGP fail */

#define CMD_DI_ENABLE				CMD_DI_BASE + 1			
#define CMD_DI_DISABLE				CMD_DI_BASE + 2

#define RSP_DI_DISABLE_OK			RSP_DI_BASE + 1			
#define RSP_DI_DISABLE_KO			RSP_DI_BASE + 2
#define RSP_DI_ENABLE_OK			RSP_DI_BASE + 3
#define RSP_DI_ENABLE_KO			RSP_DI_BASE + 4

//-----------------------------------------------------
// Eventi relativi alla categoria DO
//-----------------------------------------------------
#define EVT_DO_ENABLED				EVT_DO_BASE + 1	
#define EVT_DO_DISABLED				EVT_DO_BASE + 2
#define EVT_DO_CONF_CHANGED			EVT_DO_BASE + 3
#define EVT_DO_ON					EVT_DO_BASE + 4
#define EVT_DO_OFF					EVT_DO_BASE + 5


#define CMD_DO_ON_UNDEFINED			CMD_DO_BASE + 1	
#define CMD_DO_ON_DURATION			CMD_DO_BASE + 2
#define CMD_DO_OFF					CMD_DO_BASE + 3
#define CMD_DO_ENABLE				CMD_DO_BASE + 4			
#define CMD_DO_DISABLE				CMD_DO_BASE + 5


#define RSP_DO_ON_OK				RSP_DO_BASE + 1
#define RSP_DO_ON_KO				RSP_DO_BASE + 2
#define RSP_DO_OFF_OK				RSP_DO_BASE + 3
#define RSP_DO_OFF_KO				RSP_DO_BASE + 4
#define RSP_DO_DISABLE_OK			RSP_DO_BASE + 5
#define RSP_DO_DISABLE_KO			RSP_DO_BASE + 6
#define RSP_DO_ENABLE_OK			RSP_DO_BASE + 7
#define RSP_DO_ENABLE_KO			RSP_DO_BASE + 8

//-----------------------------------------------------
// Eventi relativi alla categoria TRK
//-----------------------------------------------------
#define EVT_TRK_DATA				EVT_TRK_BASE + 1	//Dati di traccia
#define EVT_TRK_ALARM				EVT_TRK_BASE + 2	//Intrusione rilevata
#define EVT_TRK_CONF_CHANGED		EVT_TRK_BASE + 3

//-----------------------------------------------------
// Eventi relativi alla categoria OCr
//-----------------------------------------------------
#define EVT_OCR_DATA				EVT_OCR_BASE + 1	//Dati di lettura
#define EVT_OCR_ALARM				EVT_OCR_BASE + 2	//Intrusione rilevata
#define EVT_OCR_CONF_CHANGED		EVT_OCR_BASE + 3

//-----------------------------------------------------
// Eventi relativi alla categoria ALMMGR
//-----------------------------------------------------
#define EVT_ALMMGR_CONF_CHANGED		EVT_ALMMGR_BASE + 1
#define EVT_ALMMGR_ALARM_ON			EVT_ALMMGR_BASE + 2
#define EVT_ALMMGR_ALARM_OFF		EVT_ALMMGR_BASE + 3
#define EVT_ALMMGR_ENABLE			EVT_ALMMGR_BASE + 4
#define EVT_ALMMGR_DISABLE			EVT_ALMMGR_BASE + 5
#define EVT_ALMMGR_ACK				EVT_ALMMGR_BASE + 6
#define EVT_ALMMGR_PURGE			EVT_ALMMGR_BASE + 7

#define CMD_ALMMGR_SET				CMD_ALMMGR_BASE + 1
#define CMD_ALMMGR_RESET			CMD_ALMMGR_BASE + 2
#define CMD_ALMMGR_ACK				CMD_ALMMGR_BASE + 3
#define CMD_ALMMGR_PURGE			CMD_ALMMGR_BASE + 4
#define CMD_ALMMGR_CREATE			CMD_ALMMGR_BASE + 5
#define CMD_ALMMGR_DELETE			CMD_ALMMGR_BASE + 6
#define CMD_ALMMGR_GETALMLIST		CMD_ALMMGR_BASE + 7
#define CMD_ALMMGR_ENABLE			CMD_ALMMGR_BASE + 8
#define CMD_ALMMGR_DISABLE			CMD_ALMMGR_BASE + 9

#define RSP_ALMMGR_SET_OK			RSP_ALMMGR_BASE + 1
#define RSP_ALMMGR_SET_KO			RSP_ALMMGR_BASE + 2
#define RSP_ALMMGR_RESET_OK			RSP_ALMMGR_BASE + 3
#define RSP_ALMMGR_RESET_KO			RSP_ALMMGR_BASE + 4
#define RSP_ALMMGR_ACK_OK			RSP_ALMMGR_BASE + 5
#define RSP_ALMMGR_ACK_KO			RSP_ALMMGR_BASE + 6
#define RSP_ALMMGR_PURGE_OK			RSP_ALMMGR_BASE + 7 
#define RSP_ALMMGR_PURGE_KO			RSP_ALMMGR_BASE + 8 
#define RSP_ALMMGR_CREATE_OK		RSP_ALMMGR_BASE + 9
#define RSP_ALMMGR_CREATE_KO		RSP_ALMMGR_BASE + 10
#define RSP_ALMMGR_DELETE_OK		RSP_ALMMGR_BASE + 11
#define RSP_ALMMGR_DELETE_KO		RSP_ALMMGR_BASE + 12
#define RSP_ALMMGR_GETALMLIST_OK	RSP_ALMMGR_BASE + 13  //Struttura dati
#define RSP_ALMMGR_GETALMLIST_KO	RSP_ALMMGR_BASE + 14  //Struttura dati
#define RSP_ALMMGR_ENABLE_OK		RSP_ALMMGR_BASE + 15
#define RSP_ALMMGR_ENABLE_KO		RSP_ALMMGR_BASE + 16
#define RSP_ALMMGR_DISABLE_OK		RSP_ALMMGR_BASE + 17
#define RSP_ALMMGR_DISABLE_KO		RSP_ALMMGR_BASE + 18

//-----------------------------------------------------
// Eventi relativi alla categoria READER // READER
//-----------------------------------------------------
#define EVT_READER_CONF_CHANGED			EVT_READER_BASE + 1
#define EVT_READER_AUTH				    EVT_READER_BASE + 2 //Codice letto allegato al messaggio
#define EVT_READER_NOT_AUTH				EVT_READER_BASE + 3 
#define EVT_READER_READ				    EVT_READER_BASE + 4 
#define EVT_READER_UNKNOWN				EVT_READER_BASE + 5
#define EVT_READER_ENABLED			    EVT_READER_BASE + 6
#define EVT_READER_DISABLED			    EVT_READER_BASE + 7 


#define RSP_READER_ENABLE			    RSP_READER_BASE + 1 //Indicare nel comando l'id del lettore
#define RSP_READER_DISABLE			    RSP_READER_BASE + 2 
#define RSP_READER_GETRDRLIST_OK			RSP_READER_BASE + 3  
#define RSP_READER_GETRDRLIST_KO			RSP_READER_BASE + 4  

//-----------------------------------------------------
// Comandi relativi alla categoria PTZ
//-----------------------------------------------------
#define CMD_PTZ_SETMOVESPEED		CMD_PTZ_BASE + 1
#define CMD_PTZ_MOVEUP				CMD_PTZ_BASE + 2
#define CMD_PTZ_MOVEDOWN			CMD_PTZ_BASE + 3
#define CMD_PTZ_MOVELEFT			CMD_PTZ_BASE + 4
#define CMD_PTZ_MOVERIGHT			CMD_PTZ_BASE + 5
#define CMD_PTZ_MOVEUPLEFT			CMD_PTZ_BASE + 6
#define CMD_PTZ_MOVEUPRIGHT			CMD_PTZ_BASE + 7
#define CMD_PTZ_MOVEDOWNLEFT		CMD_PTZ_BASE + 8
#define CMD_PTZ_MOVEDOWNRIGHT		CMD_PTZ_BASE + 9
#define CMD_PTZ_STOP				CMD_PTZ_BASE + 10
#define CMD_PTZ_GOTOPRESET			CMD_PTZ_BASE + 11
#define CMD_PTZ_SETPRESET			CMD_PTZ_BASE + 12
#define CMD_PTZ_LOADPRESETTABLE		CMD_PTZ_BASE + 13
#define CMD_PTZ_GOTOABSPOSITION		CMD_PTZ_BASE + 14
#define CMD_PTZ_CONNECT				CMD_PTZ_BASE + 15
#define CMD_PTZ_DISCONNECT			CMD_PTZ_BASE + 16

#define RSP_PTZ_SETMOVESPEED_OK			RSP_PTZ_BASE + 1
#define RSP_PTZ_SETMOVESPEED_KO			RSP_PTZ_BASE + 2
#define RSP_PTZ_MOVEUP_OK				RSP_PTZ_BASE + 3
#define RSP_PTZ_MOVEUP_KO				RSP_PTZ_BASE + 4
#define RSP_PTZ_MOVEDOWN_OK				RSP_PTZ_BASE + 5
#define RSP_PTZ_MOVEDOWN_KO				RSP_PTZ_BASE + 6
#define RSP_PTZ_MOVELEFT_OK				RSP_PTZ_BASE + 7
#define RSP_PTZ_MOVELEFT_KO				RSP_PTZ_BASE + 8
#define RSP_PTZ_MOVERIGHT_OK			RSP_PTZ_BASE + 9
#define RSP_PTZ_MOVERIGHT_KO			RSP_PTZ_BASE + 10
#define RSP_PTZ_MOVEUPLEFT_OK			RSP_PTZ_BASE + 11
#define RSP_PTZ_MOVEUPLEFT_KO			RSP_PTZ_BASE + 12
#define RSP_PTZ_MOVEUPRIGHT_OK			RSP_PTZ_BASE + 13
#define RSP_PTZ_MOVEUPRIGHT_KO			RSP_PTZ_BASE + 14
#define RSP_PTZ_MOVEDOWNLEFT_OK			RSP_PTZ_BASE + 15
#define RSP_PTZ_MOVEDOWNLEFT_KO			RSP_PTZ_BASE + 16
#define RSP_PTZ_MOVEDOWNRIGHT_OK		RSP_PTZ_BASE + 17
#define RSP_PTZ_MOVEDOWNRIGHT_KO		RSP_PTZ_BASE + 18
#define RSP_PTZ_STOP_OK					RSP_PTZ_BASE + 19
#define RSP_PTZ_STOP_KO					RSP_PTZ_BASE + 20
#define RSP_PTZ_GOTOPRESET_OK			RSP_PTZ_BASE + 21
#define RSP_PTZ_GOTOPRESET_KO			RSP_PTZ_BASE + 22
#define RSP_PTZ_SETPRESET_OK			RSP_PTZ_BASE + 23
#define RSP_PTZ_SETPRESET_KO			RSP_PTZ_BASE + 24
#define RSP_PTZ_LOADPRESETTABLE_OK		RSP_PTZ_BASE + 25
#define RSP_PTZ_LOADPRESETTABLE_KO		RSP_PTZ_BASE + 26
#define RSP_PTZ_GOTOABSPOSITION_OK		RSP_PTZ_BASE + 27
#define RSP_PTZ_GOTOABSPOSITION_KO		RSP_PTZ_BASE + 28
#define RSP_PTZ_CONNECT_OK				RSP_PTZ_BASE + 29
#define RSP_PTZ_CONNECT_KO				RSP_PTZ_BASE + 30
#define RSP_PTZ_DISCONNECT_OK			RSP_PTZ_BASE + 31
#define RSP_PTZ_DISCONNECT_KO			RSP_PTZ_BASE + 32


//-----------------------------------------------------
// Eventi relativi alla categoria CAMERA
//-----------------------------------------------------
#define EVT_CAMERA_ENABLED				EVT_CAMERA_BASE + 1
#define EVT_CAMERA_DISABLED				EVT_CAMERA_BASE + 2
#define EVT_CAMERA_VLOSS				EVT_CAMERA_BASE + 3
#define EVT_CAMERA_READY				EVT_CAMERA_BASE + 4

//-----------------------------------------------------
// Comandi relativi alla categoria CAMERA
//-----------------------------------------------------
#define CMD_CAMERA_SETMOVESPEED			CMD_CAMERA_BASE + 1
#define CMD_CAMERA_MOVEUP				CMD_CAMERA_BASE + 2
#define CMD_CAMERA_MOVEDOWN				CMD_CAMERA_BASE + 3
#define CMD_CAMERA_MOVELEFT				CMD_CAMERA_BASE + 4
#define CMD_CAMERA_MOVERIGHT			CMD_CAMERA_BASE + 5
#define CMD_CAMERA_MOVEUPLEFT			CMD_CAMERA_BASE + 6
#define CMD_CAMERA_MOVEUPRIGHT			CMD_CAMERA_BASE + 7
#define CMD_CAMERA_MOVEDOWNLEFT			CMD_CAMERA_BASE + 8
#define CMD_CAMERA_MOVEDOWNRIGHT		CMD_CAMERA_BASE + 9
#define CMD_CAMERA_STOP					CMD_CAMERA_BASE + 10
#define CMD_CAMERA_GOTOPRESET			CMD_CAMERA_BASE + 11
#define CMD_CAMERA_SETPRESET			CMD_CAMERA_BASE + 12
#define CMD_CAMERA_LOADPRESETTABLE		CMD_CAMERA_BASE + 13
#define CMD_CAMERA_GOTOABSPOSITION		CMD_CAMERA_BASE + 14
#define CMD_CAMERA_SETBACKLIGHTON		CMD_CAMERA_BASE + 15
#define CMD_CAMERA_SETBACKLIGHTOFF		CMD_CAMERA_BASE + 16
#define CMD_CAMERA_ZOOMIN				CMD_CAMERA_BASE + 17
#define CMD_CAMERA_ZOOMOUT				CMD_CAMERA_BASE + 18
#define CMD_CAMERA_CONNECT				CMD_CAMERA_BASE + 19
#define CMD_CAMERA_DISCONNECT			CMD_CAMERA_BASE + 20

#define RSP_CAMERA_SETMOVESPEED_OK			RSP_CAMERA_BASE + 1
#define RSP_CAMERA_SETMOVESPEED_KO			RSP_CAMERA_BASE + 2
#define RSP_CAMERA_MOVEUP_OK				RSP_CAMERA_BASE + 3
#define RSP_CAMERA_MOVEUP_KO				RSP_CAMERA_BASE + 4
#define RSP_CAMERA_MOVEDOWN_OK				RSP_CAMERA_BASE + 5
#define RSP_CAMERA_MOVEDOWN_KO				RSP_CAMERA_BASE + 6
#define RSP_CAMERA_MOVELEFT_OK				RSP_CAMERA_BASE + 7
#define RSP_CAMERA_MOVELEFT_KO				RSP_CAMERA_BASE + 8
#define RSP_CAMERA_MOVERIGHT_OK				RSP_CAMERA_BASE + 9
#define RSP_CAMERA_MOVERIGHT_KO				RSP_CAMERA_BASE + 10
#define RSP_CAMERA_MOVEUPLEFT_OK			RSP_CAMERA_BASE + 11
#define RSP_CAMERA_MOVEUPLEFT_KO			RSP_CAMERA_BASE + 12
#define RSP_CAMERA_MOVEUPRIGHT_OK			RSP_CAMERA_BASE + 13
#define RSP_CAMERA_MOVEUPRIGHT_KO			RSP_CAMERA_BASE + 14
#define RSP_CAMERA_MOVEDOWNLEFT_OK			RSP_CAMERA_BASE + 15
#define RSP_CAMERA_MOVEDOWNLEFT_KO			RSP_CAMERA_BASE + 16
#define RSP_CAMERA_MOVEDOWNRIGHT_OK			RSP_CAMERA_BASE + 17
#define RSP_CAMERA_MOVEDOWNRIGHT_KO			RSP_CAMERA_BASE + 18
#define RSP_CAMERA_STOP_OK					RSP_CAMERA_BASE + 19
#define RSP_CAMERA_STOP_KO					RSP_CAMERA_BASE + 20
#define RSP_CAMERA_GOTOPRESET_OK			RSP_CAMERA_BASE + 21
#define RSP_CAMERA_GOTOPRESET_KO			RSP_CAMERA_BASE + 22
#define RSP_CAMERA_SETPRESET_OK				RSP_CAMERA_BASE + 23
#define RSP_CAMERA_SETPRESET_KO				RSP_CAMERA_BASE + 24
#define RSP_CAMERA_LOADPRESETTABLE_OK		RSP_CAMERA_BASE + 25
#define RSP_CAMERA_LOADPRESETTABLE_KO		RSP_CAMERA_BASE + 26
#define RSP_CAMERA_GOTOABSPOSITION_OK		RSP_CAMERA_BASE + 27
#define RSP_CAMERA_GOTOABSPOSITION_KO		RSP_CAMERA_BASE + 28
#define RSP_CAMERA_SETBACKLIGHTON_OK		RSP_CAMERA_BASE + 29
#define RSP_CAMERA_SETBACKLIGHTON_KO		RSP_CAMERA_BASE + 30
#define RSP_CAMERA_SETBACKLIGHTOFF_OK		RSP_CAMERA_BASE + 31
#define RSP_CAMERA_SETBACKLIGHTOFF_KO		RSP_CAMERA_BASE + 32
#define RSP_CAMERA_ZOOMIN_OK				CMD_CAMERA_BASE + 33
#define RSP_CAMERA_ZOOMIN_KO				CMD_CAMERA_BASE + 34
#define RSP_CAMERA_ZOOMOUT_OK				CMD_CAMERA_BASE + 35
#define RSP_CAMERA_ZOOMOUT_KO				CMD_CAMERA_BASE + 36
#define RSP_CAMERA_CONNECT_OK				CMD_CAMERA_BASE + 37
#define RSP_CAMERA_CONNECT_KO				CMD_CAMERA_BASE + 38
#define RSP_CAMERA_DISCONNECT_OK			CMD_CAMERA_BASE + 39
#define RSP_CAMERA_DISCONNECT_KO			CMD_CAMERA_BASE + 40

//-----------------------------------------------------
// Eventi relativi alla categoria CTLPTZ
//-----------------------------------------------------
#define EVT_CTLPTZ_SETMOVESPEED			EVT_CTLPTZ_BASE + 1
#define EVT_CTLPTZ_MOVEUP				EVT_CTLPTZ_BASE + 2
#define EVT_CTLPTZ_MOVEDOWN				EVT_CTLPTZ_BASE + 3
#define EVT_CTLPTZ_MOVELEFT				EVT_CTLPTZ_BASE + 4
#define EVT_CTLPTZ_MOVERIGHT			EVT_CTLPTZ_BASE + 5
#define EVT_CTLPTZ_MOVEUPLEFT			EVT_CTLPTZ_BASE + 6
#define EVT_CTLPTZ_MOVEUPRIGHT			EVT_CTLPTZ_BASE + 7
#define EVT_CTLPTZ_MOVEDOWNLEFT			EVT_CTLPTZ_BASE + 8
#define EVT_CTLPTZ_MOVEDOWNRIGHT		EVT_CTLPTZ_BASE + 9
#define EVT_CTLPTZ_STOP					EVT_CTLPTZ_BASE + 10
#define EVT_CTLPTZ_GOTOPRESET			EVT_CTLPTZ_BASE + 11
#define EVT_CTLPTZ_SETPRESET			EVT_CTLPTZ_BASE + 12
#define EVT_CTLPTZ_LOADPRESETTABLE		EVT_CTLPTZ_BASE + 13
#define EVT_CTLPTZ_GOTOABSPOSITION		EVT_CTLPTZ_BASE + 14


//-----------------------------------------------------
// Eventi relativi alla categoria DVR
//-----------------------------------------------------
#define EVT_DVR_CONNECTED				EVT_DVR_BASE	+ 1			
#define EVT_DVR_DISCONNECTED			EVT_DVR_BASE	+ 2

#define CMD_DVR_EXTRACT_VIDEO			CMD_DVR_BASE	+ 1				
#define CMD_DVR_START_REC				CMD_DVR_BASE	+ 2				
#define CMD_DVR_STOP_REC				CMD_DVR_BASE	+ 3				

#define RSP_DVR_EXTRACT_VIDEO_OK		RSP_DVR_BASE	+ 1				
#define RSP_DVR_EXTRACT_VIDEO_KO		RSP_DVR_BASE	+ 2				
#define RSP_DVR_START_REC_OK			RSP_DVR_BASE	+ 3				
#define RSP_DVR_START_REC_KO			RSP_DVR_BASE	+ 4				
#define RSP_DVR_STOP_REC_OK				RSP_DVR_BASE	+ 5				
#define RSP_DVR_STOP_REC_KO				RSP_DVR_BASE	+ 6				

//-----------------------------------------------------
// Eventi relativi alla categoria VIDEOSRV
//-----------------------------------------------------

//-----------------------------------------------------
// Eventi relativi alla categoria CONTROLLO ACCESSI
//-----------------------------------------------------
#define SIDMON_UPDATE_DATA				EVT_ACCESSCTRL_BASE+1


//-----------------------------------------------------
// Strutture dati relative agli eventi categoria TRK
//-----------------------------------------------------

//-----------------------------------------------------
// Eventi relativi alla categoria CDC
//-----------------------------------------------------
#define EVT_CDC_LINKON 				EVT_CDC_BASE + 1		/**< CDC - Passaggio dallo stato OFF allo stato ON */	
#define EVT_CDC_LINKOFF				EVT_CDC_BASE + 2		/**< CDC - Passaggio dallo stato ON allo stato OFF */	
#define EVT_CDC_ENABLED				EVT_CDC_BASE + 3		/**< CDC - Abilitazione di un apparato CDC */ 	
#define EVT_CDC_DISABLED			EVT_CDC_BASE + 4		/**< CDC - Disabilitazione di un apparato CDC */ 	 	
#define EVT_CDC_CONF_CHANGED		EVT_CDC_BASE + 5		/**< CDC - Segnalazione del cambiamento di configurazione del driver */ 
#define EVT_CDC_UNDEFINED			EVT_CDC_BASE + 6		/**< CDC - DGP fail */

#define CMD_CDC_ENABLE				CMD_CDC_BASE + 1			
#define CMD_CDC_DISABLE				CMD_CDC_BASE + 2

#define RSP_CDC_DISABLE_OK			RSP_CDC_BASE + 1			
#define RSP_CDC_DISABLE_KO			RSP_CDC_BASE + 2
#define RSP_CDC_ENABLE_OK			RSP_CDC_BASE + 3
#define RSP_CDC_ENABLE_KO			RSP_CDC_BASE + 4


//#pragma pack(push,1)

struct TRKData //FOR EVENT_TRK_DATA
{
protected:
	char NAME[32];
public:
	unsigned int SIDUID;			/**< Identificativo univoco del SID mittente */
	std::string     trackId;		/**< Identificativo dell'istanza di traccia */
	std::string     info;			/**< Informazioni supplementari */
	std::string     details;		/**< Eventuali dettagli dell'oggetto/operazione */
	float x;						/**< Posizione x in coordinate cartesiane rispetto al punto di intallazione del sensore */
	float y;						/**< Posizione y in coordinate cartesiane rispetto al punto di intallazione del sensore */
	float z;						/**< Posizione z in coordinate cartesiane rispetto al punto di intallazione del sensore */
	float range;					/**< Distanza del bersaglio dal punto di installazione del sensore */
	float az;						/**< Azimuth del bersaglio rispetto al punto di installazione del sensore */
	float elevation;				/**< Elevazione del bersaglio rispetto al punto di installazione del sensore */
	std::string      subId;         /**< sensore che ha scatenato l'evento */
	std::string      datetime;		/**< data e ora dell'evento */
};

//-----------------------------------------------------
// Strutture dati relative agli eventi categoria OCR
//-----------------------------------------------------

//#pragma pack(push,1)

struct OCRData //FOR EVENT_TRK_DATA
{
protected:
	char NAME[32];
public:
	unsigned int	SIDUID;				/**< Identificativo univoco del SID mittente */
	std::string     info;				/**< Informazioni aggiuntive sull'evento dati */
	std::string     details;			/**< Eventuali dettagli dell'oggetto/operazione */
	std::string     subId;				/**< Sensore che ha scatenato l'evento (id interno al SID) */
	std::string     Code_1;				/**< Prima lettura alfanumerica effettuata tramite OCR */
	std::string     Code_2;				/**< Seconda lettura alfanumerica effettuata tramite OCR */
	std::string     Code_3;				/**< Teerza lettura alfanumerica effettuata tramite OCR */
	std::string     Code_4;				/**< Quarta lettura alfanumerica effettuata tramite OCR */
	std::string     Code_5;				/**< Quinta lettura alfanumerica effettuata tramite OCR */
	std::string     Code_6;				/**< Sesta lettura alfanumerica effettuata tramite OCR */
	std::string     Path_Img_Code_1;	/**< Path dell'immagine su cui è stata eseguita la scansione per ottenere la prima lettura OCR */
	std::string     Path_Img_Code_2;	/**< Path dell'immagine su cui è stata eseguita la scansione per ottenere la seconda lettura OCR */
	std::string     Path_Img_Code_3;	/**< Path dell'immagine su cui è stata eseguita la scansione per ottenere la terza lettura OCR */
	std::string     Path_Img_Code_4;	/**< Path dell'immagine su cui è stata eseguita la scansione per ottenere la quarta lettura OCR */
	std::string     Path_Img_Code_5;	/**< Path dell'immagine su cui è stata eseguita la scansione per ottenere la quinta lettura OCR */
	std::string     Path_Img_Code_6;	/**< Path dell'immagine su cui è stata eseguita la scansione per ottenere la sesta lettura OCR */
	std::string     datetime;			/**< Data e ora dell'evento */
};


//#pragma pack(pop)

//-------------------------------------------------------
// Strutture dati relative agli eventi categoria ALMMGR
//-------------------------------------------------------
enum enumAlarmState { ALARMOFF = 0, ALARMON = 1, ALARMENABLE = 2, ALARMDISABLE = 3 /*, ALARMSHORT=2, ALARMCUT=3 */ };
struct ALMMGRData {
protected:
	char NAME[32];
public:
	unsigned int     SIDUID;		/**<  id del SID che sta gestendo l'informazione */
	std::string      info;			/**<  descrizione oggetto */
	std::string      details;		/**<  eventuali dettagli dell'oggetto/operazione */
	std::string      subId;			/**<  id dell'allarme rilevato  */
	long             state;			/**<  1=SET, 2=RESET, 3=CUT, 4=SHORT, 5=ENABLED, 6=DISABLED */
	long             count;			/**<  numero di occorrenza della segnalazione specifica   */
	std::string      datetime;		/**<  data e ora dell'evento */
	bool			 initEvent;		/**<  Il messaggio è di inizializzazione */
	unsigned int	 processState;	/**<  1=ACTIVE, 2=ACK, 3=INACTIVE*/

	ALMMGRData()
	{
		memset(NAME, 0, sizeof(NAME));
		strncpy_s(NAME, "ALMMGRData", sizeof(NAME));
		initEvent = false;
		processState = 3; //Inactive (questo va riempito dal SID di FC)
	}
	//...................
};


//-----------------------------------------------------------------------
//		THO.25.06.2009: Struttura dati per gli eventi categoria Zona
//-----------------------------------------------------------------------
enum enumZoneState { ZONEARMED = 1, ZONEDISARMED = 0, ZONEARMFAILED = 2, ZONEDISARMFAILED = 3, ZONEDISARMNORIGHT = 4 };
struct ZONEData {
protected:
	char NAME[32];
public:
	unsigned int    SIDUID;				/**< id del SID che sta gestendo l'informazione */
	std::string     info;				/**< Stato della transazione */
	std::string     details;			/**< Descrizione della ZONA da cui è generata l'informazione */
	std::string     subId;				/**< Id della Zona da cui è stata generata l'informazione */
	std::string     datetime;			/**< Orario di lettura */
	std::string		readerId;			/**< Id del lettore da cui è stata eseguita l'arm/disrm */
	bool			initEvent;			/**<  Il messaggio è di inizializzazione */

	ZONEData() {
		memset(NAME, 0, sizeof(NAME));
		strncpy_s(NAME, "ZONEData", sizeof(NAME));
		initEvent = false;
	}
};


//-------------------------------------------------------
// Strutture dati relative agli eventi categoria READER
//-------------------------------------------------------
enum ReaderTransType { READING = 0, VALID, EXPIRED, LOST, SUSPENDED, ENABLEAREA, DISABLEAREA, UNASSIGNED };

struct READERData  {
protected:
	char NAME[32];
public:
	unsigned int    SIDUID;		/**< Identificativo univoco del SID mittente */
	std::string     info;		/**< Numero del badge */
	std::string     details;	/**< Descrizione del lettore da cui è generata l'informazione */
	std::string     subId;		/**< Id del lettore da cui è stata generata l'informzaione */
	std::string		person;		/**< Titolare del badge (se associato) */
	std::string		datetime;	/**< Orario di lettura */
	ReaderTransType type;		/**< Enumerato relativo al tipo di segnalazione effettuata dal sottosistema */
	bool			initEvent;	/**<  Il messaggio è di inizializzazione */

	READERData() {
		memset(NAME, 0, sizeof(NAME));
		strncpy_s(NAME, "READERData", sizeof(NAME));
		initEvent = false;
	}
};

//-------------------------------------------------------
// Strutture dati relative agli eventi categoria DI
//-------------------------------------------------------

enum enumDIState { DIOFF = 0, DION = 1, DISHORT = 2, DICUT = 3, DIUNDEFINED = 4 };
struct DIData {
protected:
	char NAME[32];
public:
	unsigned int    SIDUID;		/**< Identificativo univoco del SID mittente */
	std::string     info;        /**< Stato della transazione */
	std::string     details;     /**< Descrizione del punto da cui è generata l'informazione */
	std::string     subId;		/**< Id del punto da cui è stata generata l'informazione */
	std::string     datetime;    /**< Orario di lettura */
	bool			initEvent;	/**<  Il messaggio è di inizializzazione */

	DIData() {
		memset(NAME, 0, sizeof(NAME));
		strncpy_s(NAME, "DIData", sizeof(NAME));
		initEvent = false;
	}
};

//-------------------------------------------------------
// Strutture dati relative agli eventi (spontanei... ce ne sono?) categoria DO
//-------------------------------------------------------

struct DOData //For event .....
{
protected:
	char NAME[32];
public:
	unsigned int    SIDUID;		/**< Identificativo univoco del SID mittente */
	std::string     info;       /**< Stato della transazione */
	std::string     details;    /**< Descrizione del punto da cui è generata l'informazione */
	std::string     subId;		/**< Id del punto da cui è stata generata l'informazione */
	std::string		datetime;   /**< Orario di lettura */
	bool			initEvent;	/**<  Il messaggio è di inizializzazione */

	DOData()
	{
		memset(NAME, 0, sizeof(NAME));
		strncpy_s(NAME, "DOData", sizeof(NAME));
		SIDUID = 0;
		info = "";
		details = "";
		subId = "";
		datetime = "";
		initEvent = false;
	};
};
//-------------------------------------------------------

//-------------------------------------------------------
// Strutture dati relative agli eventi categoria CAMERA
//-------------------------------------------------------

enum enumCameraState { CAMENABLED, CAMDISABLED, CAMVLOSS, CAMREADY };
struct CAMData //For event .....
{
protected:
	char NAME[32];
public:
	unsigned int	SIDUID;		/**< Identificativo univoco del SID mittente */
	std::string     info;       /**< Stato della transazione */
	std::string     details;    /**< Descrizione del punto da cui è generata l'informazione */
	std::string     subId;		/**< Id del punto da cui è stata generata l'informazione */
	std::string     datetime;   /**< Orario di lettura */
	bool			initEvent;	/**<  Il messaggio è di inizializzazione */

	CAMData()
	{
		memset(NAME, 0, sizeof(NAME));
		strncpy_s(NAME, "CameraData", sizeof(NAME));
		initEvent = false;
	}
};

//-------------------------------------------------------
// Strutture dati relative agli eventi categoria CDC
//-------------------------------------------------------

enum enumCDCState { CDCLINKOFF = 0, CDCLINKON = 1, CDCUNDEFINED = 4 };
struct CDCData //For event .....
{
protected:
	char NAME[32];
public:
	unsigned int    SIDUID;		/**< Identificativo univoco del SID mittente */
	std::string     info;        /**< Stato della transazione */
	std::string     details;     /**< Descrizione dell'Concentratore di Campoche ha generata l'informazione */
	std::string     subId;		/**< Id del CDC da cui è stata generata l'informazione */
	std::string     datetime;    /**< Orario di lettura */
	bool			initEvent;	/**<  Il messaggio è di inizializzazione */

	CDCData()
	{
		memset(NAME, 0, sizeof(NAME));
		strncpy_s(NAME, "CDCData", sizeof(NAME));
		initEvent = false;
	}
};


//-------------------------------------------
//GESTIONE DEI PARAMETRI
//-------------------------------------------
/*
	 I parametri di ogni possibile istanza di azione sugli attuatori vengono memorizzati
	 sul database. All'avvio di SARA tali parametri vengono messi in memoria sulle hash tabel
	 di pertinenza dell'oggetto Action Logic.
	 I category manager per i comandi di ATTUAZIONE tutti utilizzano lo stesso prototipo

	 <nome metodo comando> (int idDevice, int idComando, TMapParametriAzione mappaParams);

 */
struct Parameter
{
protected:
	char NAME[32];
public:
	std::string sAlias;		/**< Nome del parametro così come appare sul database */
	std::string sType;		/**< Tipo del parametro così come appare sul database */
	std::string sValue;		/**< Valore del parametro così come appare sul database (stringa) */
	//Parsed values
	int			iValue;		/**< Valore del parametro parsato ad intero */
	float		fValue;		/**< Valore del parametro parsato a reale */

	Parameter()
	{
		memset(NAME, 0, sizeof(NAME));
		strncpy_s(NAME, "Parameter", sizeof(NAME));
		iValue = 0;
		fValue = 0;
	};
};

typedef std::map<std::string, Parameter*> TMapParametriAzione;

typedef struct {
	int			SIDUID;
	std::string strParam;
} UPDATE_MESSAGE;
