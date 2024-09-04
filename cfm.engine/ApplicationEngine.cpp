/****************************** Module Header ******************************\
* Module Name:  AEngine.cpp
* Project:      S000
*
*
* Base Engine class that is base class for different types of engine
*
\***************************************************************************/
#include "ApplicationEngine.h"
#include "Messages.h"

//extern  cfgParms saraRegCfg;	/**< Sara Registry Configuration */

#define BASE_TIME      1000 // Base dei tempi del timer (granularita')

#define SIDMON_TIME    1000 //(saraRegCfg.SIDMON_TIME()) // Timeout creazione SID

namespace cfm::application {
	CAEngine* CAEngine::smInstance = nullptr;

    CAEngine::CAEngine(int id) : CBaseEngine(id) {
        addEvent(IDLE, MAIN_START, &CAEngine::idleStart);

        addEvent(WAIT_DB, DB_READY, &CAEngine::waitDbDbReady);
        addEvent(WAIT_DB, DB_NOT_READY, &CAEngine::waitDbDbNotReady);
        addEvent(WAIT_DB, TERMINATE_REQ, &CAEngine::waitDbTerminateReq);

        addEvent(WAIT_SIDMON, SIDMON_READY, &CAEngine::waitSidMonReady);
        addEvent(WAIT_SIDMON, SIDMON_NOT_READY, &CAEngine::waitSidMonNotReady);
        addEvent(WAIT_SIDMON, SIDMON_TIMEOUT, &CAEngine::waitSidMonTimeout);
        addEvent(WAIT_SIDMON, TERMINATE_REQ, &CAEngine::waitSidMonTerminateReq);
        addEvent(WAIT_SIDMON, SUSPEND, &CAEngine::waitSidMonSuspend);

        addEvent(RUNNING, UPDATE_REQ, &CAEngine::runningUpdateReq);
        addEvent(RUNNING, DB_NOT_READY, &CAEngine::runningDbNotReady);
        addEvent(RUNNING, SUSPEND, &CAEngine::runningSuspend);

        addEvent(DEGRADED_UPDATE, UPDATE_DONE, &CAEngine::degradedUpdateUpdateDone);

        addEvent(DEGRADED_DB, DB_READY, &CAEngine::degradedDbDbReady);

        addEvent(WAIT_SIDMON_OFF, SIDMON_OFF, &CAEngine::waitSidMonOffSidmonOff);

        addEvent(WAIT_SIDMON_OFF2, SIDMON_OFF, &CAEngine::waitSidMonOff2SidmonOff);

        addEvent(WAIT_ALL_SIDMON_OFF, SIDMON_OFF, &CAEngine::waitAllSidMonOffSidmonOff);

        addEvent(SUSPENDED, RESUME, &CAEngine::suspendedResume);

        addEvent(TERMINATE_REQ, &CAEngine::anyTerminateReq);
        addEvent(POLLING, &CAEngine::anyPolling);
        addEvent(DB_READY, &CAEngine::anyDbReady);
        addEvent(DB_NOT_READY, &CAEngine::anyDbNotReady);
        addEvent(SUSPEND, &CAEngine::anyCommandNotSupportedInState);
        addEvent(RESUME, &CAEngine::anyCommandNotSupportedInState);
        addEvent(UPDATE_REQ, &CAEngine::anyCommandNotSupportedInState);

        timer = new TTimersThread(MAINAMODE_TIMERS_NO, BASE_TIME);
        dbReady = false;
    }

    CAEngine::~CAEngine() {
        delete timer;
    }
    /* --------------------------------------------------------------------------------------- */
    /* State Machine Actions: BEGIN ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
    /* --------------------------------------------------------------------------------------- */
    /*! \brief State: IDLE \n
 * Event: MAIN_START
 */
    void CAEngine::idleStart() {
        printf("-------STARTING APPLICATION ENGINE--------");
        printConf(opRunning);
        startDB();
        state = WAIT_DB;
    }


