/****************************** Module Header ******************************\
* Module Name:  AEngine.cpp
* Project:      S000
*
*
* User Engine class that is base class for different types of engine
*
\***************************************************************************/
#pragma once

#include "BaseEngine.h"
#include "StateMachine.h"

namespace cfm::application {
    using namespace cfm::application::domain;
    class CUserEngine : public CStateMachine <CUserEngine>, public CBaseEngine
    {
    private:
        static CUserEngine* smInstance;

        /*! \enum MAINUMODE_STATES
         *  Elenco degli stati assunti dall'automa.
         */
        typedef enum
        {
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

        CUserEngine(int id); // Costruttore

    public:
        static CUserEngine* getInstance(int id);

        ~CUserEngine();

        void executeStateMachine(unsigned int ev)
        {
            execute(ev);
        }
    };

} //end namespace