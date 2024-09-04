/****************************** Module Header ******************************\
* Module Name:  UserEngine.cpp
* Project:      S000
*
*
* User Engine class that is base class for different types of engine
*
\***************************************************************************/
#include "UserEngine.h"
#include "Messages.h"
cfm::application::CUserEngine* cfm::application::CUserEngine::smInstance = NULL;
namespace cfm::application {
    
    CUserEngine::CUserEngine(int id) : CBaseEngine(id) {
        addEvent(IDLE, MAIN_START, &CUserEngine::idleStart);

        addEvent(WAIT_DB, DB_READY, &CUserEngine::waitDbDbReady);
        addEvent(WAIT_DB, DB_NOT_READY, &CUserEngine::waitDbDbNotReady);

        addEvent(WAIT_SIDMON, SIDMON_READY, &CUserEngine::waitSidMonReady);
        addEvent(WAIT_SIDMON, SIDMON_NOT_READY, &CUserEngine::waitSidMonNotReady);

        addEvent(WAIT_SIDMON_OFF, SIDMON_OFF, &CUserEngine::waitSidMonOffSidmonOff);
    }

    CUserEngine::~CUserEngine() { }



    /* State Machine Actions: BEGIN ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

    /*! \brief State: IDLE \n
     * Event: MAIN_START
     */
    void CUserEngine::idleStart() {
        printConf(opUpdate);
        startDB();
        state = WAIT_DB;
    }

    /*! \brief State: WAIT_DB \n
     * Event: DB_READY
     */
    void CUserEngine::waitDbDbReady() {
        startOther(opUpdate);
        pSidMon = CDeviceMonitor::getInstance(regionId);
        state = WAIT_SIDMON;
    }

    /*! \brief State: WAIT_DB \n
     * Event: DB_NOT_READY
     */
    void CUserEngine::waitDbDbNotReady() {
        state = WAIT_DB;
    }

    /*! \brief State: WAIT_SIDMON \n
     * Event: SIDMON_READY
     */
    void CUserEngine::waitSidMonReady() {
        pSidMon->req_close_all();
        state = WAIT_SIDMON_OFF;
    }

    /*! \brief State: WAIT_SIDMON \n
     * Event: SIDMON_NOT_READY
     */
    void CUserEngine::waitSidMonNotReady() {
        pSidMon->req_close_all();
        state = WAIT_SIDMON_OFF;
    }

    /*! \brief State: WAIT_SIDMON_OFF \n
     * Event: SIDMON_OFF
     */
    void CUserEngine::waitSidMonOffSidmonOff() {
        terminate = true;
    }

    /* State Machine Actions: END ------------------------------------------------------------ */
    CUserEngine* CUserEngine::getInstance(int id) {
        if (smInstance == nullptr) {
            smInstance = new CUserEngine(id);
        }
        return smInstance;
    }
}