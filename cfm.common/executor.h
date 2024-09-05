#pragma once

#include <memory>
#include <mutex>
#include <future>
#include <utility>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>

namespace cfm::application::common {
/**
 * Contenitore dei task
 * se la coda sta in chiusura non accettera i tak e restituisca un futuro invalido
 * thread scoda da questa coda e se trova la coda vuota si blocca su pop e riprende
 * esecuzione sul segnale m_queueChanged.
 * Da "altra parte" la coda viene riempita con i task da fare in modaleità front o back
 *
 */
    class task_queue {
    public:
        task_queue();

        template <typename Task, typename... Args>
        auto push(Task task, Args&&... args) -> std::future<decltype(task(args...))>;

        template <typename Task, typename... Args>
        auto pushToFront(Task task, Args&&... args) -> std::future<decltype(task(args...))>;
        std::unique_ptr<std::function<void()>> pop();

        void shutdown();

        bool isShutdown() { return m_shutdown; }

    private:
        /// 
        using Queue = std::deque<std::unique_ptr<std::function<void()>>>;
        /**
        * @param front per aggingeter task in front.
        * @param task lavoro da fare.
        * @param args argumenti per eseguire task.
        * @returns A @c std::future di ritorno.
        */
        template <typename Task, typename... Args>
        auto pushTo(bool front, Task task, Args&&... args) -> std::future<decltype(task(args...))>;
        /// coda
        Queue m_queue;
        /// sincronisazione 
        std::condition_variable m_queueChanged;
        /// mutex della coda
        std::mutex m_queueMutex;
        /// flag di chiusura
        std::atomic_bool m_shutdown;
    };

    task_queue::task_queue() : m_shutdown{ false } {
    }
    template <typename Task, typename... Args>
    auto task_queue::push(Task task, Args&&... args) -> std::future<decltype(task(args...))> {
        bool front = true;
        return pushTo(!front, std::forward<Task>(task), std::forward<Args>(args)...);
    }

    template <typename Task, typename... Args>
    auto task_queue::pushToFront(Task task, Args&&... args) -> std::future<decltype(task(args...))> {
        bool front = true;
        return pushTo(front, std::forward<Task>(task), std::forward<Args>(args)...);
    }

    /**
     * Funzione utility generica che concatena le promesse ti risultato: asspeta che futuro si realizzi e
     * la riveste in altra promessa.
     *
     *
     * @param promise The @c std::promise da creare quando @c future si realizza.
     * @param future The @c std::future che viene aspettato per la realizazzione e creazione della @c promise.
     */
    template <typename T>
    inline static void forwardPromise(std::shared_ptr<std::promise<T>> promise, std::future<T>* future) {
        promise->set_value(future->get());
    }

    /**
     * Specialization della generica sopra per i valori void
     *
     * @param promise The @c std::promise da creare quando @c future si realizza.
     * @param future The @c std::future che viene aspettato per la realizazzione e creazione della @c promise.
     */
    template <>
    inline void forwardPromise<void>(std::shared_ptr<std::promise<void>> promise, std::future<void>* future) {
        future->get();
        promise->set_value();
    }

    template <typename Task, typename... Args>
    auto task_queue::pushTo(bool front, Task task, Args&&... args) -> std::future<decltype(task(args...))> {
        // Assegna gli argomenti di esecuzione al task.
        auto boundTask = std::bind(std::forward<Task>(task), std::forward<Args>(args)...);

        /*
         * Create a std::packaged_task with the correct return type. The decltype only returns the return value of the
         * boundTask. The following parentheses make it a function call with the boundTask return type. The package task
         * will then return a future of the correct type.
         *
         * Note: A std::packaged_task fulfills its future *during* the call to operator().  If the user of a
         * std::packaged_task hands it off to another thread to execute, and then waits on the future, they will be able to
         * retrieve the return value from the task and know that the task has executed, but they do not know exactly when
         * the task object has been deleted.  This distinction can be significant if the packaged task is holding onto
         * resources that need to be freed (through a std::shared_ptr for example).  If the user needs to wait for those
         * resources to be freed they have no way of knowing how long to wait.  The translated_task lambda below is a
         * workaround for this limitaion.  It executes the packaged task, thten disposes of it before passing the task's
         * return value back to the future that the user is waiting on.
         */
        using PackagedTaskType = std::packaged_task<decltype(boundTask())()>;
        auto packaged_task = std::make_shared<PackagedTaskType>(boundTask);

        // Crea promise/future che si realizzera sulla rimozione del task.
        auto cleanupPromise = std::make_shared<std::promise<decltype(task(args...))>>();
        auto cleanupFuture = cleanupPromise->get_future();

        // Rimozione del valore di ritorno del task wrappandolo in lambda senza valore di ritorno 
        auto translated_task = [packaged_task, cleanupPromise]() mutable {
            // lancio del esecuzione task.
            packaged_task->operator()();
            // presa del futuro della esecuzione.
            auto taskFuture = packaged_task->get_future();
            // rimozione del task.
            packaged_task.reset();
            // travaso del risultato nella promessa di un livello esecutivo piu alto.
            forwardPromise(cleanupPromise, &taskFuture);
        };

        // rilascio delle referenze in modo che uncia che rimane è la ref dentro lambda.
        packaged_task.reset();
        {
            std::lock_guard<std::mutex> queueLock{ m_queueMutex };
            if (!m_shutdown) {
                m_queue.emplace(front ? m_queue.begin() : m_queue.end(), new std::function<void()>(translated_task));
            }
            else {
                using FutureType = decltype(task(args...));
                return std::future<FutureType>();
            }
        }

        m_queueChanged.notify_all();
        return cleanupFuture;
    }

