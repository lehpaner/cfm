/****************************** Module Header ******************************\
* Module Name:  Timers.cpp
* Project:      S000
*
*
* Management of timers
*
\***************************************************************************/

#include "Timers.h"
#include <cassert>

namespace cfm::application {
    /*
    *\warning L'utilizzatore della classe e la risorsa che invoca \b decTimers(), devono poter condividere 
    * la zona di memoria, poiche' entrambi devono accedere alle medesime variabili. \n
    * E' inoltre vietato effettuare \b sleep() o funzioni equivalenti nella funzione di \e callback passata 
    * all'attivazione di un timer.
    * /



    /*! \brief Costruttore della classe
    *
    * Il costruttore istanzia il numero richiesto di oggetti
    * TTimerData e se viene passato l'indirizzo di una funzione
    * in \b "func", invoca la funzione stessa, passandole il puntatore
    * all'oggetto this. \n
    * Tipicamente questa modalita' puo' essere utilizzata per
    * "agganciare" il timer con un thread o un interrupt,
    * che effettui il decremento invocando il metodo
    * decTimers(), come indicato nell' \ref esempioDiTimer. \n
    * La base dei tempi e' necessaria per "raccordare" il
    * periodo di \b decTimers() con il valore in millisecondi
    * richiesto all'attivazione.
    *
    * \param n      numero di timer desiderati
    * \param msBase valore in millisecondi da usare come base dei tempi
    * \param func   eventuale funzione di callback che verra' invocata all'istanziamento
    *               dell'oggetto timer
    */

    TTimers::TTimers(int n, int msBase, void (*func)(TTimers*)) : numTimers(n), baseTime(msBase) {
        t = new TTimerData[numTimers];

        if (func)
            (*func)(this);
    }
    /* ! \brief Distruttore della classe
    *
    * Rilascia i \b TTimerData allocati dal costruttore
    */
    TTimers::~TTimers() {
        for (int i = 0; i < numTimers; i++)
            resTimeout(i);

        delete[] t;
        t = NULL;
    }



    /*!
     * \brief Decrementa tutti i timer ed eventualmente invoca le callback
     *
     * Per ogni timer, \e "locka" il timer e se non e' attivo \b (value == 0), \e "unlocka" il timer
     * e passa al prossimo. \n
     * Se invece e' attivo \b (value != 0), decrementa il timer, lo \e "unlocka" e se ha raggiunto lo \b '0',
     * invoca la procedura di \e callback. \n
     * \e "unlocka" il timer.
     */
    void TTimers::decTimers() {
        unsigned int i;
        TTimerData* p;
        /* For all timers */
        for (i = 0, p = t; i < numTimers; i++, p++) { 
        
            p->mutex->Lock(); /* Lock the timer */

            if (!p->value || --p->value) /* If timer already expired or running */
                p->mutex->Unlock(); /* Unlock the timer */
            else {
                p->mutex->Unlock(); /* Unlock the timer */

                if (p->f) /* It would never be NULL, but... */
                    (*p->f)(i, p->timeoutParameter, p->timeoutPtrParameter); /* callback procedure with parameters */
            }
        }
    }



    /*! \brief Attiva un timer
     *
     * Se il valore di timeout passato e' \b == \b 0, non setta il timer. \n
     * Altrimenti, calcola il valore dei \b tic, in funzione della base dei tempi impostata e se viene
     * uguale a zero, forza 1 per evitare che il timer non scada mai. Ovviamente, in questo caso il
     * tempo trascorso sara' un po' piu' lungo ma funzionera' \n
     *
     * \param timer indice del timer
     * \param value valore temporale
     * \param func funzione di \e callback da invocare allo scadere del timer
     * \param timeoutParameter parametro opzionale che verra' passato alla funzione di \e callback
     */
    void TTimers::setTimeout(unsigned int timer, unsigned int value, void (*func)(unsigned int, unsigned int, void*),
        unsigned int timeoutParameter, void* timeoutPtrParameter) {
        if (!value) return;

        assert(timer < numTimers);

        t[timer].mutex->Lock();
        t[timer].timeoutParameter = timeoutParameter;
        t[timer].timeoutPtrParameter = timeoutPtrParameter;
        t[timer].refreshValue = (value / baseTime)
            ? value / baseTime
            : 1;
        t[timer].f = func;
        t[timer].value = t[timer].refreshValue;
        t[timer].mutex->Unlock();
    }



