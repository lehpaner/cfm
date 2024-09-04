/****************************** Module Header ******************************\
* Module Name:  StateMachine.h
* Project:      S000
*
*
* State machine base class to be extended for implementation
*
\***************************************************************************/
#include<cassert>
#pragma once
// macro event distinguishing two levels of triggering
#define MACRO_EVENT(w1,w2) ((unsigned int) ((unsigned int) w1 << (sizeof(unsigned int) * 4) | (unsigned int) w2))

namespace cfm::application {
    template <class cInstance>
    class TCallback {
    private:
        typedef void (cInstance::* tAction)();

    public:
        tAction pAction;
        //Construct
        TCallback(tAction pActionPointer) : pAction(pActionPointer){
        }
        void execute(cInstance* cInst) const {
            (cInst->*pAction)();
        }
    };



    template <class TStateMachine>
    class CStateMachine {
    private:
        typedef unsigned int EVENT;
        typedef void (TStateMachine::* tAction)();
        typedef std::map<EVENT, void*> EVENTMAP;
        typedef std::map<unsigned int, EVENTMAP> STATEMAP;

        STATEMAP stateMap;
        EVENTMAP globalEventMap;
        STATEMAP::iterator sit; // State map iterator
        EVENTMAP::iterator eit; // Event map iterator

    protected:
        int state;
        // Constructor
        CStateMachine() : state(0)  {
        }
        //destructor
        ~CStateMachine() {
            for (sit = stateMap.begin(); sit != stateMap.end(); sit++) {
                for (eit = sit->second.begin(); eit != sit->second.end(); eit++) {
                    delete (TCallback <TStateMachine> *) eit->second;
                }
                sit->second.clear();
            }
            stateMap.clear();

            for (eit = globalEventMap.begin(); eit != globalEventMap.end(); eit++) {
                delete (TCallback <TStateMachine> *) eit->second;
            }
            globalEventMap.clear();
        }
        void addEvent(unsigned int st, EVENT ev, tAction pAction) {
            if ((eit = stateMap[st].find(ev)) != stateMap[st].end())
                delete (TCallback <TStateMachine> *) eit->second;

            stateMap[st][ev] = (void*) new TCallback <TStateMachine>(pAction);
        }
        void addEvent(EVENT ev, tAction pAction) {
            globalEventMap[ev] = (void*) new TCallback <TStateMachine>(pAction);
        }
        void copyState(unsigned int newSt, unsigned int oldSt) {
            assert(((stateMap.find(oldSt)) != stateMap.end()) && (stateMap.find(newSt) == stateMap.end()));

            for (eit = stateMap[oldSt].begin(); eit != stateMap[oldSt].end(); eit++)
                stateMap[newSt][eit->first] = (void*) new TCallback <TStateMachine>(((TCallback <TStateMachine> *) eit->second)->pAction);
        }
        void dummyAction() {
        }

    public:
        bool execute(EVENT ev) {
            if (!ev) return false; // to avoid triggering of machine with passed 0 value

            if (((eit = stateMap[state].find(ev)) == stateMap[state].end())
                && ((eit = globalEventMap.find(ev)) == globalEventMap.end()))
                return false; // Recived event not expected for a current state;
            //Executes action
            ((TCallback <TStateMachine> *) eit->second)->execute((TStateMachine*)this);
            // next state has to be defined! in alternative "HORROR"!
            assert(stateMap.find(state) != stateMap.end()); 

            return true;
        }
    };

}