    /*! \brief State: WAIT_DB \n
     * Event: DB_READY
     */
    void CAEngine::waitDbDbReady() {
        dbReady = true;

        if (!startOther(opRunning))
            state = SUSPENDED;
        else {
            pSidMon = CDeviceMonitor::getInstance(regionId);

            state = WAIT_SIDMON;
        }
    }


    /*! \brief State: WAIT_DB \n
     * Event: DB_NOT_READY
     */
    void CAEngine::waitDbDbNotReady() {
    }


    /*! \brief State: WAIT_DB \n
     * Event: TERMINATE_REQ
     */
    void CAEngine::waitDbTerminateReq() {
        terminate = true;
    }


    /*! \brief State: WAIT_SIDMON \n
     * Event: SIDMON_READY
     */
    void CAEngine::waitSidMonReady() {
        state = RUNNING;
    }


    /*! \brief State: WAIT_SIDMON \n
     * Event: SIDMON_NOT_READY
     */
    void CAEngine::waitSidMonNotReady() {
        timer->setTimeout(SIDMON_TIMER, SIDMON_TIME, mainTo, SIDMON_TIMEOUT, this);
    }


    /*! \brief State: WAIT_SIDMON \n
     * Event: SIDMON_TIMEOUT
     */
    void CAEngine::waitSidMonTimeout()
    {
        pSidMon->req_start_all();
    }


    /*! \brief State: WAIT_SIDMON \n
     * Event: TERMINATE_REQ
     */
    void CAEngine::waitSidMonTerminateReq() {
        timer->resTimeout(SIDMON_TIMER);

        anyTerminateReq();
    }


    /*! \brief State: WAIT_SIDMON \n
     * Event: SUSPEND
     */
    void CAEngine::waitSidMonSuspend() {
        //Pekmez MessageSender::getInstance()->Send("nothing to suspend!\r\n", (void*)msg.wParam);
    }


    /*! \brief State: RUNNING \n
     * Event: UPDATE_REQ
     */
    void CAEngine::runningUpdateReq() {
        state = DEGRADED_UPDATE;
    }


    /*! \brief State: ANY \n
     * Event: TERMINATE_REQ
     */
    void CAEngine::anyTerminateReq() {
        if (pSidMon) {
            pSidMon->req_stop_all();

            isMySidmonOff = false;

            sendToAllOtherStateMachines(msg.message, msg.wParam, regionId);
        } else {
            isMySidmonOff = true;

            if (getInstancesSize() == 1)
                terminate = true;
        }
        state = WAIT_ALL_SIDMON_OFF;
    }


    /*! \brief State: RUNNING \n
     * Event: DB_NOT_READY
     */
    void CAEngine::runningDbNotReady() {
        sendToAllOtherStateMachines(msg.message, msg.wParam, regionId);

        dbReady = false;

        state = DEGRADED_DB;
    }


    /*! \brief State: RUNNING \n
     * Event: SUSPEND
     */
    void CAEngine::runningSuspend() {
        monitorSocket = nullptr; //Pekmez(TcpSocket*)msg.wParam;

        pSidMon->req_stop_all();

        state = WAIT_SIDMON_OFF2;
    }


    /*! \brief State: DEGRADED_UPDATE \n
     * Event: UPDATE_DONE
     */
    void CAEngine::degradedUpdateUpdateDone() {
        pSidMon->req_stop_all();

        state = WAIT_SIDMON_OFF;
    }


    /*! \brief State: DEGRADED_DB \n
     * Event: DB_READY
     */
    void CAEngine::degradedDbDbReady() {
        sendToAllOtherStateMachines(msg.message, msg.wParam, regionId);

        dbReady = true;

        state = RUNNING;
    }


    /*! \brief State: ANY \n
     * Event: DB_NOT_READY
     */
    void CAEngine::anyDbNotReady() {
        sendToAllOtherStateMachines(msg.message, msg.wParam, regionId);
        dbReady = false;
    }