    /*! \brief Disattiva un timer
     *
     * \param timer indice del timer
     */
    void TTimers::resTimeout(unsigned int timer) {
        assert(timer < numTimers);

        t[timer].mutex->Lock();
        if (t[timer].value) {
            t[timer].value = 0;
        }
        t[timer].mutex->Unlock();
    }



    /*! \brief Ritorna il valore attuale del timer
     *
     * \note Il valore attuale corrisponde al numero di tic rimasti
     * allo scadere del timer.
     * E' utilizzabile anche come booleano per verificare se il timer e' attivo o meno.
     * Se e' \b != \b 0 e' attivo, altrimenti e' disattivo.
     *
     * \param timer indice del timer
     * \return ritorna il valore attuale del timer
     */
    unsigned int TTimers::timeoutRunning(unsigned int timer) {
        assert(timer < numTimers);

        unsigned int value;

        t[timer].mutex->Lock();
        value = t[timer].value * baseTime;
        t[timer].mutex->Unlock();

        return value;
    }



    /*! \brief Ripristina il valore iniziale del timer
     *
     * E' utile per ripristinare l'ultimo valore di timeout del timer
     * senza doverne ricordare il valore.
     *
     * \param timer indice del timer
     */
    void TTimers::refreshTimeout(unsigned int timer) {
        assert(timer < numTimers);

        t[timer].mutex->Lock();
        t[timer].value = t[timer].refreshValue;
        t[timer].mutex->Unlock();
    }

    /*! \file      timersthread.cpp
    * \brief      Codice della classe del thread che gestisce il timer
    *
    * Societa':   I&SI \n
    * Progetto:   SARA \n
    *
    * \version    2.3
    * \date       18/05/2010
    * \author     Mladen Seget
    *
    * Questa classe denominata \b TTimersThread,  deriva da \b TTimers ed e' utile per gestire il \e Thread
     * dei servizi timer.
    *
    * Un programma, per ottenere i servizi di timing deve istanziare un oggetto \b TTimersThread. A questo punto
    * viene creato un \e Thread il quale invochera' le \e callback richieste all'attivazione dei timer, senza
    * comportare problemi di visibilita' perche' il \e Thread condivide la memoria del processo creatore.
    *
    * Per ogni oggetto \b TTimersThread, il \e Thread e' unico, per qualsiasi numero di timer richiesto.
    *
    * Poiche' il \e Thread del timer e il processo ospite condividono i dati, per evitare effetti collaterali
    * indesiderati, i dati interni sensibili del timer sono protetti da mutex.
    *
    * \warning E' vietato effettuare \b sleep() o funzioni equivalenti nella funzione di \e callback passata
    * all'attivazione di un timer.
    */
    /*!\brief Costruttore della classe
    *
    * \param n      numero di timer desiderati
    * \param msBase valore in millisecondi da usare come base dei tempi
    */
    TTimersThread::TTimersThread(int n, int msBase) : TTimers(n, msBase) {
        baseTime = msBase;
        shutdown = false;
        m_thread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)timerThread, this, 0, &m_dwThreadId);

    }
    /*! \brief Distruttore della classe
    *
    */
    TTimersThread::~TTimersThread() {
        shutdown = true;
        Sleep(baseTime * 2);
        if (m_thread)
            ::CloseHandle(m_thread);
    }

    /*! \brief Thread del timer.
    *
    * Ciclo infinito in cui viene invocato il  metodo
    * \b decTimers() e viene atteso un tempo \b baseTime. \n
    * Il metodo \b decTimers() decremeta i timers ed in
    * caso di scadenza invoca la procedura di timeout
    * passata al "settaggio" del timer.
    *
    * \param timerObject puntatore all'oggetto TTimers (this)
    */
    ThreadFunc_t TTimersThread::timerThread(void* timerObject) {
        TTimersThread* p = (TTimersThread*)timerObject;
        ::Sleep(p->baseTime);

        while (!p->shutdown) {
            p->decTimers();
            ::Sleep(p->baseTime);
        }
        return 0;
    }
}