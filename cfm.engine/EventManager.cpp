/****************************** Module Header ******************************\
* Module Name:  EventManager.h
* Project:      S000
*
* Reacts to the events in system
*
\***************************************************************************/

#include "EventManager.h"
#include "DatabaseManager.h"
#include "ConfigParams.h"

extern cfm::application::CConfigParms cfmRegCfg;

namespace cfm::application {
	//Per rendere visibile il thread a KERNEL
	int EventManagerId = 0;			/**< Utilizzata dai Category Manager */

	CEventManager::CEventManager() {
//		actionLogic = nullptr;
		mappaEventi = DBManager::getInstance()->getEvents();

	}
	//----------------------------------------------------------------------------
	CfmDevices_Table* CEventManager::CheckDevice(unsigned long iDev, char* msg) {
		char logBuf[1024];
		CfmDevices_Table* clonedData = DBManager::getInstance()->getDevice(iDev);
		if (clonedData == NULL) {
			sprintf_s(logBuf, sizeof(logBuf), "%s - NON CONFIGURATO", msg);
			sLogger->Log(logBuf, CLogger::DEBUG_LEVEL_VERY_LOW);
		}
		else if (clonedData->bDeleted == recDeleted) {
			sprintf_s(logBuf, sizeof(logBuf), "%s - CANCELLATO", msg);
			sLogger->Log(logBuf, CLogger::DEBUG_LEVEL_VERY_LOW);
			delete clonedData;
			clonedData = NULL;
		}
		return clonedData;
	}
	//----------------------------------------------------------------------------

