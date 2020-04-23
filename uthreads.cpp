//
// Created by shahaf.hermann on 12/04/2020.
//
#include "uthreads.h"
#include "Thread.h"
#include <algorithm>
#include <queue>
#include <iostream>
#include <map>
#include <signal.h>
#include <stdio.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/time.h>
#include <memory>
#include <set>

#define BLOCK sigprocmask(SIG_BLOCK, &maskedSet, NULL)
#define UNBLOCK sigprocmask(SIG_UNBLOCK, &maskedSet, NULL)
#define SPAWN_ERR "thread library error: number of concurrent threads exceeded the limit \
                                                                                (MAX_THREAD_NUM)!"

#define PRIORITY_ERR "thread library error: tid or priority not found!"
#define TERMINATE_ERR "thread library error: cannot terminate, tid not found!"
#define SIGACTION_ERR "system error: sigaction error"
#define SETITIMER_ERR "system error: setitimer error"
#define INIT_ERR "thread library error: invalid quantum input on init"
#define BLOCK_ERR "thread library error: main thread cannot be blocked or tid to block not found"
#define RESUME_ERR "thread library error: tid to resume not found"
#define GET_QUANTUM_ERR "thread library error: tid to get quantum for not found"
#define BLOCKED_OR_TERMINATED -1
#define FAILURE -1
#define SUCCESS 0

typedef  std::shared_ptr<Thread> smartThreadPtr;

/**The READY que, stores threads whose state is ready **/
std::deque<int> readyQ; //TODO

/**The id of the currently running thread **/
int runningId;

/**The number of threads currently existing **/
int numOfThreads;

/**A map whose keys and values are thread id's and their matching Thread object respectively  **/
std::map<int , smartThreadPtr> idMap;

/**A deep copy of the first argument of uthreads_init**/
int *priorityArray;

/**The second argument of uthreads_init **/
int numOfPriorities;

/**The total number of quantum in which threads occupied the CPU **/
int totalNumOfQuantum = 0;

/**The timer **/
struct itimerval timer;

/** The masked set of signals**/
sigset_t maskedSet;

/**A set containing all the id numbers available for new threads **/
std::set<int> availableIds;

/**
 * Handler function for the signal SIGVTALRM.
 * Manage the ready queue as ell as set the timer for the appropriate quantum.
 * @param placeHolder Handler functions require an int input parameter, but our function had no
 *                    use of that.
 */
void scheduler(int placeHolder){
    if(runningId != BLOCKED_OR_TERMINATED){
        int ret_val = sigsetjmp(idMap[runningId]->getEnv(),1);
        if (ret_val == 1) {
            return;
        }
        if(!readyQ.empty()){
            readyQ.push_back(runningId);
            idMap[runningId]->setState(READY);
        }
    }

    int quantum = priorityArray[idMap[readyQ.front()]->getPriority()];
    int quantum_seconds = quantum / 1000000;
    int quantum_mseconds = quantum % 1000000;

    timer.it_value.tv_sec = quantum_seconds;
    timer.it_value.tv_usec = quantum_mseconds;

    timer.it_interval.tv_sec = quantum_seconds;
    timer.it_interval.tv_usec = quantum_mseconds;

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL)) {
        std::cerr << SETITIMER_ERR << std::endl;
        delete priorityArray;
        exit(1);
    }

    if(!readyQ.empty()) {
        runningId = readyQ.front();
        readyQ.pop_front();
        idMap[runningId]->setState(RUNNING);
        idMap[runningId]->incrementQuantum();
        ++totalNumOfQuantum;
        siglongjmp(idMap[runningId]->getEnv(), 1);
    }
    else if(runningId != BLOCKED_OR_TERMINATED){
        idMap[runningId]->setState(RUNNING);
        idMap[runningId]->incrementQuantum();
        ++totalNumOfQuantum;
    }

}

/**
 * Initializes the available ids set by inserting all numbers in range [0, MAX_THREAD_NUM].
 */
void init_available_ids() {
    for (int i = 0; i < MAX_THREAD_NUM; ++i) {
        availableIds.insert(i);
    }
}

int uthread_init(int *quantum_usecs, int size){
    for(int i = 0; i < size; ++i){
        if(quantum_usecs[i] < 0){
            std::cerr << INIT_ERR << std::endl;
            return FAILURE;
        }
    }

    if(size <=0){
        std::cerr << INIT_ERR << std::endl;
        return FAILURE;
    }

    init_available_ids();
    struct sigaction sa = {};

    // Install timer_handler as the signal handler for SIGVTALRM.
    sa.sa_handler = &scheduler;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
        std::cerr << SIGACTION_ERR << std::endl;
        exit(1);
    }
    sigemptyset(&maskedSet);
    sigaddset(&maskedSet, SIGVTALRM);
    runningId = 0;
    priorityArray = new int[size];
    for(int i = 0; i < size; ++i){
        priorityArray[i] = quantum_usecs[i];
    }
    numOfPriorities = size;
    idMap = {};
    uthread_spawn(nullptr, 0);
    sigsetjmp(idMap[0]->getEnv(), 1);
    scheduler(0);
    return SUCCESS;
}

