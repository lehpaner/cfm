/****************************** Module Header ******************************\
* Module Name:  AEngine.cpp
* Project:      S000
*
*
* Base Engine class that is base class for different types of engine
*
\***************************************************************************/
#pragma once
#include "BaseEngine.h"
#include "StateMachine.h"
#include "Timers.h"


namespace cfm::application {
    class CAEngine : public CStateMachine <CAEngine>, public CBaseEngine {
    private:
        static CAEngine* smInstance;

        TTimersThread* timer;
        bool isMySidmonOff;
        bool dbReady;

        /*! \enum MAINAMODE_STATES
         *  Elenco degli stati assunti dall'automa.
         */
        typedef enum {
            IDLE,
            WAIT_DB,
            WAIT_SIDMON,
            RUNNING,
            DEGRADED_DB,
            DEGRADED_UPDATE,
            WAIT_SIDMON_OFF,
            WAIT_SIDMON_OFF2,
            WAIT_ALL_SIDMON_OFF,
            SUSPENDED,

            MAINAMODE_STATES_NO
        } MAINAMODE_STATES;

        /*! \enum MAINAMODE_TIMERS
         *  Elenco dei timers utilizzati dal processo.
         */
        typedef enum {
            SIDMON_TIMER = 0,

            MAINAMODE_TIMERS_NO
        } MAINAMODE_TIMERS;

        /* State Machine Actions: BEGIN ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
        void idleStart();
        void waitDbDbReady();
        void waitDbDbNotReady();
        void waitDbTerminateReq();
        void waitSidMonReady();
        void waitSidMonNotReady();
        void waitSidMonTimeout();
        void waitSidMonTerminateReq();
        void waitSidMonSuspend();
        void runningUpdateReq();
        void runningDbNotReady();
        void runningSuspend();
        void degradedUpdateUpdateDone();
        void degradedDbDbReady();
        void waitSidMonOffSidmonOff();
        void waitSidMonOff2SidmonOff();
        void anyTerminateReq();
        void anyDbReady();
        void anyDbNotReady();
        void anyPolling();
        void waitAllSidMonOffSidmonOff();
        void suspendedResume();
        void anyCommandNotSupportedInState();
        /* State Machine Actions: END ------------------------------------------------------------ */

        CAEngine(int id); // Costruttore

        static void mainTo(unsigned int timer, unsigned int param, void* ptrParam);

    public:
        static CAEngine* getInstance(int id);
        std::string getState() {
            switch (state) {
            case IDLE:
                return "IDLE";
            case WAIT_DB:
                return "WAIT_DB";
            case WAIT_SIDMON:
                return "WAIT_SIDMON";
            case RUNNING:
                return "RUNNING";
            case DEGRADED_DB:
                return "DEGRADED_DB";
            case DEGRADED_UPDATE:
                return "DEGRADED_UPDATE";
            case WAIT_SIDMON_OFF:
                return "WAIT_SIDMON_OFF";
            case WAIT_SIDMON_OFF2:
                return "WAIT_SIDMON_OFF2";
            case WAIT_ALL_SIDMON_OFF:
                return "WAIT_ALL_SIDMON_OFF";
            case SUSPENDED:
                return "SUSPENDED";

            default:
                return "";
            }
        }

        ~CAEngine();

        void executeStateMachine(unsigned int ev) {
            printf("APPLICATION ENGINE BEFORE MESSAGE EXECUTE STATE-%s \n", getState().c_str());
            execute(ev);
            printf("APPLICATION ENGINE AFTER MESSAGE EXECUTE STATE-%s \n", getState().c_str());
        }
    };

    //CBaseEngine* getAModeInstance(int id);
} //end namespace