    /*! \brief State: ANY \n
     * Event: DB_READY
     */
    void CAEngine::anyDbReady() {
        sendToAllOtherStateMachines(msg.message, msg.wParam, regionId);

        dbReady = true;
    }


    /*! \brief State: WAIT_SIDMON_OFF \n
     * Event: SIDMON_OFF
     */
    void CAEngine::waitSidMonOffSidmonOff() {
        if (msg.wParam) // Se e' solo l'avviso di un MainMaster
            return;

        pSidMon->req_start_all();

        state = WAIT_SIDMON;
    }


    /*! \brief State: WAIT_SIDMON_OFF2 \n
     * Event: SIDMON_OFF
     */
    void CAEngine::waitSidMonOff2SidmonOff() {
        //Pekmez MessageSender::getInstance()->Send("stopped\r\n", monitorSocket);
        state = SUSPENDED;
    }


    /*! \brief State: WAIT_ALL_SIDMON_OFF \n
     * Event: SIDMON_OFF
     */
    void CAEngine::waitAllSidMonOffSidmonOff() {
        assert(msg.wParam != regionId); // Solo i SID di MainMaster devono impostare questo parametro.

        if (msg.wParam)
            destroyInstance((int)msg.wParam);
        else
            isMySidmonOff = true;

        if (isMySidmonOff && (getInstancesSize() == 1))
            terminate = true;
    }


    /*! \brief State: SUSPENDED \n
     * Event: RESUME
     */
    void CAEngine::suspendedResume() {
        if (!pSidMon)
            pSidMon = CDeviceMonitor::getInstance(regionId);

        pSidMon->req_start_all();

        state = WAIT_SIDMON;
    }


    /*! \brief State: ANY \n
     * Event: POLLING
     */
    void CAEngine::anyPolling() {
        switch (state) {
        case SUSPENDED:
            //Pekmez MessageSender::getInstance()->Send("suspended\r\n", (void*)msg.wParam);
            break;

        case DEGRADED_DB:
        case WAIT_DB:
            //Pekmez MessageSender::getInstance()->Send("degraded db\r\n", (void*)msg.wParam);
            break;

        case DEGRADED_UPDATE:
            //Pekmez MessageSender::getInstance()->Send("degraded update\r\n", (void*)msg.wParam);
            break;

        default:
            //if (dbReady)
            //    MessageSender::getInstance()->Send("running\r\n", (void*)msg.wParam);
            //else
            //    MessageSender::getInstance()->Send("degraded db\r\n", (void*)msg.wParam);
            break;
        }
    }


    /*! \brief State: ANY \n
     * Event: SUSPEND / RESUME / UPDATE_REQ
     */
    void CAEngine::anyCommandNotSupportedInState() {
        //Pekmez MessageSender::getInstance()->Send("Command not supported in state " + getState() + ".\r\n", (void*)msg.wParam);
    }

    /* --------------------------------------------------------------------------------------- */
    /* State Machine Actions: END   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
    /* --------------------------------------------------------------------------------------- */



    CAEngine* CAEngine::getInstance(int id) {
        if (smInstance == NULL) {
            smInstance = new CAEngine(id);
        }
        return smInstance;
    }



    CBaseEngine* getAModeInstance(int id) {
        return CAEngine::getInstance(id);
    }



    /*! \brief Funzione di timeout lanciata dal timer mediante il thread timerThread
     *
     * Invia timeout a se stesso.
     *
     * \param timer indice del timer
     * \param parameter parametro intero eventuale passato
     * \param ptrParam parametro puntatore eventuale passato
     */
    void CAEngine::mainTo(unsigned int timer, unsigned int param, void* ptrParam)
    {
        CAEngine* p = (CAEngine*)ptrParam;

        PostThreadMessage(p->GetThreadId(), param, 0, p->regionId);
    }


}