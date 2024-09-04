/****************************** Module Header ******************************\
* Module Name:  ConsoleEngine.cpp
* Project:      S000
*
*
* Base Engine class that is base class for different types of engine
*
\***************************************************************************/
#include "ConsoleEngine.h"
#include "Messages.h"
cfm::application::CConsoleEngine* cfm::application::CConsoleEngine::smInstance = NULL;
namespace cfm::application {

    CConsoleEngine::CConsoleEngine(int id) : CBaseEngine(id) {
        addEvent(IDLE, MAIN_START, &CConsoleEngine::idleStart);

        addEvent(WAIT_DB, DB_READY, &CConsoleEngine::waitDbDbReady);
        addEvent(WAIT_DB, DB_NOT_READY, &CConsoleEngine::waitDbDbNotReady);

        addEvent(WAIT_SIDMON, SIDMON_READY, &CConsoleEngine::waitSidMonReady);
        addEvent(WAIT_SIDMON, SIDMON_NOT_READY, &CConsoleEngine::waitSidMonNotReady);

        addEvent(WAIT_SIDMON_OFF, SIDMON_OFF, &CConsoleEngine::waitSidMonOffSidmonOff);
    }

    CConsoleEngine::~CConsoleEngine() { }

    /* State Machine Actions: BEGIN ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

    /*! \brief State: IDLE \n
    * Event: MAIN_START
    */
    void CConsoleEngine::idleStart() {
        printConf(opConfig);
        startDB();
        state = WAIT_DB;
    }

    /*! \brief State: WAIT_DB \n
     * Event: DB_READY
     */
    void CConsoleEngine::waitDbDbReady() {
        startOther(opConfig);
        pSidMon = CDeviceMonitor::getInstance(regionId);
        state = WAIT_SIDMON;
    }

    /*! \brief State: WAIT_DB \n
     * Event: DB_NOT_READY
     */
    void CConsoleEngine::waitDbDbNotReady() {
        state = WAIT_DB;
    }

    /*! \brief State: WAIT_SIDMON \n
     * Event: SIDMON_READY
     */
    void CConsoleEngine::waitSidMonReady(){
        pSidMon->req_close_all();
        state = WAIT_SIDMON_OFF;
    }

    /*! \brief State: WAIT_SIDMON \n
     * Event: SIDMON_NOT_READY
     */
    void CConsoleEngine::waitSidMonNotReady() {
        pSidMon->req_close_all();
        state = WAIT_SIDMON_OFF;
    }

    /*! \brief State: WAIT_SIDMON_OFF \n
     * Event: SIDMON_OFF
     */
    void CConsoleEngine::waitSidMonOffSidmonOff() {
        terminate = true;
    }

    /* State Machine Actions: END ------------------------------------------------------------ */

    CConsoleEngine* CConsoleEngine::getInstance(int id) {
        if (smInstance == NULL) {
            smInstance = new CConsoleEngine(id);
        }
        return smInstance;
    }

}