	DWORD CEventManager::Run(LPVOID /* arg */) {

		EventManagerId = this->m_ThreadCtx.m_dwTID;

		sLogger = CLogger::getInstance();

		//Visibilità del ThEventManager agli altri moduli
		sLogger->BindEventManagerId(EventManagerId);
//Pekmez		IdThEventManager = EventManagerId;

		bool bRestoreAfterScan = (cfmRegCfg.RESTORE_AFTER_SCAN()) ? true : false;
		std::string region = "1"; //IntToStr(saraRegCfg.SARA_REGION_ID());

		//--------------------------------------------------------------------------
		//Ciclo principale del Thread in cui ci si mette in ascolto dei messaggi
		//--------------------------------------------------------------------------
		bool terminate = false;
		while (!terminate) {
			MSG msg;
			terminate = !GetMessageA(&msg, NULL, 0, 0);
			int message = msg.message;
			Sleep(10);
			//switch (message)
			//{
			//case RULES_RELOAD_REQ:
			//	actionLogic->Init();
			//	MessageSender::getInstance()->Send("Rules reloaded on region " + region + ".\r\n", (void*)msg.lParam);
			//	break;
			//	//-----------------------------
			//	// START MANAGING TRACKS EVENTS
			//	//-----------------------------
			//case EVT_TRK_DATA:
			//{
			//	PREPARE_DATA(TRKData)
			//		//int iDev = CatTrk::getInstance()->(data->SIDUID,  data->subId);

			//		sprintf_s(logBuf, sizeof(logBuf), "TRACCIA [TRK]ID:%s x:%f y:%f z:%f SIDUID:%i", data->trackId.c_str(), data->x, data->y, data->z, (int)msg.lParam);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_VERY_HIGH, (int)msg.lParam); //Lo logga solo in debug mode
			//	sprintf_s(logBuf, sizeof(logBuf), "TRACCIA [TRK] az:%f el:%f range:%f ts:%s", data->az, data->elevation, data->range, data->datetime.c_str());
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_VERY_HIGH, (int)msg.lParam); //Lo logga solo in debug mode
			//	//Database 
			//	//DBM->LogTrack(1 , data->trackId, data->x, data->y, data->z, data->az, data->range, data->elevation);
			//	//Processo la regola relativa all'evento
			//	actionLogic->ProcessEventRule(TRK_SENT, data->SIDUID, data->subId, TRK, data);

			//	//SaraDevices_Table *clonedData = DBManager::getInstance()->getDevice(iDev);
			//	//PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_TRK, (WPARAM) clonedData, (LPARAM) TRK_SENT);
			//}
			//break;
			////-----------------------------
			//// END MANAGING TRACKS EVENTS
			////-----------------------------

			////-----------------------------
			//// START MANAGING OCR EVENTS
			////-----------------------------
			//case EVT_OCR_DATA:
			//{
			//	OCRData* data = (OCRData*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "RECORD [OCR] CODE_1: %s SIDUID:%i", data->Code_1.c_str(), (int)msg.lParam);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_VERY_HIGH, (int)msg.lParam); //Lo logga solo in debug mode
			//	//Database 
			//	//DBM->LogTrack(1 , data->trackId, data->x, data->y, data->z, data->az, data->range, data->elevation);
			//	//Processo la regola relativa all'evento
			//	actionLogic->ProcessEventRule(OCR_SENT, data->SIDUID, data->subId, OCR, data);
			//	//Libero la memoria del pacchetto dati
			//	//delete data;
			//}
			//break;
			////-----------------------------
			//// END MANAGING OCR EVENTS
			////-----------------------------

			////------------------------------
			//// START MANAGING ALARM EVENTS
			////------------------------------
			//case EVT_ALMMGR_ALARM_ON:
			//{
			//	PREPARE_DATA_INIT(ALMMGRData)
			//		int iDev = CatAlmMgr::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR(ON):  %s  : %s] - [DATETIME: %s]  - SIDUID:%i",
			//		data->info.c_str(), data->details.c_str(), data->datetime.c_str(), sidId);
			//	actionLogic->ProcessEventRule(ALARM_ON, sidId, data->subId, ALMMGR, data, data->datetime);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ALARM, (WPARAM)clonedData, (LPARAM)ALARM_ON);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_ALMMGR_ALARM_OFF:
			//{
			//	PREPARE_DATA_INIT(ALMMGRData)

			//		int iDev = CatAlmMgr::getInstance()->getIdDevice(data->SIDUID, data->subId);
			//	int status = data->state;

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR(OFF):  %s  : %s] - [DATETIME: %s]  - SIDUID:%i",
			//		data->info.c_str(), data->details.c_str(), data->datetime.c_str(), sidId);
			//	actionLogic->ProcessEventRule(ALARM_OFF, sidId, data->subId, ALMMGR, data, data->datetime);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//		{
			//			if (status == 2)
			//				clonedData->PhisicalStatus = 0; //WRAPPA FC
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ALARM, (WPARAM)clonedData, (LPARAM)ALARM_OFF);
			//		}
			//		else
			//			delete clonedData;
			//	}

			//}
			//break;

			//case EVT_ALMMGR_ENABLE:
			//{
			//	PREPARE_DATA_INIT(ALMMGRData)

			//		int iDev = CatAlmMgr::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR(ENABLE):  %s  : %s] - [DATETIME: %s]  - SIDUID:%i",
			//		data->info.c_str(), data->details.c_str(), data->datetime.c_str(), sidId);
			//	actionLogic->ProcessEventRule(ALARM_ENABLED, sidId, data->subId, ALMMGR, data, data->datetime);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ALARM, (WPARAM)clonedData, (LPARAM)ALARM_ENABLED);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_ALMMGR_DISABLE:
			//{
			//	PREPARE_DATA_INIT(ALMMGRData)
			//		int iDev = CatAlmMgr::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR(DISABLE):  %s  : %s] - [DATETIME: %s]  - SIDUID:%i",
			//		data->info.c_str(), data->details.c_str(), data->datetime.c_str(), sidId);
			//	actionLogic->ProcessEventRule(ALARM_DISABLED, sidId, data->subId, ALMMGR, data, data->datetime);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ALARM, (WPARAM)clonedData, (LPARAM)ALARM_DISABLED);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_ALMMGR_ACK:
			//{
			//	PREPARE_DATA_INIT(ALMMGRData)
			//		int iDev = CatAlmMgr::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR(ACK):  %s  : %s] - [DATETIME: %s]  - SIDUID:%i",
			//		data->info.c_str(), data->details.c_str(), data->datetime.c_str(), sidId);
			//	actionLogic->ProcessEventRule(ALARM_ACKED, sidId, data->subId, ALMMGR, data, data->datetime);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ALARM, (WPARAM)clonedData, (LPARAM)ALARM_ACKED);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_ALMMGR_PURGE:
			//{
			//	PREPARE_DATA_INIT(ALMMGRData)
			//		int iDev = CatAlmMgr::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR(PURGE):  %s  : %s] - [DATETIME: %s]  - SIDUID:%i",
			//		data->info.c_str(), data->details.c_str(), data->datetime.c_str(), sidId);
			//	actionLogic->ProcessEventRule(ALARM_PURGED, sidId, data->subId, ALMMGR, data, data->datetime);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ALARM, (WPARAM)clonedData, (LPARAM)ALARM_PURGED);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;


			////------------------------------
			//// STOP MANAGING ALARM EVENTS
			////------------------------------
			////---------------------------------------
			//// START MANAGING READER READERS EVENTS
			////---------------------------------------
			//case EVT_READER_ENABLED:
			//{
			//	READERData* data = (READERData*)msg.wParam;
			//	int sidId = data->SIDUID;

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [READER:%s] [DATETIME: %s] [READER ID:%s] [READER:%s] - [SIDUID:%i]",
			//		data->info.c_str(), data->datetime.c_str(), data->subId.c_str(), data->details.c_str(), data->SIDUID);
			//	actionLogic->ProcessEventRule(READER_ENABLED, data->SIDUID, data->subId, READER, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);
			//}
			//break;

			//case EVT_READER_DISABLED:
			//{
			//	READERData* data = (READERData*)msg.wParam;
			//	int sidId = data->SIDUID;

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [READER:%s] [DATETIME: %s] [READER ID:%s] [READER:%s] - [SIDUID:%i]",
			//		data->info.c_str(), data->datetime.c_str(), data->subId.c_str(), data->details.c_str(), data->SIDUID);
			//	actionLogic->ProcessEventRule(READER_DISABLED, data->SIDUID, data->subId, READER, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);
			//}
			//break;

			//case EVT_READER_READ:
			//{
			//	READERData* data = (READERData*)msg.wParam;
			//	int sidId = data->SIDUID;

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [READER:%s] [DATETIME: %s] [READER ID:%s] [READER:%s] - [SIDUID:%i]",
			//		data->info.c_str(), data->datetime.c_str(), data->subId.c_str(), data->details.c_str(), data->SIDUID);
			//	actionLogic->ProcessEventRule(BDG_SWIPE, data->SIDUID, data->subId, READER, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);
			//}
			//break;

			//case EVT_READER_AUTH:
			//{
			//	PREPARE_DATA_INIT(READERData)
			//		int iDev = CatReader::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [BDGAUTH:%s] [DATETIME: %s] [READER ID:%s] [READER:%s] - [SIDUID:%i]",
			//		data->info.c_str(), data->datetime.c_str(), data->subId.c_str(), data->details.c_str(), data->SIDUID);
			//	actionLogic->ProcessEventRule(BDG_AUTH, data->SIDUID, data->subId, READER, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_READER, (WPARAM)clonedData, (LPARAM)BDG_AUTH);
			//		else
			//			delete clonedData;
			//	}

			//}
			//break;

			//case EVT_READER_UNKNOWN:
			//{
			//	PREPARE_DATA_INIT(READERData)
			//		int iDev = CatReader::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [BDGUNKNOWN:%s] [DATETIME: %s] [READER ID:%s] [READER:%s] - [SIDUID:%i]",
			//		data->info.c_str(), data->datetime.c_str(), data->subId.c_str(), data->details.c_str(), data->SIDUID);
			//	actionLogic->ProcessEventRule(READER_BDGUNKNOWN, data->SIDUID, data->subId, READER, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_READER, (WPARAM)clonedData, (LPARAM)READER_BDGUNKNOWN);
			//		else
			//			delete clonedData;
			//	}

			//}
			//break;

			//case EVT_READER_NOT_AUTH:
			//{
			//	PREPARE_DATA_INIT(READERData)
			//		int iDev = CatReader::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [BDGNOAUTH:%s] [DATETIME: %s] [READER ID:%s] [READER:%s] - [SIDUID:%i]",
			//		data->info.c_str(), data->datetime.c_str(), data->subId.c_str(), data->details.c_str(), data->SIDUID);
			//	actionLogic->ProcessEventRule(BDG_NOAUTH, data->SIDUID, data->subId, READER, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_READER, (WPARAM)clonedData, (LPARAM)BDG_NOAUTH);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			////---------------------------------------
			//// STOP MANAGING READER READERS EVENTS
			////---------------------------------------

			//// OSC 01/07/2009 
			////---------------------------------------
			//// START MANAGING ZONE EVENTS
			////---------------------------------------
			//case EVT_ZONE_ARMED:
			//{
			//	PREPARE_DATA_INIT(ZONEData)
			//		int iDev = CatZone::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ZONE:%s] [DATETIME: %s] [:%s] [:%s] - [SIDUID:%i]",
			//		data->info.c_str(), data->datetime.c_str(), data->subId.c_str(), data->details.c_str(), sidId);
			//	actionLogic->ProcessEventRule(ZONE_ARMED, sidId, data->subId, ZONE, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	//if (clonedData!=NULL)
			//	//	PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ZONE, (WPARAM) clonedData, (LPARAM) ZONE_ARMED);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ZONE, (WPARAM)clonedData, (LPARAM)ZONE_ARMED);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_ZONE_DISARMED:
			//{
			//	PREPARE_DATA_INIT(ZONEData)
			//		int iDev = CatZone::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ZONE:%s] [DATETIME: %s] [:%s] [:%s] - [SIDUID:%i]",
			//		data->info.c_str(), data->datetime.c_str(), data->subId.c_str(), data->details.c_str(), sidId);
			//	actionLogic->ProcessEventRule(ZONE_DISARMED, sidId, data->subId, ZONE, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	//if (clonedData!=NULL)
			//	//	PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ZONE, (WPARAM) clonedData, (LPARAM) ZONE_DISARMED);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ZONE, (WPARAM)clonedData, (LPARAM)ZONE_DISARMED);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_ZONE_ARM_FAILED:
			//{
			//	PREPARE_DATA_INIT(ZONEData)
			//		int iDev = CatZone::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ZONE:%s] [DATETIME: %s] [:%s] [:%s] - [SIDUID:%i]",
			//		data->info.c_str(), data->datetime.c_str(), data->subId.c_str(), data->details.c_str(), sidId);
			//	actionLogic->ProcessEventRule(ZONE_ARMFAILED, sidId, data->subId, ZONE, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	//if (clonedData!=NULL)
			//	//	PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ZONE, (WPARAM) clonedData, (LPARAM) ZONE_ARMFAILED);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ZONE, (WPARAM)clonedData, (LPARAM)ZONE_ARMFAILED);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_ZONE_DISARM_FAILED:
			//{
			//	PREPARE_DATA_INIT(ZONEData)
			//		int iDev = CatZone::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ZONE:%s] [DATETIME: %s] [:%s] [:%s] - [SIDUID:%i]",
			//		data->info.c_str(), data->datetime.c_str(), data->subId.c_str(), data->details.c_str(), sidId);
			//	actionLogic->ProcessEventRule(ZONE_DISARMFAILED, sidId, data->subId, ZONE, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	//if (clonedData!=NULL)
			//	//	PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ZONE, (WPARAM) clonedData, (LPARAM) ZONE_DISARMFAILED);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ZONE, (WPARAM)clonedData, (LPARAM)ZONE_DISARMFAILED);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_ZONE_DISARM_NO_RIGHT:
			//{
			//	PREPARE_DATA_INIT(ZONEData)
			//		int iDev = CatZone::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ZONE:%s] [DATETIME: %s] [:%s] [:%s] - [SIDUID:%i]",
			//		data->info.c_str(), data->datetime.c_str(), data->subId.c_str(), data->details.c_str(), sidId);
			//	actionLogic->ProcessEventRule(ZONE_DISARMNORIGHT, sidId, data->subId, ZONE, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	//if (clonedData!=NULL)
			//	//	PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ZONE, (WPARAM) clonedData, (LPARAM) ZONE_DISARMNORIGHT);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ZONE, (WPARAM)clonedData, (LPARAM)ZONE_DISARMNORIGHT);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;
			////---------------------------------------
			//// STOP MANAGING ZONE EVENTS
			////---------------------------------------
			//// OSC 01/07/2009 Fine

			////---------------------------------------
			//// START MANAGING DIGITAL INPUTS EVENTS
			////---------------------------------------
			//case EVT_DI_ON:
			//{
			//	PREPARE_DATA_INIT(DIData)
			//		int iDev = CatDI::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DI] DIGITAL INPUT ON [%s] [DATETIME: %s] [STATUS:%s] - [SIDUID:%i]",
			//		data->details.c_str(), data->datetime.c_str(), data->info.c_str(), sidId);
			//	actionLogic->ProcessEventRule(DI_ON, sidId, data->subId, DI, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DI, (WPARAM)clonedData, (LPARAM)DI_ON);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_DI_OFF:
			//{
			//	PREPARE_DATA_INIT(DIData)
			//		int iDev = CatDI::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DI] DIGITAL INPUT OFF [%s] [DATETIME: %s] [STATUS:%s] - [SIDUID:%i]",
			//		data->details.c_str(), data->datetime.c_str(), data->info.c_str(), sidId);
			//	actionLogic->ProcessEventRule(DI_OFF, sidId, data->subId, DI, data, data->datetime);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DI, (WPARAM)clonedData, (LPARAM)DI_OFF);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_DI_SHORT:
			//{
			//	PREPARE_DATA_INIT(DIData)
			//		int iDev = CatDI::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DI] DIGITAL INPUT SHORT [%s] [DATETIME: %s] [STATUS:%s] - [SIDUID:%i]",
			//		data->details.c_str(), data->datetime.c_str(), data->info.c_str(), sidId);
			//	actionLogic->ProcessEventRule(DI_SHORT, sidId, data->subId, DI, data, data->datetime);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DI, (WPARAM)clonedData, (LPARAM)DI_SHORT);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_DI_CUT:
			//{
			//	PREPARE_DATA_INIT(DIData)
			//		int iDev = CatDI::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DI] DIGITAL INPUT CUT [%s] [DATETIME: %s] [STATUS:%s] - [SIDUID:%i]",
			//		data->details.c_str(), data->datetime.c_str(), data->info.c_str(), sidId);
			//	actionLogic->ProcessEventRule(DI_CUT, sidId, data->subId, DI, data, data->datetime);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DI, (WPARAM)clonedData, (LPARAM)DI_CUT);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_DI_ENABLED:
			//{
			//	PREPARE_DATA_INIT(DIData)
			//		int iDev = CatDI::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DI] DIGITAL INPUT ENABLED [%s] [DATETIME: %s] [STATUS:%s] - [SIDUID:%i]",
			//		data->details.c_str(), data->datetime.c_str(), data->info.c_str(), sidId);
			//	actionLogic->ProcessEventRule(DI_ENABLED, sidId, data->subId, DI, data, data->datetime); //UPDATE DB... e azioni....
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DI, (WPARAM)clonedData, (LPARAM)DI_ENABLED);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_DI_DISABLED:
			//{
			//	PREPARE_DATA_INIT(DIData)
			//		int iDev = CatDI::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DI] DIGITAL INPUT DISABLED [%s] [DATETIME: %s] [STATUS:%s] - [SIDUID:%i]",
			//		data->details.c_str(), data->datetime.c_str(), data->info.c_str(), sidId);
			//	actionLogic->ProcessEventRule(DI_DISABLED, sidId, data->subId, DI, data, data->datetime);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DI, (WPARAM)clonedData, (LPARAM)DI_DISABLED);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;
			////---------------------------------------
			//// STOP MANAGING DIGITAL INPUTS EVENTS
			////---------------------------------------

			////--------------------------------------------------------
			////  START MANAGING RESPONSES FOR DIGITAL INPUT COMMANDS
			////--------------------------------------------------------
			//case RSP_DI_ENABLE_OK:
			//case RSP_DI_ENABLE_KO:
			//{
			//	//PayloadRespBase *data = (PayloadRespBase *) msg.wParam;
			//	PREPARE_DATA(PayloadRespBase);
			//	int iDev = CatDI::getInstance()->getIdDevice(data->SIDUID, data->id);

			//	if (message == RSP_DI_ENABLE_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DI] [CMD: %i] ENABLE DI OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DI] [CMD: %i] ENABLE DI KO", data->idCommand);

			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, DI, message, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);
			//	delete data;

			//	SaraDevices_Table* clonedData = DBManager::getInstance()->getDevice(iDev);
			//	//if(clonedData != NULL && !clonedData->initEvent)
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DI, (WPARAM)clonedData, (LPARAM)DI_ENABLED);
			//}
			//break;

			//case RSP_DI_DISABLE_OK:
			//case RSP_DI_DISABLE_KO:
			//{
			//	//PayloadRespBase *data = (PayloadRespBase *) msg.wParam;
			//	PREPARE_DATA(PayloadRespBase);
			//	int iDev = CatDI::getInstance()->getIdDevice(data->SIDUID, data->id);

			//	if (message == RSP_DI_DISABLE_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DI] [CMD: %i] DISABLE DI OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DI] [CMD: %i] DISABLE DI KO", data->idCommand);

			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, DI, message, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);
			//	delete data;

			//	SaraDevices_Table* clonedData = DBManager::getInstance()->getDevice(iDev);
			//	//if(clonedData != NULL && !clonedData->initEvent)
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DI, (WPARAM)clonedData, (LPARAM)DI_DISABLED);
			//}
			//break;

			////---------------------------------------
			//// START MANAGING DIGITAL OUTPUTS EVENTS
			////---------------------------------------
			//case EVT_DO_ON:
			//{
			//	PREPARE_DATA(DOData)
			//		int iDev = CatDO::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DO] DIGITAL OUTPUT ON [%s] [DATETIME: %s] [STATUS:%s] - [SIDUID:%i]",
			//		data->details.c_str(), data->datetime.c_str(), data->info.c_str(), sidId);
			//	actionLogic->ProcessEventRule(DOON, sidId, data->subId, DO, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DO, (WPARAM)clonedData, (LPARAM)DOON);
			//}
			//break;

			//case EVT_DO_OFF:
			//{
			//	PREPARE_DATA(DOData)
			//		int iDev = CatDO::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DO] DIGITAL OUTPUT OFF [%s] [DATETIME: %s] [STATUS:%s] - [SIDUID:%i]",
			//		data->details.c_str(), data->datetime.c_str(), data->info.c_str(), sidId);
			//	actionLogic->ProcessEventRule(DOOFF, sidId, data->subId, DO, data, data->datetime);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DO, (WPARAM)clonedData, (LPARAM)DOOFF);
			//}
			//break;

			//case EVT_DO_ENABLED:
			//{
			//	PREPARE_DATA(DOData)
			//		int iDev = CatDO::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DO] DIGITAL OUTPUT ENABLED [%s] [DATETIME: %s] [STATUS:%s] - [SIDUID:%i]",
			//		data->details.c_str(), data->datetime.c_str(), data->info.c_str(), sidId);
			//	actionLogic->ProcessEventRule(DOENABLED, sidId, data->subId, DO, data, data->datetime); //UPDATE DB... e azioni....
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DO, (WPARAM)clonedData, (LPARAM)DOENABLED);
			//}
			//break;

			//case EVT_DO_DISABLED:
			//{
			//	PREPARE_DATA(DOData)
			//		int iDev = CatDO::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DO] DIGITAL OUTPUT DISABLED [%s] [DATETIME: %s] [STATUS:%s] - [SIDUID:%i]",
			//		data->details.c_str(), data->datetime.c_str(), data->info.c_str(), sidId);
			//	actionLogic->ProcessEventRule(DODISABLED, sidId, data->subId, DO, data, data->datetime);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DO, (WPARAM)clonedData, (LPARAM)DODISABLED);
			//}
			//break;
			////---------------------------------------
			//// STOP MANAGING DIGITAL OUTPUTS EVENTS
			////---------------------------------------

			////--------------------------------------------------------
			////  START MANAGING RESPONSES FOR DIGITAL OUTPUT COMMANDS
			////--------------------------------------------------------
			//case RSP_DO_ON_OK:
			//case RSP_DO_ON_KO:
			//{
			//	//PayloadRespBase *data = (PayloadRespBase *) msg.wParam;
			//	PREPARE_DATA(PayloadRespBase);
			//	int iDev = CatDO::getInstance()->getIdDevice(data->SIDUID, data->id);

			//	if (message == RSP_DO_ON_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DO] [CMD: %i] DO ON OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DO] [CMD: %i] DO ON KO", data->idCommand);

			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, DO, message, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);
			//	delete data;

			//	SaraDevices_Table* clonedData = DBManager::getInstance()->getDevice(iDev);
			//	//if(clonedData != NULL && !clonedData->initEvent)
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DO, (WPARAM)clonedData, (LPARAM)DOON);
			//}
			//break;

			//case RSP_DO_OFF_KO:
			//case RSP_DO_OFF_OK:
			//{
			//	//PayloadRespBase *data = (PayloadRespBase *) msg.wParam;
			//	PREPARE_DATA(PayloadRespBase);
			//	int iDev = CatDO::getInstance()->getIdDevice(data->SIDUID, data->id);

			//	if (message == RSP_DO_OFF_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DO] [CMD: %i] DO OFF OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DO] [CMD: %i] DO OFF KO", data->idCommand);

			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, DO, message, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);
			//	delete data;

			//	SaraDevices_Table* clonedData = DBManager::getInstance()->getDevice(iDev);
			//	//if(clonedData != NULL && !clonedData->initEvent)
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DO, (WPARAM)clonedData, (LPARAM)DOOFF);
			//}
			//break;

			//case RSP_DO_ENABLE_OK:
			//case RSP_DO_ENABLE_KO:
			//{
			//	//PayloadRespBase *data = (PayloadRespBase *) msg.wParam;
			//	PREPARE_DATA(PayloadRespBase);
			//	int iDev = CatDO::getInstance()->getIdDevice(data->SIDUID, data->id);

			//	if (message == RSP_DO_ENABLE_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DO] [CMD: %i] ENABLE DO OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DO] [CMD: %i] ENABLE DO KO", data->idCommand);

			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, DO, message, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);
			//	delete data;

			//	SaraDevices_Table* clonedData = DBManager::getInstance()->getDevice(iDev);
			//	//if(clonedData != NULL && !clonedData->initEvent)
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DO, (WPARAM)clonedData, (LPARAM)DOENABLED);
			//}
			//break;

			//case RSP_DO_DISABLE_OK:
			//case RSP_DO_DISABLE_KO:
			//{
			//	//PayloadRespBase *data = (PayloadRespBase *) msg.wParam;
			//	PREPARE_DATA(PayloadRespBase);
			//	int iDev = CatDO::getInstance()->getIdDevice(data->SIDUID, data->id);

			//	if (message == RSP_DO_DISABLE_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DO] [CMD: %i] DISABLE DO OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [DO] [CMD: %i] DISABLE DO KO", data->idCommand);

			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, DO, message, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);
			//	delete data;

			//	SaraDevices_Table* clonedData = DBManager::getInstance()->getDevice(iDev);
			//	//if(clonedData != NULL && !clonedData->initEvent)
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_DO, (WPARAM)clonedData, (LPARAM)DODISABLED);
			//}
			//break;
			////--------------------------------------------------------
			////  END MANAGING RESPONSES FOR DIGITAL OUTPUT COMMANDS
			////--------------------------------------------------------

			////--------------------------------------------------------
			////  START MANAGING RESPONSES FOR ZONE COMMANDS
			////--------------------------------------------------------
			//case RSP_ZONE_ARMED_OK:
			//case RSP_ZONE_ARMED_KO:
			//{
			//	//PayloadRespBase *data = (PayloadRespBase *) msg.wParam;
			//	PREPARE_DATA(PayloadRespBase);
			//	int iDev = CatZone::getInstance()->getIdDevice(data->SIDUID, data->id);

			//	if (message == RSP_ZONE_ARMED_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ZONE] [CMD: %i] ZONE ARMED OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ZONE] [CMD: %i] ZONE ARMED KO", data->idCommand);

			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, ZONE, message, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);
			//	delete data;

			//	SaraDevices_Table* clonedData = DBManager::getInstance()->getDevice(iDev);
			//	//if(clonedData != NULL && !clonedData->initEvent)
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ZONE, (WPARAM)clonedData, (LPARAM)ZONE_ARMED);
			//}
			//break;

			//case RSP_ZONE_DISARMED_OK:
			//case RSP_ZONE_DISARMED_KO:
			//{
			//	//PayloadRespBase *data = (PayloadRespBase *) msg.wParam;
			//	PREPARE_DATA(PayloadRespBase);
			//	int iDev = CatZone::getInstance()->getIdDevice(data->SIDUID, data->id);

			//	if (message == RSP_ZONE_DISARMED_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ZONE] [CMD: %i] ZONE DISARMED OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ZONE] [CMD: %i] ZONE DISARMED KO", data->idCommand);

			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, ZONE, message, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);
			//	delete data;

			//	SaraDevices_Table* clonedData = DBManager::getInstance()->getDevice(iDev);
			//	//if(clonedData != NULL && !clonedData->initEvent)
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ZONE, (WPARAM)clonedData, (LPARAM)ZONE_DISARMED);
			//}
			//break;
			////--------------------------------------------------------
			////  END MANAGING RESPONSES FOR ZONE COMMANDS
			////--------------------------------------------------------

			////-------------------------------------------------------
			////  START MANAGING RESPONSES FOR ALARM COMMANDS
			////-------------------------------------------------------
			//case RSP_ALMMGR_SET_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR] [CMD: %i] SETALARM OK", data->idCommand);

			//	//28 09 2007
			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, ALMMGR, RSP_ALMMGR_SET_OK, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_ALMMGR_SET_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR] [CMD: %i] SETALARM KO", data->idCommand);

			//	//28 09 2007
			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, ALMMGR, RSP_ALMMGR_SET_KO, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_ALMMGR_RESET_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR] [CMD: %i] RESET ALARM OK", data->idCommand);

			//	//28 09 2007
			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, ALMMGR, RSP_ALMMGR_RESET_OK, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_ALMMGR_RESET_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR] [CMD: %i] RESET ALARM KO", data->idCommand);

			//	//28 09 2007
			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, ALMMGR, RSP_ALMMGR_RESET_KO, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_ALMMGR_ENABLE_OK:
			//case RSP_ALMMGR_ENABLE_KO:
			//{
			//	//PayloadRespBase *data = (PayloadRespBase *) msg.wParam;
			//	PREPARE_DATA(PayloadRespBase);
			//	int iDev = CatAlmMgr::getInstance()->getIdDevice(data->SIDUID, data->id);

			//	if (message == RSP_ALMMGR_ENABLE_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR] [CMD: %i] ENABLE ALARM OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR] [CMD: %i] ENABLE ALARM KO", data->idCommand);

			//	//28 09 2007
			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, ALMMGR, message, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	SaraDevices_Table* clonedData = DBManager::getInstance()->getDevice(iDev);
			//	//if(clonedData != NULL && !clonedData->initEvent)
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ALARM, (WPARAM)clonedData, (LPARAM)ALARM_ENABLED);
			//}
			//break;

			//case RSP_ALMMGR_DISABLE_OK:
			//case RSP_ALMMGR_DISABLE_KO:
			//{
			//	//PayloadRespBase *data = (PayloadRespBase *) msg.wParam;
			//	PREPARE_DATA(PayloadRespBase);
			//	int iDev = CatAlmMgr::getInstance()->getIdDevice(data->SIDUID, data->id);

			//	if (message == RSP_ALMMGR_DISABLE_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR] [CMD: %i] DISABLE ALARM OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR] [CMD: %i] DISABLE ALARM KO", data->idCommand);

			//	//28 09 2007
			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, ALMMGR, message, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	SaraDevices_Table* clonedData = DBManager::getInstance()->getDevice(iDev);
			//	//if(clonedData != NULL && !clonedData->initEvent)
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ALARM, (WPARAM)clonedData, (LPARAM)ALARM_DISABLED);
			//}
			//break;

			//case RSP_ALMMGR_ACK_OK:
			//case RSP_ALMMGR_ACK_KO:
			//{
			//	//PayloadRespBase *data = (PayloadRespBase *) msg.wParam;
			//	PREPARE_DATA(PayloadRespBase);
			//	int iDev = CatAlmMgr::getInstance()->getIdDevice(data->SIDUID, data->id);

			//	if (message == RSP_ALMMGR_ACK_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR] [CMD: %i] ACK ALARM OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR] [CMD: %i] ACK ALARM KO", data->idCommand);

			//	//28 09 2007
			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, ALMMGR, message, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	SaraDevices_Table* clonedData = DBManager::getInstance()->getDevice(iDev);
			//	//if(clonedData != NULL && !clonedData->initEvent)
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ALARM, (WPARAM)clonedData, (LPARAM)ALARM_ACKED);
			//}
			//break;

			////
			//case RSP_ALMMGR_PURGE_OK:
			//case RSP_ALMMGR_PURGE_KO:
			//{
			//	//PayloadRespBase *data = (PayloadRespBase *) msg.wParam;
			//	PREPARE_DATA(PayloadRespBase);
			//	int iDev = CatAlmMgr::getInstance()->getIdDevice(data->SIDUID, data->id);

			//	if (message == RSP_ALMMGR_PURGE_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR] [CMD: %i] PURGE ALARM OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR] [CMD: %i] PURGE ALARM KO", data->idCommand);

			//	//28 09 2007
			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, ALMMGR, message, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	SaraDevices_Table* clonedData = DBManager::getInstance()->getDevice(iDev);
			//	//if(clonedData != NULL && !clonedData->initEvent)
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ALARM, (WPARAM)clonedData, (LPARAM)ALARM_PURGED);
			//}
			//break;

			////
			//case RSP_ALMMGR_DELETE_OK:
			//case RSP_ALMMGR_DELETE_KO:
			//{
			//	//PayloadRespBase *data = (PayloadRespBase *) msg.wParam;
			//	PREPARE_DATA(PayloadRespBase);
			//	int iDev = CatAlmMgr::getInstance()->getIdDevice(data->SIDUID, data->id);

			//	if (message == RSP_ALMMGR_DELETE_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR] [CMD: %i] DELETE ALARM OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [ALMMGR] [CMD: %i] DELETE ALARM KO", data->idCommand);

			//	//28 09 2007
			//	actionLogic->ProcessActionResp(data->SIDUID, data->id, ALMMGR, message, data->sSourceTS);

			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	SaraDevices_Table* clonedData = DBManager::getInstance()->getDevice(iDev);
			//	//if(clonedData != NULL && !clonedData->initEvent)
			//	if (clonedData != NULL)
			//		PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_ALARM, (WPARAM)clonedData, (LPARAM)ALARM_DELETED);
			//}
			//break;


			////-------------------------------------------------------
			////  STOP MANAGING RESPONSES FOR ALARM COMMANDS
			////-------------------------------------------------------

			////------------------------------------------
			////INIZIO GESTIONE RISPOSTE A COMANDI PTZ
			////------------------------------------------
			//case RSP_PTZ_SETMOVESPEED_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] SETMOVESPEED OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_SETMOVESPEED_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] SETMOVESPEED KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVEUP_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVEUP OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVEUP_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVEUP KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVEDOWN_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVEDOWN OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVEDOWN_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVEDOWN KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVELEFT_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVELEFT OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVELEFT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVELEFT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVERIGHT_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVERIGHT OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVERIGHT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVERIGHT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVEUPRIGHT_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVEUPRIGHT OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVEUPRIGHT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVEUPRIGHT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVEUPLEFT_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVEUPLEFT OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVEUPLEFT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVEUPLEFT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVEDOWNRIGHT_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVEDOWNRIGHT OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVEDOWNRIGHT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVEDOWNRIGHT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVEDOWNLEFT_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVEDOWNLEFT OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_MOVEDOWNLEFT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] MOVEDOWNLEFT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_STOP_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] STOP OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_STOP_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] STOP KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_GOTOPRESET_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] GOTOPRESET OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_GOTOPRESET_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] GOTOPRESET KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_SETPRESET_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] SETPRESET OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_SETPRESET_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] SETPRESET KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_LOADPRESETTABLE_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] LOADPRESETTABLE OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_LOADPRESETTABLE_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] LOADPRESETTABLE KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_GOTOABSPOSITION_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] GOTOABSPOSITION OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_GOTOABSPOSITION_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] GOTOABSPOSITION KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_CONNECT_OK:
			//case RSP_PTZ_CONNECT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	if (message == RSP_PTZ_CONNECT_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] CONNECT OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] CONNECT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_PTZ_DISCONNECT_OK:
			//case RSP_PTZ_DISCONNECT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	if (message == RSP_PTZ_DISCONNECT_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] DISCONNECT OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [PTZ] [CMD: %i] DISCONNECT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			////-----------------------------------------
			////FINE GESTIONE RISPOSTE A COMANDI PTZ
			////-----------------------------------------

			////------------------------------------------
			////INIZIO GESTIONE RISPOSTE A COMANDI CAMERA
			////------------------------------------------
			//case RSP_CAMERA_SETMOVESPEED_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] SETMOVESPEED OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_SETMOVESPEED_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] SETMOVESPEED KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVEUP_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVEUP OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVEUP_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVEUP KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVEDOWN_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVEDOWN OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVEDOWN_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVEDOWN KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVELEFT_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVELEFT OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVELEFT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVELEFT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVERIGHT_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVERIGHT OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVERIGHT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVERIGHT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVEUPRIGHT_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVEUPRIGHT OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVEUPRIGHT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVEUPRIGHT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVEUPLEFT_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVEUPLEFT OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVEUPLEFT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVEUPLEFT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVEDOWNRIGHT_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVEDOWNRIGHT OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVEDOWNRIGHT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVEDOWNRIGHT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVEDOWNLEFT_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVEDOWNLEFT OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_MOVEDOWNLEFT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] MOVEDOWNLEFT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_STOP_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] STOP OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_STOP_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] STOP KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_GOTOPRESET_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] GOTOPRESET OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_GOTOPRESET_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] GOTOPRESET KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_SETPRESET_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] SETPRESET OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_SETPRESET_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] SETPRESET KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_LOADPRESETTABLE_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] LOADPRESETTABLE OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_LOADPRESETTABLE_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] LOADPRESETTABLE KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_GOTOABSPOSITION_OK:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] GOTOABSPOSITION OK", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_GOTOABSPOSITION_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] GOTOABSPOSITION KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_CONNECT_OK:
			//case RSP_CAMERA_CONNECT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	if (message == RSP_CAMERA_CONNECT_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] CONNECT OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] CONNECT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case RSP_CAMERA_DISCONNECT_OK:
			//case RSP_CAMERA_DISCONNECT_KO:
			//{
			//	PayloadRespBase* data = (PayloadRespBase*)msg.wParam;
			//	if (message == RSP_CAMERA_DISCONNECT_OK)
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] DISCONNECT OK", data->idCommand);
			//	else
			//		sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] [CMD: %i] DISCONNECT KO", data->idCommand);
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM);
			//	delete data;
			//}
			//break;

			//case EVT_CAMERA_ENABLED:
			//case EVT_CAMERA_DISABLED:

			//	break;

			//case EVT_CAMERA_VLOSS:
			//{
			//	PREPARE_DATA_INIT(CAMData)
			//		int iDev = CatCAMERA::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] VIDEO LOSS [%s] [DATETIME: %s] [STATUS:%s] - [SIDUID:%i]",
			//		data->details.c_str(), data->datetime.c_str(), data->info.c_str(), sidId);
			//	actionLogic->ProcessEventRule(CAMERA_VLOSS, sidId, data->subId, CAMERA, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Per ora non deve produrre evento di PUSH verso il WebMonitor
			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	//if (clonedData!=NULL)
			//	//	PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_CAMERA, (WPARAM) clonedData, (LPARAM) CAMERA_VLOSS);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_CAMERA, (WPARAM)clonedData, (LPARAM)CAMERA_VLOSS);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_CAMERA_READY:
			//{
			//	PREPARE_DATA_INIT(CAMData)
			//		int iDev = CatCAMERA::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CAMERA] VIDEO READY [%s] [DATETIME: %s] [STATUS:%s] - [SIDUID:%i]",
			//		data->details.c_str(), data->datetime.c_str(), data->info.c_str(), sidId);
			//	actionLogic->ProcessEventRule(CAMERA_READY, sidId, data->subId, CAMERA, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Per ora non deve produrre evento di PUSH verso il WebMonitor
			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	//if (clonedData!=NULL)
			//	//	PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_CAMERA, (WPARAM) clonedData, (LPARAM) CAMERA_READY);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_CAMERA, (WPARAM)clonedData, (LPARAM)CAMERA_READY);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			////-----------------------------------------
			////FINE GESTIONE RISPOSTE A COMANDI CAMERA
			////-----------------------------------------

			////-----------------------------------------
			////INIZIO GESTIONE RISPOSTE A COMANDI CDC
			////-----------------------------------------

			//case EVT_CDC_LINKOFF:
			//{
			//	PREPARE_DATA_INIT(CDCData)
			//		int iDev = CatCDC::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CDC] OFFLINE [%s] [DATETIME: %s] [STATUS:%s] - [SIDUID:%i]",
			//		data->details.c_str(), data->datetime.c_str(), data->info.c_str(), sidId);
			//	actionLogic->ProcessEventRule(CDC_OFFLINE, sidId, data->subId, CDC, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Per ora non deve produrre evento di PUSH verso il WebMonitor
			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	//if (clonedData!=NULL)
			//	//	PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_CDC, (WPARAM) clonedData, (LPARAM) CDC_OFFLINE);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_CDC, (WPARAM)clonedData, (LPARAM)CDC_OFFLINE);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			//case EVT_CDC_LINKON:
			//{
			//	PREPARE_DATA_INIT(CDCData)
			//		int iDev = CatCDC::getInstance()->getIdDevice(data->SIDUID, data->subId);

			//	sprintf_s(logBuf, sizeof(logBuf), "[SRK] [CDC] ONLINE [%s] [DATETIME: %s] [STATUS:%s] - [SIDUID:%i]",
			//		data->details.c_str(), data->datetime.c_str(), data->info.c_str(), sidId);
			//	actionLogic->ProcessEventRule(CDC_ONLINE, sidId, data->subId, CDC, data, data->datetime);
			//	//delete data;
			//	sLogger->Log(logBuf, SaraLogger::DEBUG_LEVEL_MEDIUM, sidId);

			//	// Per ora non deve produrre evento di PUSH verso il WebMonitor
			//	// Se il device non è presente nel DB, evidenziare differenze in 
			//	// configurazione e scartare evento
			//	SaraDevices_Table* clonedData = CheckDevice(iDev, logBuf);
			//	//if (clonedData!=NULL)
			//	//	PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_CDC, (WPARAM) clonedData, (LPARAM) CDC_ONLINE);
			//	if (clonedData != NULL)
			//	{
			//		if (!initEvent)
			//			PostThreadMessage(ThCommAdapterMonitor::getInstance()->GetThreadId(), EVENT_PUSH_CDC, (WPARAM)clonedData, (LPARAM)CDC_ONLINE);
			//		else
			//			delete clonedData;
			//	}
			//}
			//break;

			////-----------------------------------------
			////FINE GESTIONE RISPOSTE A COMANDI CDC
			////-----------------------------------------

			////-------------------------------------------------------
			////   LOOGING SYSTEM MESSAGES FROM SID
			////-------------------------------------------------------
			//case MSG_TO_LOG:
			//{
			//	SystemMessage* sm = (SystemMessage*)msg.wParam;
			//	sLogger->Log(sm);
			//}
			//break;

			////-------------------------------------------------------
			////   LOOGING ERROR MESSAGES FROM SID
			////-------------------------------------------------------
			//case ERR_TO_LOG: /** GUG20070615 */
			//{
			//	SystemError* se = (SystemError*)msg.wParam;
			//	sLogger->Log(se);
			//}
			//break;

			//default:
			//{
			//	TranslateMessage(&msg);  //Dispatch dei messaggi
			//	DispatchMessage(&msg);
			//}
		   //}
		}



		return 0;
	}

	

}