/**
 * Find the lowest available id from the set.
 * @return The lowest available ID found in the set, or -1 if the set is empty.
 */
int getLowestIdAvailable(){
    if (availableIds.empty()) {
        return FAILURE;
    }
    int id = *availableIds.cbegin();
    availableIds.erase(id);
    return id;
}

int uthread_spawn(void (*f)(void), int priority){
    BLOCK;
    int id = getLowestIdAvailable();
    if(id == FAILURE){
        std::cerr << SPAWN_ERR << std::endl;
        UNBLOCK;
        return FAILURE;
    }

    bool found = false;
    for(int i = 0; i < numOfPriorities; ++i){
        if(i == priority){
            found = true;
            break;
        }
    }
    if(!found){
        std::cerr << PRIORITY_ERR << std::endl;
        UNBLOCK;
        return FAILURE;
    }

    smartThreadPtr newThread(new Thread(id, priority, f));
    ++numOfThreads;
    if(id != 0){
        readyQ.push_back(id);
    }
    idMap[id] = newThread;
    UNBLOCK;
    return id;
}

int uthread_change_priority(int tid, int priority){
    BLOCK;
    if(idMap.find(tid) == idMap.end() || priority >= numOfPriorities){
        std::cerr << PRIORITY_ERR << std::endl;
        UNBLOCK;
        return FAILURE;
    }
    idMap[tid]->setPriority(priority);
    UNBLOCK;
    return SUCCESS;
}

/**
 * Generic function for deleting an element with index tid from a given container. If the item
 * doesn't exist in the container, nothing happens.
 */
template<typename T>
void removeElement(T& container, int const& tid) {
    auto position = std::find(container.cbegin(), container.cend(), tid);
    if(position != container.cend()){
        container.erase(position);
    }
}

int uthread_terminate(int tid){
    BLOCK;
    if(idMap.find(tid) == idMap.end()){
        std::cerr << TERMINATE_ERR << std::endl;
        UNBLOCK;
        return FAILURE;
    }

    if(tid == 0){
        idMap.clear();
        delete priorityArray;
        exit(0);
    }

    idMap.erase(tid);
    availableIds.insert(tid);
    --numOfThreads;
    removeElement(readyQ, tid);

    if(tid == runningId) {
        runningId = BLOCKED_OR_TERMINATED;
        UNBLOCK;
        scheduler(0);
    }
    UNBLOCK;
    return SUCCESS;
}

int uthread_block(int tid){
    BLOCK;
    if(idMap.find(tid) == idMap.end() || tid == 0){
        std::cerr << BLOCK_ERR << std::endl;
        UNBLOCK;
        return FAILURE;
    }
    if(idMap[tid]->getState() == BLOCKED){
        UNBLOCK;
        return SUCCESS;
    }
    idMap[tid]->setState(BLOCKED);
    removeElement(readyQ, tid); // remove from READY queue if there

    if(tid == runningId){
        int ret_val = sigsetjmp(idMap[runningId]->getEnv(),1);
        if (ret_val == 1) {
            UNBLOCK;
            return SUCCESS;
        }
        runningId = BLOCKED_OR_TERMINATED;
        UNBLOCK;
        scheduler(0);
    }
    UNBLOCK;
    return SUCCESS;
}

int uthread_resume(int tid){
    BLOCK;
    if(idMap.find(tid) == idMap.end()){
        std::cerr << RESUME_ERR << std::endl;
        UNBLOCK;
        return FAILURE;
    }
    if(idMap[tid]->getState() != BLOCKED){
        UNBLOCK;
        return SUCCESS;
    }
    idMap[tid]->setState(READY);
    readyQ.push_back(tid);
    UNBLOCK;
    return SUCCESS;
}

int uthread_get_tid(){
    return runningId;
}

int uthread_get_total_quantums(){
    return totalNumOfQuantum;
}

int uthread_get_quantums(int tid){
    BLOCK;

    if(idMap.find(tid) == idMap.end()){
        std::cerr << GET_QUANTUM_ERR << std::endl;
        UNBLOCK;
        return FAILURE;
    }
    UNBLOCK;
    return idMap[tid]->getNumOfQuantum();
}