    std::unique_ptr<std::function<void()>> task_queue::pop() {
        std::unique_lock<std::mutex> queueLock{ m_queueMutex };
        auto shouldNotWait = [this]() { return m_shutdown || !m_queue.empty(); };
        if (!shouldNotWait()) {
            m_queueChanged.wait(queueLock, shouldNotWait);
        }
        if (!m_queue.empty()) {
            auto task = std::move(m_queue.front());
            m_queue.pop_front();
            return task;
        }
        return nullptr;
    }
    void task_queue::shutdown() {
        std::lock_guard<std::mutex> queueLock{ m_queueMutex };
        m_queue.clear();
        m_shutdown = true;
        m_queueChanged.notify_all();
    }

    /*******************************************************************************
     * Questo thread è allaciato sulla coda ed esegue tutto cio che trova dentro.
     * su
     *******************************************************************************/
    class task_thread {
    public:
        /// 
        task_thread(std::shared_ptr<task_queue> taskQueue);
        ///
        ~task_thread();
        ///
        void start() {
            m_thread = std::thread{ std::bind(&task_thread::processTasksLoop, this) };
        }
        ///
        bool isShutdown() { return m_shutdown; }

    private:
        /// main del esecuzione
        void processTasksLoop();
        /// puntatore sulla coda.
        std::weak_ptr<task_queue> m_taskQueue;
        /// flag di chiusura.
        std::atomic_bool m_shutdown;
        /// thread di esecuzione.
        std::thread m_thread;
    };

    task_thread::task_thread(std::shared_ptr<task_queue> taskQueue)
        :m_taskQueue{ taskQueue }, m_shutdown{ false } {
    }
    task_thread::~task_thread() {
        m_shutdown = true;
        if (m_thread.joinable()) { m_thread.join(); }
    }
    /**
     * ciclo principale della esecuzione dei task in coda finche non arivi shutdown
     */
    void task_thread::processTasksLoop() {
        while (!m_shutdown) {
            auto m_actualTaskQueue = m_taskQueue.lock();

            if (m_actualTaskQueue && !m_actualTaskQueue->isShutdown()) {
                auto task = m_actualTaskQueue->pop();
                if (task) { task->operator()(); }
            }
            else {
                m_shutdown = true;
            }
        }
    }

    /*******************************************************************************
     * executor per eendere chiamate asincrone.
     * serve per delegare la esecuzione delle funzioni restituendo l'oggetto futuro
     * accetta lambda, std::function, std::bind, functor
     ******************************************************************************/
    class executor {
    public:
        /**
          * constructor.
          */
        executor() : m_taskQueue{ std::make_shared<task_queue>() }, m_taskThread{ std::make_unique<task_thread>(m_taskQueue) } {
            m_taskThread->start();
        }
        /**
         * destructor.
         */
        ~executor() { shutdown(); }

        template <typename Task, typename... Args>
        auto submit(Task task, Args&&... args) -> std::future<decltype(task(args...))>;

        template <typename Task, typename... Args>
        auto submitToFront(Task task, Args&&... args) -> std::future<decltype(task(args...))>;

        /**
         * asspetta la esecuzione dei task in coda.
         */
        void waitForSubmittedTasks() {
            std::promise<void> flushedPromise;
            auto flushedFuture = flushedPromise.get_future();
            auto task = [&flushedPromise]() { flushedPromise.set_value(); };
            submit(task);
            flushedFuture.get();
        }

        /// pulisce la coda della esecuzione e rifiuta ulteriori task.
        void shutdown();
        /// executor e vivo o sta per morire.
        bool isShutdown() {
            return m_taskQueue->isShutdown();
        }

    private:
        /// coda dei task da eseguire.
        std::shared_ptr<task_queue> m_taskQueue;

        /// thread dedicato alla esecuzione .
        std::unique_ptr<task_thread> m_taskThread;
    };
    void executor::shutdown() {
        m_taskQueue->shutdown();
        m_taskThread.reset();
    }
    template <typename Task, typename... Args>
    auto executor::submit(Task task, Args&&... args) -> std::future<decltype(task(args...))> {
        return m_taskQueue->push(task, std::forward<Args>(args)...);
    }

    template <typename Task, typename... Args>
    auto executor::submitToFront(Task task, Args&&... args) -> std::future<decltype(task(args...))> {
        return m_taskQueue->pushToFront(task, std::forward<Args>(args)...);
    }
}