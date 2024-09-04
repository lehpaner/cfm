/****************************** Module Header ******************************\
* Module Name:  AEngine.cpp
* Project:      S000
*
*
* Console Engine class that is base class for different types of engine
*
\***************************************************************************/
#pragma once

#include "BaseEngine.h"
#include "StateMachine.h"

namespace cfm::application {
    class CConsoleEngine : public CStateMachine <CConsoleEngine>, public CBaseEngine {
    private:
        static CConsoleEngine* smInstance;

        /*! \enum MAINCMODE_STATES
         *  Elenco degli stati assunti dall'automa.
         */
        typedef enum {
            IDLE,
            WAIT_DB,
            WAIT_SIDMON,
            WAIT_SIDMON_OFF,

            MAINCMODE_STATES_NO
        } MAINCMODE_STATES;

        /* State Machine Actions: BEGIN ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
        void idleStart();
        void waitDbDbReady();
        void waitDbDbNotReady();
        void waitSidMonReady();
        void waitSidMonNotReady();
        void waitSidMonOffSidmonOff();
        /* State Machine Actions: END ------------------------------------------------------------ */

        CConsoleEngine(int id); // Costruttore

    public:
        static CConsoleEngine* getInstance(int id);

        ~CConsoleEngine();

        void executeStateMachine(unsigned int ev) {
            execute(ev);
        }
    };

} //end namespace